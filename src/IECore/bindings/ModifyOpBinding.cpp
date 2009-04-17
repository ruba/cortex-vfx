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

#include <boost/python.hpp>

#include "IECore/ModifyOp.h"
#include "IECore/Parameter.h"
#include "IECore/Object.h"
#include "IECore/CompoundObject.h"
#include "IECore/bindings/IntrusivePtrPatch.h"
#include "IECore/bindings/WrapperToPython.h"
#include "IECore/bindings/RunTimeTypedBinding.h"

using namespace boost;
using namespace boost::python;

namespace IECore {

class ModifyOpWrap : public ModifyOp, public Wrapper<ModifyOp>
{
	public :
		
		ModifyOpWrap( PyObject *self, const std::string name, const std::string description, ParameterPtr resultParameter, ParameterPtr inputParameter )
			: ModifyOp( name, description, resultParameter, inputParameter ), Wrapper<ModifyOp>( self, this )
		{
		};
		
		virtual void modify( ObjectPtr object, ConstCompoundObjectPtr operands )
		{
			//// \todo We may want to call operands->copy() here instead of casting away the constness. If the Python code being called
			/// here actually attempts to change the CompoundObject, then any C++ calling code might get confused when a suposedly const value
			/// changes unexpectedly. Check any performance overhead of the copy.
			this->get_override( "modify" )( object, const_pointer_cast<CompoundObject>( operands ) );
		};

};
IE_CORE_DECLAREPTR( ModifyOpWrap );

void bindModifyOp()
{
	typedef class_< ModifyOp, ModifyOpWrapPtr, boost::noncopyable, bases<Op> > ModifyOpPyClass;
	ModifyOpPyClass( "ModifyOp", no_init )
		.def( init< const std::string, const std::string, ParameterPtr, ParameterPtr >() )
		.IE_COREPYTHON_DEFRUNTIMETYPEDSTATICMETHODS(ModifyOp)
	;
	
	WrapperToPython<ModifyOpPtr>();

	INTRUSIVE_PTR_PATCH( ModifyOp, ModifyOpPyClass );
	implicitly_convertible<ModifyOpPtr, OpPtr>();	

}

} // namespace IECore
