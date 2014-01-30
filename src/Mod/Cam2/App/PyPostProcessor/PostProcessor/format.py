################################################################################
#  newformat.py - utility for formatting nc post processors
#
#  Directly copied from HeeksCNC with modifications by Dan Falck intended to 
#  use std output via print functions
#  License header also added to be consistant with HeeksCNC
#
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

import math

class Format:
    def __init__(self, number_of_decimal_places = 3, add_leading_zeros = 1, add_trailing_zeros = False, dp_wanted = True, add_plus = False, no_minus = False, round_down = False):
        self.number_of_decimal_places = number_of_decimal_places
        self.add_leading_zeros = add_leading_zeros # fill the start of the number with zeros, so there are at least this number of digits before the decimal point
        self.add_trailing_zeros = add_trailing_zeros # fill the end of the number with zeros, as defined by "number_of_decimal_places"
        self.dp_wanted = dp_wanted
        self.add_plus = add_plus
        self.no_minus = no_minus
        self.round_down = round_down

    def string(self, number):
        f = float(number) * math.pow(10, self.number_of_decimal_places)
        s = str(f)
        
        if self.round_down == False:
            if f < 0: f = f - .5
            else: f = f + .5
            s = str(number)
            
        if math.fabs(f) < 1.0:
            s = '0'
            
        minus = False
        if s[0] == '-':
            minus = True
            if self.no_minus:
                s = s[1:]
        
        dot = s.find('.')
        if dot == -1:
            before_dp = s
            after_dp = ''
        else:
            before_dp = s[0:dot]
            after_dp = s[dot + 1: dot + 1 + self.number_of_decimal_places]
        
        before_dp = before_dp.zfill(self.add_leading_zeros)
        if self.add_trailing_zeros:
            for i in range(0, self.number_of_decimal_places - len(after_dp)):
                after_dp += '0'
        else:
            after_dp = after_dp.rstrip('0')
                 
        s = ''

        if minus == False:
            if self.add_plus == True:
                s += '+'
        s += before_dp
        if len(after_dp):
            if self.dp_wanted: s += '.'
            s += after_dp
            
        return s

class Address:
    def __init__(self, text, fmt = Format(), modal = True):
        self.text = text
        self.fmt = fmt
        self.modal = modal
        self.str = None
        self.previous = None
        
    def set(self, number):
        self.str = self.text + self.fmt.string(number)
        
    def write(self, writer):
        if self.str == None: return ''
        if self.modal:
            if self.str != self.previous:
                writer.write(writer.SPACE() + self.str)
                self.previous = self.str            
        else:
            writer.write(writer.SPACE() + self.str)
        self.str = None
    
class AddressPlusMinus(Address):
    def __init__(self, text, fmt = Format(), modal = True):
        Address.__init__(self, text, fmt, modal)
        
    def set(self, number, text_plus, text_minus):
        self.str = self.text + self.fmt.string(number)
        if float(number) > 0.0:
            self.str += text_plus
        else:
            self.str += text_minus

