#/***************************************************************************
# *   Copyright (c) 2012 Andrew Robinson  (andrewjrobinson@gmail.com)       *
# *                                                                         *
# *   This file is part of the FreeCAD CAx development system.              *
# *                                                                         *
# *   This library is free software; you can redistribute it and/or         *
# *   modify it under the terms of the GNU Library General Public           *
# *   License as published by the Free Software Foundation; either          *
# *   version 2 of the License, or (at your option) any later version.      *
# *                                                                         *
# *   This library  is distributed in the hope that it will be useful,      *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU Library General Public License for more details.                  *
# *                                                                         *
# *   You should have received a copy of the GNU Library General Public     *
# *   License along with this library; see the file COPYING.LIB. If not,    *
# *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
# *   Suite 330, Boston, MA  02111-1307, USA                                *
# *                                                                         *
# ***************************************************************************/


import tokenize as tok
import types
from cStringIO import StringIO



def test():
    '''Runs a test string through the formatToolPath function and saves the
    result in /tmp/testtp.html'''
    formatted = formatToolPath(teststr)
    fout = open('/tmp/testtp.html', 'w')
    fout.write(header)
    fout.write(formatted)
    fout.write(footer)
    fout.close()
        

def formatToolPath(toolpath):
    '''takes a string of unformated Toolpath commands and applies Source-
    formatting to it.  The returned value is a string of html formatted code'''
    
    output = ""
    
    # load the formatting data
    tokenClass = _setupTokenMappings()
    
    # split the toolpath up into lines for the parser
    tokenParser = tok.generate_tokens(StringLineParser(toolpath))
    
    # tokenise the python code and render has HTML
    currentClass = None
    indent = None
    currentIndent = ""
    indentLevel = 0
    indentHTML = '<span class="indent"></span>'
    for toknum, tokval, _, _, _ in tokenParser:
#        print "'%s' [%s) %s]" % (tokval, toknum, tok.tok_name[toknum])
        # calculate the new class
        newClass = None
        if toknum in tokenClass:
            tokcls = tokenClass[toknum]
            if type(tokcls) == types.DictType:
                if tokval in tokcls:
                    newClass = tokcls[tokval]
                elif '' in tokcls:
                    newClass = tokcls['']
            else:
                if toknum == tok.NUMBER:
                    tokval = str(float(tokval))
                elif toknum == tok.COMMA:
                    tokval += ' '
                elif toknum == tok.INDENT:
                    if not indent:
                        indent = tokval
                    indentLevel = tokval.count(indent)
                    currentIndent = tokval.replace(indent, indentHTML)
                    tokval = ""
                elif toknum == tok.DEDENT:
                    indentLevel -= 1
                    currentIndent = indentHTML * indentLevel
                    output = output[:-len(indentHTML)]
                newClass = tokcls
        
        # change the class (if necessory) and render token
        if toknum == tok.INDENT:
            output += indentHTML
        output += setClass(newClass, currentClass) + tokval
        currentClass = newClass
        if toknum in (tok.NEWLINE, tok.NL):
            output += currentIndent
    # next token
    
    return '<font class="default">%s</font>' % output.replace('\n', '<br />\n')
# End format()

def setClass(new, old):
    '''Changes the class to the new value'''
    content = ""
    if new != old:
        if old != None:
            content += "</font>"
        if new != None:
            content += '<font class="%s">' % new
    return content
            
class StringLineParser(object):
    '''Simple generator that returns the lines from its input string'''
    def __init__(self, string):
        self.gen = self.generator(string)
    def __call__(self):
        return self.gen.next()
    def generator(self, string):
        stri = StringIO(string)
        while True:
            nl = stri.readline()
            if nl != '':
                yield nl
            else:
                raise StopIteration

