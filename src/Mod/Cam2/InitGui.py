# Sketcher gui init module
# (c) 2003 Juergen Riegel
#
# Gathering all the information to start FreeCAD
# This is the second one of three init scripts, the third one
# runs when the gui is up

#***************************************************************************
#*   (c) Juergen Riegel (juergen.riegel@web.de) 2002                       *
#*                                                                         *
#*   This file is part of the FreeCAD CAx development system.              *
#*                                                                         *
#*   This program is free software; you can redistribute it and/or modify  *
#*   it under the terms of the GNU General Public License (GPL)            *
#*   as published by the Free Software Foundation; either version 2 of     *
#*   the License, or (at your option) any later version.                   *
#*   for detail see the LICENCE text file.                                 *
#*                                                                         *
#*   FreeCAD is distributed in the hope that it will be useful,            *
#*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
#*   GNU Library General Public License for more details.                  *
#*                                                                         *
#*   You should have received a copy of the GNU Library General Public     *
#*   License along with FreeCAD; if not, write to the Free Software        *
#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  *
#*   USA                                                                   *
#*                                                                         *
#*   Juergen Riegel 2002                                                   *
#***************************************************************************/

class CamWorkbench ( Workbench ):
  "Cam workbench object"
  Icon = """
/* XPM */
static char * cam16x16_xpm[] = {
"16 16 139 2",
"      c None",
".     c #302400",
"+     c #473500",
"@     c #DDA600",
"#     c #BA8B00",
"$     c #7A5B00",
"%     c #5E4600",
"&     c #453300",
"*     c #554000",
"=     c #F6B800",
"-     c #FEBF00",
";     c #BFAF00",
">     c #D4B400",
",     c #F9BE00",
"'     c #FFBF00",
")     c #E3AA00",
"!     c #A97E00",
"~     c #705400",
"{     c #2C2000",
"]     c #674D00",
"^     c #FCBD00",
"/     c #DBB600",
"(     c #B3AC00",
"_     c #90A400",
":     c #81A000",
"<     c #198600",
"[     c #709C00",
"}     c #5C4600",
"|     c #5D4500",
"1     c #7D5D00",
"2     c #4C9300",
"3     c #098200",
"4     c #95A500",
"5     c #799E00",
"6     c #108400",
"7     c #0C8300",
"8     c #99A600",
"9     c #543E00",
"0     c #9B7400",
"a     c #664D00",
"b     c #2A1F00",
"c     c #816000",
"d     c #C59400",
"e     c #C0AC00",
"f     c #8AA200",
"g     c #84A100",
"h     c #248900",
"i     c #439100",
"j     c #DDB700",
"k     c #FEBE00",
"l     c #4E3A00",
"m     c #C99700",
"n     c #584200",
"o     c #6C5100",
"p     c #B78900",
"q     c #876600",
"r     c #4F3B00",
"s     c #513D00",
"t     c #5F4700",
"u     c #886600",
"v     c #CC9900",
"w     c #F5B800",
"x     c #4A3700",
"y     c #A87D00",
"z     c #FDBE00",
"A     c #221900",
"B     c #816100",
"C     c #AB8100",
"D     c #FEBC00",
"E     c #E27D00",
"F     c #D76500",
"G     c #DB6D00",
"H     c #CC8800",
"I     c #7C5D00",
"J     c #322500",
"K     c #C29100",
"L     c #0F0B00",
"M     c #AD8200",
"N     c #372900",
"O     c #967100",
"P     c #A07800",
"Q     c #FCB800",
"R     c #B92200",
"S     c #AA0000",
"T     c #B41600",
"U     c #C13400",
"V     c #AE0A00",
"W     c #DD7200",
"X     c #654C00",
"Y     c #B88A00",
"Z     c #110D00",
"`     c #3B2C00",
" .    c #634A00",
"..    c #AD8100",
"+.    c #967000",
"@.    c #DC7000",
"#.    c #C74100",
"$.    c #F7AD00",
"%.    c #EE9900",
"&.    c #574100",
"*.    c #2F2300",
"=.    c #C49300",
"-.    c #8A6700",
";.    c #D35C00",
">.    c #E68700",
",.    c #7A5C00",
"'.    c #926D00",
").    c #9C7500",
"!.    c #5D4600",
"~.    c #806000",
"{.    c #F2A100",
"].    c #AA0100",
"^.    c #876400",
"/.    c #755800",
"(.    c #AC8100",
"_.    c #654B00",
":.    c #D66200",
"<.    c #AE0800",
"[.    c #C33800",
"}.    c #D35D00",
"|.    c #D45E00",
"1.    c #936E00",
"2.    c #604800",
"3.    c #C09000",
"4.    c #6C5000",
"5.    c #3A2B00",
"6.    c #6B5100",
"7.    c #A27A00",
"8.    c #D99700",
"9.    c #D66300",
"0.    c #CE5000",
"a.    c #DE7500",
"b.    c #886500",
"c.    c #725500",
"d.    c #3E2E00",
"e.    c #584100",
"f.    c #705300",
"g.    c #987200",
"h.    c #382A00",
"            .                   ",
"          + @ # $ % &           ",
"        * = - ; > , ' ) ! ~ {   ",
"      ] ^ / ( _ : < [ ' ' } |   ",
"    1 ' 2 3 4 5 6 7 8 ' 9 0 a   ",
"  b c d e f g h i j k l m n o   ",
"  p @ q r s t u v w x y z A B   ",
"  C ' D E F G H I J K L M N O   ",
"  P Q R S T U V W X Y Z `  ...  ",
"  +.@.S #.' ' $.%.~ P &.*. .=.  ",
"  -.;.S >.' ' ' ' ,.-.+.'.).!.  ",
"  ~.{.].;.' ' ' ' ^./.(.k _.    ",
"  /.' :.<.[.}.|.' 1.2.3.4.      ",
"  5.6.7.8.9.0.a.' P b.c.        ",
"          d.e.f.(.g.h.          ",
"                                "};
                """
  MenuText = "Cam"
  ToolTip  = "Cam workbench"

  def Initialize(self):
    # load the module
    import CamGui
    import Cam
  def GetClassName(self):
    return "CamGui::Workbench"

Gui.addWorkbench(CamWorkbench())