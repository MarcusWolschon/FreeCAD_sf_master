################################################################################
# newnc.py
#
#  Base class for NC code creation
#  And global functions for calling current creator
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
################################################################################
#
#	This file contains TWO distinct features.  The first is to define
#	a Creator class that forms the basis of all conversion from the
#	ToolPath language (i.e. rapid(), feed() etc.) to the GCode (G00, G01 etc.)
#
#	The second is to define the COMPLETE series of global functions available
#	to all ToolPath generating modules.  These global functions simply
#	call their counterpart methods in the Creator class.
#
#	The implementation of this Creator class is by way of a base class
#	available for other scripts to override.  We have four main levels
#	available for overriding.  
#
#	The base class version of Creator, found in this nc.py file, holds 
#	methods that simply raise a 'not implemented' exception.  By having
#	this mechanism at the base, we can ensure that the various combinations
#	of overriding classes that follow will either result in all necessary
#	methods being implemented or that the operator will be warned if our
#	ToolPath generator is issuing a command that our machine controller
#	does not support.  eg: NURBS definitions for toolpaths.
#
#	The next is contained in iso.py.  This holds a basic implementation 
#	of only those methods that are likely to be available in all machine 
#	processors (i.e. LinuxCNC, Mach3 etc.)  
#
#	The next layer is based on the machine controllers themselves.  eg:
#	linuxcnc.py, mach3.py etc.  These files override those methods in
#	the iso/nc layers with implementations that are specific to their
#	corresponding machine controller. (eg: the WORK_OFFSET() method
#	produces G10 L2 for Mach3 but G10 L20 for LinuxCNC)
#
#	Finally, we have machine-specific implementations. These are the
#	places where most user configuration must go.  These implementations
#	base their definition of the Creator class on the corresponding
#	machine controller and then override those methods where they want
#	some particular function to occur differently for their machine.
#
#	An example of all of this layering is for a Hafco HM50 model milling
# 	machine that is controlled using LinuxCNC.  This implementation
#	defines an hm50.py file that defines a Creator class that inherits
#	from that defined in the linuxcnc.py file.  The Creator class
#	found within linuxcnc.py is based on that found in the iso.py file.
#	The Creator class defined in iso.py is based on that found in
#	nc.py.  The global functions in nc.py call the Creator class methods
#	which, through our layering of inheritance, ends up calling
#	the most appropriate version of each method at any of the layers
#	of inheritance.
#
################################################################################

import sys
import os
ncOFF = 0
ncLEFT = -1
ncRIGHT = +1
ncCW = -1
ncCCW = +1
ncMIST = 1
ncFLOOD = 2

