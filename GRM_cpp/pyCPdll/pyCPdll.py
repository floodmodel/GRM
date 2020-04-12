#from ctypes import *
#dll = windll.LoadLibrary('mydll') 
#print(dll.Func("Hello", 1))
#gdl= windll.LoadLibrary('D:/Github/GRM/GRM_cpp/x64/Release/GRM.dll')
import sys, platform
import ctypes, ctypes.util
from decimal import *
# Find the library and load it
gdl_path = ctypes.util.find_library("D:/Github/GRM/GRM_cpp/x64/Release/GRM.dll")
if not gdl_path:
    print("Unable to find the specified library.")
    sys.exit() 
try:
     gdl = ctypes.CDLL(gdl_path)
except OSError:
    print("Unable to load the system C library")
    sys.exit()


#gdl = ctypes.cdll.LoadLibrary('D:/Github/GRM/GRM_cpp/x64/Release/GRM.dll')


class cgrmWS(object):
 
    def __init__(self, a, b): 
        gdl.testClass_new.argtypes = [ctypes.c_int, ctypes.c_int]
        gdl.testClass_new.restype = ctypes.c_void_p

        gdl.testClass_plus.argtypes = [ctypes.c_void_p]
        gdl.testClass_plus.restype = ctypes.c_int

        gdl.grmWS_multi.argtypes = [ctypes.c_void_p]
        gdl.grmWS_multi.restype = Decimal

        gdl.ain.argtypes = [ctypes.c_void_p]
        gdl.ain.restype = ctypes.c_int

        gdl.bin.argtypes = [ctypes.c_void_p]
        gdl.bin.restype = ctypes.c_int
        
        self.obj = gdl.testClass_new(a, b)
 
    def grmWS_plus(self): 
        return gdl.testClass_plus(self.obj)
 
    def grmWS_multi(self):         
        return gdl.testClass_multi(self.obj)

    def grmWS_a(self):
        return gdl.ain(self.obj)

    def grmWS_b(self):
        return gdl.bin(self.obj)
 
if __name__ == '__main__': 
    f = cgrmWS(1,2)    

a= f.grmWS_a
b=f.grmWS_b
print(a)
print(b)


value= f.grmWS_plus()
print(value)

#value= f.grmWS_multi()
#print(value)



#grmf_plus2=grmf
#['grmPlus2']
#value = grmf_plus2(1,2)
#print(value)

#grmf_plus=grmf['grmWS_plus']
#value = grmf_plus(2,3)
#print(value)

#grmf_multi=grmf['grmWS_multi']
#value = grmf_plus(2,3)
#print(value)




#class grmDLL(object):
 
#    def __init__( self ):
 
#        self.lib = cdll.LoadLibrary('D:/Github/GRM/GRM_cpp/x64/Release/GRM.dll')
#        self.obj = self.lib.CHello_new()
 
#    def printOut( self ):
 
#        self.lib.CHello_print( self.obj )
 
#    def push_back( self , s ) :
         
#        self.lib.CHello_push_back( self.obj, s )
 
#if __name__ == '__main__':
 
#    f = grmDLL()
#    f.push_back("-------------------------------------")
#    f.push_back("  hello world ")
#    f.push_back("-------------------------------------")
#    f.printOut()



