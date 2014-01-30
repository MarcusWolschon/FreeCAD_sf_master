# PyTPG implementations

To create a new PyTPG implementation you need to do the following:
* Create a package to store your implemention in this (Cam/App/PyTPG) directory
* In your __init__.py (Cam/App/PyTPG/MyTPG/__init__.py]) module you need to import all your TPG's
 * from MyTPG import MyTPG
* Implement your tpg 
 * Usually in a file called Cam/App/PyTPG/MyTPG/MyTPG.py but can be different
 * You can use what ever structure in your module you like and include any libraries you want (that satisfy the FreeCAD licence)
 * If you can help it don't import any of your sub modules until your TPG is instanciated to improve the load time.
 * @see Cam/App/PyTPG/ExampleTPG/ExampleTPG.py and Cam/App/PyCam.py for information on how to implement the PyTPG API
 * Generate a new id for your PyTPG (Run 'import uuid; str(uuid.uuid1())' from a python console to get a new one)
* Import your module from PyTPG import script (Cam/App/PyTPG/__init__.py)
 * import MyTPG
* Add all your MyTPG files to the TPG_Scripts variable in Cam/App/CMakeLists.txt (Line #110 (approx))
 * # MyTPG module
 * PyTPG/MyTPG/__init__.py
 * PyTPG/MyTPG/MyTPG.py
 * #any other python files you need with your PyTPG