################################################################################
class Creator:
    def __init__(self):
        pass

    ############################################################################
    ##  Internals

    def file_open(self, name):
        self.file = open(name, 'w')

    def file_close(self):
        self.file.close()

    def write(self, s):
	"""
	We use sys.stdout.write(s) to avoid the implicit space and newline characters
	added by the print() call.  This ends up calling the PythonStdout::write() method
	in PostProcessor.cpp.
	"""
        sys.stdout.write(s)

    ############################################################################
    ##  Programs

    def program_begin(self, id, comment=''):
        """Begin a program"""
        raise RuntimeError( __name__ + ' not implemented' )

    def program_stop(self, optional=False):
        """Stop the machine"""
        raise RuntimeError( __name__ + ' not implemented' )

    def program_end(self):
        """End the program"""
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Subprograms

    def sub_begin(self, id, name=''):
        """Begin a subprogram"""
        raise RuntimeError( __name__ + ' not implemented' )

    def sub_call(self, id):
        """Call a subprogram"""
        raise RuntimeError( __name__ + ' not implemented' )

    def sub_end(self):
        """Return from a subprogram"""
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Settings

    def imperial(self):
        """Set imperial units"""
        raise RuntimeError( __name__ + ' not implemented' )

    def metric(self):
        """Set metric units"""
        raise RuntimeError( __name__ + ' not implemented' )

    def machine_units_metric(self, is_metric):
        """Define machine units just in case we want to look at the machine variable values"""
        raise RuntimeError( __name__ + ' not implemented' )

    def absolute(self):
        """Set absolute coordinates"""
        raise RuntimeError( __name__ + ' not implemented' )

    def incremental(self):
        """Set incremental coordinates"""
        raise RuntimeError( __name__ + ' not implemented' )

    def polar(self, on=True):
        """Set polar coordinates"""
        raise RuntimeError( __name__ + ' not implemented' )

    def set_plane(self, plane):
        """Set plane"""
        raise RuntimeError( __name__ + ' not implemented' )

    def set_temporary_origin(self, x=None, y=None, z=None, a=None, b=None, c=None):
        """Set temporary origin G92"""
        raise RuntimeError( __name__ + ' not implemented' )

    def remove_temporary_origin(self):
        """Remote temporary origin G92.1"""
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Tools

    def tool_change(self, id, description=None):
        """Change the tool"""
        raise RuntimeError( __name__ + ' not implemented' )

    def predefined_position(self, type):
        """Move to pre-defined location (G28 or G30)"""
        raise RuntimeError( __name__ + ' not implemented' )

    def tool_defn(self, id, name='', radius=None, length=None, gradient=None):
        """Define a tool"""
        raise RuntimeError( __name__ + ' not implemented' )

    def offset_radius(self, id, radius=None):
        """Set tool radius offsetting"""
        raise RuntimeError( __name__ + ' not implemented' )

    def offset_length(self, id, length=None):
        """Set tool length offsetting"""
        raise RuntimeError( __name__ + ' not implemented' )

    def measure_and_offset_tool(self, distance=None, switch_offset_variable_name=None, fixture_offset_variable_name=None, feed_rate=None, use_m66_to_confirm_probe_state=None, m66_input_pin_number=None ):
        """Set tool length offsetting"""
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Datums

    def datum_shift(self, x=None, y=None, z=None, a=None, b=None, c=None):
        """Shift the datum"""
        raise RuntimeError( __name__ + ' not implemented' )

    def datum_set(self, x=None, y=None, z=None, a=None, b=None, c=None):
        """Set the datum"""
        raise RuntimeError( __name__ + ' not implemented' )

    def workplane(self, id):
        """Set the workplane"""
        raise RuntimeError( __name__ + ' not implemented' )

    def clearanceplane(self,z=None):
        """set clearance plane"""
        raise RuntimeError( __name__ + ' not implemented' )

    def work_offset(self, workplane, x=None, y=None, z=None, a=None, b=None, c=None, xy_plane_rotation=None ):
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  APT360 like Transformation Definitions
    ##  These definitions were created while looking at Irvin Kraal's book on APT
    ##  - Numerical Control Progamming in APT - page 211

    def matrix(self,a1=None,b1=None,c1=None,a2=None,b2=None,c2=None,a3=None,b3=None,c3=None):
        """Create a matrix for transformations"""
        raise RuntimeError( __name__ + ' not implemented' )

    def translate(self,x=None,y=None,z=None):
        """Translate in x,y,z direction"""
        raise RuntimeError( __name__ + ' not implemented' )

    def rotate(self,xyrot=None,yzrot=None,zxrot=None,angle=None):
        """Rotate about a coordinate axis"""
        raise RuntimeError( __name__ + ' not implemented' )
        
    def scale(self,k=None):
        """Scale by factor k"""
        raise RuntimeError( __name__ + ' not implemented' )

    def matrix_product(self,matrix1=None,matrix2=None):
        """Create matrix that is the product of two other matrices"""
        raise RuntimeError( __name__ + ' not implemented' )

    def mirror_plane(self,plane1=None,plane2=None,plane3=None):
        """Mirror image about one or more coordinate planes"""
        raise RuntimeError( __name__ + ' not implemented' )

    def mirror_line(self,line=None):
        """Mirror about a line"""
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Rates + Modes

    def feedrate(self, f):
        """Set the feedrate"""
        raise RuntimeError( __name__ + ' not implemented' )

    def feedrate_hv(self, fh, fv):
        """Set the horizontal and vertical feedrates"""
        raise RuntimeError( __name__ + ' not implemented' )

    def spindle(self, s, clockwise=True):
        """Set the spindle speed"""
        raise RuntimeError( __name__ + ' not implemented' )

    def coolant(self, mode=0):
        """Set the coolant mode"""
        raise RuntimeError( __name__ + ' not implemented' )

    def gearrange(self, gear=0):
        """Set the gear range"""
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Moves

    def rapid(self, x=None, y=None, z=None, a=None, b=None, c=None, machine_coordinates=None):
        """Rapid move"""
        raise RuntimeError( __name__ + ' not implemented' )

    def feed(self, x=None, y=None, z=None):
        """Feed move"""
        raise RuntimeError( __name__ + ' not implemented' )

    def arc_cw(self, x=None, y=None, z=None, i=None, j=None, k=None, r=None):
        """Clockwise arc move"""
        raise RuntimeError( __name__ + ' not implemented' )

    def arc_ccw(self, x=None, y=None, z=None, i=None, j=None, k=None, r=None):
        """Counterclockwise arc move"""
        raise RuntimeError( __name__ + ' not implemented' )

    def dwell(self, t):
        """Dwell"""
        raise RuntimeError( __name__ + ' not implemented' )

    def rapid_home(self, x=None, y=None, z=None, a=None, b=None, c=None):
        """Rapid relative to home position"""
        raise RuntimeError( __name__ + ' not implemented' )

    def rapid_unhome(self):
        """Return from rapid home"""
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Cutter radius compensation

    def use_CRC(self):
        """CRC"""
        return False

    ############################################################################
    ##  Cycles

    def pattern(self):
        """Simple pattern eg. circle, rect"""
        raise RuntimeError( __name__ + ' not implemented' )

    def pocket(self):
        """Pocket routine"""
        raise RuntimeError( __name__ + ' not implemented' )

    def profile(self):
        """Profile routine"""
        raise RuntimeError( __name__ + ' not implemented' )

    def drill(self, x=None, y=None, z=None, depth=None, standoff=None, dwell=None, peck_depth=None, retract_mode=None, clearance_height=None):
        """Drilling routines"""
        raise RuntimeError( __name__ + ' not implemented' )

    def tap(self, x=None, y=None, z=None, zretract=None, depth=None, standoff=None, dwell_bottom=None, pitch=None, stoppos=None, spin_in=None, spin_out=None, tap_mode=None, direction=None):
        """Tapping routines"""
        raise RuntimeError( __name__ + ' not implemented' )

    def boring(self, x=None, y=None, z=None, depth=None, standoff=None, dwell=None, retract_mode=None, spindle_mode=None, clearance_height=None):
        """Boring routines"""
        raise RuntimeError( __name__ + ' not implemented' )

    def end_canned_cycle(self):
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  Misc

    def comment(self, text):
        """Insert a comment"""
        raise RuntimeError( __name__ + ' not implemented' )

    def insert(self, text):
        """APT style INSERT statement"""
        raise RuntimeError( __name__ + ' not implemented' )

    def block_delete(self, on=False):
        """block to ignore if block delete switch is on"""
        raise RuntimeError( __name__ + ' not implemented' )

    def variable(self, id):
        """Insert a variable"""
        raise RuntimeError( __name__ + ' not implemented' )

    def variable_set(self, id, value):
        """Set a variable"""
        raise RuntimeError( __name__ + ' not implemented' )

    def probe_linear_centre_outside(self, x1=None, y1=None, depth=None, x2=None, y2=None ):
        raise RuntimeError( __name__ + ' not implemented' )

    def probe_single_point(self, point_along_edge_x=None, \
		    		 point_along_edge_y=None, \
				 depth=None, \
				 retracted_point_x=None, \
				 retracted_point_y=None, \
				 destination_point_x=None, \
				 destination_point_y=None, \
				 intersection_variable_x=None, \
				 intersection_variable_y=None, \
				 probe_offset_x_component=None, \
				 probe_offset_y_component=None, \
				 use_m66_to_confirm_probe_state=None, \
				 m66_input_pin_number=None ):
        raise RuntimeError( __name__ + ' not implemented' )

    def probe_grid(self, x_increment=None, \
		    	x_count=None, \
			y_increment=None, \
			y_count=None, \
			z_safety=None, \
			z_probe=None, \
			feed_rate=None, filename=None):
        raise RuntimeError( __name__ + ' not implemented' )

    def probe_downward_point(self, depth=None, \
		    		intersection_variable_z=None, \
				touch_off_as_z=None, \
				rapid_down_to_height=None, \
				feedrate=None, \
				use_m66_to_confirm_probe_state=None, \
				m66_input_pin_number=None):
        raise RuntimeError( __name__ + ' not implemented' )

    def report_probe_results(self, 	x1=None, y1=None, z1=None, \
		    			x2=None, y2=None, z2=None, \
					x3=None, y3=None, z3=None, \
					x4=None, y4=None, z4=None, \
					x5=None, y5=None, z5=None, \
					x6=None, y6=None, z6=None, \
					xml_file_name=None ):
        raise RuntimeError( __name__ + ' not implemented' )

    def open_log_file(self, xml_file_name=None ):
        raise RuntimeError( __name__ + ' not implemented' )

    def log_coordinate(self, x=None, y=None, z=None):
        raise RuntimeError( __name__ + ' not implemented' )

    def log_message(self, message=None):
        raise RuntimeError( __name__ + ' not implemented' )

    def message(self, text=None):
        raise RuntimeError( __name__ + ' not implemented' )

    def close_log_file(self):
        raise RuntimeError( __name__ + ' not implemented' )

    def rapid_to_midpoint(self, x1=None, y1=None, z1=None, x2=None, y2=None, z2=None):
        raise RuntimeError( __name__ + ' not implemented' )

    def rapid_to_intersection(self, 	x1, y1, \
		    			x2, y2, \
					x3, y3, \
					x4, y4, \
					intersection_x, intersection_y, \
					ua_numerator, ua_denominator, ua, \
					ub_numerator, ub):
        raise RuntimeError( __name__ + ' not implemented' )

    def rapid_to_rotated_coordinate(self, x1, y1, x2, y2, ref_x, ref_y, x_current, y_current, x_final, y_final):
        raise RuntimeError( __name__ + ' not implemented' )

    def set_path_control_mode(self, mode, motion_blending_tolerance, naive_cam_tolerance ):
        raise RuntimeError( __name__ + ' not implemented' )

    ############################################################################
    ##  NC code creator for additive machines like RepRap

    def wipe(self):
        """wipe routine"""
        raise RuntimeError( __name__ + ' not implemented' )

    def extruder_on(self):
        """Turn on the extruder"""
        raise RuntimeError( __name__ + ' not implemented' )

    def extruder_off(self):
        """turn off the extruder"""
        raise RuntimeError( __name__ + ' not implemented' )

    def set_extruder_flowrate(self, flowrate):
        """Set the flowrate for the extruder"""
        raise RuntimeError( __name__ + ' not implemented' )

    def extruder_temp(self, temp):
        """Set the extruder temp in celsius"""
        raise RuntimeError( __name__ + ' not implemented' )

    def fan_on(self):
        """turn on the cooling fan"""
        raise RuntimeError( __name__ + ' not implemented' )

    def fan_off(self):
        """turn off the cooling fan"""
        raise RuntimeError( __name__ + ' not implemented' )

    def build_bed_temp(self, temp):
        """Set the bed temp in celsius"""
        raise RuntimeError( __name__ + ' not implemented' )

    def chamber_temp(self, temp):
        """Set the chamber temp in celsius"""
        raise RuntimeError( __name__ + ' not implemented' )

