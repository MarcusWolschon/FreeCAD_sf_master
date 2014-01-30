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

#import logging, traceback
import Cam
from PyCam.PyCam import PyTPGBase, UnimplementedTPError


__all__ = ['ExampleTPG']


class ExampleTPG(PyTPGBase):
    '''This is an example TPG that demonstrates how to create a TPG for the 
    CAM2 module.'''
    
#    '''
#    Each settings is made up of a 6-tuple (<name>, <label>, <type>, 
#    <defaultvalue>, <unit>, <helptext>)
#    
#    NOTE: All values must be a string, unicode or None
#    <name>
#        A unique (within the action) identifier for the given setting.  It may 
#        be preceded by one or more group names (separated by ':') which are 
#        used by the UI to construct a setting Tree.
#        
#    <label>
#        The text to display to the user for this setting.
#    
#    <type>
#        May be 'Cam::Group' for a group header (i.e. not a real setting) OR one
#        of the built in types
#        @see: http://sourceforge.net/apps/mediawiki/free-cad/index.php?title=Scripted_objects#Available_properties
#        
#    <defaultvalue>
#        The default value that will appear on screen before the user makes 
#        changes.
#        
#    <unit>
#        Optional help text that identifies the units that this setting expects
#        
#    <helptext>
#        Optional description of this setting; it is useful to include examples 
#        of desirable options
#    '''
    settings = None
    
    # The static identification and descriptive information 
    id = u'10bf335e-2491-11e2-8f39-08002734b94f' #this NEEDS to be unique!!!  @see PyTPGBase.id for details how to generate one.
    name = u'ExampleTPG' # should match classname
    description = u'This is a simple Example Python TPG'
        
    def _defineSettings(self):
        '''Support method to setup the settings structure'''
        
        self.settings = Cam.TPGSettings()
        self.settings.addSettingDefinition('default', Cam.TPGSettingDefinition('geometry', 'Geometry', 'Cam::Textbox', '1', 'mm', 'How close to run tool to final depth'))
        self.settings.addSettingDefinition('default', Cam.TPGSettingDefinition('tolerance', 'Tolerance', 'Cam::Textbox', '1', 'mm', 'How close to run tool to final depth'))
    
    def getActions(self):
        '''Returns a list of actions that this TPG offers.  Actions are like
        methods on a class; they allow a TPG to perform user-selectable tasks.
        i.e. they might calculate the ToolPath using different strategies, 
        they might allow a multi-step strategy to be performed in individual
        steps OR they might provide testing or debug tasks.  If the TPG only
        supports a single action then it should be called 'default'.'''
        
        if self.settings is None:
            self._defineSettings()
        
        return self.settings.keys()

    def getSettingDefinitions(self):
        '''Returns a list of settings that the TPG uses for each 'Action'.
        
        @return: Cam.TPGSettings instance containing the settings
        '''
        try:
            print("Example.getSettingDefinitions() starting")
            if self.settings is None:
                self._defineSettings()
        except Exception as e:
            print e
        return self.settings
        
    def run(self, settings, toolpath, action):
        '''Runs the selected action and returns the resulting TP'''
        
        # First tell the UI we have begun processing
#        self.updateProgress('STARTED', 1)
        
        # get the Input geometry (use getModel*(name) methods to get geometry in various formats)
#        name = settings.getSetting('InputGeometry') # NOTE: this isn't implemented yet
#        geom = self.getModelFreeCAD(name)
        
        if action == 'default':
            # Add toolpath primitives
            # TODO: actually use the geometry
#            self.updateProgress('RUNNING', 2)
            toolpath.addToolPath('rapid(0,0,0.1)')
            toolpath.addToolPath('feed(0,0,-0.5)')
            toolpath.addToolPath('feed(1,0,-0.5)')
            toolpath.addToolPath('feed(1,1,-0.5)')
#            self.updateProgress('RUNNING', 55)
            toolpath.addToolPath('feed(0,1,-0.5)')
            toolpath.addToolPath('feed(0,0,-0.5)')
            toolpath.addToolPath('rapid(0,0,0.1)')
#            self.updateProgress('RUNNING', 99)
        elif action == 'test':
            print 'Testing ExampleTPG'
        
        # Let the UI know we are done
#        self.updateProgress('FINISHED', 100)
        
        return toolpath
    # end run()
