//////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2008, Image Engine Design Inc. All rights reserved.
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

#include "IECore/FromCoreConverter.h"
#include "IECore/CompoundParameter.h"
#include "IECore/NullObject.h"

using namespace IECore;

FromCoreConverter::FromCoreConverter(  const std::string &name, const std::string &description, TypeId supportedType )
	:	Converter( name, description )
{
	m_srcParameter = new ObjectParameter( "src", "The object to be converted.", new NullObject(), supportedType );
	parameters()->addParameter( m_srcParameter );
}

FromCoreConverter::FromCoreConverter( const std::string &name, const std::string &description, const ObjectParameter::TypeIdSet &supportedTypes )
	:	Converter( name, description )
{
	m_srcParameter = new ObjectParameter( "src", "The object to be converted.", new NullObject(), supportedTypes );
	parameters()->addParameter( m_srcParameter );
}

FromCoreConverter::FromCoreConverter( const std::string &name, const std::string &description, const TypeId *supportedTypes )
	:	Converter( name, description )
{
	m_srcParameter = new ObjectParameter( "src", "The object to be converted.", new NullObject(), supportedTypes );
	parameters()->addParameter( m_srcParameter );
}
		
FromCoreConverter::~FromCoreConverter()
{
}

ObjectParameterPtr FromCoreConverter::srcParameter()
{
	return m_srcParameter;
}

ConstObjectParameterPtr FromCoreConverter::srcParameter() const
{
	return m_srcParameter;
}