################################################################################
creator = Creator()

############################################################################
##  Internals

def write(s):
    creator.write(s)
    

def output(filename):
    creator.file_open(filename)

############################################################################
##  Programs

def program_begin(id, name=''):
    creator.program_begin(id, name)

def program_stop(optional=False):
    creator.program_stop(optional)


def program_end():
    creator.program_end()

############################################################################
##  Subprograms

def sub_begin(id, name=''):
    creator.sub_begin(id, name)

def sub_call(id):
    creator.sub_call(id)

def sub_end():
    creator.sub_end()


############################################################################
##  Settings

def imperial():
    creator.imperial()

def metric():
    creator.metric()

def machine_units_metric(is_metric):
    creator.machine_units_metric(is_metric)

def absolute():
    creator.absolute()

def incremental():
    creator.incremental()

def polar(on=True):
    creator.polar(on)

def set_plane(plane):
    creator.set_plane(plane)

def set_temporary_origin(x=None, y=None, z=None, a=None, b=None, c=None):
    creator.set_temporary_origin(x,y,z,a,b,c)

def remove_temporary_origin():
    creator.remove_temporary_origin()

############################################################################
##  Tools

def tool_change(id, description=None):
    creator.tool_change(id, description)

def predefined_position(type):
    creator.predefined_position(type)

