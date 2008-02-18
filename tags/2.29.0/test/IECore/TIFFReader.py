##########################################################################
#
#  Copyright (c) 2007, Image Engine Design Inc. All rights reserved.
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

import os
import os.path
import unittest
import sys
import IECore


class TestTIFFReader(unittest.TestCase):

	testoutfile = "test/IECore/data/tiff/testoutput.tif"

	def testConstruction(self):
                
		r = IECore.Reader.create( "test/IECore/data/tiff/rgb_black_circle.256x256.tiff" )
		self.assertEqual(type(r), IECore.TIFFImageReader)
		
	def testRead1(self):
		
		r = IECore.Reader.create( "test/IECore/data/tiff/rgb_black_circle.256x256.tiff" )
		self.assertEqual(type(r), IECore.TIFFImageReader)
		
		r.parameters().dataWindow.setValue(IECore.Box2iData(IECore.Box2i(IECore.V2i(0, 0), IECore.V2i(100, 100))))
		
		self.assertRaises( RuntimeError, r.read )
		

	def testRead2(self):
	
		for f in [ "bluegreen_noise.400x300.tif", "maya.tiff" ] :
										
			r = IECore.Reader.create( os.path.join( "test/IECore/data/tiff",  f) )
			self.assertEqual(type(r), IECore.TIFFImageReader)
		
			img = r.read()
			self.assertEqual(type(img), IECore.ImagePrimitive)
		
			# write test (TIFF -> TIFF)
			w = IECore.Writer.create(img, self.testoutfile)
			self.assertEqual(type(w), IECore.TIFFImageWriter)
		
			w.write()
		
	def testErrors(self):	
	
		# We don't currently handle 1-bit, 2-bit, or 4-bit tiffs	
		
		for bitDepth in [ 1, 2, 4 ]:
		
			r = IECore.Reader.create( "test/IECore/data/tiff/rgb_black_circle.256x256.%dbit.tiff" % ( bitDepth ) )
			self.assertEqual(type(r), IECore.TIFFImageReader)
		
			self.assertRaises( RuntimeError, r.read )
		
	def testCompressionWrite(self):
		
		r = IECore.Reader.create( "test/IECore/data/tiff/bluegreen_noise.400x300.tif" )
		self.assertEqual(type(r), IECore.TIFFImageReader)
		
		img = r.read()
		self.assertEqual(type(img), IECore.ImagePrimitive)
		
		w = IECore.Writer.create( img, self.testoutfile )
		compressions = w.parameters()['compression'].presets()
		self.assertEqual( type(w), IECore.TIFFImageWriter )
		
		for compression in compressions.keys():
			cw = IECore.Writer.create(img, '.'.join([self.testoutfile, compression, 'tif']))
			cw.parameters().compression.setValue(compressions[compression])
			cw.write()
			
	def tearDown(self):
		
		# cleanup
	
		if os.path.isfile( self.testoutfile ) :	
			os.remove( self.testoutfile )				
                			
if __name__ == "__main__":
	unittest.main()   
	
