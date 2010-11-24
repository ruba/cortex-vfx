##########################################################################
#
#  Copyright (c) 2010, Image Engine Design Inc. All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of Image Engine Design nor the names of any
#       other contributors to this software may be used to endorse or
#       promote products derived from this software without specific prior
#       written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
#  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
##########################################################################

import IECore

class FnParameterisedHolder :

	def __init__( self, node ) :
	
		if isinstance( node, basestring ) :
			self.__node = nuke.toNode( node )
		else :
			self.__node = node

	def node( self ) :
	
		return self.__node

	def setParameterised( self, className, classVersion, searchPathEnvVar ) :
	
		if classVersion is None or classVersion < 0 :
			classVersions = IECore.ClassLoader.defaultLoader( searchPathEnvVar ).versions( className )
			classVersion = classVersions[-1] if classVersions else 0 
	
		self.__node.knob( "className" ).setValue( className )
		self.__node.knob( "classVersion" ).setValue( classVersion )
		self.__node.knob( "classSearchPathEnvVar" ).setValue( searchPathEnvVar )
		
		# trigger load
		loadKnob = self.__node.knob( "classLoad" )
		loadKnob.setValue( loadKnob.getValue() + 1 )
		
	## Returns a tuple of the form ( parameterised, className, classVersion, searchPathEnvVar ).
	# Note that currently parameterised will always be None.
	# \todo Implement parameterised return value.
	# This is hard because in Nuke a single node may hold many DD::Image::Ops, for
	# different output contexts. Each of those will store a different Parameterised
	# instance, so there's no such thing as a single instance to be returned. Ideally I think we need to create
	# a unique instance representing the current time.
	def getParameterised( self ) :
	
		return ( 
			None,
			self.__node.knob( "className" ).getText(),
			int( self.__node.knob( "classVersion" ).getValue() ),
			self.__node.knob( "classSearchPathEnvVar" ).getText(),		
		)
	
