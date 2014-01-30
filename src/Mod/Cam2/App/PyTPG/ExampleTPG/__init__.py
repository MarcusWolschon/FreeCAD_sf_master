

# You need to import the classes that will be used as TPG's here so python can
# find them when it loads this package
# TODO: improve search so this isn't needed

from ExampleTPG import ExampleTPG
# from SecondPyTPG import SecondPyTPG

# tell Python what modules are in this package
__all__ = [
           'ExampleTPG', 
           'SecondPyTPG',
           ]
