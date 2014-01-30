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

import types, inspect, uuid, os, tempfile

import Cam, FreeCADGui

PyTPGFactoryInst = None

def scanPlugins():
    '''Short-cut to 'PyTPGFactory.instance().scanPlugins()' '''
    return PyTPGFactory.instance().scanPlugins()

def printPlugins():
    '''Short-cut to 'PyTPGFactory.instance().printPlugins()' '''
    return PyTPGFactory.instance().printPlugins()

def init():
    '''Initialises the PyTPG connection between Python and C++'''
    man = PyTPGFactory.instance()
    if PyTPGFactoryInst and PyTPGFactoryInst != man:
        Cam._registerPyTPGFactory_(man)

class PyTPGBase(object):
    '''The Base class for all Python TPG's'''
    ## Class attributes ##
    id = None # Generate once by running 'import uuid; str(uuid.uuid1())' in python e.g. 'b7b4ada0-23e8-11e2-a9d9-08002734b94f'
    name = "PyTPGBase" # Should match Class name
    description = "This PyTPG forgot to override the description" # to help the user choose your TPG
    
    def __init__(self):
        '''The constructor.  Only called when the PyTPG is added to the FreeCAD project'''
        self._toolPath = None
    
    ## Support methods ##
    
    ## C++ to Python API ##
    # Class methods for TPG list #
    @classmethod
    def getId(cls):
        '''Get the Unique identifier of this PyTPG'''
        return cls.id
    
    @classmethod
    def getName(cls):
        '''Get the user-friendly name of this PyTPG'''
        return cls.name
    
    @classmethod
    def getDescription(cls):
        '''Get the description of this PyTPG'''
        return cls.description
    
    # Methods for once added to Project #
    def getActions(self):
        '''Returns a list of actions that this PyTPG offers'''
        return []

    def getSettingDefinitions(self):
        '''Returns a list of settings that the TPG uses for each 'Action'.
        
        @return: Cam.TPGSettings instance containing the settings
        '''

        return Cam.TPGSettings()
        
    def run(self, settings, toolpath, action):
        '''Runs the selected action and returns the resulting TP'''
        raise UnimplementedTPError(action) # The run() method of this TP hasn't been implemented
    
#     def getToolPath(self):
#         '''Gets a reference to the current ToolPath.  This can be called while 
#         the run method is being executed in another thread to see the progress'''
#         return self._toolPath
    
    ## Python to C++ API ##
    # Call these methods to obtain/send data from/to the C++ side
    
    def updateProgress(self, state, progress):
        '''Updates the UI to show the progress of the PyTPG's processing.
        States:
        - 'UNDEFINED': The default state before anything is done.
        - 'LOADED': Un-used at the moment.
        - 'INITIALISED':  UIManager sets this state just before calling the PyTPG's run method.
        - 'STARTED': PyTPG should set this state as soon as it enters the run method.
        - 'RUNNING': PyTPG should set this state after things are setup and about to start main loop.
        - 'ERROR': If processing fails then set this state to indicate this.  Run method should stop execution soon after this.
        - 'FINISHED': When processing is finished successfully set state to this.  Run method should stop execution soon after this.
        
        @param state: string, one of the states above.
        @param progress: int, the overall progress (percentage) of running this TPG.  Should be >= 1 and <= 99 as 0 and 100 are used by UIanager 
        '''
        # call the same method on the C++ side.
        Cam.updateProgressPyTPG(self.id, state, progress)
    
    ## Python TPG API ##
    def getModelSTL(self, name):
        ''' Gets the Geometry (a single part) in ASCII STL format.
        @param name: The name of the part to obtain in stl format
        @return: string, the entire ASCII STL formatted file or None
        @note: Not thread-safe but unlikely to fail.
        '''
        
        # create a new temp filename that doesn't already exist
        tmpfilename = None
        while tmpfilename == None:
            tmpfilename = "%s/cam%s.stl" % (tempfile.gettempdir(), str(uuid.uuid1()))
            if os.path.exists(tmpfilename):
                tmpfilename = None # try again
        
        # Make the STL formated temp file
        part = self.getModelFreeCAD(name)
        if part:
            try:
                part.exportStl(tmpfilename)
            except:
                print ("Failed to convert part to STL format!")
        
            # read/delete STL temp file
            stlfile = open(tmpfilename)
            stlstr = stlfile.read()
            os.remove(tmpfilename)
            
            return stlstr
        return None
    
    def getModelFreeCAD(self, name):
        ''' Gets the named geometry (single part) as a FreeCAD TopoShape object.
        @param name: The name of the part to obtain in freecad toposhape format
        @return: TopoShape, the part or None
        '''
        try:
            return FreeCADGui.ActiveDocument.getObject(name).Object.Shape
        except:
            print ("Failed to get part!")
        return None
    
