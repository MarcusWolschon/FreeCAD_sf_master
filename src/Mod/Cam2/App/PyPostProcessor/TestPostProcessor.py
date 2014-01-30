'''
/***************************************************************************
 *   Copyright (c) 2012 Andrew Robinson  (andrewjrobinson@gmail.com)       *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

Created on 01/03/2013

@author: arobinson
'''
from PostProcessorAPI import PostProcessor,PyToolPath

def testPP():
    '''Simple test function that demonstrates how to test the post processor'''
    
    pp = PostProcessor()
    
    # get the postprocessor list
    pplist = pp.getPostProcessorList()
    
    print '------------------------------------'
    print 'Post-Processors: '
    for pypp in pplist:
        print '- %s' % (pypp,)
    print '------------------------------------'
    
    # make a toolpath
    tp = PyToolPath()
    tp.addCommand('rapid(z=1)');
    tp.addCommand('rapid(0,0,1)');
    tp.addCommand('feed(0,0,-1)');
    tp.addCommand('feed(0,1)');
    tp.addCommand('feed(1,1)');
    tp.addCommand('feed(1,0)');
    tp.addCommand('feed(0,0)');
    tp.addCommand('rapid(0,0,1)');
    print 'Tool-path:'
    print tp
    print '------------------------------------'
    
    # run the post-processor
    if len(pplist) > 0:
        mp = pp.postProcess(tp, pplist[0][0]) # get the uuid of the first post-processor
        
        print 'Machine program:'
        print mp
        print '------------------------------------'
    else:
        print 'No post-processors found!'

        print 'Done!'
