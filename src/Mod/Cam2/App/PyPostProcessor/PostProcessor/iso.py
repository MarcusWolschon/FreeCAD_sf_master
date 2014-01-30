################################################################################
# iso.py
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
import math
from format import Format
from format import *

################################################################################
class Creator(nc.Creator):

    def __init__(self):
        nc.Creator.__init__(self)

        self.a = 0
        self.b = 0
        self.c = 0
        self.f = Address('F', fmt = Format(number_of_decimal_places = 2))
        self.g_plane = Address('G', fmt = Format(number_of_decimal_places = 0))
        self.i = 0
        self.j = 0
        self.k = 0
        self.n = 10
        self.r = 0
        self.s = AddressPlusMinus('S', fmt = Format(number_of_decimal_places = 2), modal = False)
        self.t = None
        self.x = 0
        self.y = 0
        self.z = 500
        self.prev_f = ''
        self.prev_drill = ''
        self.prev_retract = ''
        self.prev_z = ''
        self.useCrc = False
        self.useCrcCenterline = False
        self.gCRC = ''
        self.fmt = Format()
        self.ffmt = Format(number_of_decimal_places = 2)
        self.sfmt = Format(number_of_decimal_places = 1)
        self.machine_coordinates = False
	self.tool_length_compenstation_enabled = False
	self.gcode_is_metric = True
	self.machine_is_metric = False	# Needed when looking at machine variables such as #5063
	self.current_workplane = 1;	# G54 by default.
	self.loop_number = 100		# To name while loops in a unique way.

        
    ############################################################################
    ##  Codes

    def SPACE(self): return(' ')
    def FEEDRATE(self): return('F') 
    def FORMAT_FEEDRATE(self): return('%.2f') 
    def FORMAT_ANG(self): return('%.1f')
    def FORMAT_TIME(self): return('%.2f')
    def FORMAT_DWELL(self): return('P%f')

    def BLOCK(self): return('N%i')

    def COMMENT(self,comment):
	# Replace any embedded round brackets with curly braces so that the GCode
	# interpreter does not have trouble with nested comments.
	_comment = comment.replace('(','{')
	_comment = _comment.replace(')','}')
        return( ('(%s)' % _comment ) )

    def VARIABLE(self): return( '#%i')
    def VARIABLE_SET(self): return( '=%.3f')

    def PROGRAM(self): return( 'O%i')
    def PROGRAM_END(self): return( 'M02')

    def SUBPROG_CALL(self): return( 'M98' + self.SPACE() + 'P%i')
    def SUBPROG_END(self): return( 'M99')

    def STOP_OPTIONAL(self): return('M01')
    def STOP(self): return('M00')

    def IMPERIAL(self): return('G20')
    def METRIC(self): return('G21')
    def ABSOLUTE(self): return('G90')
    def INCREMENTAL(self): return('G91')
    def SET_TEMPORARY_COORDINATE_SYSTEM(self): return('G92')
    def REMOVE_TEMPORARY_COORDINATE_SYSTEM(self): return('G92.1')
    def POLAR_ON(self): return('G16')
    def POLAR_OFF(self): return('G15')
    def PLANE_XY(self): return('G17')
    def PLANE_XZ(self): return('G18')
    def PLANE_YZ(self): return('G19')

    def TOOL(self): return('T%i' + self.SPACE() + 'M06')
    def TOOL_DEFINITION(self): return('G10' + self.SPACE() + 'L1')
    def WORK_OFFSET(self): return('G10 L10' + self.SPACE())

    def WORKPLANE(self): return('G%i')
    def WORKPLANE_BASE(self): return(53)

    def SPINDLE_CW(self): return('M03')
    def SPINDLE_CCW(self): return('M04')
    def SPINDLE_STOPPED(self): return('M05')
    def COOLANT_OFF(self): return('M09')
    def COOLANT_MIST(self): return('M07')
    def COOLANT_FLOOD(self): return('M08')
    def GEAR_OFF(self): return('?')
    def GEAR(self): return('M%i')
    def GEAR_BASE(self): return(37)

    def RAPID(self): return('G00')
    def FEED(self): return('G01')
    def ARC_CW(self): return('G02')
    def ARC_CCW(self): return('G03')
    def NURBS_BEGIN(self): return('G5.2')
    def NURBS_END(self): return('G5.3')
    def WEIGHT(self): return('P')
    def DEGREE(self): return('L')
    def DWELL(self): return('G04')
    def DRILL(self): return('G81')
    def DRILL_WITH_DWELL(self, format, dwell): return('G82' + self.SPACE() + (format.string(dwell)))
    def PECK_DRILL(self): return('G83')
    def PECK_DEPTH(self, format, depth): return('Q' + (format.string(depth)))
    def RETRACT(self, format, height): return('R' + (format.string(height)))
    def END_CANNED_CYCLE(self): return('G80')
    def TAP(self): return('G84')
    def TAP_DEPTH(self, format, depth): return(self.SPACE() + 'K' + (format.string(depth)))
    def BORE_FEED_OUT(self): return('G85')
    def BORE_SPINDLE_STOP_RAPID_OUT(self): return('G86')
    def BORE_DWELL_FEED_OUT(self, format, dwell): return('G89') + self.SPACE() + (format % dwell)

    def X(self): return('X')
    def Y(self): return('Y')
    def Z(self): return('Z')
    def A(self): return('A')
    def B(self): return('B')
    def C(self): return('C')
    def CENTRE_X(self): return('I')
    def CENTRE_Y(self): return('J')
    def CENTRE_Z(self): return('K')
    def RADIUS(self): return('R')
    def TIME(self): return('P')

    def PROBE_TOWARDS_WITH_SIGNAL(self): return('G38.2')
    def PROBE_TOWARDS_WITHOUT_SIGNAL(self): return('G38.3')
    def PROBE_AWAY_WITH_SIGNAL(self): return('G38.4')
    def PROBE_AWAY_WITHOUT_SIGNAL(self): return('G38.5')

    def MACHINE_COORDINATES(self): return('G53')

    def EXACT_PATH_MODE(self): return('G61')
    def EXACT_STOP_MODE(self): return('G61.1')
        
    def ENABLE_TOOL_LENGTH_COMPENSATION(self): return('G43.1')
    def DISABLE_TOOL_LENGTH_COMPENSATION(self): return('G49')

    ############################################################################
    ##  Internals

    def write_feedrate(self):
        self.write(self.f)

    def write_blocknum(self):
        self.write((self.BLOCK() % self.n) + self.SPACE())
        self.n += 10
    
    ############################################################################
    ##  Programs

    def program_begin(self, id, comment):
        self.write( ('(' + comment + ')' + '\n') )
        self.write_blocknum()
        self.write( self.DISABLE_TOOL_LENGTH_COMPENSATION() + '\t(Ensure tool length compensation is OFF)\n' )
        self.tool_length_compenstation_enabled = False
        self.remove_temporary_origin()
        for _coordinate_system_number in range(1,10):
            self.work_offset(workplane=_coordinate_system_number, xy_plane_rotation=0.0)

    def program_stop(self, optional=False):
        self.write_blocknum()
        if (optional) : 
            self.write(self.STOP_OPTIONAL() + '\n')
        else : 
            self.write(self.STOP() + '\n')


    def program_end(self):
	if self.tool_length_compenstation_enabled:
           self.write_blocknum()
           self.write( self.DISABLE_TOOL_LENGTH_COMPENSATION() + '\t(Disable tool length compensation)\n' )
	   self.tool_length_compenstation_enabled = False

	for _coordinate_system_number in range(1,10):
		self.work_offset(workplane=_coordinate_system_number, xy_plane_rotation=0.0)

        self.write_blocknum()
        self.write(self.PROGRAM_END() + '\n')

    ############################################################################
    ##  Subprograms
    
    def sub_begin(self, id, name=''):
        self.write((self.PROGRAM() % id) + self.SPACE() + (self.COMMENT(name)))
        self.write('\n')

    def sub_call(self, id):
        self.write_blocknum()
        self.write((self.SUBPROG_CALL() % id) + '\n')

    def sub_end(self):
        self.write_blocknum()
        self.write(self.SUBPROG_END() + '\n')

    ############################################################################
    ##  Settings
    
    def imperial(self):
        self.write_blocknum()
	self.write( self.IMPERIAL() + '\t(Imperial Values)\n')
        self.fmt.number_of_decimal_places = 4
	self.gcode_is_metric = False

    def metric(self):
        self.write_blocknum()
        self.write( self.METRIC() + '\t (Metric Values)\n' )
        self.fmt.number_of_decimal_places = 3
	self.gcode_is_metric = True

    def machine_units_metric(self, is_metric):
	self.machine_is_metric = is_metric

    def absolute(self):
        self.write_blocknum()
        self.write( self.ABSOLUTE() + '\t (Absolute Coordinates)\n')

    def polar(self, on=True):
        if (on) : 
            self.write_blocknum()
	    self.write(self.POLAR_ON() + '\t(Plar ON)\n' )
        else :
            self.write_blocknum()
	    self.write(self.POLAR_OFF() + '\t(Plar OFF)\n' )

    def set_plane(self, plane):
        if (plane == 0) : 
            self.write_blocknum()
            self.write(self.PLANE_XY() + '\t (Select XY Plane)\n')
        elif (plane == 1) :
            self.write_blocknum()
            self.write(self.PLANE_XZ() + '\t (Select XZ Plane)\n')
        elif (plane == 2) : 
            self.write_blocknum()
            self.write(self.PLANE_YZ() + '\t (Select YZ Plane)\n')

    def set_temporary_origin(self, x=None, y=None, z=None, a=None, b=None, c=None):
        self.write_blocknum()
        self.write((self.SET_TEMPORARY_COORDINATE_SYSTEM()))
        if (x != None): self.write( self.SPACE() + 'X ' + (self.fmt.string(x)) )
        if (y != None): self.write( self.SPACE() + 'Y ' + (self.fmt.string(y)) )
        if (z != None): self.write( self.SPACE() + 'Z ' + (self.fmt.string(z)) )
        if (a != None): self.write( self.SPACE() + 'A ' + (self.fmt.string(a)) )
        if (b != None): self.write( self.SPACE() + 'B ' + (self.fmt.string(b)) )
        if (c != None): self.write( self.SPACE() + 'C ' + (self.fmt.string(c)) )
	self.write('\t(set temporary origin)')
        self.write('\n')

    def remove_temporary_origin(self):
        self.write_blocknum()
        self.write((self.REMOVE_TEMPORARY_COORDINATE_SYSTEM()))
	self.write('\t(revert to previous coordinate system)')
        self.write('\n')

    ############################################################################
    ##  Tools

    def tool_change(self, id, description=None):
	if self.tool_length_compenstation_enabled:
           self.write_blocknum()
           self.write( self.DISABLE_TOOL_LENGTH_COMPENSATION() + '\n' )
	   self.tool_length_compenstation_enabled = False

	if description != None:
		self.message(description)

        self.write_blocknum()
        self.write((self.TOOL() % id) + '\n')
        self.t = id

    def predefined_position(self, type):
	"""Move to a predefined position (G28 or G30)"""
        self.write_blocknum()
        self.write(type + self.SPACE() + '(Move to the predefined position)\n')

    def tool_defn(self, id, name='', radius=None, length=None, gradient=None):
        self.write_blocknum()
        self.write(self.TOOL_DEFINITION())
        self.write(self.SPACE() + ('P%i' % id) + ' ')

        if (radius != None):
            self.write(self.SPACE() + ('R%.3f' % radius))

        if (length != None):
            self.write(self.SPACE() + 'Z%.3f' % length)

        self.write('\n')

    ############################################################################
    ##  Datums
    
    # This is the coordinate system we're using.  G54->G59, G59.1, G59.2, G59.3
    # These are selected by values from 1 to 9 inclusive.
    def workplane(self, id):
	self.current_workplane = id;
        if ((id >= 1) and (id <= 6)):
            self.write_blocknum()
            self.write( (self.WORKPLANE() % (id + self.WORKPLANE_BASE())) + '\t (Select Relative Coordinate System)\n')
        if ((id >= 7) and (id <= 9)):
            self.write_blocknum()
            self.write( ((self.WORKPLANE() % (6 + self.WORKPLANE_BASE())) + ('.%i' % (id - 6))) + '\t (Select Relative Coordinate System)\n')
      
    def work_offset(self, workplane, x=None, y=None, z=None, a=None, b=None, c=None, xy_plane_rotation=None ):
        pass

    ############################################################################
    ##  Rates + Modes

    def feedrate(self, f):
        self.f = self.SPACE() + self.FEEDRATE() + self.ffmt.string(f)
        self.fhv = False

    def feedrate_hv(self, fh, fv):
        self.fh = fh
        self.fv = fv
        self.fhv = True
	self.calc_feedrate_hv( fh, fv )

    def calc_feedrate_hv(self, h, v):
        if math.fabs(v) > math.fabs(h * 2):
            # some horizontal, so it should be fine to use the horizontal feed rate
            self.f = self.SPACE() + self.FEEDRATE() + self.ffmt.string(self.fv)
        else:
            # not much, if any horizontal component, so use the vertical feed rate
            self.f = self.SPACE() + self.FEEDRATE() + self.ffmt.string(self.fh)

    def spindle(self, s, clockwise):
        if s == 0.0:
            self.write_blocknum()
            self.write(self.SPINDLE_STOPPED() + '\n')
	    return

        if (clockwise == True or clockwise == 'True'):
            self.write_blocknum()
            self.write('S' + str(s) + self.SPACE() + self.SPINDLE_CW() + '\n')
        else:
            self.write_blocknum()
	    self.write( 'S' + str(s) + self.SPACE() + self.SPINDLE_CCW() + '\n')

    def coolant(self, mode=0):
        if (mode <= 0) :   
            self.write_blocknum()
	    self.write(self.COOLANT_OFF() + '\n')
        elif (mode == 1) :
            self.write_blocknum()
	    self.write(self.COOLANT_MIST() + '\n')
        elif (mode == 2) : 
            self.write_blocknum()
	    self.write(self.COOLANT_FLOOD() + '\n')

    def gearrange(self, gear=0):
        if (gear <= 0) :
            self.write_blocknum()
            self.write(self.GEAR_OFF() + '\n')
        elif (gear <= 4) :
            self.write_blocknum()
	    self.write(self.GEAR() % (gear + GEAR_BASE()) + '\n')

    ############################################################################
    ##  Moves

    def rapid(self, x=None, y=None, z=None, a=None, b=None, c=None, machine_coordinates=None ):
        self.write_blocknum()

        if self.machine_coordinates != False or (machine_coordinates != None and machine_coordinates == True):
            self.write( self.MACHINE_COORDINATES() + self.SPACE() )

        self.write(self.RAPID())
        if (x != None):
            self.write(self.SPACE() + self.X() + (self.fmt.string(x)))
            self.x = x
        if (y != None):
            self.write(self.SPACE() + self.Y() + (self.fmt.string(y)))
            self.y = y
        if (z != None):
            self.write(self.SPACE() + self.Z() + (self.fmt.string(z)))
            self.z = z

        if (a != None):
            self.write(self.SPACE() + self.A() + (self.fmt.string(a)))
            self.a = a

        if (b != None):
            self.write(self.SPACE() + self.B() + (self.fmt.string(b)))
            self.b = b

        if (c != None):
            self.write(self.SPACE() + self.C() + (self.fmt.string(c)))
            self.c = c
        self.write('\n')

    def feed(self, x=None, y=None, z=None):
        if self.same_xyz(x, y, z): return
        self.write_blocknum()
        self.write(self.FEED())
        dx = dy = dz = 0
        if (x != None):
            self.write(self.SPACE() + self.X() + (self.fmt.string(x)))
	    dx = x - self.x
            self.x = x
        if (y != None):
            self.write(self.SPACE() + self.Y() + (self.fmt.string(y)))
	    dy = y - self.y
            self.y = y
        if (z != None):
            self.write(self.SPACE() + self.Z() + (self.fmt.string(z)))
	    dz = z - self.z
            self.z = z
        self.write('\n')

    def same_xyz(self, x=None, y=None, z=None):
        if (x != None):
            if (self.fmt.string(x)) != (self.fmt.string(self.x)):
                return False
        if (y != None):
            if (self.fmt.string(y)) != (self.fmt.string(self.y)):
                return False
        if (z != None):
            if (self.fmt.string(z)) != (self.fmt.string(self.z)):
                return False
            
        return True
    
    def get_quadrant(self, dx, dy):
        if dx < 0:
            if dy < 0:
                return 2
            else:
                return 1
        else:
            if dy < 0:
                return 3
            else:
                return 0
    
    def quadrant_start(self, q, i, j, rad):
        while q > 3: q = q - 4
        if q == 0:
            return i + rad, j
        if q == 1:
            return i, j + rad
        if q == 2:
            return i - rad, j
        return i, j - rad

    def quadrant_end(self, q, i, j, rad):
        return self.quadrant_start(q + 1, i, j, rad)

    def arc(self, cw, x=None, y=None, z=None, i=None, j=None, k=None, r=None):
        #if self.same_xyz(x, y, z): return
        self.write_blocknum()
        if cw: self.write(self.ARC_CW())
        else:  self.write(self.ARC_CCW())

        if (x != None):
            self.write(self.SPACE() + self.X() + (self.fmt.string(x)))
        if (y != None):
            self.write(self.SPACE() + self.Y() + (self.fmt.string(y)))
        if (z != None):
            self.write(self.SPACE() + self.Z() + (self.fmt.string(z)))
        if (i != None):
            self.write(self.SPACE() + self.CENTRE_X() + self.fmt.string(i))
        if (j != None):
            self.write(self.SPACE() + self.CENTRE_Y() + self.fmt.string(j))
        if (k != None):
            self.write(self.SPACE() + self.CENTRE_Z() + self.fmt.string(k))
        if (r != None):
            self.write(self.SPACE() + self.RADIUS() + self.fmt.string(r))
