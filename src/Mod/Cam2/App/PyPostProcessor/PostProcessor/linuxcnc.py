################################################################################
# linuxcnc.py
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
	def WORK_OFFSET(self): return('G10 L20' + self.SPACE())

	def report_probe_results(self, x1=None, y1=None, z1=None, \
					x2=None, y2=None, z2=None, \
					x3=None, y3=None, z3=None, \
					x4=None, y4=None, z4=None, \
					x5=None, y5=None, z5=None, \
					x6=None, y6=None, z6=None, \
					xml_file_name=None ):
		if (xml_file_name != None):
			self.comment('Generate an XML document describing the probed coordinates found');
			self.write_blocknum()
			self.write('(LOGOPEN,')
			self.write(xml_file_name)
			self.write(')\n')

		self.write_blocknum()
		self.write('(LOG,<POINTS>)\n')

		if ((x1 != None) or (y1 != None) or (z1 != None)):
			self.write_blocknum()
			self.write('(LOG,<POINT>)\n')

		if (x1 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + x1 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<X>#<_value></X>)\n')

		if (y1 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + y1 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Y>#<_value></Y>)\n')

		if (z1 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + z1 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Z>#<_value></Z>)\n')

		if ((x1 != None) or (y1 != None) or (z1 != None)):
			self.write_blocknum()
			self.write('(LOG,</POINT>)\n')

		if ((x2 != None) or (y2 != None) or (z2 != None)):
			self.write_blocknum()
			self.write('(LOG,<POINT>)\n')

		if (x2 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + x2 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<X>#<_value></X>)\n')

		if (y2 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + y2 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Y>#<_value></Y>)\n')

		if (z2 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + z2 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Z>#<_value></Z>)\n')

		if ((x2 != None) or (y2 != None) or (z2 != None)):
			self.write_blocknum()
			self.write('(LOG,</POINT>)\n')

		if ((x3 != None) or (y3 != None) or (z3 != None)):
			self.write_blocknum()
			self.write('(LOG,<POINT>)\n')

		if (x3 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + x3 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<X>#<_value></X>)\n')

		if (y3 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + y3 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Y>#<_value></Y>)\n')

		if (z3 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + z3 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Z>#<_value></Z>)\n')

		if ((x3 != None) or (y3 != None) or (z3 != None)):
			self.write_blocknum()
			self.write('(LOG,</POINT>)\n')

		if ((x4 != None) or (y4 != None) or (z4 != None)):
			self.write_blocknum()
			self.write('(LOG,<POINT>)\n')

		if (x4 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + x4 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<X>#<_value></X>)\n')

		if (y4 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + y4 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Y>#<_value></Y>)\n')

		if (z4 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + z4 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Z>#<_value></Z>)\n')

		if ((x4 != None) or (y4 != None) or (z4 != None)):
			self.write_blocknum()
			self.write('(LOG,</POINT>)\n')

		if ((x5 != None) or (y5 != None) or (z5 != None)):
			self.write_blocknum()
			self.write('(LOG,<POINT>)\n')

		if (x5 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + x5 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<X>#<_value></X>)\n')

		if (y5 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + y5 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Y>#<_value></Y>)\n')

		if (z5 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + z5 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Z>#<_value></Z>)\n')

		if ((x5 != None) or (y5 != None) or (z5 != None)):
			self.write_blocknum()
			self.write('(LOG,</POINT>)\n')

		if ((x6 != None) or (y6 != None) or (z6 != None)):
			self.write_blocknum()
			self.write('(LOG,<POINT>)\n')

		if (x6 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + x6 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<X>#<_value></X>)\n')

		if (y6 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + y6 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Y>#<_value></Y>)\n')

		if (z6 != None):
			self.write_blocknum()
			self.write('#<_value>=[' + z6 + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Z>#<_value></Z>)\n')

		if ((x6 != None) or (y6 != None) or (z6 != None)):
			self.write_blocknum()
			self.write('(LOG,</POINT>)\n')

		self.write_blocknum()
		self.write('(LOG,</POINTS>)\n')

		if (xml_file_name != None):
			self.write_blocknum()
			self.write('(LOGCLOSE)\n')

	def confirm_probe_input(self, want_it_to_be_tripped, use_m66_to_confirm_probe_state=None, m66_input_pin_number=None ):
		if ((use_m66_to_confirm_probe_state != None) and \
		    ((use_m66_to_confirm_probe_state == True) or (use_m66_to_confirm_probe_state == 'True'))) and \
		    (m66_input_pin_number != None):

	        	self.comment('Confirm the probe state before we begin probing')

			_do_loop_number = self.loop_number
			self.loop_number = self.loop_number+1
			_if_loop_number = self.loop_number
			self.loop_number = self.loop_number+1

			self.write('#<counter_' + self.fmt.string(_do_loop_number) + '> = 0\n')
			self.write('O' + self.fmt.string(_do_loop_number) + ' DO\n')
			self.write('#<counter_' + self.fmt.string(_do_loop_number) + '> = [#<counter_' + self.fmt.string(_do_loop_number) + '> + 1]\n')
			self.write('M66 P' + self.fmt.string(m66_input_pin_number) + ' L 0\t(Test motion.digital-in-0' + self.fmt.string(m66_input_pin_number) + ')\n')

			if want_it_to_be_tripped == True:
				self.write('O' + self.fmt.string(_if_loop_number) + ' IF [#5399 EQ 0]\n')
				self.message('The probe is NOT currently tripped.  Give it a wiggle and press cycle start to continue')
			else:
				self.write('O' + self.fmt.string(_if_loop_number) + ' IF [#5399 NE 0]\n')
				self.message('The probe is already tripped.  Give it a wiggle and press cycle start to continue')

			self.program_stop(optional=False)
			self.write('M66 P ' + self.fmt.string(m66_input_pin_number) + ' L 0\t(Test motion.digital-in-0' + self.fmt.string(m66_input_pin_number) + ')\n')
			self.write('O' + self.fmt.string(_if_loop_number) + ' ENDIF\n')

			if want_it_to_be_tripped == True:
				self.write('O' + self.fmt.string(_do_loop_number) + ' WHILE [[#5399 EQ 0] AND [#<counter_' + self.fmt.string(_do_loop_number) + '> LT 10]]\n')
			else:
				self.write('O' + self.fmt.string(_do_loop_number) + ' WHILE [[#5399 NE 0] AND [#<counter_' + self.fmt.string(_do_loop_number) + '> LT 10]]\n')

	# This routine uses the G92 coordinate system offsets to establish a temporary coordinate
	# system at the machine's current position.  It can then use absolute coordinates relative
	# to this position which makes coding easy.  It then moves to the 'point along edge' which
	# should be above the workpiece but still on one edge.  It then backs off from the edge
	# to the 'retracted point'.  It then plunges down by the depth value specified.  It then
	# probes back towards the 'destination point'.  The probed X,Y location are stored
	# into the 'intersection variable' variables.  Finally the machine moves back to the
	# original location.  This is important so that the results of multiple calls to this
	# routine may be compared meaningfully.
	#
	# If 'use_m66_to_confirm_probe_state' is True then the code assumes that (for LinuxCNC at least)
	# the probe's input signal has been tied to the input pin as per the "M66 Wait on Input"
	# section of http://linuxcnc.org/docs/html/gcode/m-code.html#sec:M66-Input-Control
	#
	# i.e. something like;
	# net signal-name motion.digital-in-00 <= parport.0.pin10-in
	#
	def probe_single_point(self, \
		point_along_edge_x=None, point_along_edge_y=None, depth=None, \
		retracted_point_x=None, retracted_point_y=None, \
		destination_point_x=None, destination_point_y=None, \
		intersection_variable_x=None, intersection_variable_y=None, \
		probe_offset_x_component=None, probe_offset_y_component=None, \
		use_m66_to_confirm_probe_state=None, \
		m66_input_pin_number=None ):

		self.set_temporary_origin(x=0.0, y=0.0, z=0.0)

		if (self.fhv) : self.calc_feedrate_hv(1, 0)
		self.write_blocknum()
		self.write_feedrate()
		self.write('\t(Set the feed rate for probing)\n')

		self.rapid(point_along_edge_x,point_along_edge_y)
		self.rapid(retracted_point_x,retracted_point_y)
		self.feed(z=-1.0 * depth)

		self.confirm_probe_input(False, use_m66_to_confirm_probe_state, m66_input_pin_number )

		self.write_blocknum()
		self.write((self.PROBE_TOWARDS_WITH_SIGNAL() + (' X ' + (self.fmt.string(destination_point_x)) + ' Y ' + (self.fmt.string(destination_point_y)) ) + ('\t(Probe towards our destination point)\n')))
        
		self.confirm_probe_input(True, use_m66_to_confirm_probe_state, m66_input_pin_number )

	        self.comment('Back off the workpiece and re-probe more slowly')
		self.write_blocknum()
		self.write((self.PROBE_AWAY_WITHOUT_SIGNAL() + (' X ' + self.fmt.string(retracted_point_x) + ' ') + ' Y ' + self.fmt.string(retracted_point_y) + ' ') + ('\t(Move back away until the probe untrips)\n'))

	        self.write_blocknum();
		self.write(self.RAPID())
		self.write(self.SPACE() + ' X [#5061 - [ 0.5 * [' + probe_offset_x_component + ']]] ')
		self.write(self.SPACE() + ' Y [#5062 - [ 0.5 * [' + probe_offset_y_component + ']]]\n')

		self.write_blocknum()
		self.write(self.FEEDRATE() + self.ffmt.string(self.fh / 2.0) + '\n')

		self.confirm_probe_input(False, use_m66_to_confirm_probe_state, m66_input_pin_number )

		self.write_blocknum()
		self.write((self.PROBE_TOWARDS_WITH_SIGNAL() + (' X ' + (self.fmt.string(destination_point_x)) + ' Y ' + (self.fmt.string(destination_point_y)) ) + ('\t(Probe towards our destination point)\n')))

		self.write_blocknum()
		self.write(('#' + intersection_variable_x + ' = [ [' + probe_offset_x_component + '] + #5061]\n'))
		self.write_blocknum()
		self.write(('#' + intersection_variable_y + ' = [ [' + probe_offset_y_component + '] + #5062]\n'))

		self.comment('Now move back to the original location')
		self.rapid(retracted_point_x,retracted_point_y)
		self.rapid(z=0)
		self.rapid(point_along_edge_x,point_along_edge_y)
		self.rapid(x=0, y=0)

		self.remove_temporary_origin()


	def probe_downward_point(self, depth=None, intersection_variable_z=None, touch_off_as_z=None, rapid_down_to_height=None, feedrate=None,	use_m66_to_confirm_probe_state=None, m66_input_pin_number=None ):
		"""
		This routine starts at the machine's current location and probes down (in Z) for the depth specified.
		It stores the RELATIVE coordinate in the variable name specified (as a GCode variable such as #<G54_offset>).
		i.e. since we're using the relative coordinate system (G92), the Z value stored in the #5063 variable
		(in emc2 at least) is in those relative coordinates as opposed to machine or G54 coordinates.

		If the touch_off_as_z variable is given then the current coordinate system (G54, G55 etc.) has its Z offset set
		to the probed point's location as the 'touch_off_as_z' value.  eg: 0.0
		This requires that the rapid_down_to_height value is also given.  We don't really want to rapid to the probed location
		because we don't want to trip the probe during this movement.  As an alternative, we rapid down to this height
		above the probed point and use the G10 L20 command to tell it that we are now at the 'rapid_down_to_height' above
		the zero point for the specified coordinat system.
	
		If 'use_m66_to_confirm_probe_state' is True then the code assumes that (for LinuxCNC at least)
		the probe's input signal has been tied to the input pin as per the "M66 Wait on Input"
		section of http://linuxcnc.org/docs/html/gcode/m-code.html#sec:M66-Input-Control

		i.e. something like;
		net signal-name motion.digital-in-00 <= parport.0.pin10-in
		"""

		self.set_temporary_origin(x=0.0, y=0.0, z=0.0)

		self.feedrate( feedrate )
		self.write_blocknum()
		self.write_feedrate()
		self.write('\n')

		self.confirm_probe_input(False, use_m66_to_confirm_probe_state, m66_input_pin_number )

		self.write_blocknum()
		self.write((self.PROBE_TOWARDS_WITH_SIGNAL() + ' Z ' + (self.fmt.string(depth)) + ('\t(Probe towards our destination point)\n')))

		self.confirm_probe_input(True, use_m66_to_confirm_probe_state, m66_input_pin_number )

	        self.comment('Back off the workpiece and re-probe more slowly')
		self.write_blocknum()
		self.write((self.PROBE_AWAY_WITHOUT_SIGNAL() + (' Z 0.0 ') + ('\t(Move back away until the probe untrips)\n')))

		self.feedrate( feedrate / 2.0 )
		self.write_blocknum()
		self.write_feedrate()
		self.write('\n')

		self.confirm_probe_input(False, use_m66_to_confirm_probe_state, m66_input_pin_number )

		self.write_blocknum()
		self.write((self.PROBE_TOWARDS_WITH_SIGNAL() + ' Z ' + (self.fmt.string(depth)) + ('\t(Probe towards our destination point)\n')))
		self.comment('Store the probed location somewhere we can get it again later')
		self.write_blocknum()
		self.write(('#' + intersection_variable_z + '= #5063\n'))

		if touch_off_as_z != None and rapid_down_to_height != None:
			self.write_blocknum()
			self.write(self.FEED() + self.SPACE() + self.Z() + '[ #' + intersection_variable_z + ' + ' + self.fmt.string(rapid_down_to_height) + ' ]\n')
			self.remove_temporary_origin()
	
			self.write_blocknum()
			self.write(self.WORK_OFFSET() + ' P' + str(self.current_workplane) + self.SPACE() + self.Z() + self.SPACE() + self.fmt.string(rapid_down_to_height + touch_off_as_z) + '\n')
		else:
			self.remove_temporary_origin()


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

	def open_log_file(self, xml_file_name=None ):
		self.write_blocknum()
		self.write('(LOGOPEN,')
		self.write(xml_file_name)
		self.write(')\n')
			
	def close_log_file(self):
		self.write_blocknum()
		self.write('(LOGCLOSE)\n')
			
	def log_coordinate(self, x=None, y=None, z=None):
		if ((x != None) or (y != None) or (z != None)):
			self.write_blocknum()
			self.write('(LOG,<POINT>)\n')

		if (x != None):
			self.write_blocknum()
			self.write('#<_value>=[' + x + ']\n')
			self.write_blocknum()
			self.write('(LOG,<X>#<_value></X>)\n')

		if (y != None):
			self.write_blocknum()
			self.write('#<_value>=[' + y + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Y>#<_value></Y>)\n')

		if (z != None):
			self.write_blocknum()
			self.write('#<_value>=[' + z + ']\n')
			self.write_blocknum()
			self.write('(LOG,<Z>#<_value></Z>)\n')

		if ((x != None) or (y != None) or (z != None)):
			self.write_blocknum()
			self.write('(LOG,</POINT>)\n')

	def debug_message(self, text=None ):
		# Replace any embedded round brackets with curly braces so that the EMC2 GCode
		# interpreter will not have trouble with the nested comment format.
		_message = text.replace('(','{')
		_message = _message.replace(')','}')
		self.write_blocknum()
		self.write('(DEBUG,' + _message + ')\n')
		
	def log_message(self, text=None ):
		# Replace any embedded round brackets with curly braces so that the EMC2 GCode
		# interpreter will not have trouble with the nested comment format.
		_message = text.replace('(','{')
		_message = _message.replace(')','}')
		self.write_blocknum()
		self.write('(LOG,' + _message + ')\n')
		
	def message(self, text=None ):
		# Replace any embedded round brackets with curly braces so that the EMC2 GCode
		# interpreter will not have trouble with the nested comment format.
		_message = text.replace('(','{')
		_message = _message.replace(')','}')
		self.write_blocknum()
		self.write('(MSG,' + _message + ')\n')
		
	def tool_defn(self, id, name='', radius=None, length=None, gradient=None):
		pass

	def nurbs_begin_definition(self, id, degree=None, x=None, y=None, weight=None):
		self.write_blocknum()
		self.write( self.NURBS_BEGIN() )
		self.write( self.SPACE() + self.DEGREE() + (self.fmt.string(degree + 1) ) )
		self.write( self.SPACE() + self.X() + (self.fmt.string(x) ) )
		self.write( self.SPACE() + self.Y() + (self.fmt.string(y) ) )
		self.write( self.SPACE() + self.WEIGHT() + (self.fmt.string(weight) ) )
		self.write( '\n')

	def nurbs_add_pole(self, id, x=None, y=None, weight=None):
		self.write_blocknum()
		self.write( self.SPACE() + self.X() + (self.fmt.string(x) ) )
		self.write( self.SPACE() )
		self.write( self.SPACE() + self.Y() + (self.fmt.string(y) ) )
		self.write( self.SPACE() )
		self.write( self.SPACE() + self.WEIGHT() + (self.fmt.string(weight) ) )
		self.write( '\n')

	def nurbs_end_definition(self, id):
		self.write_blocknum()
		self.write( self.NURBS_END() )
		self.write( '\n')

	def variable_set(self, id, value):
		self.write_blocknum()
		self.write('#' + id + ' = ' + value + '\n')

	def measure_and_offset_tool(self, distance=None, switch_offset_variable_name=None, fixture_offset_variable_name=None, feed_rate=None, \
					use_m66_to_confirm_probe_state=None, \
					m66_input_pin_number=None ):
	        self.write_blocknum()
		self.write(self.DISABLE_TOOL_LENGTH_COMPENSATION() + ' (Turn OFF tool length compensation)\n');
		self.tool_length_compenstation_enabled = False

		self.set_temporary_origin(x=0.0, y=0.0, z=0.0)

		self.confirm_probe_input(False, use_m66_to_confirm_probe_state, m66_input_pin_number )

		# Probe downwards until we hit the tool length measurement switch
		self.write_blocknum()
		self.write((self.PROBE_TOWARDS_WITH_SIGNAL() + self.SPACE() + (self.Z() + (self.fmt.string(-1.0 * float(distance))))) )
		self.write( self.SPACE() + self.FEEDRATE() + self.fmt.string(feed_rate) + '\t(Probe down to find the tool length switch)\n' )

		self.write_blocknum()
		self.write(self.ENABLE_TOOL_LENGTH_COMPENSATION() + self.SPACE() + self.Z() + '[#5063 - #' + switch_offset_variable_name + ']' + ' (Turn ON tool length compensation)\n');
		self.tool_length_compenstation_enabled = True

		self.remove_temporary_origin()

	def probe_grid(self, x_increment, x_count, y_increment, y_count, z_safety, z_probe, feed_rate, filename):
	        self.write_blocknum()
		self.write(self.FEEDRATE() + self.fmt.string(feed_rate) + '\n' )
		self.set_temporary_origin(x=0.0, y=0.0, z=0.0)
	
		self.write( self.SPACE() + '#<x_start>=0.0\t(X start)\n');
		self.write( self.SPACE() + '#<x_increment>=' + self.fmt.string(x_increment) + '\t(X increment)\n')
		self.write( self.SPACE() + '#<x_count_max>=' + self.fmt.string(x_count) + '\t(X count)\n')
		self.write( self.SPACE() + '#<y_start>=0.0\n')
		self.write( self.SPACE() + '#<y_increment>=' + self.fmt.string(y_increment) + '\n')
		self.write( self.SPACE() + '#<y_count_max>=' + self.fmt.string(y_count) + '\t(Y count)\n')
		self.write( self.SPACE() + '#<z_safety>=' + self.fmt.string(z_safety) + '\t(Z safety)\n')
		self.write( self.SPACE() + '#<z_probe>=' + self.fmt.string(z_probe) + '\t(Z probe)\n')
		self.write( self.SPACE() + '(LOGOPEN,' + filename + ')\n')
		self.write( self.SPACE() + '(LOG,<POINTS>)\n')
		self.write( self.SPACE() + '#<x_count>=0\n')
		self.write( self.SPACE() + '#<y_count>=0\n')
		self.write( self.SPACE() + 'G00 Z#<z_safety>\n')
		self.write( self.SPACE() + 'O1 while [#<y_count> lt #<y_count_max>]\n')
		self.write( self.SPACE() + '#<x_count>=0\n')
		self.write( self.SPACE() + 'G00 Y[#<y_start>+#<y_increment>*#<y_count>]\n')
		self.write( self.SPACE() + 'O2 while [#<x_count> lt #<x_count_max>]\n')
		self.write( self.SPACE() + 'O3 if [[#<y_count>/2] - fix[#<y_count>/2] eq 0]\n')
		self.write( self.SPACE() + '#<x_target>=[#<x_start>+#<x_increment>*#<x_count>]\n')
		self.write( self.SPACE() + 'O3 else\n')
		self.write( self.SPACE() + '#<x_target>=[#<x_start>+#<x_increment>*[#<x_count_max>-#<x_count> - 1]]\n')
		self.write( self.SPACE() + 'O3 endif\n')
		self.write( self.SPACE() + '#5070=1\n')
		self.write( self.SPACE() + 'O4 while [#5070 NE 0]\n')
		self.write( self.SPACE() + 'G38.5 z#<z_safety>\n')
		self.write( self.SPACE() + 'G38.3 x#<x_target>\n')
		self.write( self.SPACE() + 'O4 endwhile\n')
		self.write( self.SPACE() + 'G00 Z[#5063 + 1.0] (move upwards to really make sure we are ready to probe)\n')
		self.write( self.SPACE() + 'G04 P 0.1 (Pause to allow vibration to stop)\n')
		self.write( self.SPACE() + 'G38.3 Z#<z_probe>\n')
		self.write( self.SPACE() + 'O5 if [ #5071 NE 0 ]\n')
		self.write( self.SPACE() + '(MSG,Press cycle start when ready to continue)\n')
		self.write( self.SPACE() + 'O5 else\n')
		# self.write( self.SPACE() + '(LOG,G1 X#5061 Y#5062 Z#5063)\n')
		self.write( self.SPACE() + '(LOG,<POINT>)\n')
		self.write( self.SPACE() + '(LOG,<ROW>#<x_count></ROW>)\n')
		self.write( self.SPACE() + '(LOG,<COL>#<y_count></COL>)\n')
		self.write( self.SPACE() + '(LOG,<X>#5061</X>)\n')
		self.write( self.SPACE() + '(LOG,<Y>#5062</Y>)\n')
		self.write( self.SPACE() + '(LOG,<Z>#5063</Z>)\n')
		self.write( self.SPACE() + '(LOG,</POINT>)\n')
		self.write( self.SPACE() + 'O5 endif\n')
		self.write( self.SPACE() + '#<x_count> = [#<x_count> + 1]\n')
		self.write( self.SPACE() + 'O2 endwhile\n')
		self.write( self.SPACE() + 'G0 Z#<z_safety>\n')
		self.write( self.SPACE() + '#<y_count>=[#<y_count> + 1]\n')
		self.write( self.SPACE() + 'O1 endwhile\n')
		self.write( self.SPACE() + '(LOG,</POINTS>)\n')
		self.write( self.SPACE() + '(LOGCLOSE)\n')
		self.write( self.SPACE() + 'G00 Z#<z_safety>\n')
		self.write( self.SPACE() + 'G00 X#<x_start> Y#<y_start>\n')

		self.remove_temporary_origin()
nc.creator = Creator()