def tool_defn(id, name='', radius=None, length=None, gradient=None):
    creator.tool_defn(id, name, radius, length, gradient)

def offset_radius(id, radius=None):
    creator.offset_radius(id, radius)

def offset_length(id, length=None):
    creator.offset_length(id, length)

def measure_and_offset_tool(distance=None, switch_offset_variable_name=None, fixture_offset_variable_name=None, feed_rate=None, use_m66_to_confirm_probe_state=None, m66_input_pin_number=None ):
    creator.measure_and_offset_tool(distance, switch_offset_variable_name, fixture_offset_variable_name, feed_rate, use_m66_to_confirm_probe_state, m66_input_pin_number )

############################################################################
##  Datums

def datum_shift(x=None, y=None, z=None, a=None, b=None, c=None):
    creator.datum_shift(x, y, z, a, b, c)

def datum_set(x=None, y=None, z=None, a=None, b=None, c=None):
    creator.datum_set(x, y, z, a, b, c)

def workplane(id):
    creator.workplane(id)

def clearanceplane(z=None):
    creator.clearanceplane(z)

def work_offset(workplane, x=None, y=None, z=None, a=None, b=None, c=None, xy_plane_rotation=None ):
    creator.work_offset( workplane, x, y, z, a, b, c, xy_plane_rotation )

