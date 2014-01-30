################################################################################
# mach3.py
#
#  Directly copied from HeeksCNC with modifications by Dan Falck intended to 
#  use std output via print functions
#  License header also added to be consistant with HeeksCNC

# work derived from HeeksCNC and Hirutso Enni, 2009-01-13
# altered by Dan Falck 2010-08-04
# added tap() arguments Michael Haberler 2010-10-07
#    New BSD License
#    Copyright (c) <2013>, <Dan Heeks>
#    All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#        * Redistributions of source code must retain the above copyright
#          notice, this list of conditions and the following disclaimer.
#        * Redistributions in binary form must reproduce the above copyright
#          notice, this list of conditions and the following disclaimer in the
#          documentation and/or other materials provided with the distribution.
#        * Neither the name of the <organization> nor the
#          names of its contributors may be used to endorse or promote products
#          derived from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
#    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###############################################################################

import nc
import iso
import math

class Creator(iso.Creator):
	def init(self): 
		iso.Creator.init(self) 

	def SPACE(self): return(' ')
	def WORK_OFFSET(self): return('G10 L2' + self.SPACE())
        
	def program_begin(self, id, comment):
		self.write( ('(' + 'GCode created using the FreeCAD Mach3 post processor' + ')' + '\n') )
		self.write( ('(' + comment + ')' + '\n') )

 ############################################################################
    ##  Settings
    
	def work_offset(self, workplane, x=None, y=None, z=None, a=None, b=None, c=None, xy_plane_rotation=None ):
		if x != None or y != None or z != None or a != None or b != None or c != None:
			self.write_blocknum()
			self.write(self.WORK_OFFSET() + ' P' + str(workplane))
			if x != None:
				self.write(self.SPACE() + self.X() + self.SPACE() + self.fmt.string(x))
			if y != None:
				self.write(self.SPACE() + self.Y() + self.SPACE() + self.fmt.string(y))
			if z != None:
				self.write(self.SPACE() + self.Z() + self.SPACE() + self.fmt.string(z))
			if a != None:
				self.write(self.SPACE() + self.A() + self.SPACE() + self.fmt.string(a))
			if b != None:
				self.write(self.SPACE() + self.B() + self.SPACE() + self.fmt.string(b))
			if c != None:
				self.write(self.SPACE() + self.C() + self.SPACE() + self.fmt.string(c))
			self.write('\n')

		if xy_plane_rotation != None:
			self.write_blocknum()
			self.write('G10 L2 P' + str(workplane))
			self.write(self.SPACE() + 'R' + self.SPACE() + self.fmt.string(xy_plane_rotation))
			self.write(self.SPACE() + '(set the XY plane rotation)');
			self.write('\n')

	def debug_message(self, text=None ):
		self.message(text)
		
	def log_message(self, text=None ):
		self.message(text)
		
	def message(self, text=None ):
		# Replace any embedded round brackets with curly braces so that the EMC2 GCode
		# interpreter will not have trouble with the nested comment format.
		_message = text.replace('(','{')
		_message = _message.replace(')','}')
		self.write_blocknum()
		self.write('(MSG,' + _message + ')\n')
		
	def tool_defn(self, id, name='', radius=None, length=None, gradient=None):
		pass

nc.creator = Creator()

