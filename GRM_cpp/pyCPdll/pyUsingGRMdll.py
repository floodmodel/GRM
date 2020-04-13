import sys, platform
import ctypes, ctypes.util
import enum
from ctypes import *

gdl_path = ctypes.util.find_library("D:/Github/GRM/GRM_cpp/x64/Release/GRM.dll") # here, grm dll file full path and name
if not gdl_path:
    print("Unable to find the specified library.")
    sys.exit() 
try:
     gdl = ctypes.CDLL(gdl_path)
except OSError:
    print("Unable to load the system C library")
    sys.exit()


fpn_gmp = "D:\\Github\\zTestSet_GRM_SampleWC_cpp\\SampleProject.gmp"
#fpn_gmp = "D:/Github/zTestSet_GRM_SampleWC_cpp/SampleProject.gmp"
#fpn_gmp = "D:/Github/zTestSet_GRM_SampleGHG_cpp/GHG500_20200407_cpp.gmp"

# codes to implement the class in grm.dll
class unSaturatedKType(enum.Enum):
	Constant=0
	Linear=1
	Exponential=2
	usKNone=3

class swsParameters(Structure):
	_fields_ = [("wsid", ctypes.c_int),
        ("iniSaturation", ctypes.c_double),
        ("minSlopeOF", ctypes.c_double),
        ("unSatKType", ctypes.c_int),# unSaturatedKType,
        ("coefUnsaturatedK", ctypes.c_double),
        ("minSlopeChBed", ctypes.c_double),
        ("minChBaseWidth", ctypes.c_double),
        ("chRoughness", ctypes.c_double),
        ("dryStreamOrder", ctypes.c_int),
        ("iniFlow", ctypes.c_double),
        ("ccLCRoughness", ctypes.c_double),
        ("ccPorosity", ctypes.c_double),
        ("ccWFSuctionHead", ctypes.c_double),
        ("ccHydraulicK", ctypes.c_double),
        ("ccSoilDepth", ctypes.c_double),
        ("userSet", ctypes.c_int)]

#def decode_ctype(x):
#    return bytes(x, encoding='utf-8')
    #return ctypes.c_char_p(x).value.decode('utf-8')