############################################################################
##  APT360 like Transformation Definitions
##  These definitions were created while looking at Irvin Kraal's book on APT
##  - Numerical Control Progamming in APT - page 211

def matrix(a1=None,b1=None,c1=None,a2=None,b2=None,c2=None,a3=None,b3=None,c3=None):
    creator.matrix(a1,b1,c1,a2,b2,c2,a3,b3,c3)

def translate(x=None,y=None,z=None):
    creator.translate(x,y,z)

def rotate(xyrot=None,yzrot=None,zxrot=None,angle=None):
    creator.rotate(xyrot,yzrot,zxrot,angle)

def scale(k=None):
    creator.scale(k)

def matrix_product(matrix1=None,matrix2=None):
    creator.matrix_product(matrix1,matrix2)

def mirror_plane(plane1=None,plane2=None,plane3=None):
    creator.mirror_plane(plane1,plane2,plane3)

def mirror_line(line=None):
    creator.mirror_line(line)

############################################################################
##  Rates + Modes

def feedrate(f):
    creator.feedrate(f)

def feedrate_hv(fh, fv):
    creator.feedrate_hv(fh, fv)

def spindle(s, clockwise=True):
    creator.spindle(s, clockwise)

def coolant(mode=0):
    creator.coolant(mode)

def gearrange(gear=0):
    creator.gearrange(gear)

############################################################################
##  Moves

def rapid(x=None, y=None, z=None, a=None, b=None, c=None, machine_coordinates=None):
    creator.rapid(x, y, z, a, b, c, machine_coordinates)

def feed(x=None, y=None, z=None):
    creator.feed(x, y, z)

def arc_cw(x=None, y=None, z=None, i=None, j=None, k=None, r=None):
    creator.arc_cw(x, y, z, i, j, k, r)

def arc_ccw(x=None, y=None, z=None, i=None, j=None, k=None, r=None):
    creator.arc_ccw(x, y, z, i, j, k, r)

def dwell(t):
    creator.dwell(t)

def rapid_home(x=None, y=None, z=None, a=None, b=None, c=None):
    creator.rapid_home(x, y, z, a, b, c)

def rapid_unhome():
    creator.rapid_unhome()

############################################################################
##  Cutter radius compensation

def use_CRC():
    return creator.use_CRC()

def CRC_nominal_path():
    return creator.CRC_nominal_path()

def start_CRC(left = True, radius = 0.0):
    creator.start_CRC(left, radius)

def end_CRC():
    creator.end_CRC()

############################################################################
##  Cycles

def pattern():
    creator.pattern()

def pocket():
    creator.pocket()

def profile():
    creator.profile()

def drill(x=None, y=None, z=None, depth=None, standoff=None, dwell=None, peck_depth=None, retract_mode=None, clearance_height=None):
    creator.drill(x, y, z, depth, standoff, dwell, peck_depth, retract_mode, clearance_height)

def tap(x=None, y=None, z=None, zretract=None, depth=None, standoff=None, dwell_bottom=None, pitch=None, stoppos=None, spin_in=None, spin_out=None, tap_mode=None, direction=None):
    creator.tap(x, y, z, zretract, depth, standoff, dwell_bottom, pitch, stoppos, spin_in, spin_out, tap_mode, direction)

def boring(x=None, y=None, z=None, depth=None, standoff=None, dwell=None, retract_mode=None, spindle_mode=None, clearance_height=None):
    creator.boring(x, y, z, depth, standoff, dwell, retract_mode, spindle_mode, clearance_height)

