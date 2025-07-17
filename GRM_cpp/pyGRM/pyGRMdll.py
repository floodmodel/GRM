# codes to implement the class in grm.dll
import sys, platform
import ctypes, ctypes.util
import enum
from ctypes import *

# here, grm dll file full path and name
gdl_path = ctypes.util.find_library("D:/Github/GRMv2023/GRMv2023_cpp/x64/Release/GRM.dll") 
#gdl_path = ctypes.util.find_library("D:/Github/GRMv2023/GRMv2023_cpp/x64/Debug/GRM.dll") # this line is for combined debugging with GRM c++ code.


if not gdl_path:
    print("Unable to find the specified library.")
    sys.exit() 
try:
     gdl = ctypes.CDLL(gdl_path)
except OSError:
    print("Unable to load the system C library")
    sys.exit()

class unSaturatedKType(enum.Enum): #grm 코드에 있는 순서와 맞춘다.
	Constant=1
	Linear=2
	Exponential=3
	usKNone=0

class PETmethod(enum.Enum): #grm 코드에 있는 순서와 맞춘다.
	BC = 1 # BlaneyCriddle
	FPM = 2  # FAOPenmanMonteith
	HMN = 3  # Hamon
	HRGV = 4  #  Hargreaves
	PT = 5  # PriestleyTaylor	
	JH = 6  # JensenHaise
	TRC = 7 #Turc
	Constant = 8
	UserET = 9 # PET 혹은 AET
	petNone = 0

class SnowMeltMethod(enum.Enum): #grm 코드에 있는 순서와 맞춘다.
	Anderson = 1
	Constant = 8
	UserData = 9
	smNone = 0

class InterceptionMethod(enum.Enum): #grm 코드에 있는 순서와 맞춘다.
	LAIRatio = 1
	Constant = 8
	UserData = 9
	icNone = 0

class swsParameters(Structure):  #grm 코드에 있는 구조체와 내용 맞춘다. 순서도 맞게 해야 한다.
	_fields_ = [("wsid", ctypes.c_int),
        ("iniSaturation", ctypes.c_double),
        ("iniLossPRCP_mm", ctypes.c_double),
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
        ("interceptMethod", ctypes.c_int),
        ("potentialETMethod", ctypes.c_int),
        ("etCoeff", ctypes.c_double),
        ("snowMeltMethod", ctypes.c_int),
        ("tempSnowRain", ctypes.c_double),
        ("smeltingTemp", ctypes.c_double),
        ("snowCovRatio", ctypes.c_double),
        ("smeltCoef", ctypes.c_double),
        ("userSet", ctypes.c_int)]

class defaultWSPars:
    IniSaturation = 0.9
    IniLossPRCP_mm=0.0
    MinSlopeOF = 0.0001
    MinSlopeChBed = 0.0001
    IniFlow = 0.0
    ChRoughness = 0.045
    DryStreamOrder = 0
    CalCoefLCRoughness = 1.0
    CalCoefSoilDepth = 1.0
    CalCoefPorosity = 1.0
    CalCoefWFSuctionHead = 1.0
    CalCoefHydraulicK = 1.0
    UnsturatedType = 2   #Linear
    CoefUnsatruatedk = 0.2
    InterceptMethod = 0  #None
    PotentialETMethod = 0  #None
    ETCoeff = 0.6
    SnowMeltMethod = 0    #None
    TempSnowRain = 0.0
    SMeltingTemp = 4.0
    SnowCovRatio = 0.7
    SMeltCoef = 1.0
        # =========================