#  class grmWSinfo(object) start =========== 
class grmWSinfo(object): 
    def __init__(self, fpn_gmp): 
        gdl.grmWSinfo_new_gmpFile.argtypes =[ctypes.c_char_p] #[ctypes.c_char_p] #  # string
        gdl.grmWSinfo_new_gmpFile.restype = ctypes.c_void_p

        gdl.isInWatershedArea.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.isInWatershedArea.restype = ctypes.c_bool

        gdl.upStreamWSIDs.argtypes = [ctypes.c_void_p, ctypes.c_int]
        gdl.upStreamWSIDs.restype =  ctypes.Array # ctypes.Array(ctypes.c_int) #vector<int>

        gdl.upStreamWSCount.argtypes = [ctypes.c_void_p, ctypes.c_int]
        gdl.upStreamWSCount.restype = ctypes.c_int

        gdl.downStreamWSIDs.argtypes = [ctypes.c_void_p, ctypes.c_int]
        gdl.downStreamWSIDs.restype = ctypes.Array #  ctypes.Array(ctypes.c_int) #vector<int>

        gdl.downStreamWSCount.argtypes = [ctypes.c_void_p, ctypes.c_int]
        gdl.downStreamWSCount.restype = ctypes.c_int

        gdl.watershedID.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.watershedID.restype = ctypes.c_int

        gdl.flowDirection.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.flowDirection.restype = ctypes.c_char_p # string

        gdl.flowAccumulation.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.flowAccumulation.restype = ctypes.c_int

        gdl.slope.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.slope.restype = ctypes.c_double

        gdl.streamValue.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.streamValue.restype = ctypes.c_int

        gdl.cellFlowTypeOfaCell.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.cellFlowTypeOfaCell.restype = ctypes.c_char_p #string

        gdl.landCoverValue.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.landCoverValue.restype = ctypes.c_int

        gdl.soilTextureValue.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.soilTextureValue.restype = ctypes.c_int

        gdl.soilDepthValue.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.soilDepthValue.restype = ctypes.c_int

        gdl.allCellsInUpstreamArea.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.allCellsInUpstreamArea.restype =ctypes.Array  # ctypes.Array(ctypes.c_char_p) # vector<string>

        gdl.setOneSWSParsAndUpdateAllSWSUsingNetwork.argtypes = [ctypes.c_void_p, 
            ctypes.c_int, ctypes.c_double,
            ctypes.c_double, ctypes.c_char_p, ctypes.c_double,
			ctypes.c_double, ctypes.c_double, ctypes.c_double,
			ctypes.c_int, ctypes.c_double,
			ctypes.c_double, ctypes.c_double, ctypes.c_double,
			ctypes.c_double, ctypes.c_double]
     #     grmWSinfo* f, 
			#int wsid, double iniSat,
			#double minSlopeLandSurface, string unSKType, double coefUnsK,
			#double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
			#int dryStreamOrder, double ccLCRoughness,
			#double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
			#double ccSoilHydraulicCond, double iniFlow = 0)
        gdl.setOneSWSParsAndUpdateAllSWSUsingNetwork.restype = ctypes.c_bool

        gdl.updateAllSubWatershedParametersUsingNetwork.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.updateAllSubWatershedParametersUsingNetwork.restype = None # void

        gdl.subwatershedPars.argtypes =[ctypes.c_void_p, ctypes.c_int]
        gdl.subwatershedPars.restype = swsParameters

        gdl.removeUserParametersSetting.argtypes =[ctypes.c_void_p, ctypes.c_int]
        gdl.removeUserParametersSetting.restype = ctypes.c_bool
         
        gdl.facMaxCellxCol.argtypes= [ctypes.c_void_p]
        gdl.facMaxCellxCol.restype = ctypes.c_int
        gdl.facMaxCellyRow.argtypes= [ctypes.c_void_p]
        gdl.facMaxCellyRow.restype = ctypes.c_int
        gdl.WSIDsAll.argtypes= [ctypes.c_void_p]
        gdl.WSIDsAll.restype = ctypes.Array
        gdl.WScount.argtypes= [ctypes.c_void_p]
        gdl.WScount.restype = ctypes.c_int
        gdl.mostDownStreamWSIDs.argtypes= [ctypes.c_void_p]
        gdl.mostDownStreamWSIDs.restype = ctypes.Array
        gdl.cellCountInWatershed.argtypes= [ctypes.c_void_p]
        gdl.cellCountInWatershed.restype = ctypes.c_int
        gdl.cellSize.argtypes= [ctypes.c_void_p]
        gdl.cellSize.restype = ctypes.c_int

        bfpn_gmp = fpn_gmp.encode('utf-8')
        self.obj = gdl.grmWSinfo_new_gmpFile(bfpn_gmp)
        self.facMaxCellxCol = gdl.facMaxCellxCol(self.obj)
        self.facMaxCellyRow = gdl.facMaxCellyRow(self.obj)
        self.WSIDsAll = gdl.WSIDsAll(self.obj)
        self.WScount = gdl.WScount(self.obj)
        self.mostDownStreamWSIDs = gdl.mostDownStreamWSIDs(self.obj) 
        self.cellCountInWatershed=gdl.cellCountInWatershed(self.obj)
        self.cellSize=gdl.cellSize(self.obj)

    def isInWatershedArea(self, colXAryidx, rowYAryidx):
        return gdl.isInWatershedArea(self.obj, colXAryidx, rowYAryidx)

    def upStreamWSIDs(self, currentWSID):
        return gdl.upStreamWSIDs(self.obj, currentWSID)

    def upStreamWSCount(self, currentWSID):
        return gdl.upStreamWSCount(self.obj, currentWSID)

    def downStreamWSIDs(self, currentWSID):
        return gdl.downStreamWSIDs(self.obj, currentWSID)

    def downStreamWSCount(self, currentWSID):
        return gdl.downStreamWSCount(self.obj, currentWSID)

    def watershedID(self,  colXAryidx,  rowYAryidx): # 배열 인덱스 사용
        return gdl.watershedID(self.obj,  colXAryidx,  rowYAryidx)

    def flowDirection(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.flowDirection(self.obj, colXAryidx, rowYAryidx)

    def flowAccumulation(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.flowAccumulation(self.obj, colXAryidx, rowYAryidx)

    def slope(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.slope(self.obj, colXAryidx, rowYAryidx)

    def streamValue(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.streamValue(self.obj, colXAryidx, rowYAryidx)

    def cellFlowType(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.cellFlowTypeOfaCell(self.obj, colXAryidx, rowYAryidx)

    def landCoverValue(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.landCoverValue(self.obj, colXAryidx, rowYAryidx)

    def soilTextureValue(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.soilTextureValue(self.obj, colXAryidx, rowYAryidx)

    def soilDepthValue(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.soilDepthValue(self.obj, colXAryidx, rowYAryidx)

    def allCellsInUpstreamArea(self, colXAryidx, rowYAryidx): #    Select all cells in upstream area of a input cell position. Return string list of cell positions - "column, row".`
        return gdl.allCellsInUpstreamArea(self.obj, colXAryidx, rowYAryidx)

	# If this class was instanced by using gmp file --"grmWS(self, string gmpFPN)".		
    def setOneSWSParsAndUpdateAllSWSUsingNetwork(self, wsid, iniSat,
		minSlopeLandSurface, unSKType, coefUnsK,
		minSlopeChannel, minChannelBaseWidth, roughnessChannel,
		dryStreamOrder, ccLCRoughness,
		ccSoilDepth, ccPorosity, ccWFSuctionHead,
		ccSoilHydraulicCond, iniFlow = 0):
        return gdl.setOneSWSParsAndUpdateAllSWSUsingNetwork(self.obj, wsid, iniSat,
		minSlopeLandSurface, unSKType, coefUnsK,
		minSlopeChannel, minChannelBaseWidth, roughnessChannel,
		dryStreamOrder, ccLCRoughness,
		ccSoilDepth, ccPorosity, ccWFSuctionHead,
		ccSoilHydraulicCond, iniFlow = 0)

    def updateAllSubWatershedParametersUsingNetwork(self):
        return gdl.updateAllSubWatershedParametersUsingNetwork(self.obj)

    def subwatershedPars(self, wsid):
        return gdl.subwatershedPars(self.obj, wsid)

    def removeUserParametersSetting(self, wsid):
        return gdl.removeUserParametersSetting(self.obj, wsid)
#  class grmWSinfo(object) end =========== 


wsi=grmWSinfo(fpn_gmp)

print(wsi.cellSize)



# #sample codes to use c++ dll class
#class cgrmWS(object): 
#    def __init__(self, a, b): 
#        gdl.testClass_new.argtypes = [ctypes.c_int, ctypes.c_int]
#        gdl.testClass_new.restype = ctypes.c_void_p

#        gdl.testClass_plus.argtypes = [ctypes.c_void_p]
#        gdl.testClass_plus.restype = ctypes.c_int

#        gdl.testClass_multi.argtypes = [ctypes.c_void_p]
#        gdl.testClass_multi.restype = ctypes.c_int

#        gdl.ain.argtypes = [ctypes.c_void_p]
#        gdl.ain.restype = ctypes.c_int

#        gdl.bin.argtypes = [ctypes.c_void_p]
#        gdl.bin.restype = ctypes.c_int
        
#        self.obj = gdl.testClass_new(a, b)
#        self.grmWS_a = gdl.ain(self.obj) # public property로 
#        self.grmWS_b =  gdl.bin(self.obj)# public property로 

#    def grmWS_plus(self): 
#        return gdl.testClass_plus(self.obj)
 
#    def grmWS_multi(self):         
#        return gdl.testClass_multi(self.obj)

#    def grmWS_af(self):#이건 method로
#        return gdl.ain(self.obj)

#    def grmWS_bf(self): #이건 method로
#        return gdl.bin(self.obj) 

#f = cgrmWS(1,2)
#a= f.grmWS_a
#print(a)
#a= f.grmWS_af()
#print(a)
#b=f.grmWS_b
#print(b)
#b=f.grmWS_bf()
#print(b)
#value= f.grmWS_plus()
#print(value)
#value= f.grmWS_multi()
#print(value)




