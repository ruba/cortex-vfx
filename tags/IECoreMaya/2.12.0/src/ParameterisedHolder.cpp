//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the name of Image Engine Design nor the names of any
//       other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////

#include "boost/python.hpp"
#include "boost/format.hpp"

#include "maya/MPxNode.h"
#include "maya/MPxLocatorNode.h"
#include "maya/MPxDeformerNode.h"
#include "maya/MPxObjectSet.h"
#include "maya/MPxFieldNode.h"
#include "maya/MPxSurfaceShape.h"
#include "maya/MPxComponentShape.h"
#include "maya/MFnTypedAttribute.h"
#include "maya/MFnNumericAttribute.h"
#include "maya/MFnDependencyNode.h"
#include "maya/MFnCompoundAttribute.h"
#include "maya/MObjectArray.h"
#include "maya/MPlugArray.h"
#include "maya/MDGModifier.h"
#include "maya/MNodeMessage.h"
#include "maya/MFnExpression.h"

#include "IECoreMaya/ParameterisedHolder.h"
#include "IECoreMaya/Parameter.h"
#include "IECoreMaya/PythonCmd.h"

#include "IECore/MessageHandler.h"
#include "IECore/CompoundParameter.h"
#include "IECore/Object.h"
#include "IECore/TypedParameter.h"

#include "typeIds/TypeIds.h"

#include <stdlib.h>

using namespace IECore;
using namespace IECoreMaya;
using namespace boost::python;
using namespace boost;
using namespace std;

template<typename B>
MObject ParameterisedHolder<B>::aParameterisedClassName;
template<typename B>
MObject ParameterisedHolder<B>::aParameterisedVersion;
template<typename B>
MObject ParameterisedHolder<B>::aParameterisedSearchPathEnvVar;

template<typename B>
ParameterisedHolder<B>::PLCB::PLCB( ParameterisedHolder<B> *node) : m_node(node)
{
}

template<typename B>
void ParameterisedHolder<B>::PLCB::postLoad()
{
	assert(m_node);
	m_node->getParameterised();
}

template<typename B>
ParameterisedHolder<B>::ParameterisedHolder()
	:	m_parameterised( 0 ), m_failedToLoad( false )
{
	m_plcb = new PLCB( this );
}

template<typename B>
ParameterisedHolder<B>::~ParameterisedHolder()
{
}

template<typename B>
void ParameterisedHolder<B>::postConstructor()
{
	B::setExistWithoutInConnections(true);
	B::setExistWithoutOutConnections(true);	
}

template<typename B>
bool ParameterisedHolder<B>::isAbstractClass()
{
	return true;
}

template<typename B>
void *ParameterisedHolder<B>::creator()
{
	return new ParameterisedHolder<B>();
}

// this is a bit of a fiasco. when we came to make ParameterisedHolders
// derived from MPxSurfaceShape we discovered that those already have an
// attribute named "clsn", but we were using that for the short name
// for the class name plug on all parameterised holders. these template
// functions allow us to continue to use "clsn" with non-SurfaceShape
// base classes and use a unique name with SurfaceShape derived classes.
// that way we maintain backwards compatibility with old scenes (but not
// those containing ProceduralHolders, as that's about to switch to
// derive from SurfaceShape dammit).
/// \todo With the next major version sort this fiasco out and use just
/// one unique name for all nodes. 
template<typename B>
static MString classNamePlugShortName()
{
	return "clsn";
}

template<>
static MString classNamePlugShortName<MPxSurfaceShape>()
{
	return "className";
}

template<>
static MString classNamePlugShortName<MPxComponentShape>()
{
	return "className";
}