def _setupTokenMappings():
    '''Returns a dictionary of token ids to css class mapping'''
    
    tokenclasses = {}
    tokenclasses[tok.ENDMARKER] = None
    tokenclasses[tok.NAME] = {'Rapid':'rapid',
            'Feed':'feed',
            'SetTool':'tool',
            'ToolLibrary':'tool',
            'SetMachine':'machine',
            'MachineLibrary':'machine',
            'ArcCW':'arccw',
            'ArcCCW':'arcccw',
            'Dwell':'dwell',
            'Checkpoint':'checkpoint',
            'Custom':'custom',
            'Comment':'comment',
            'Group':'group',
            'SubGroup':'subgroup',
            'SubSubGroup':'subsubgroup',
            '':'othername',}
    tokenclasses[tok.NUMBER] = 'number'
    tokenclasses[tok.STRING] = 'string'
    tokenclasses[tok.NEWLINE] = None
    tokenclasses[tok.INDENT] = None
    tokenclasses[tok.DEDENT] = None
    tokenclasses[tok.LPAR] = 'bracket'
    tokenclasses[tok.RPAR] = 'bracket'
    tokenclasses[tok.LSQB] = 'bracket'
    tokenclasses[tok.RSQB] = 'bracket'
    tokenclasses[tok.COLON] = None
    tokenclasses[tok.COMMA] = None
    tokenclasses[tok.SEMI] = None
    tokenclasses[tok.PLUS] = None
    tokenclasses[tok.MINUS] = None
    tokenclasses[tok.STAR] = None
    tokenclasses[tok.SLASH] = None
    tokenclasses[tok.VBAR] = None
    tokenclasses[tok.AMPER] = None
    tokenclasses[tok.LESS] = None
    tokenclasses[tok.GREATER] = None
    tokenclasses[tok.EQUAL] = None
    tokenclasses[tok.DOT] = None
    tokenclasses[tok.PERCENT] = None
    tokenclasses[tok.BACKQUOTE] = None
    tokenclasses[tok.LBRACE] = 'bracket'
    tokenclasses[tok.RBRACE] = 'bracket'
    tokenclasses[tok.EQEQUAL] = None
    tokenclasses[tok.NOTEQUAL] = None
    tokenclasses[tok.LESSEQUAL] = None
    tokenclasses[tok.GREATEREQUAL] = None
    tokenclasses[tok.TILDE] = None
    tokenclasses[tok.CIRCUMFLEX] = None
    tokenclasses[tok.LEFTSHIFT] = None
    tokenclasses[tok.RIGHTSHIFT] = None
    tokenclasses[tok.DOUBLESTAR] = None
    tokenclasses[tok.PLUSEQUAL] = None
    tokenclasses[tok.MINEQUAL] = None
    tokenclasses[tok.STAREQUAL] = None
    tokenclasses[tok.SLASHEQUAL] = None
    tokenclasses[tok.PERCENTEQUAL] = None
    tokenclasses[tok.AMPEREQUAL] = None
    tokenclasses[tok.VBAREQUAL] = None
    tokenclasses[tok.CIRCUMFLEXEQUAL] = None
    tokenclasses[tok.LEFTSHIFTEQUAL] = None
    tokenclasses[tok.RIGHTSHIFTEQUAL] = None
    tokenclasses[tok.DOUBLESTAREQUAL] = None
    tokenclasses[tok.DOUBLESLASH] = None
    tokenclasses[tok.DOUBLESLASHEQUAL] = None
    tokenclasses[tok.AT] = None
    tokenclasses[tok.OP] = None
    tokenclasses[tok.ERRORTOKEN] = None
    tokenclasses[tok.COMMENT] = 'comment'
    tokenclasses[tok.NL] = None # empty line
    tokenclasses[tok.NT_OFFSET] = None
    
    return tokenclasses
# End _setupTokenMappings()

header = '''<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
<html><head><meta name="qrichtext" content="1" />
<style type="text/css">
span.indent {display: inline-block; width: 20px;}
.default {font-family:'Courier'; font-size:10pt;}
.header {color: #5c46ff;}
.comment {color: #3f7f5f;}
.rapid {color: #9b0000;font-weight: bold;}
.feed {color: #025000;font-weight: bold;}
.control {color: #0d8f94;}
.string {color: #2a00ff;}
.tool {color: #727272;}
.machine {color: #7f0055;}
.number {color: purple;font-weight: bold; font-style: italic;}
.name {color: grey;}
.othername {color: purple;}
</style>
</head>
<body>
<p>'''
footer = '''</p>
</body></html>
'''
teststr = """'''
Generated by FreeCAD/CAM Module
Date: 2012-10-25 22:00
Document: Unnamed.fcstd
'''
# Setup
MachineLibrary('MyMachines')
SetMachine('3-Axis Desktop CNC')
ToolLibrary('Default')

# Box - Rough
SetTool('3.175mm Carbide')
Rapid (0,1,1)
Rapid    (0,1,0.9999)
Feed   (0,1,0.5)
Feed(1,1,z=0.5)
#...

if (SmartSelect()):
    DoSomethingElse()
    if (Greet()):
        Comment("Hello")
        # same comment
    # new level
    a = 1
    b = 2
c = 3

# Box - Finish
SetTool('3.175mm Ball')
"""

# testing run hook
if __name__ == "__main__":
    test()