# End PyTPGBase

class PyTPGFactory(object):
    '''The Manager class on the Python side of the API.'''
    
    _instance_ = None
    @classmethod
    def instance(cls):
        '''Obtain a reference to the PyTPGFactory singleton'''
        if not cls._instance_:
            cls._instance_ = PyTPGFactory()
        return cls._instance_
    
    def __init__(self, searchpath=['PyTPG']):
        '''Use PyTPGFactory.instance() to get the singleton instance'''
        self._searchpath = searchpath
        self._tpgs = {} # elements {<id>: <class>, ...}

    ## C++ to Python API ##
    def scanPlugins(self):
        '''Search for Python TPGs and return a list of Python Descriptor tuples
        [(<id>, <name>, <description>), ...]'''
        try:
            self._scanDirs()
        except:
            pass
        results = []
        for key in self._tpgs:
            tpg = self._tpgs[key]
            results.append((tpg.id, tpg.name, tpg.description))
        return results
    
    def getPlugin(self, pid):
        '''Gets the Python TPG Class (as opposed to Instance) for the selected id'''
        if pid in self._tpgs:
            return self._tpgs[pid]
        return None
    
    def printPlugins(self):
        '''Print out a user-friendly list of plugins'''
        print "Known python plugins (TPGs):"
        for key in self._tpgs:
            tpgcls = self._tpgs[key]
            print "> %s [%s] '%s'" % (tpgcls.getName(), tpgcls.getId(), tpgcls.getDescription())
    
    def pytest(self, val):
        print "pytest(%s) [%s]" % (val, type(val))
        self.val = val
    
    ## Support Methods ##
    def _scanDirs(self, reload = False):
        '''Scans the entire searchpath for TPGs'''
        for path in self._searchpath:
            self._scanDir(path, reload)
        
    def _scanDir(self, path, reload = False):
        '''Scans <path> package for TPGs'''
        package = __import__(path, globals(), locals(), [], -1)
        # loop through all modules (or special packages @see examples in tpg folder)
        for modname in dir(package):
            mod = getattr(package, modname)
#            print "Scanning: %s (%s)" % (modname, type(mod))
            if type(mod) == types.ModuleType:
                # loop through all the definitions in the module
                for classname in dir(mod):
                    cls = getattr(mod, classname)
#                    print " - %s (%s)" % (classname, type(cls))
                    # filter out only Classes that extend the PyTPGBase superclass
                    if inspect.isclass(cls) and issubclass(cls, PyTPGBase) and cls != PyTPGBase:
                        if reload or classname not in self._tpgs:
                            self._tpgs[cls.getId()] = cls
            # check if they included a class rather than a module
            elif inspect.isclass(mod) and issubclass(mod, PyTPGBase) and mod != PyTPGBase:
                if reload or modname not in self._tpgs:
                    self._tpgs[mod.getId()] = mod
# End PyTPGFactory


class TPError(Exception):
    '''A super class for all Tool Path Errors'''
    

class UnimplementedTPError(TPError):
    def __init__(self, name):
        self.name = name
    def __str__(self):
        return "Unimplemented Tool Path Runner: '%s'" % self.name
    
    
