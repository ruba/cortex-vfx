##########################################################################
#
#  Copyright (c) 2008-2010, Image Engine Design Inc. All rights reserved.
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

import os.path
import maya.cmds

import IECore
import IECoreMaya

class FromMayaMeshConverterTest( IECoreMaya.TestCase ) :

	def testFactory( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere, IECore.TypeId.MeshPrimitive )
		self.assert_( converter.isInstanceOf( IECore.TypeId( IECoreMaya.TypeId.FromMayaMeshConverter ) ) )

	def testConstructor( self ) :
	
		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaMeshConverter( sphere )
		
		m = converter.convert()
		
		self.failUnless( isinstance( m, IECore.MeshPrimitive ) )

	def testParameters( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["interpolation"].getTypedValue(), "linear" )
		p = converter.convert()
		self.assertEqual( p.interpolation, "linear" )
		converter["interpolation"].setTypedValue( "catmullClark" )
		p = converter.convert()
		self.assertEqual( p.interpolation, "catmullClark" )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["points"].getTypedValue(), True )
		self.assert_( "P" in converter.convert() )
		converter["points"].setTypedValue( False )
		self.assert_( not "P" in converter.convert() )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["normals"].getTypedValue(), True )
		self.assert_( "N" in converter.convert() )
		converter["normals"].setTypedValue( False )
		self.assert_( not "N" in converter.convert() )

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )
		self.assertEqual( converter["st"].getTypedValue(), True )
		self.assert_( "s" in converter.convert() )
		self.assert_( "t" in converter.convert() )
		converter["st"].setTypedValue( False )
		self.assert_( not "s" in converter.convert() )
		self.assert_( not "t" in converter.convert() )

	def testSphere( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )

		m = converter.convert()

		# check topology
		self.assertEqual( m.verticesPerFace.size(), 50 )
		self.assertEqual( m.variableSize( IECore.PrimitiveVariable.Interpolation.Uniform ), 50 )
		self.assertEqual( m.variableSize( IECore.PrimitiveVariable.Interpolation.Vertex ), 42 )
		self.assertEqual( m["P"].data.size(), 42 )
		self.assertEqual( m["N"].data.size(), 180 )
		self.assertEqual( m["s"].data.size(), 180 )
		self.assertEqual( m["t"].data.size(), 180 )
		self.assert_( m["P"].data == converter.points() )
		self.assert_( m["N"].data == converter.normals() )
		self.assert_( m["s"].data == converter.s( "map1" ) )
		self.assert_( m["t"].data == converter.t( "map1" ) )

		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ), IECore.V3f( 1.0001 ) ).contains( m.bound() ) )
		self.assert_( m.bound().contains( IECore.Box3f( IECore.V3f( -0.90 ), IECore.V3f( 0.90 ) ) ) )

	def testSpaces( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		maya.cmds.move( 1, 2, 3, sphere )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )

		self.assertEqual( converter["space"].getNumericValue(), IECoreMaya.FromMayaCurveConverter.Space.Object )
		m = converter.convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ), IECore.V3f( 1.0001 ) ).contains( m.bound() ) )

		converter["space"].setNumericValue( IECoreMaya.FromMayaShapeConverter.Space.World )
		m = converter.convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ) + IECore.V3f( 1, 2, 3 ), IECore.V3f( 1.0001 ) + IECore.V3f( 1, 2, 3 ) ).contains( m.bound() ) )

	def testNormalsOnlyWhenLinear( self ) :

		# adding normals to a mesh which will be rendered subdivided is a very bad thing to do.
		# make sure we aren't doing it.

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( sphere )

		m = converter.convert()
		self.assert_( "N" in m )

		converter["interpolation"].setTypedValue( "catmullClark" )
		m = converter.convert()
		self.assert_( not "N" in m )

	def testWindingOrder( self ) :

		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]

		converter = IECoreMaya.FromMayaShapeConverter.create( plane )

		m = converter.convert()

		p = m["P"].data
		vertexIds = m.vertexIds
		self.assertEqual( vertexIds.size(), 4 )
		loop = IECore.V3fVectorData( [ p[vertexIds[0]], p[vertexIds[1]], p[vertexIds[2]], p[vertexIds[3]] ] )

		self.assert_( IECore.polygonNormal( loop ).equalWithAbsError( IECore.V3f( 0, 1, 0 ), 0.0001 ) )

	def testBlindData( self ) :

		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]

		maya.cmds.addAttr( plane, dataType="string", longName="ieString" )
		maya.cmds.setAttr( plane + ".ieString", "banana", type="string" )

		converter = IECoreMaya.FromMayaShapeConverter.create( plane )
		m = converter.convert()

		self.assertEqual( len( m.blindData().keys() ), 2 )
		self.assertEqual( m.blindData()["name"], IECore.StringData( "pPlaneShape1" ) )
		self.assertEqual( m.blindData()["ieString"], IECore.StringData( "banana" ) )

	def testPrimVars( self ) :

		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]

		maya.cmds.addAttr( plane, attributeType="float", longName="delightDouble", defaultValue=1 )
		maya.cmds.addAttr( plane, dataType="doubleArray", longName="delightDoubleArray" )
		maya.cmds.setAttr( plane + ".delightDoubleArray", ( 10, 11, 12, 13 ), type="doubleArray" )

		converter = IECoreMaya.FromMayaShapeConverter.create( plane, IECore.MeshPrimitive.staticTypeId() )
		m = converter.convert()

		self.assertEqual( len( m.keys() ), 10 )
		self.assertEqual( m["Double"].interpolation, IECore.PrimitiveVariable.Interpolation.Constant )
		self.assertEqual( m["Double"].data, IECore.FloatData( 1 ) )
		self.assertEqual( m["DoubleArray"].interpolation, IECore.PrimitiveVariable.Interpolation.Vertex )
		self.assertEqual( m["DoubleArray"].data, IECore.FloatVectorData( [ 10, 11, 12, 13 ] ) )

	def testConvertFromPlug( self ) :

		sphere = maya.cmds.polySphere( subdivisionsX=10, subdivisionsY=5, constructionHistory=False )
		maya.cmds.move( 1, 2, 3, sphere )
		sphere = maya.cmds.listRelatives( sphere, shapes=True )[0]

		converter = IECoreMaya.FromMayaPlugConverter.create( sphere + ".worldMesh" )

		converter["space"].setNumericValue( IECoreMaya.FromMayaShapeConverter.Space.World )
		m = converter.convert()
		self.assert_( IECore.Box3f( IECore.V3f( -1.0001 ) + IECore.V3f( 1, 2, 3 ), IECore.V3f( 1.0001 ) + IECore.V3f( 1, 2, 3 ) ).contains( m.bound() ) )

	def testSharedSTIndices( self ) :
	
		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/twoTrianglesWithSharedUVs.ma", force = True, open = True )
		
		mesh = IECoreMaya.FromMayaShapeConverter.create( "pPlaneShape1" ).convert()
		
		self.failUnless( "stIndices" in mesh )
		self.assertEqual( mesh["stIndices"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["stIndices"].data, IECore.IntVectorData( [ 0, 1, 2, 2, 1, 3 ] ) )
		
	def testSplitSTIndices( self ) :
			
		maya.cmds.file( os.path.dirname( __file__ ) + "/scenes/twoTrianglesWithSplitUVs.ma", force = True, open = True )
		
		mesh = IECoreMaya.FromMayaShapeConverter.create( "pPlaneShape1" ).convert()
		
		self.failUnless( "stIndices" in mesh )
		self.assertEqual( mesh["stIndices"].interpolation, IECore.PrimitiveVariable.Interpolation.FaceVarying )
		self.assertEqual( mesh["stIndices"].data, IECore.IntVectorData( [ 0, 1, 5, 2, 4, 3 ] ) )	

	def testExtraSTs( self ) :
	
		plane = maya.cmds.polyPlane( ch=False, subdivisionsX=1, subdivisionsY=1 )
		plane = maya.cmds.listRelatives( plane, shapes=True )[0]
		
		converter = IECoreMaya.FromMayaShapeConverter.create( plane, IECore.MeshPrimitive.staticTypeId() )
		m = converter.convert()
		
		self.assert_( "s" in m )
		self.assert_( "t" in m )
		self.assert_( "stIndices" in m )
		self.assert_( "map1_s" in m )
		self.assert_( "map1_t" in m )
		self.assert_( "map1Indices" in m )
		
		maya.cmds.polyUVSet( plane, copy=True, uvSet="map1", newUVSet="map2" )
		
		m = converter.convert()
				
		self.assert_( "s" in m )
		self.assert_( "t" in m )
		self.assert_( "stIndices" in m )
		self.assert_( "map1_s" in m )
		self.assert_( "map1_t" in m )
		self.assert_( "map1Indices" in m )
		self.assert_( "map2_s" in m )
		self.assert_( "map2_t" in m )
		self.assert_( "map2Indices" in m )
		
		
if __name__ == "__main__":
	IECoreMaya.TestProgram()
