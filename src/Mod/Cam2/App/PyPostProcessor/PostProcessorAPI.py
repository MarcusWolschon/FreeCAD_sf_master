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
@author: dfalck
'''

class PyToolPath(object):
    '''
    Stores a single ToolPath program 
    
    A Temporary class that will be replaced by one that functions the same that
    is implemented from the C++ side when integrated with the C++ Cam Workbench
    '''
    
    def __init__(self):
        self.toolpathcmds = []
        
    def addCommand(self, cmd):
        '''Adds a toolpath command to the toolpath'''
        self.toolpathcmds.append(cmd)
    
    # implement the builtin functions so it can function as an iterator and list
    def __iter__(self):
        return iter(self.toolpathcmds)
    def __len__(self):
        return len(self.toolpathcmds)
    def __getitem__(self, *args):
        return self.toolpathcmds.__getitem__(*args)
    def __str__(self):
        return '\n'.join(self.toolpathcmds)
        

class PyMachineProgram(object):
    '''
    Stores a machine specific control program
    
    A Temporary class that will be replaced by one that functions the same that
    is implemented from the C++ side when integrated with the C++ Cam Workbench
    '''
    
    def __init__(self):
        ''''''
        self.machinecodecmds = []
        
    def addCommand(self, cmd):
        '''Adds a machine specific command to the machine-program'''
        self.machinecodecmds.append(cmd)
        
    def __str__(self):
        return '\n'.join(self.machinecodecmds)
        

class PostProcessor(object):
    '''
    The Python side of the PostProcessor API.  This class will be instanciated
    once by the C++ code and used to find and run the Post Processors.
    '''
    
    def __init__(self):
        self._ppcache = None
    
    def getPostProcessorList(self, rescan = True):
        '''
        Gets a list of available post processors.
        
        @param rescan: A boolean value indicating if a full rescan should be done (it will scan on first load anyway)
        @return: a list of 3-tuples (<id>, <name>, <description>)
        '''
        
        postProcessors = []
        
        # only refresh the cache if requested to OR on first load
        if rescan or not self._ppcache:
            self._ppcache = []
            
            # TODO: make this read from the PyPostProcessor directory for all python \
            #  files that implement a (yet to be defined) interface.  
            # Something like this might be suitable:
            ## __ppuuid__ = '{01056c2c-824b-11e2-b45b-902b343fd17b}'
            ## __ppname__ = 'Standard GCode'
            ## __ppdesc__ = 'A standards compliant GCode Post-Processor :P'
            
            # some example (valid) result that should be deleted when it loads the
            # real list from file
            ppuuid = '{01056c2c-824b-11e2-b45b-902b343fd17b}'
            ppname = 'Standard GCode'
            ppdesc = 'A standards compliant GCode Post-Processor :P'
            self._ppcache.append((ppuuid, ppname, ppdesc))
        
        # copy the internal cache
        for pp in self._ppcache:
            postProcessors.append(pp)
        
        return postProcessors
    
    def postProcess(self, toolpath, postProcessor):
        '''
        Executes the post-processor on the given toolpath.
        
        @param toolpath: the PyToolPath instance to post-process
        @param postProcessor: the post-processor (uuid) to use (str instance)
        @return: the result of running the post processor (PyMachineProgram instance)
        '''
        
        # TODO: load the post-processor implementation here

        machineProg = PyMachineProgram()
        
        # NOTES:
        # - if you want to get the toolpath program as a single string (that could be run 
        #   as a python script) you can do:
        #    >>> tpstr = '\n'.join(toolpath)
        # - if you want to loop through each cmd in the toolpath you could do:
        #    >>> for tpcmd in toolpath:
        #    ...     print tpcmd
        # - from my browsing of the HeeksCNC post-processor it might be best to give the 
        #   PyMachineProgram instance to the post-processor class and let it add commands 
        #   like:
        #    >>> machineProg.addCommand('; this is where the machine specific command is placed')
        # - the exec() builtin function might be useful for running toolpath (python code) 
        #   though it does open up a huge security hole.  We can probably trust the TPG 
        #   devs for now but in the future we might lock that down a fair bit:
        #    >>> exec(tpstr, globals(), locals())
        # - check out App.PyCam.PyTPGFactory._scandirs() method for some code that 
        #   searches for python files
            
        # some code to make it do some work (delete me)
        machineProg.addCommand('''; the post processor isn't implemented yet :'(''')
        
        return machineProg
            