template<typename B>
MStatus ParameterisedHolder<B>::initialize()
{
	MStatus s;
	MFnTypedAttribute tAttr;
	MFnNumericAttribute nAttr;
	
	aParameterisedClassName = tAttr.create( "className", classNamePlugShortName<B>(), MFnData::kString );
	tAttr.setReadable(true);
	tAttr.setWritable(true);
	tAttr.setStorable(true);	
	tAttr.setConnectable(false);
	tAttr.setHidden(true);
	
	s = B::addAttribute( aParameterisedClassName );
	assert(s);
	
	aParameterisedVersion = nAttr.create("version", "ver", MFnNumericData::kInt, 1, &s );
	assert(s);
	nAttr.setReadable(true);
	nAttr.setWritable(true);
	nAttr.setStorable(true);	
	nAttr.setConnectable(false);
	nAttr.setHidden(true);
	
	
	s = B::addAttribute( aParameterisedVersion );
	assert(s);
	
	aParameterisedSearchPathEnvVar = tAttr.create("searchPathEnvVar", "spev", MFnData::kString );
	tAttr.setReadable(true);
	tAttr.setWritable(true);
	tAttr.setStorable(true);	
	tAttr.setConnectable(false);
	tAttr.setHidden(true);	
	
	s = B::addAttribute( aParameterisedSearchPathEnvVar );	
	assert(s);
	
	return MS::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::setParameterised( IECore::ParameterisedPtr p )
{
	MPlug pClassName( B::thisMObject(), aParameterisedClassName );
	MPlug pVersion( B::thisMObject(), aParameterisedVersion );
	MPlug pSearchPathEnvVar( B::thisMObject(), aParameterisedSearchPathEnvVar );
	pClassName.setValue( "" );
	pVersion.setValue( 0 );
	pSearchPathEnvVar.setValue( "" );

	m_parameterised = p;
	m_failedToLoad = false;
	
	MStatus s = createAndRemoveAttributes();
	if( !s )
	{
		m_parameterised = 0;
		return s;
	}
	
	return MStatus::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::setParameterised( const std::string &className, int classVersion, const std::string &searchPathEnvVar )
{
	MPlug pClassName( B::thisMObject(), aParameterisedClassName );
	MPlug pVersion( B::thisMObject(), aParameterisedVersion );
	MPlug pSearchPathEnvVar( B::thisMObject(), aParameterisedSearchPathEnvVar );
	MStatus s = pClassName.setValue( className.c_str() );
	pVersion.setValue( classVersion );
	pSearchPathEnvVar.setValue( searchPathEnvVar.c_str() );
			
	m_parameterised = 0;
	m_failedToLoad = false;
	
	if( getParameterised() )
	{
		return MStatus::kSuccess;
	}

	return MStatus::kFailure;
}

template<typename B>
IECore::ParameterisedPtr ParameterisedHolder<B>::getParameterised( std::string *classNameOut, int *classVersionOut, std::string *searchPathEnvVarOut )
{
	MPlug pClassName( B::thisMObject(), aParameterisedClassName );
	MPlug pVersion( B::thisMObject(), aParameterisedVersion );
	MPlug pSearchPathEnvVar( B::thisMObject(), aParameterisedSearchPathEnvVar );
	
	MString className;
	int version;
	MString searchPathEnvVar;
	
	MStatus s = pClassName.getValue( className );
		
	pVersion.getValue( version );
	pSearchPathEnvVar.getValue( searchPathEnvVar );	
		
	if( !m_parameterised && !m_failedToLoad )
	{
		m_failedToLoad = true;
		if( className!="" )
		{
			m_parameterised = loadClass( className, version, searchPathEnvVar );
			if( m_parameterised )
			{
				// we'll only create and remove attributes if we loaded successfully.
				// this avoids the situation where the loading fails due to some
				// correctable error, but we've just deleted all the attributes with
				// all the settings and connections important to the user.
				if( createAndRemoveAttributes() )
				{
					m_failedToLoad = false;
				}
			}
		}
	}
	
	// fill output parameters
	if( m_parameterised )
	{
		if( classNameOut )
		{
			*classNameOut = className.asChar();
		}
		if( classVersionOut )
		{
			*classVersionOut = version;
		}
		if( searchPathEnvVarOut )
		{
			*searchPathEnvVarOut = searchPathEnvVar.asChar();
		}
	}
	
	return m_parameterised;
}

template<typename B>
MStatus ParameterisedHolder<B>::setNodeValues()
{
	// to update the parameter->name map if necessary
	getParameterised();
	
	MFnDependencyNode fnDN( B::thisMObject() );
	
	ParameterToAttributeNameMap::const_iterator it;
	for( it=m_parametersToAttributeNames.begin(); it!=m_parametersToAttributeNames.end(); it++ )
	{
		MPlug p = fnDN.findPlug( it->second );
		if( p.isNull() )
		{
			return MStatus::kFailure;
		}
		try
		{
			MStatus s = Parameter::setValue( it->first, p );
			if( !s )
			{
				return s;
			}
		}
		catch( ... )
		{
			msg( Msg::Error, "ParameterisedHolder::setNodeValues", "Caught exception while setting value." );
			return MStatus::kFailure;
		}
	}
	return MStatus::kSuccess;
}
		
template<typename B>
MStatus ParameterisedHolder<B>::setParameterisedValues()
{
	// to update the parameter->name map if necessary
	getParameterised();
	
	MFnDependencyNode fnDN( B::thisMObject() );
	
	ParameterToAttributeNameMap::const_iterator it;
	for( it=m_parametersToAttributeNames.begin(); it!=m_parametersToAttributeNames.end(); it++ )
	{
		MPlug p = fnDN.findPlug( it->second );
		if( p.isNull() )
		{
			return MStatus::kFailure;
		}
		try
		{
			MStatus s = Parameter::setValue( p, it->first );
			if( !s )
			{
				return s;
			}
		}
		catch( ... )
		{
			msg( Msg::Error, "ParameterisedHolder::setParameterisedValues", "Caught exception while setting value." );
			return MStatus::kFailure;
		}
	}
	return MStatus::kSuccess;
}

template<typename B>
MPlug ParameterisedHolder<B>::parameterPlug( IECore::ConstParameterPtr parameter )
{
	// to update the parameter->name map if necessary
	getParameterised();
	
	ParameterToAttributeNameMap::const_iterator it = m_parametersToAttributeNames.find( const_pointer_cast<IECore::Parameter>( parameter ) );
	if( it==m_parametersToAttributeNames.end() )
	{
		return MPlug();
	}
	
	MFnDependencyNode fnDN( B::thisMObject() );
	return MPlug( B::thisMObject(), fnDN.attribute( it->second ) );
}

template<typename B>
IECore::ParameterPtr ParameterisedHolder<B>::plugParameter( const MPlug &plug )
{
	assert( ! plug.isNull() );
	
	// to update the parameter->name map if necessary
	getParameterised();
	AttributeNameToParameterMap::const_iterator it = m_attributeNamesToParameters.find( plug.partialName() );
	if( it==m_attributeNamesToParameters.end() )
	{
		return 0;
	}
	return it->second;
}

template<typename B>
IECore::ParameterisedPtr ParameterisedHolder<B>::loadClass( const MString &className, int classVersion, const MString &searchPathEnvVar )
{
	const char *searchPath = getenv( searchPathEnvVar.asChar() );
	if( !searchPath )
	{
		searchPath = "";
	}	

	string toExecute = boost::str( format( 
			"IECore.ClassLoader( IECore.SearchPath( os.path.expandvars(\"%s\"), \":\" ) ).load( \"%s\", %d )()\n"
		) % searchPath % className.asChar() % classVersion
	);

	try
	{
		PyRun_String( 
			"import os.path",
			Py_file_input, PythonCmd::globalContext().ptr(),
			PythonCmd::globalContext().ptr()
		);

		handle<> resultHandle( PyRun_String( 
			toExecute.c_str(),
			Py_eval_input, PythonCmd::globalContext().ptr(),
			PythonCmd::globalContext().ptr() )
		);
		object result( resultHandle );
		return extract<ParameterisedPtr>( result )();
	}
	catch( error_already_set & )
	{
		PyErr_Print();
	}
	catch( ... )
	{
		msg( Msg::Error, "ParameterisedHolder::loadClass",
			boost::format( "Unable to load class \"%s\" version %d." ) % className.asChar() % classVersion );
	}
	return 0;
}

template<typename B>
MStatus ParameterisedHolder<B>::createAndRemoveAttributes()
{
	m_attributeNamesToParameters.clear();
	m_parametersToAttributeNames.clear();
	
	MStatus s;
	if( m_parameterised )
	{
		s = createAttributesWalk( m_parameterised->parameters(), "parm" );
		if( !s )
		{
			msg( Msg::Error, "ParameterisedHolder::createAndRemoveAttributes", boost::format( "Unable to create attributes to represent class." ) );
			return s;
		}
	}
			
	s = removeUnecessaryAttributes();
	if( !s )
	{
		msg( Msg::Error, "ParameterisedHolder::createAndRemoveAttributes", "Failed to remove unecessary attributes." );
		return s;
	}
	
	return MS::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::createAttributesWalk( IECore::ConstCompoundParameterPtr parameter, const std::string &rootName )
{
	MFnDependencyNode fnDN( B::thisMObject() );
	const CompoundParameter::ParameterVector &children = parameter->orderedParameters();
	for( size_t i=0; i<children.size(); i++ )
	{
		CompoundParameterPtr compoundChild = runTimeCast<CompoundParameter>( children[i] );
		if( compoundChild )
		{
			MStatus s = createAttributesWalk( compoundChild, rootName + "_" + compoundChild->name() );
			if( !s )
			{
				return s;
			}
		}
		else
		{
			string attributeName = rootName + "_" + children[i]->name();
			MString mAttributeName = attributeName.c_str();
			
			m_attributeNamesToParameters[mAttributeName] = children[i];
			m_parametersToAttributeNames[children[i]] = mAttributeName;
			
			MPlugArray connectionsFromMe, connectionsToMe;
			
			MObject attribute = fnDN.attribute( mAttributeName );
			if( !attribute.isNull() )
			{
				MStatus s = IECoreMaya::Parameter::update( children[i], attribute );
				if( s )
				{
					// we're all good - go on to the next parameter
					continue;
				}

				// failed to update (parameter type probably changed).
				// remove the current attribute and fall through to the create
				// code
				
				MPlug plug( B::thisMObject(), attribute );
				plug.connectedTo( connectionsFromMe, false, true );
				plug.connectedTo( connectionsToMe, true, false );
									
				fnDN.removeAttribute( attribute );
			}
			
			attribute = IECoreMaya::Parameter::create( children[i], mAttributeName );
			MStatus s = fnDN.addAttribute( attribute );
			if( !s )
			{
				return s;
			}
			
			if ( connectionsFromMe.length() || connectionsToMe.length() )
			{
				MDGModifier dgMod;
				MPlug plug( B::thisMObject(), attribute );	
				for (unsigned i = 0; i < connectionsFromMe.length(); i++)
				{
					dgMod.connect( plug, connectionsFromMe[i] );
				}
				for (unsigned i = 0; i < connectionsToMe.length(); i++)
				{
					dgMod.connect( connectionsToMe[i], plug );
				}
				
				dgMod.doIt();												
			}


			if( !attribute.isNull() )
			{
				MPlug plug( B::thisMObject(), attribute );
				connectionsToMe.clear();
				plug.connectedTo( connectionsToMe, true, false );

				if ( connectionsToMe.length() == 0 )
				{
					// creates default connections based on userData["maya"]["defaultConnection"] value...
					CompoundObject::ObjectMap &userData = children[i]->userData()->members();
					CompoundObject::ObjectMap::const_iterator it = userData.find( "maya" );
					if ( it != userData.end() && it->second->typeId() == CompoundObjectTypeId )
					{
						CompoundObject::ObjectMap &mayaUserData = static_pointer_cast<CompoundObject>(it->second)->members();
						it = mayaUserData.find( "defaultConnection" );
						if ( it != mayaUserData.end() && it->second->typeId() == StringDataTypeId )
						{
							std::string defaultConnection = static_pointer_cast<StringData>(it->second)->readable();
							std::string cmd = string( "connectAttr " ) + defaultConnection + " " + plug.partialName( true ).asChar();
							MDGModifier dgMod;
							dgMod.commandToExecute( cmd.c_str() );
							dgMod.doIt();												
						}
						it = mayaUserData.find( "defaultExpression" );
						if ( it != mayaUserData.end() && it->second->typeId() == StringDataTypeId )
						{
							std::string defaultExpression = static_pointer_cast<StringData>(it->second)->readable();
							MString cmd = plug.partialName( true ) + defaultExpression.c_str();
							MFnExpression expFn;
							expFn.create( cmd );
						}
					}
				}
			}

			if( children[i]->isInstanceOf( IECore::StringParameter::staticTypeId() ) )
			{
				// because default value setting for string attributes doesn't work,
				// the Parameter::create method won't have set any value. do that ourselves
				// here.
				MPlug plug( B::thisMObject(), attribute );
				MStatus s = IECoreMaya::Parameter::setValue( children[i], plug );
				if( !s )
				{
					return s;
				}
			}
		}
	}
	
	return MS::kSuccess;
}

template<typename B>
MStatus ParameterisedHolder<B>::removeUnecessaryAttributes()
{
	/// \todo Make this a parameter, but only when the library's major version is incremented
	const std::string &rootName = "parm_";

	MObjectArray toRemove;
	MFnDependencyNode fnDN( B::thisMObject() );
	for( unsigned i=0; i<fnDN.attributeCount(); i++ )
	{
		MObject attr = fnDN.attribute( i );
		MFnAttribute fnAttr( attr );
		if( std::string( fnAttr.name().substring( 0, rootName.size() ).asChar() ) == rootName )
		{
			if( m_attributeNamesToParameters.find( fnAttr.name() )==m_attributeNamesToParameters.end() )
			{
				if( fnAttr.parent().isNull() )
				{
					toRemove.append( attr );
				}
				else
				{
					// we don't need to remove attributes which are the children
					// of compounds as they'll be removed when their parent is removed
				}
			}
		}
	}
	for( unsigned i=0; i<toRemove.length(); i++ )
	{
		MStatus s = fnDN.removeAttribute( toRemove[i] );
		if( !s )
		{
			return s;
		}
	}
	return MStatus::kSuccess;
}

// specialisations of the different typeIds	
template<>
MTypeId ParameterisedHolderNode::id( ParameterisedHolderNodeId );

template<>
MString ParameterisedHolderNode::typeName( "ieParameterisedHolderNode" );

template<>
MTypeId ParameterisedHolderLocator::id( ParameterisedHolderLocatorId );

template<>
MString ParameterisedHolderLocator::typeName( "ieParameterisedHolderLocator" );

template<>
MTypeId ParameterisedHolderDeformer::id( ParameterisedHolderDeformerId );

template<>
MString ParameterisedHolderDeformer::typeName( "ieParameterisedHolderDeformer" );

template<>
MTypeId ParameterisedHolderField::id( ParameterisedHolderFieldId );

template<>
MString ParameterisedHolderField::typeName( "ieParameterisedHolderField" );

template<>
MTypeId ParameterisedHolderSet::id( ParameterisedHolderSetId );

template<>
MString ParameterisedHolderSet::typeName( "ieParameterisedHolderSet" );

template<>
MTypeId ParameterisedHolderSurfaceShape::id( ParameterisedHolderSurfaceShapeId );

template<>
MString ParameterisedHolderSurfaceShape::typeName( "ieParameterisedHolderSurfaceShape" );

template<>
MTypeId ParameterisedHolderComponentShape::id( ParameterisedHolderComponentShapeId );

template<>
MString ParameterisedHolderComponentShape::typeName( "ieParameterisedHolderComponentShape" );
						
// explicit instantiation
template class ParameterisedHolder<MPxNode>;
template class ParameterisedHolder<MPxLocatorNode>;
template class ParameterisedHolder<MPxDeformerNode>;
template class ParameterisedHolder<MPxFieldNode>;
template class ParameterisedHolder<MPxObjectSet>;
template class ParameterisedHolder<MPxSurfaceShape>;
template class ParameterisedHolder<MPxComponentShape>;