#       use horizontal feed rate
        self.write('\n')
        if (x != None):
            self.x = x
        if (y != None):
            self.y = y
        if (z != None):
            self.z = z

    def arc_cw(self, x=None, y=None, z=None, i=None, j=None, k=None, r=None):
        self.arc(True, x, y, z, i, j, k, r)

    def arc_ccw(self, x=None, y=None, z=None, i=None, j=None, k=None, r=None):
        self.arc(False, x, y, z, i, j, k, r)

    def dwell(self, t):
        self.write_blocknum()
        self.write(self.DWELL() + (self.TIME() % t))
        self.write('\n')

    ############################################################################
    ##  CRC
    
    def use_CRC(self):
        return self.useCrc

    def CRC_nominal_path(self):
        return self.useCrcCenterline

    def start_CRC(self, left = True, radius = 0.0):
        if self.t == None:
            raise "No tool specified for start_CRC()"
        self.write_blocknum()
        if left:
            self.write(('G41' + self.SPACE() + 'D%i') % self.t  + '\t(start left cutter radius compensation)\n' )
        else:
            self.write(('G42' + self.SPACE() + 'D%i') % self.t  + '\t(start right cutter radius compensation)\n' )

    def end_CRC(self):
        self.write_blocknum()
        self.write(self.SPACE() + 'G40\t(end cutter radius compensation)\n')

    ############################################################################
    ##  Cycles

    def boring(self, x=None, y=None, z=None, depth=None, standoff=None, dwell=None, retract_mode=None, spindle_mode=None, clearance_height=None):
    	"""
	The boring routine supports
		G85 - Boring, no dwell, feed out.
		G86 - Boring, spindle stop, rapid out.
		G89 - Boring, with dwell, feed out
    
	The x,y,z values are INITIAL locations (above the hole to be made.  This is in contrast to
	the Z value used in the G8[1-3] cycles where the Z value is that of the BOTTOM of the hole.
	Instead, this routine combines the Z value and the depth value to determine the bottom of
	the hole.
	The standoff value is the distance up from the 'z' value (normally just above the surface) where the bit retracts
	to in order to clear the swarf.  This combines with 'z' to form the 'R' value in the G8[1-3] cycles.
    
	The peck_depth value is the incremental depth (Q value) that tells the peck drilling
	cycle how deep to go on each peck until the full depth is achieved.
   
	NOTE: This routine forces the mode to absolute mode so that the values  passed into
	the G8[1-3] cycles make sense.  I don't know how to find the mode to revert it so I won't
	revert it.  I must set the mode so that I can be sure the values I'm passing in make
	sense to the end-machine.
	"""

	if (standoff == None):        
		# This is a bad thing.  All the boring cycles need a retraction (and starting) height.        
		return

	if (clearance_height == None):
		clearance_height = standoff
           
	if (z == None): 
		return    # We need a Z value as well.  This input parameter represents the top of the hole

	# Set the retraction point to the 'standoff' distance above the starting z height.        
	retract_height = z + standoff

	self.rapid(x=x,y=y)
	self.rapid(z=standoff)
	
	self.write_blocknum()
        
	if (dwell == None) or (dwell == 0):
		# No dwell.

		if (spindle_mode == 0):
			# spindle stop
			if (retract_mode == 0):
				# rapid retraction
				self.write('G86' + self.SPACE())
			else:
				# feed retract
				self.write('G85' + self.SPACE())
		else:
			# spindle run mode.
			if (retract_mode == 0):
				# rapid retraction
				self.write('G85' + self.SPACE())
			else:
				# feed retract
				self.write('G85' + self.SPACE())

	else:
		# With dwell.
		if (spindle_mode == 0):
			# spindle stop

			if (retract_mode == 0):
				# rapid retraction
				self.write('G86' + self.SPACE())
			else:
				# feed retract
				self.write('G89' + self.SPACE())
		else:
			# spindle run mode.

			if (retract_mode == 0):
				# rapid retraction
				self.write('G85' + self.SPACE())
			else:
				# feed retract
				self.write('G89' + self.SPACE())

                self.write( 'P' + (self.fmt.string(dwell)) + self.SPACE())

        if (x != None):        
            dx = x - self.x        
            self.write(self.SPACE() + self.X() + (self.fmt.string(x)))        
            self.x = x 
       
        if (y != None):        
            dy = y - self.y        
            self.write(self.SPACE() + self.Y() + (self.fmt.string(y)))        
            self.y = y
                      
        dz = (z + standoff) - self.z # In the end, we will be standoff distance above the z value passed in.

        self.write(self.SPACE() + self.Z() + (self.fmt.string(z - depth)))    # This is the 'z' value for the bottom of the hole.
        self.z = (z + standoff)            # We want to remember where z is at the end (at the top of the hole)

        self.write(self.SPACE() + self.RETRACT(self.fmt, retract_height))
           
        self.write('\n')
       
	self.rapid(z=clearance_height)



    # The drill routine supports drilling (G81), drilling with dwell (G82) and peck drilling (G83).
    # The x,y,z values are INITIAL locations (above the hole to be made.  This is in contrast to
    # the Z value used in the G8[1-3] cycles where the Z value is that of the BOTTOM of the hole.
    # Instead, this routine combines the Z value and the depth value to determine the bottom of
    # the hole.
    #
    # The standoff value is the distance up from the 'z' value (normally just above the surface) where the bit retracts
    # to in order to clear the swarf.  This combines with 'z' to form the 'R' value in the G8[1-3] cycles.
    #
    # The peck_depth value is the incremental depth (Q value) that tells the peck drilling
    # cycle how deep to go on each peck until the full depth is achieved.
    #
    # NOTE: This routine forces the mode to absolute mode so that the values  passed into
    # the G8[1-3] cycles make sense.  I don't know how to find the mode to revert it so I won't
    # revert it.  I must set the mode so that I can be sure the values I'm passing in make
    # sense to the end-machine.
    #
    def drill(self, x=None, y=None, z=None, depth=None, standoff=None, dwell=None, peck_depth=None, retract_mode=None, clearance_height=None):
        if (standoff == None):        
        # This is a bad thing.  All the drilling cycles need a retraction (and starting) height.        
            return

	if (clearance_height == None):
		clearance_height = standoff
           
        if (z == None): 
            return    # We need a Z value as well.  This input parameter represents the top of the hole

	self.rapid(x=x,y=y)
	self.rapid(z=standoff)
	
        self.write_blocknum()
        
        if (peck_depth != 0):        
            # We're pecking.  Let's find a tree. 
            self.write(self.PECK_DRILL() + self.SPACE() + self.PECK_DEPTH(self.fmt, peck_depth))  
        else:        
            # We're either just drilling or drilling with dwell.        
            if (dwell == 0):        
                # We're just drilling. 
                self.write(self.DRILL())  
            else:        
                # We're drilling with dwell.
                self.write(self.DRILL_WITH_DWELL(self.FORMAT_DWELL(),dwell))  
    
    # Set the retraction point to the 'standoff' distance above the starting z height.        
        retract_height = z + standoff        
        if (x != None):        
            dx = x - self.x        
            self.write(self.SPACE() + self.X() + (self.fmt.string(x)))        
            self.x = x 
       
        if (y != None):        
            dy = y - self.y        
            self.write(self.SPACE() + self.Y() + (self.fmt.string(y)))        
            self.y = y
                      
        dz = (z + standoff) - self.z # In the end, we will be standoff distance above the z value passed in.

        self.write(self.SPACE() + self.Z() + (self.fmt.string(z - depth)))    # This is the 'z' value for the bottom of the hole.
        self.z = (z + standoff)            # We want to remember where z is at the end (at the top of the hole)

        self.write(self.SPACE() + self.RETRACT(self.fmt, retract_height))
           
        self.write('\n')
       
	self.rapid(z=clearance_height)


 
    # G33.1 tapping with EMC for now
    # unsynchronized (chuck) taps NIY (tap_mode = 1)
    
    def tap(self, x=None, y=None, z=None, \
		    zretract=None, depth=None, standoff=None, \
		    dwell_bottom=None, pitch=None, stoppos=None, \
		    spin_in=None, spin_out=None, tap_mode=None, \
		    direction=None, clearance_height=None):
        # mystery parameters: 
        # zretract=None, dwell_bottom=None,pitch=None, stoppos=None, spin_in=None, spin_out=None):
        # I dont see how to map these to EMC Gcode

        if (standoff == None):		
                # This is a bad thing.  All the drilling cycles need a retraction (and starting) height.		
                return
        if (z == None): 
                return	# We need a Z value as well.  This input parameter represents the top of the hole 
        if (pitch == None): 
                return	# We need a pitch value.
        if (direction == None): 
                return	# We need a direction value.

        if (tap_mode != 0):
                raise "only rigid tapping currently supported"

	if (clearance_height == None):
		clearance_height = standoff

        # rapid to starting point; z first, then x,y iff given

        # Set the retraction point to the 'standoff' distance above the starting z height.		
        retract_height = z + standoff

        # unsure if this is needed:
	self.rapid(z = clearance_height)

        # then continue to x,y if given
        if (x != None) or (y != None):
                        self.write_blocknum()				
                        self.write(self.RAPID() )		   

                        if (x != None):		
                                        self.write(self.X() + self.fmt.string(x) + self.SPACE())
                                        self.x = x 

                        if (y != None):		
                                        self.write(self.Y() + self.fmt.string(y) + self.SPACE())		
                                        self.y = y
                        self.write('\n')

	self.rapid(z = retract_height)

        self.write_blocknum()				
        self.write( self.TAP() )
        self.write( self.TAP_DEPTH(self.ffmt,pitch) + self.SPACE() )			
        self.write(self.Z() + self.fmt.string(z - depth))	# This is the 'z' value for the bottom of the tap.
        self.write(self.RETRACT( self.ffmt, retract_height))	# This is the 'z' value at the top of the tap.
        self.write('\n')

        self.z = retract_height	# this cycle returns to the start position, so remember that as z value
        
    def end_canned_cycle(self):
        self.write_blocknum()
        self.write(self.END_CANNED_CYCLE() + '\n')
        self.prev_drill = ''
        self.prev_z = ''   
        self.prev_f = '' 
        self.prev_retract = ''
    
    ############################################################################
    ##  Misc

    def comment(self, text):
        self.write_blocknum()
        self.write((self.COMMENT(text) + '\n'))

    def variable(self, id):
        return (self.VARIABLE() % id)

    def variable_set(self, id, value):
        self.write_blocknum()
        self.write(self.SPACE() + (self.VARIABLE() % id) + self.SPACE() + (self.VARIABLE_SET() % value) + '\n')

    # Rapid movement to the midpoint between the two points specified.
    # NOTE: The points are specified either as strings representing numbers or as strings
    # representing variable names.  This allows the HeeksCNC module to determine which
    # variable names are used in these various routines.
    def rapid_to_midpoint(self, x1=None, y1=None, z1=None, x2=None, y2=None, z2=None):
        self.write_blocknum()
        self.write(self.RAPID())
        if ((x1 != None) and (x2 != None)):
            self.write((' X ' + '[[[' + x1 + ' - ' + x2 + '] / 2.0] + ' + x2 + ']'))

        if ((y1 != None) and (y2 != None)):
            self.write((' Y ' + '[[[' + y1 + ' - ' + y2 + '] / 2.0] + ' + y2 + ']'))

        if ((z1 != None) and (z2 != None)):
            self.write((' Z ' + '[[[' + z1 + ' - ' + z2 + '] / 2.0] + ' + z2 + ']'))

        self.write('\n')

    def BEST_POSSIBLE_SPEED(self, motion_blending_tolerance, naive_cam_tolerance): 
	    statement = 'G64'

	    if (motion_blending_tolerance > 0):
		    statement += ' P ' + str(motion_blending_tolerance)

	    if (naive_cam_tolerance > 0):
		    statement += ' Q ' + str(naive_cam_tolerance)

	    return(statement)
            
    def set_path_control_mode(self, mode, motion_blending_tolerance, naive_cam_tolerance ):
        self.write_blocknum()
        if (mode == 0):
            self.write( self.EXACT_PATH_MODE() + '\n' )
        if (mode == 1):
            self.write( self.EXACT_STOP_MODE() + '\n' )
        if (mode == 2):
            self.write( self.BEST_POSSIBLE_SPEED( motion_blending_tolerance, naive_cam_tolerance ) + '\n' )
        

################################################################################

nc.creator = Creator()