#  class grmWSinfo(object) start =========== 
class grmWSinfo(object): 
    # fpnGMP_OR_fdirType 여기에 gmp 파일을 입력하든지, flowDirectionType을 입력하든지 선택
    #      flowDirectionType 을 입력할 경우에는 다른 파일들도 argument로 입력해야 한다. 
    def __init__(self, fpnGMP_OR_fdirType, 
                fpnDEM="", # gmp 파일로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수
                fpnDomain="", # gmp 파일로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수
                fpnSlope="", # gmp 파일로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수
                fpnFdir="",  # gmp 파일로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수
                fpnFac="", # gmp 파일로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수
                fpnStream = "", # gmp 로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수아님
                fpnLandCover = "",   # gmp 로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수아님
                fpnSoilTexture = "",  # gmp 로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수아님
                fpnSoilDepth = "",	 # gmp 로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수아님
                fpnIniSoilSaturationRatio = "", # gmp 로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수아님
                pfnIniChannelFlow = "", # gmp 로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수아님
                fpnChannelWidth = ""): # gmp 로 인스턴싱 할경우 입력하면 안됨, 입력파일들로 인스턴싱 할경우 필수아님
        gdl.grmWSinfo_new_inputFiles.argtypes =[ctypes.c_char_p, ctypes.c_char_p, 
                ctypes.c_char_p, ctypes.c_char_p,
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

        gdl.setOneSWSParsAndUpdateAllSWSUsingNetwork.argtypes = [ctypes.c_void_p, 
            ctypes.c_int, ctypes.c_double, ctypes.c_double,
            ctypes.c_double,  ctypes.c_int, ctypes.c_double, 
			ctypes.c_double, ctypes.c_double, ctypes.c_double,
			ctypes.c_int, ctypes.c_double,
			ctypes.c_double, ctypes.c_double, ctypes.c_double,
			ctypes.c_double,
            ctypes.c_int,
            ctypes.c_int, ctypes.c_double,
            ctypes.c_int, ctypes.c_double, ctypes.c_double,
            ctypes.c_double, ctypes.c_double,
            ctypes.c_double]
     #     grmWSinfo* f
     #      // grm.h 에서 명시된 인터페이스는 아래와 같다. 
     #    bool setOneSWSParsAndUpdateAllSWSUsingNetwork(int wsid, double iniSat, double iniLossPRCP_mm,
	 #    double minSlopeLandSurface, unSaturatedKType unSKType, double coefUnsK,
     #    double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
     #    int dryStreamOrder, double ccLCRoughness,
     #    double ccPorosity, double ccWFSuctionHead, double ccSoilHydraulicCond,
     #    double ccSoilDepth,
     #    InterceptionMethod interceptMethod,
     #    PETmethod potentialETMethod, double etCoeff,
     #    SnowMeltMethod snowMeltMethod, double tempSnowRain, double smeltingTemp,
     #    double snowCovRatio, double smeltCoef,
     #    double iniFlow = 0);

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

        gdl.FDtype.argtypes= [ctypes.c_void_p]
        gdl.FDtype.restype = ctypes.c_char_p  # string

        bfpnGmpORfdirType = fpnGMP_OR_fdirType.encode('utf-8')
        if fpnDomain=="":
            self.obj = gdl.grmWSinfo_new_gmpFile(bfpnGmpORfdirType)
        else :
            bfpnDEM =fpnDEM.encode('utf-8')
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
            self.obj = gdl.grmWSinfo_new_inputFiles(bfpnGmpORfdirType, bfpnDEM, 
                    bfpnDomain, bfpnSlope, 
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
        self.fdType_fromGMP = gdl.FDtype(self.obj)

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

    def setOneSWSParsAndUpdateAllSWSUsingNetwork(self, wsid, iniSat, iniLossPRCP_mm,
		minSlopeLandSurface, unSKType, coefUnsK,
		minSlopeChannel, minChannelBaseWidth, roughnessChannel,
		dryStreamOrder, ccLCRoughness,
		ccPorosity, ccWFSuctionHead, ccSoilHydraulicCond, 
        ccSoilDepth, # 이 변수 위치 변경됨. 여기로 이동. 2023.03.02
        #==========추가됨. 2023.03.02===
        interceptMethod,
		potentialETMethod, etCoeff,
		snowMeltMethod, tempSnowRain, smeltingTemp, snowCovRatio, smeltCoef,
        #=========================
        iniFlow=0.0):
        return gdl.setOneSWSParsAndUpdateAllSWSUsingNetwork(self.obj, wsid, iniSat,
		minSlopeLandSurface, unSKType, coefUnsK,
		minSlopeChannel, minChannelBaseWidth, roughnessChannel,
		dryStreamOrder, ccLCRoughness,
		ccPorosity, ccWFSuctionHead,	ccSoilHydraulicCond, 
        ccSoilDepth, # 이 변수 위치 변경됨. 여기로 이동. 2023.03.02
        #==========추가됨. 2023.03.02===
        interceptMethod,
		potentialETMethod, etCoeff,
		snowMeltMethod, tempSnowRain, smeltingTemp, snowCovRatio, smeltCoef,
        #=========================
        iniFlow)

    def updateAllSubWatershedParametersUsingNetwork(self):
        return gdl.updateAllSubWatershedParametersUsingNetwork(self.obj)

    def subwatershedPars(self, wsid):
        return gdl.subwatershedPars(self.obj, wsid)

    def removeUserParametersSetting(self, wsid):
        return gdl.removeUserParametersSetting(self.obj, wsid)
#  class grmWSinfo(object) end =========== 

# sample code to use grmWSinfo class

###================================================================
### gmp 파일로 grmWSinfo class 를 인스턴싱 할 경우 ==============================
### gmp 파일에서 ProjectSetting 테이블까지만 채워져 있어도 사용할 수 있다. 
#fpn_gmp = "D:\GRM_ex\Han\20220208_aju\Hanriver_GRM_calib_Run_pre.gmp" #"C:\GRM\SampleGHG\GHG500.gmp"
fpn_gmp  = "D:/Github/zTestSet_GRM2023_SampleJeju/20231123_01_JJD_QGRMtest_v2023.gmp"
#fpn_gmp  = "D:\Github\zTestSetPyGRM\SampleWC\SampleWiCheon.gmp"

wsi=grmWSinfo(fpn_gmp) # gmp file path and name / [ctypes.c_char_p] -> ctypes.c_void_p
###================================================================
###================================================================


##================================================================
## gmp 에 저장된 GRM 입력 파일로 grmWSinfo class 를 인스턴싱 할 경우 ============================
#fdType = "StartsFromE_TauDEM"
##fpn_domain = "C:\GRM\SampleGHG\watershed/GHG_Watershed.asc"
##fpn_slope = "C:\GRM\SampleGHG\watershed/GHG_Slope_ST.asc"
##fpn_fd = "C:\GRM\SampleGHG\watershed/GHG_FDir.asc"
##fpn_fa = "C:\GRM\SampleGHG\watershed/GHG_FAc.asc"
##fpn_stream = "C:\GRM\SampleGHG\watershed/GHG_Stream.asc"
##fpn_lc="C:\GRM\SampleGHG\watershed/GHG_lc.asc"
##fpn_st="C:\GRM\SampleGHG\watershed/GHG_SoilTexture.asc"
##fpn_sd="C:\GRM\SampleGHG\watershed/GHG_SoilDepth.asc"

##fpn_domain = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiWatershed.asc";
##fpn_slope = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/Wi_Slope_ST.ASC";
##fpn_fd = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiFDir.ASC";
##fpn_fa = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiFAc.asc";
##fpn_stream = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/WiStream6.asc";
##fpn_lc = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/wilc200.asc";
##fpn_st = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/wistext200.asc";
##fpn_sd = "D:/Github/zTestSet_GRM_SampleWC_cpp/Data/wisdepth200.asc";

#fpn_domain = 'D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_Watershed.asc';
#fpn_slope = "D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_Slope_ST.asc";
#fpn_fd = "D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_FDir.asc";
#fpn_fa = "D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_FAc.asc";
#fpn_stream = "D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_Stream.asc";
#fpn_lc = "D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_lc.asc";
#fpn_st = "D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_SoilTexture.asc";
#fpn_sd = "D:\\GRM_ex\\SpeedTest\\GHG\\watershed\\GHG_SoilDepth.asc";
#fpn_ssi=''
#fpn_iniCF=''
#fpn_CW=''


#wsi=grmWSinfo(fdType,  
#    fpn_domain,
#    fpn_slope,
#    fpn_fd,
#    fpn_fa,
#    fpn_stream, 
#    fpn_lc,
#    fpn_st,
#    fpn_sd, fpn_ssi, fpn_iniCF, fpn_CW)
##================================================================
##================================================================

# 여기서는 정보를 얻고자 하는 셀위치 혹은 유역 번호를 지정 =========================
xCol = 222  # 정보를 얻고자 하는 셀 위치
yRow = 153 # 정보를 얻고자 하는 셀 위치
wsid = 100  # 정보를 얻고자 하는 유역 번호
#=================================================================

print(" ")
print("From here, pyGRM.dll writes the values.")

a = wsi.isInWatershedArea(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_bool
print("Is the cell [", xCol, ", ", yRow, "] in the watershed area ? :", a)

a = wsi.upStreamWSCount(wsid) # current ws id / [ctypes.c_int] -> ctypes.c_int
print("The upstream WS count of WSID = ", wsid, " : ", a)
if a > 0 :
    a = wsi.upStreamWSIDs(wsid) # current ws id / [ctypes.c_int] -> ctypes.POINTER(ctypes.c_int)
    for i in range(wsi.upStreamWSCount(wsid)): # Because 'a' is pointer, it have to be iterated by using wsi.upStreamWSCount(wsid)
        print("  upStreamWSIDs :", a[i]) # if -1, there is no upstream watershed.

a = wsi.downStreamWSCount(wsid) # current ws id / [ctypes.c_int] -> ctypes.c_int
print("The downstream WS count of WSID = ", wsid, " : ", a)
if a > 0 :
    a = wsi.downStreamWSIDs(wsid) # current ws id / [ctypes.c_int] -> types.POINTER(ctypes.c_int) 
    for i in range(wsi.downStreamWSCount(wsid)):  # Because 'a' is pointer, it have to be iterated by using wsi.downStreamWSCount(wsid)
        print("  downStreamWSIDs : ", a[i]) # if -1, there is no downstream watershed.

a = wsi.watershedID(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("WatershedID of current cell [", xCol, ", ", yRow, "] : ", a)   # if -1, the cell is out of the simulation domain bound

a = wsi.flowDirection(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_char_p

## GRM.dll을 release 모드로 빌드 된 것을 참조하면 애러 없다. debug로 빌드된 dll 참조하면 애러 발생한다.
print("FlowDirection of current cell [", xCol, ", ", yRow, "] : ", a.decode('utf-8')) #if 'OFWB', the cell is out of the simulation domain bound
#enum class flowDirection8 of GRM.dll : E8 = 1, NE8 = 2, N8 = 3, NW8 = 4, W8 = 5, SW8 = 6, S8 = 7, SE8 = 8, None8 = 0

a = wsi.flowAccumulation(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("FlowAccumulation of current cell [", xCol, ", ", yRow, "] : ", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.slope(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_double
print("The slope of current cell [", xCol, ", ", yRow, "] : ", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.streamValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("The stream value of current cell [", xCol, ", ", yRow, "] : ", a) # if -1, the cell is out of the simulation domain bound

## GRM.dll을 release 모드로 빌드 된 것을 참조하면 애러 없다. debug로 빌드된 dll 참조하면 애러 발생한다.
a = wsi.cellFlowTypeACell(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_char_p
print("The cellFlowTypeACell of current cell [", xCol, ", ", yRow, "] : ", a.decode('utf-8')) #if 'OFWB', the cell is out of the simulation domain bound

a = wsi.landCoverValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("landCoverValue of current cell [", xCol, ", ", yRow, "] : ", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.soilTextureValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("soilTextureValue of current cell [", xCol, ", ", yRow, "] : ", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.soilDepthValue(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("soilDepthValue of current cell [", xCol, ", ", yRow, "] : ", a)  # if -1, the cell is out of the simulation domain bound

a = wsi.cellCountInUpstreamArea(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.c_int
print("cellCountInUpstreamArea of current cell [", xCol, ", ", yRow, "] : ", a)  

a= wsi.allCellsInUpstreamArea(xCol, yRow) # cell position(x, y) / [ctypes.c_int, ctypes.c_int] -> ctypes.POINTER(ctypes.c_char_p)
for i in range(wsi.cellCountInUpstreamArea(xCol, yRow)):  # Because 'a' is pointer, it have to be iterated by using wsi.cellCountInUpstreamArea(xCol, yRow)
    print("  allCellsInUpstreamArea :", a[i].decode('utf-8')) # if -1, there is no upstream cell.

swp=swsParameters()

#이건 gmp 파일에서 매개변수를 받는 경우 
swp = wsi.subwatershedPars(wsid) # current ws id / [ctypes.c_int] -> swsParameters
print("subwatershedPars. wsid :", swp.wsid)
print("subwatershedPars. iniSaturation :", swp.iniSaturation)
print("subwatershedPars. iniLossPRCP_mm :", swp.iniLossPRCP_mm)
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
#==========추가됨. 2023.03.02===
print("subwatershedPars. interceptMethod :", swp.interceptMethod, "  Name :", InterceptionMethod(swp.interceptMethod).name)  
    # LAIRatio = 1, Constant = 8, UserData = 9, icNone = 10
print("subwatershedPars. potentialETMethod :", swp.potentialETMethod, "  Name :", PETmethod(swp.potentialETMethod).name) 
    # PenmanMonteith = 1, BlaneyCriddle = 2, Hamon = 3, PriestleyTaylor = 4, Hargreaves = 5, JensenHaise = 6, Turc = 7, Constant = 8, UserData = 9, petNone = 10
print("subwatershedPars. etCoeff :", swp.etCoeff)
print("subwatershedPars. snowMeltMethod :", swp.snowMeltMethod, "  Name :", SnowMeltMethod(swp.snowMeltMethod).name)  
    # Anderson = 1, Constant = 8, UserData = 9, smNone = 10
print("subwatershedPars. tempSnowRain :", swp.tempSnowRain)
print("subwatershedPars. smeltingTemp :", swp.smeltingTemp)
print("subwatershedPars. snowCovRatio :", swp.snowCovRatio)
print("subwatershedPars. smeltCoef :", swp.smeltCoef)
#=========================
print("subwatershedPars. userSet :", swp.userSet)    # 1 : true, 0: false

# ========================================================
# 한 유의 매개변수를 이용해서 전체 유역의 매개변수를 업데이트 하기
# setOneSWSParsAndUpdateAllSWSUsingNetwork() function just update memory. The gmp file is not revised.
# If you want to revise the gmp file, you need a writing process using updated parameters in memory.
# To get the updated paramters in memory for a subwatershed, you can use subwatershedPars() function.

#swp에 저장된 매개변수(gmp 파일로 인스턴싱 할경우 등)를 이용해서 전체 유역 매개변수 업데이트 하는 경우,  
#a = wsi.setOneSWSParsAndUpdateAllSWSUsingNetwork(swp.wsid,  swp.iniSaturation, swp.iniLossPRCP_mm   #  setOneSWSParsAndUpdateAllSWSUsingNetwork -> ctypes.c_bool
#                           , swp.minSlopeOF, swp.unSatKType, swp.coefUnsaturatedK
#                           , swp.minSlopeChBed, swp.minChBaseWidth, swp.chRoughness
#                           , swp.dryStreamOrder, swp.ccLCRoughness
#                           , swp.ccSoilDepth, swp.ccPorosity, swp.ccWFSuctionHead
#                           , swp.ccHydraulicK, swp.iniFlow) 
			#int wsid, double iniSat,
			#double minSlopeLandSurface, int unSKType, double coefUnsK,
			#double minSlopeChannel, double minChannelBaseWidth, double roughnessChannel,
			#int dryStreamOrder, double ccLCRoughness,
			#double ccSoilDepth, double ccPorosity, double ccWFSuctionHead,
			#double ccSoilHydraulicCond, double iniFlow = 0)

##GUI에서 받은 매개변수를 사용할 경우, GUI에서 받은 매개변수를 사용할 경우에는 swp의 매개변수 항목을 직접 입력해 줘야 한다. 
#a = wsi.setOneSWSParsAndUpdateAllSWSUsingNetwork(1,  0.5
#                           , 0.0001, 1, 0.2
#                           , 0.008, 30, 0.045
#                           , 0, 1
#                           , 1, 1,1
#                           , 1, 20.0) 

# ========================================================

# 업데이트 된 매개변수 조회하기
wsid = 100 # 1번은 샘플. setOneSWSParsAndUpdateAllSWSUsingNetwork()에서 업데이트 된 매개변수 조회할 유역 id 지정한다. 
swp = wsi.subwatershedPars(wsid) # current ws id / [ctypes.c_int] -> swsParameters
print("subwatershedPars. wsid :", swp.wsid)
print("subwatershedPars. iniSaturation :", swp.iniSaturation)
print("subwatershedPars. iniLossPRCP_mm :", swp.iniLossPRCP_mm)
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
#==========추가됨. 2023.03.02===
print("subwatershedPars. interceptMethod :", swp.interceptMethod, "  Name :", InterceptionMethod(swp.interceptMethod).name)  
    # LAIRatio = 1, Constant = 8, UserData = 9, icNone = 10
print("subwatershedPars. potentialETMethod :", swp.potentialETMethod, "  Name :", PETmethod(swp.potentialETMethod).name) 
    # PenmanMonteith = 1, BlaneyCriddle = 2, Hamon = 3, PriestleyTaylor = 4, Hargreaves = 5, JensenHaise = 6, Turc = 7, Constant = 8, UserData = 9, petNone = 10
print("subwatershedPars. etCoeff :", swp.etCoeff)
print("subwatershedPars. snowMeltMethod :", swp.snowMeltMethod)
print("subwatershedPars. snowMeltMethod :", swp.snowMeltMethod, "  Name :", SnowMeltMethod(swp.snowMeltMethod).name)  
    # Anderson = 1, Constant = 8, UserData = 9, smNone = 10
print("subwatershedPars. tempSnowRain :", swp.tempSnowRain)
print("subwatershedPars. smeltingTemp :", swp.smeltingTemp)
print("subwatershedPars. snowCovRatio :", swp.snowCovRatio)
print("subwatershedPars. smeltCoef :", swp.smeltCoef)
#=========================
print("subwatershedPars. userSet :", swp.userSet)   # 1 : true, 0: false

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
for i in range(wsi.WScount):   # Because 'a' is pointer, it have to be iterated by using wsi.WScount
    print("  WSIDsAll :", a[i]) # if -1, there is no downstream watershed.

a = wsi.mostDownStreamWSCount #  property ->  ctypes.POINTER(ctypes.c_int)
print("mostDownStreamWSCount :", a)

a = wsi.mostDownStreamWSIDs #  property ->  ctypes.POINTER(ctypes.c_int)
for i in range(wsi.mostDownStreamWSCount): # Because 'a' is pointer, it have to be iterated by using wsi.mostDownStreamWSCount 
    print("  mostDownStreamWSIDs :", a[i]) # if -1, there is no downstream watershed.

a = wsi.cellCountInWatershed #  property -> ctypes.c_int
print("cellCountInWatershed :", a)

a = wsi.cellSize #  property -> ctypes.c_double
print("cellSize :", a)




