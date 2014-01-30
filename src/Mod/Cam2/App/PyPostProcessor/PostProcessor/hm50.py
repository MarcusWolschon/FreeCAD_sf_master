import iso
import linuxcnc

class Creator(linuxcnc.Creator):
	def init(self): 
		iso.Creator.init(self) 
	
iso.creator = Creator()
