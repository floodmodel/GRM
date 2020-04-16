# codes to implement the class in grm.dll
import sys, platform
import ctypes, ctypes.util
import enum
from ctypes import *

# here, grm dll file full path and name
gdl_path = ctypes.util.find_library("D:/Github/GRM/GRM_cpp/x64/Release/GRM.dll") 

if not gdl_path:
    print("Unable to find the specified library.")
    sys.exit() 
try:
     gdl = ctypes.CDLL(gdl_path)
except OSError:
    print("Unable to load the system C library")
    sys.exit()

class unSaturatedKType(enum.Enum): #grm 코드에 있는 순서와 맞춘다.
	Constant=0
	Linear=1
	Exponential=2
	usKNone=3

class swsParameters(Structure):  #grm 코드에 있는 내용과 맞춘다.
	_fields_ = [("wsid", ctypes.c_int),
        ("iniSaturation", ctypes.c_double),
        ("minSlopeOF", ctypes.c_double),
        ("unSatKType", ctypes.c_int),  # unSaturatedKType,
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


#  class grmWSinfo(object) start =========== 
class grmWSinfo(object): 
    #def __init__(self, fpn_gmp): 
    def __init__(self, fpnGMP_OR_fdirType, 
                fpnDomain="", 
                fpnSlope="", 
                fpnFdir="",  
                fpnFac="", 
                fpnStream = "", 
                fpnLandCover = "",   
                fpnSoilTexture = "",  
                fpnSoilDepth = "",	 
                fpnIniSoilSaturationRatio = "", 
                pfnIniChannelFlow = "", 
                fpnChannelWidth = ""):
        #fdirType 혹은 gmpfpn, fpnDomain, fpnSlope, fpnFdir,  fpnFac, 
        #fpnStream = "", fpnLandCover = "",   fpnSoilTexture = "",  fpnSoilDepth = "",	 fpnIniSoilSaturationRatio = "", pfnIniChannelFlow = "", fpnChannelWidth = ""
        gdl.grmWSinfo_new_inputFiles.argtypes =[ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p,
                ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p, 
                ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p,
                ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p] 
        	# grmWSinfo* grmWSinfo_new_inputFiles(string fdirType, string fpnDomain, string fpnSlope, 
               #     string fpnFdir, string fpnFac, string fpnStream = "", 
               #     string fpnLandCover = "",  string fpnSoilTexture = "", string fpnSoilDepth = "",
		       #     string fpnIniSoilSaturationRatio = "", string pfnIniChannelFlow = "", string fpnChannelWidth = "")
        gdl.grmWSinfo_new_inputFiles.restype = ctypes.c_void_p

        gdl.grmWSinfo_new_gmpFile.argtypes =[ctypes.c_char_p] # string
        gdl.grmWSinfo_new_gmpFile.restype = ctypes.c_void_p

        gdl.isInWatershedArea.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.isInWatershedArea.restype = ctypes.c_bool

        gdl.upStreamWSIDs.argtypes = [ctypes.c_void_p, ctypes.c_int]
        gdl.upStreamWSIDs.restype =   ctypes.POINTER(ctypes.c_int) #int*

        gdl.upStreamWSCount.argtypes = [ctypes.c_void_p, ctypes.c_int]
        gdl.upStreamWSCount.restype = ctypes.c_int

        gdl.downStreamWSIDs.argtypes = [ctypes.c_void_p, ctypes.c_int]
        gdl.downStreamWSIDs.restype = ctypes.POINTER(ctypes.c_int) #int*

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

        gdl.cellFlowTypeACell.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.cellFlowTypeACell.restype = ctypes.c_char_p #string

        gdl.landCoverValue.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.landCoverValue.restype = ctypes.c_int

        gdl.soilTextureValue.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.soilTextureValue.restype = ctypes.c_int

        gdl.soilDepthValue.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.soilDepthValue.restype = ctypes.c_int

        gdl.cellCountInUpstreamArea.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.cellCountInUpstreamArea.restype =ctypes.c_int

        gdl.allCellsInUpstreamArea.argtypes =[ctypes.c_void_p, ctypes.c_int, ctypes.c_int]
        gdl.allCellsInUpstreamArea.restype = ctypes.POINTER(ctypes.c_char_p)
        ctypes.POINTER
        gdl.setOneSWSParsAndUpdateAllSWSUsingNetwork.argtypes = [ctypes.c_void_p, 
            ctypes.c_int, ctypes.c_double,
            ctypes.c_double,  ctypes.c_int, ctypes.c_double, # unSaturatedKType :  ctypes.c_int
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

        gdl.updateAllSubWatershedParametersUsingNetwork.argtypes =[ctypes.c_void_p]
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
        gdl.WSIDsAll.restype = ctypes.POINTER(ctypes.c_int) # int *

        gdl.WScount.argtypes= [ctypes.c_void_p]
        gdl.WScount.restype = ctypes.c_int

        gdl.mostDownStreamWSIDs.argtypes= [ctypes.c_void_p]
        gdl.mostDownStreamWSIDs.restype =  ctypes.POINTER(ctypes.c_int)# int *

        gdl.mostDownStreamWSCount.argtypes = [ctypes.c_void_p]
        gdl.mostDownStreamWSCount.restype = ctypes.c_int

        gdl.cellCountInWatershed.argtypes= [ctypes.c_void_p]
        gdl.cellCountInWatershed.restype = ctypes.c_int

        gdl.cellSize.argtypes= [ctypes.c_void_p]
        gdl.cellSize.restype = ctypes.c_double

        bfpnGMPfdirTypeOR = fpnGMP_OR_fdirType.encode('utf-8')
        if fpnDomain=="":
            self.obj = gdl.grmWSinfo_new_gmpFile(bfpnGMPfdirTypeOR)
        else :
            bfpnDomain =fpnDomain.encode('utf-8')
            bfpnSlope =fpnSlope.encode('utf-8')
            bfpnFdir = fpnFdir.encode('utf-8')
            bfpnFac = fpnFac.encode('utf-8')
            bfpnStream=fpnStream.encode('utf-8')
            bfpnLandCover = fpnLandCover.encode('utf-8')
            bfpnSoilTexture = fpnSoilTexture.encode('utf-8')
            bfpnSoilDepth = fpnSoilDepth.encode('utf-8')
            bfpnIniSoilSaturationRatio = fpnIniSoilSaturationRatio.encode('utf-8')
            bpfnIniChannelFlow = pfnIniChannelFlow.encode('utf-8')
            bfpnChannelWidth = fpnChannelWidth.encode('utf-8')
            self.obj = gdl.grmWSinfo_new_inputFiles(bfpnGMPfdirTypeOR, bfpnDomain, bfpnSlope, 
                    bfpnFdir, bfpnFac, bfpnStream, 
                    bfpnLandCover, bfpnSoilTexture, bfpnSoilDepth, 
                    bfpnIniSoilSaturationRatio, bpfnIniChannelFlow, bfpnChannelWidth)

        self.facMaxCellxCol = gdl.facMaxCellxCol(self.obj)
        self.facMaxCellyRow = gdl.facMaxCellyRow(self.obj)
        self.WSIDsAll = gdl.WSIDsAll(self.obj)
        self.WScount = gdl.WScount(self.obj)
        self.mostDownStreamWSIDs = gdl.mostDownStreamWSIDs(self.obj) 
        self.mostDownStreamWSCount = gdl.mostDownStreamWSCount(self.obj) 
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

    def cellFlowTypeACell(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.cellFlowTypeACell(self.obj, colXAryidx, rowYAryidx)

    def landCoverValue(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.landCoverValue(self.obj, colXAryidx, rowYAryidx)

    def soilTextureValue(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.soilTextureValue(self.obj, colXAryidx, rowYAryidx)

    def soilDepthValue(self, colXAryidx, rowYAryidx):# 배열 인덱스 사용
        return gdl.soilDepthValue(self.obj, colXAryidx, rowYAryidx)

    def allCellsInUpstreamArea(self, colXAryidx, rowYAryidx): #    Select all cells in upstream area of a input cell position. Return string list of cell positions - "column, row".`
        return gdl.allCellsInUpstreamArea(self.obj, colXAryidx, rowYAryidx)

    def cellCountInUpstreamArea(self, colXAryidx, rowYAryidx): #    Select all cells in upstream area of a input cell position. Return string list of cell positions - "column, row".`
        return gdl.cellCountInUpstreamArea(self.obj, colXAryidx, rowYAryidx)

	# If this class was instanced by using gmp file --"grmWS(self, string gmpFPN)".		
    def setOneSWSParsAndUpdateAllSWSUsingNetwork(self, wsid, iniSat,
		minSlopeLandSurface, unSKType, coefUnsK,
		minSlopeChannel, minChannelBaseWidth, roughnessChannel,
		dryStreamOrder, ccLCRoughness,
		ccSoilDepth, ccPorosity, ccWFSuctionHead,
		ccSoilHydraulicCond, iniFlow=0.0):
        return gdl.setOneSWSParsAndUpdateAllSWSUsingNetwork(self.obj, wsid, iniSat,
		minSlopeLandSurface, unSKType, coefUnsK,
		minSlopeChannel, minChannelBaseWidth, roughnessChannel,
		dryStreamOrder, ccLCRoughness,
		ccSoilDepth, ccPorosity, ccWFSuctionHead,
		ccSoilHydraulicCond, iniFlow)

    def updateAllSubWatershedParametersUsingNetwork(self):
        return gdl.updateAllSubWatershedParametersUsingNetwork(self.obj)

    def subwatershedPars(self, wsid):
        return gdl.subwatershedPars(self.obj, wsid)

    def removeUserParametersSetting(self, wsid):
        return gdl.removeUserParametersSetting(self.obj, wsid)
#  class grmWSinfo(object) end =========== 

# sample code to use grmWSinfo class

### gmp 파일로 grmWSinfo class 를 인스턴싱 할 경우 ==============================
### gmp 파일에서 ProjectSetting 테이블까지만 채워져 있어도 사용할 수 있다. 
#fpn_gmp = "C:\GRM\SampleGHG\GHG500.gmp"
#wsi=grmWSinfo(fpn_gmp) # gmp file path and name / [ctypes.c_char_p] -> ctypes.c_void_p
##================================================================

# gmp 입력 파일로 grmWSinfo class 를 인스턴싱 할 경우 ============================
fdType = "StartsFromE_TauDEM"
fpn_domain = "C:\GRM\SampleGHG\watershed/GHG_Watershed.asc"
fpn_slope = "C:\GRM\SampleGHG\watershed/GHG_Slope_ST.asc"
fpn_fd = "C:\GRM\SampleGHG\watershed/GHG_FDir.asc"
fpn_fa = "C:\GRM\SampleGHG\watershed/GHG_FAc.asc"
fpn_stream = "C:\GRM\SampleGHG\watershed/GHG_Stream.asc"
fpn_lc="C:\GRM\SampleGHG\watershed/GHG_lc.asc"
fpn_st="C:\GRM\SampleGHG\watershed/GHG_SoilTexture.asc"
fpn_sd="C:\GRM\SampleGHG\watershed/GHG_SoilDepth.asc"
wsi=grmWSinfo(fdType,  
    fpn_domain,
    fpn_slope,
    fpn_fd,
    fpn_fa,
    fpn_stream, 
    fpn_lc,
    fpn_st,
    fpn_sd, "", "", "")
#================================================================

# 여기서는 정보를 얻고자 하는 셀위치 혹은 유역 번호를 지정 =========================
xCol = 21 #9  # 정보를 얻고자 하는 셀 위치
yRow = 49 #93 # 정보를 얻고자 하는 셀 위치
wsid = 1   # 정보를 얻고자 하는 유역 번호

a = wsi.isInWatershedArea(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_bool
print("isInWatershedArea :", a)

a = wsi.upStreamWSCount(wsid) # current ws id / [ctypes.c_int] -> ctypes.c_int
print("upStreamWSCount :", a)

a = wsi.upStreamWSIDs(wsid) # current ws id / [ctypes.c_int] -> ctypes.POINTER(ctypes.c_int)
for i in range(wsi.upStreamWSCount(wsid)):
    print("  upStreamWSIDs :", a[i]) # if -1, there is no upstream watershed.

a = wsi.downStreamWSCount(wsid) # current ws id / [ctypes.c_int] -> ctypes.c_int
print("downStreamWSCount :", a)

a = wsi.downStreamWSIDs(wsid) # current ws id / [ctypes.c_int] -> types.POINTER(ctypes.c_int) 
for i in range(wsi.downStreamWSCount(wsid)):
    print("  downStreamWSIDs :", a[i]) # if -1, there is no downstream watershed.

a = wsi.watershedID(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("watershedID :", a)   # if -1, the cell is out of the simulation domain bound

a = wsi.flowDirection(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_char_p
print("flowDirection :", a.decode('utf-8')) #if 'OFWB', the cell is out of the simulation domain bound
#enum class flowDirection8 of GRM.dll : E8 = 1, SE8 = 2, S8 = 3, SW8 = 4, W8 = 5, NW8 = 6, N8 = 7, NE8 = 8, None8 = 0

a = wsi.flowAccumulation(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("flowAccumulation :", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.slope(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_double
print("slope :", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.streamValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("streamValue :", a) # if -1, the cell is out of the simulation domain bound

a = wsi.cellFlowTypeACell(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_char_p
print("cellFlowTypeACell :", a.decode('utf-8')) #if 'OFWB', the cell is out of the simulation domain bound

a = wsi.landCoverValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("landCoverValue :", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.soilTextureValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("soilTextureValue :", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.soilDepthValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("soilDepthValue :", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.cellCountInUpstreamArea(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("cellCountInUpstreamArea :", a)  

# 여기서 상류셀 개수가 많아지면, 애러 발생할때 있다.. 해결 방법은? 박책임님 검토 필요함. 2020.04.16. 최
b= wsi.allCellsInUpstreamArea(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.POINTER(ctypes.c_char_p)
for i in range(wsi.cellCountInUpstreamArea(xCol, yRow)):
    print("  allCellsInUpstreamArea :", b[i].decode('utf-8')) # if -1, there is no downstream watershed.

swp=swsParameters()

#이건 gmp 파일에서 읽은 매개변수를 적용하는 경우 ================================

swp = wsi.subwatershedPars(wsid) # current ws id / [ctypes.c_int] -> swsParameters
print("subwatershedPars. wsid :", swp.wsid)
print("subwatershedPars. iniSaturation :", swp.iniSaturation)
print("subwatershedPars. minSlopeOF :", swp.minSlopeOF)
print("subwatershedPars. unSatKType :", swp.unSatKType, "  Name :", unSaturatedKType(swp.unSatKType).name)  # 	Constant=0, Linear=1, Exponential=2, usKNone=3
print("subwatershedPars. coefUnsaturatedK :", swp.coefUnsaturatedK)
print("subwatershedPars. minSlopeChBed :", swp.minSlopeChBed)
print("subwatershedPars. minChBaseWidth :", swp.minChBaseWidth)
print("subwatershedPars. chRoughness :", swp.chRoughness)
print("subwatershedPars. dryStreamOrder :", swp.dryStreamOrder)
print("subwatershedPars. iniFlow :", swp.iniFlow)
print("subwatershedPars. ccLCRoughness :", swp.ccLCRoughness)
print("subwatershedPars. ccPorosity :", swp.ccPorosity)
print("subwatershedPars. ccWFSuctionHead :", swp.ccWFSuctionHead)
print("subwatershedPars. ccHydraulicK :", swp.ccHydraulicK)
print("subwatershedPars. ccSoilDepth :", swp.ccSoilDepth)
print("subwatershedPars. userSet :", swp.userSet)    


#이건 GUI에서 받은 매개변수를 적용하는 경우 ================================


a = wsi.setOneSWSParsAndUpdateAllSWSUsingNetwork(swp.wsid,  swp.iniSaturation#  watershed parameters -> ctypes.c_bool
                           , swp.minSlopeOF, swp.unSatKType, swp.coefUnsaturatedK
                           , swp.minSlopeChBed, swp.minChBaseWidth, swp.chRoughness
                           , swp.dryStreamOrder, swp.ccLCRoughness
                           , swp.ccSoilDepth, swp.ccPorosity, swp.ccWFSuctionHead
                           , swp.ccHydraulicK, swp.iniFlow) 
			#int wsid, double iniSat,
			#double minSlopeLandSurface, string unSKType, double coefUnsK,
			#double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
			#int dryStreamOrder, double ccLCRoughness,
			#double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
			#double ccSoilHydraulicCond, double iniFlow = 0)
print("setOneSWSParsAndUpdateAllSWSUsingNetwork :", a)

a = wsi.updateAllSubWatershedParametersUsingNetwork() # A funtion with no parameter. -> void
print("updateAllSubWatershedParametersUsingNetwork :", a)

a = wsi.removeUserParametersSetting(wsid) # current ws id / [ctypes.c_int] -> ctypes.c_bool
print("removeUserParametersSetting :", a)         

a = wsi.facMaxCellxCol # property -> ctypes.c_int
print("facMaxCellxCol :", a)

a = wsi.facMaxCellyRow # property -> ctypes.c_int
print("facMaxCellyRow :", a)


a = wsi.WScount #  property -> ctypes.c_int
print("WScount :", a)

a = wsi.WSIDsAll #  property ->  ctypes.POINTER(ctypes.c_int)
for i in range(wsi.WScount):
    print("  WSIDsAll :", a[i]) # if -1, there is no downstream watershed.

a = wsi.mostDownStreamWSCount #  property ->  ctypes.POINTER(ctypes.c_int)
print("mostDownStreamWSCount :", a)

a = wsi.mostDownStreamWSIDs #  property ->  ctypes.POINTER(ctypes.c_int)
for i in range(wsi.mostDownStreamWSCount):
    print("  mostDownStreamWSIDs :", a[i]) # if -1, there is no downstream watershed.

a = wsi.cellCountInWatershed #  property -> ctypes.c_int
print("cellCountInWatershed :", a)

a = wsi.cellSize #  property -> ctypes.c_double
print("cellSize :", a)