def end_canned_cycle():
    creator.end_canned_cycle()

def peck(count, first, last=None, step=0.0):
    pecks = []
    peck = first
    if (last == None) : last = first
    for i in range(0,count):
        pecks.append(peck)
        if (peck - step > last) : peck -= step
    return pecks

############################################################################
##  Misc

def comment(text):
    creator.comment(text)

def insert(text):
    creator.insert(text)

def block_delete(on=False):
    creator.block_delete(on)   

def variable(id):
    creator.variable(id)

def variable_set(id, value):
    creator.variable_set(id, value)

def probe_single_point(point_along_edge_x=None, point_along_edge_y=None, depth=None, retracted_point_x=None, retracted_point_y=None, destination_point_x=None, destination_point_y=None, intersection_variable_x=None, intersection_variable_y=None, probe_offset_x_component=None, probe_offset_y_component=None, use_m66_to_confirm_probe_state=None, m66_input_pin_number=None ):
    creator.probe_single_point(point_along_edge_x, point_along_edge_y, depth, retracted_point_x, retracted_point_y, destination_point_x, destination_point_y, intersection_variable_x, intersection_variable_y, probe_offset_x_component, probe_offset_y_component, use_m66_to_confirm_probe_state, m66_input_pin_number )

def probe_grid(x_increment=None, x_count=None, y_increment=None, y_count=None, z_safety=None, z_probe=None, feed_rate=None, filename=None):
    creator.probe_grid(x_increment, x_count, y_increment, y_count, z_safety, z_probe, feed_rate, filename)

def probe_downward_point(depth=None, intersection_variable_z=None, touch_off_as_z=None, rapid_down_to_height=None, feedrate=None, use_m66_to_confirm_probe_state=None, m66_input_pin_number=None):
    creator.probe_downward_point(depth, intersection_variable_z, touch_off_as_z, rapid_down_to_height, feedrate, use_m66_to_confirm_probe_state, m66_input_pin_number)


def report_probe_results(x1=None, y1=None, z1=None, x2=None, y2=None, z2=None, x3=None, y3=None, z3=None, x4=None, y4=None, z4=None, x5=None, y5=None, z5=None, x6=None, y6=None, z6=None, xml_file_name=None ):
    creator.report_probe_results(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, x5, y5, z5, x6, y6, z6, xml_file_name)

def open_log_file(xml_file_name=None ):
    creator.open_log_file(xml_file_name)

def log_coordinate(x=None, y=None, z=None):
    creator.log_coordinate(x, y, z)

def log_message(message=None):
    creator.log_message(message)

def message(text=None):
    creator.message(text)

def close_log_file():
    creator.close_log_file()

def rapid_to_midpoint(x1=None, y1=None, z1=None, x2=None, y2=None, z2=None):
    creator.rapid_to_midpoint(x1, y1, z1, x2, y2, z2)

def rapid_to_intersection(x1, y1, x2, y2, x3, y3, x4, y4, intersection_x, intersection_y, ua_numerator, ua_denominator, ua, ub_numerator, ub):
    creator.rapid_to_intersection(x1, y1, x2, y2, x3, y3, x4, y4, intersection_x, intersection_y, ua_numerator, ua_denominator, ua, ub_numerator, ub)

def rapid_to_rotated_coordinate(x1, y1, x2, y2, ref_x, ref_y, x_current, y_current, x_final, y_final):
    creator.rapid_to_rotated_coordinate(x1, y1, x2, y2, ref_x, ref_y, x_current, y_current, x_final, y_final)

def set_path_control_mode(mode, motion_blending_tolerance, naive_cam_tolerance ):
    creator.set_path_control_mode(mode, motion_blending_tolerance, naive_cam_tolerance )

############################################################################
##  NC code creator for additive machines like RepRap

def wipe():
    creator.wipe()

def extruder_on():
    creator.extruder_on()

def extruder_off():
    creator.extruder_off()

def set_extruder_flowrate(flowrate):
    creator.set_extruder_flowrate(flowrate)

def extruder_temp(temp=None):
    creator.extruder_temp(temp)

def fan_on():
    creator.fan_on()

def fan_off():
    creator.fan_off()

def build_bed_temp(temp=None):
    creator.build_bed_temp(temp)

def chamber_temp(temp=None):
    creator.chamber_temp(temp)

