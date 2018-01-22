Imports System.Threading

Public Class cPEST
    Public Event PESTEnded(ByVal sender As cPEST)

    Public Const FILE_NAME_PEST_CONTROL_FILE As String = "pcf.pst"
    Public Const FILE_NAME_PEST_INSTRUCTION_FILE As String = "pif.pst"
    Public Const FILE_NAME_PEST_TEMPLATE_FILE As String = "ptf.pst"
    Public Const FILE_NAME_PPEST_RUN_MANAGEMENT_FILE As String = "pcf.rmf"
    Public Const FILE_NAME_BATCH_CPOY_PPEST As String = "PPESTCopy.bat"
    Public Const FILE_NAME_BATCH_CPOY_PEST As String = "PPESTCopy.bat"
    Public Const FILE_NAME_BATCH_RUN_PEST As String = "PPESTR.bat"
    Public Const FILE_NAME_BATCH_RUN_PPEST As String = "PESTR.bat"
    Public Const FILE_NAME_BATCH_RUN_PSLAVE As String = "PSLAVER.bat"
    Public Const FILE_NAME_PPEST_EXE As String = "ppest.exe"
    Public Const FILE_NAME_PEST_EXE As String = "pest.exe"
    Public Const FILE_NAME_PSLAVE_EXE As String = "pslave.exe"
    ''' <summary>
    ''' GRM 출력 파일에서 계산 결과를 읽을 watchpoint column 순서  
    ''' 1부터 시작
    ''' </summary>
    ''' <remarks></remarks>
    Public mWPColumnIndex As Integer
    Public Enum PestParGroupName
        ISSR
        MSLS
        MSCB
        MCW
        CRC
        IDSO
        LCRC
        SP
        SWFSH
        SHC
        SD
    End Enum

    Public Structure ParameterChecked
        Dim ISSRisChecked As Boolean
        Dim MSLSisChecked As Boolean
        Dim MSCBisChecked As Boolean
        Dim MCWisChecked As Boolean
        Dim CRCisChecked As Boolean
        Dim IDSOisChecked As Boolean
        Dim LCRCisChecked As Boolean
        Dim SPisChecked As Boolean
        Dim SWFSHisChecked As Boolean
        Dim SHCisChecked As Boolean
        Dim SDisChecked As Boolean
    End Structure

    ''' <summary>
    ''' Control data : 1## /
    ''' Parameter groups : 2## /
    ''' Parameter data : 3## /
    ''' Observation groups : 4## /
    ''' Observation data : 5## /
    ''' Model command line : 6## /
    ''' Model input/output : 7## /
    ''' Prior information : 8## /
    ''' </summary>
    ''' <remarks></remarks>
    Public Structure PAR_PCF
#Region " Control data : 1##"
        ''' <summary>
        ''' If it takes the value “restart”, PEST will dump the contents of many of its data arrays to a binary file 
        ''' (named case.rst where case is the current case name) at the beginning of each optimisation iteration; 
        ''' this allows PEST to be restarted later if execution is prematurely terminated.
        ''' If the RSTFLE variable is set to “norestart”, PEST will not intermittently dump 
        ''' its array or Jacobian data; hence a later re-commencement of execution after premature termination is impossible.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n100_RSTFLE As String

        ''' <summary>
        ''' If set to “estimation” PEST will run in parameter estimation mode (its traditional mode of operation); 
        ''' if set to “prediction” PEST will run in predictive analysis mode; if set to “regularisation” PEST will run in regularisation mode.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n101_PESTMODE As String

        ''' <summary>
        ''' Total number of parameters used for the current PEST case, including adjustable, fixed and tied parameters; NPAR must be supplied as an integer.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n102_NPAR As Integer

        ''' <summary>
        ''' Represents the total number of observations used in the current case.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n103_NOBS As Integer

        ''' <summary>
        ''' The number of parameter groups; parameter groups are discussed in detail below. NPARGP is an integer variable.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n104_NPARGP As Integer

        ''' <summary>
        ''' NPRIOR is the number of articles of prior information that you wish to include in the parameter estimation process. 
        ''' If there are no articles of prior information, NPRIOR must be zero.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n105_NPRIOR As Integer

        ''' <summary>
        ''' NOBSGP is the number of observation groups used in the current case.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n106_NOBSGP As Integer

        ''' <summary>
        ''' The number of model input files which contain parameters; PEST must write each of these files prior to a model run.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n107_NTPLFLE As Integer

        ''' <summary>
        ''' This is the number of instruction files. There must be one instruction file for each model output file 
        ''' containing model-generated observations which PEST uses in the determination of the objective function.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n108_NINSFLE As Integer

        ''' <summary>
        ''' If it is supplied to PEST as “single”, PEST writes parameters to model input files using single precision protocol; 
        ''' ie. parameter values will never be greater than 13 characters in length 
        ''' (even if the parameter space allows for a greater length) and the exponentiation character is “e”. 
        ''' If PRECIS is supplied as “double”, parameter values are written to model input files using double 
        ''' precision protocol; the maximum parameter value length is 23 characters and the exponentiation symbol is “d”.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n109_PRECIS As String

        ''' <summary>
        ''' If DPOINT is provided with the value “nopoint” PEST will 
        ''' omit the decimal point from representations of parameter values 
        '''on model input files if the decimal point is redundant, 
        '''thus making room for the use of one extra significant figure. 
        '''If DPOINT is supplied as “point”, PEST will ensure that the decimal point is always present.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n110_DPOINT As String

        ''' <summary>
        '''n10_NUMCOM, n11_JACFILE, and n12_MESSFILE are used to control the manner 
        '''in which PEST can obtain derivatives 
        '''directly from the model if these are available; see Chapter 9. 
        '''For normal operation these should be set at 1, 0 and 0 respectively. 
        ''' </summary>
        ''' <remarks></remarks>
        Dim n111_NUMCOM As Integer

        ''' <summary>
        '''n10_NUMCOM, n11_JACFILE, and n12_MESSFILE are used to control the manner 
        '''in which PEST can obtain derivatives 
        '''directly from the model if these are available; see Chapter 9. 
        '''For normal operation these should be set at 1, 0 and 0 respectively. 
        ''' </summary>
        ''' <remarks></remarks>
        Dim n112_JACFILE As Integer

        ''' <summary>
        '''n10_NUMCOM, n11_JACFILE, and n12_MESSFILE are used to control the manner 
        '''in which PEST can obtain derivatives 
        '''directly from the model if these are available; see Chapter 9. 
        '''For normal operation these should be set at 1, 0 and 0 respectively. 
        ''' </summary>
        ''' <remarks></remarks>
        Dim n113_MESSFILE As Integer

        ''' <summary>
        ''' This real variable is the initial Marquardt lambda. 
        ''' PEST attempts parameter improvement using a number of 
        ''' different Marquardt lambdas during any one optimisation 
        ''' iteration; however, in the course of the overall parameter 
        ''' estimation process, the Marquardt lambda 
        ''' generally gets smaller. An initial value of 1.0 to 10.0 is appropriate 
        ''' for most models, though if PEST complains that the normal 
        ''' matrix is not positive definite, you will need to provide 
        ''' a higher initial Marquardt lambda.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n114_RLAMBDA1 As Single

        ''' <summary>
        ''' RLAMFAC must be greater than 1.0; a value of 2.0 seems to work well on most occasions.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n115_RLAMFAC As Single

        ''' <summary>
        ''' PHIRATSUF (which stands for “phi ratio sufficient”) is a real variable for 
        ''' which a value of 0.3 is often appropriate.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n116_PHIRATSUF As Single

        ''' <summary>
        ''' A suitable value for PHIREDLAM is often around 0.01. 
        ''' If it is set too large, the criterion for moving on to the next optimisation 
        ''' iteration is too easily met and PEST is not given the 
        ''' opportunity of adjusting lambda to its optimal value for 
        ''' that particular stage of the parameter estimation process. 
        ''' On the other hand if PHIREDLAM is set too low, 
        ''' PEST will test too many Marquardt lambdas on each 
        ''' optimisation iteration when it would be better off starting a new iteration.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n117_PHIREDLAM As Single

        ''' <summary>
        ''' This integer variable places an upper limit on the number of lambdas 
        ''' that PEST can test during any one optimisation iteration. 
        ''' It should normally be set between 5 and 10; however 
        ''' if RLAMBDA1 is set to zero (as may be wise when using singular 
        ''' value decomposition) it must be set to 1
        ''' </summary>
        ''' <remarks></remarks>
        Dim n118_NUMLAM As Integer

        ''' <summary>
        ''' RELPARMAX is the maximum relative change that 
        ''' a parameter is allowed to undergo between optimisation 
        ''' iterations, whereas FACPARMAX is the maximum factor 
        ''' change that a parameter is allowed to undergo.
        ''' If you are unsure of how to set these parameters, 
        ''' a value of 5 for each of them is often suitable.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n119_RELPARMAX As Single

        ''' <summary>
        ''' RELPARMAX is the maximum relative change that 
        ''' a parameter is allowed to undergo between optimisation 
        ''' iterations, whereas FACPARMAX is the maximum factor 
        ''' change that a parameter is allowed to undergo.
        ''' If you are unsure of how to set these parameters, 
        ''' a value of 5 for each of them is often suitable.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n120_FACPARMAX As Single

        ''' <summary>
        ''' If the absolute value of a parameter falls below FACORIG times 
        ''' its original value, then FACORIG times its original value 
        ''' is substituted for the denominator of equation 4.3. 
        ''' For factor-limited parameters, a similar modification to equation 4.4 applies. 
        ''' Thus the constraints that apply to a growth in absolute value 
        ''' of a parameter are lifted when its absolute value has become less than FACORIG times 
        ''' its original absolute value. 
        ''' However, where PEST wishes to reduce the parameter’s absolute value even further, factor-limitations are not lifted; 
        ''' nor are relative limitations lifted if RELPARMAX is less than 1. 
        ''' FACORIG is not used to adjust limits for log-transformed parameters.
        '''FACORIG must be greater than zero. A value of 0.001 is often suitable.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n121_FACORIG As Single

        ''' <summary>
        ''' A value of 0.1 is often suitable for PHIREDSWH. If it is set too high, PEST may make the switch 
        ''' to three-point derivatives calculation before it needs to; the result will be that more model runs will 
        ''' be required to fill the Jacobian matrix than are really needed at that stage of the estimation process. 
        ''' If PHIREDSWH is set too low, PEST may waste an optimisation iteration or two in lowering the 
        ''' objective function to a smaller extent than would have been possible if it had made an earlier 
        ''' switch to central derivatives calculation. Note that PHIREDSWH should be set considerably 
        ''' higher than the input variable PHIREDSTP which sets one of the termination criteria on the 
        ''' basis of the relative objective function reduction between optimisation iterations.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n122_PHIREDSWH As Single

        ''' <summary>
        ''' NOPTMAX sets the maximum number of optimisation iterations that PEST is permitted to 
        ''' undertake on a particular parameter estimation run. If you want to ensure that PEST termination 
        ''' is triggered by other criteria, more indicative of parameter convergence to an optimal set or of the 
        ''' futility of further processing, you should set this variable very high. 
        ''' A value of 20 to 30 is often appropriate.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n123_NOPTMAX As Integer

        ''' <summary>
        ''' PHIREDSTP is a real variable whereas NPHISTP is an integer variable.
        ''' For many cases 0.005 and 4 are suitable values for PHIREDSTP and NPHISTP respectively.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n124_PHIREDSTP As Single

        ''' <summary>
        ''' PHIREDSTP is a real variable whereas NPHISTP is an integer variable.
        ''' For many cases 0.005 and 4 are suitable values for PHIREDSTP and NPHISTP respectively.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n125_NPHISTP As Integer

        ''' <summary>
        ''' If PEST has failed to lower the objective function over NPHINORED successive iterations, 
        ''' it will terminate execution. NPHINORED is an integer variable; a value of 3 or 4 is often suitable.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n126_NPHINORED As Integer

        ''' <summary>
        ''' If the magnitude of the maximum relative parameter change between 
        ''' optimisation iterations is less than RELPARSTP over NRELPAR successive iterations, PEST will cease execution.
        ''' RELPARSTP is a real variable for which a value of 0.01 is often suitable. 
        ''' NRELPAR is an integer variable; a value of 3 or 4 is normally satisfactory.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n127_RELPARSTP As Single

        ''' <summary>
        ''' If the magnitude of the maximum relative parameter change between 
        ''' optimisation iterations is less than RELPARSTP over NRELPAR successive iterations, PEST will cease execution.
        ''' RELPARSTP is a real variable for which a value of 0.01 is often suitable. 
        ''' NRELPAR is an integer variable; a value of 3 or 4 is normally satisfactory.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n128_NRELPAR As Integer

        ''' <summary>
        ''' At the end of each optimisation iteration PEST writes a “matrix file” containing the covariance 
        ''' and correlation coefficient matrices, as well as the eigenvectors and eigenvalues 
        ''' of the covariance matrix based on current parameter values. 
        ''' The settings of the ICOV, ICOR and IEIG variables determine which (if any) of 
        ''' these data are recorded on the matrix file. A setting of 1 for each of these variables 
        ''' will result in the corresponding data being recorded on the matrix file. On the other 
        ''' hand, a setting of 0 will result in the corresponding data not being recorded.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n129_ICOV As Integer

        ''' <summary>
        ''' At the end of each optimisation iteration PEST writes a “matrix file” containing the covariance 
        ''' and correlation coefficient matrices, as well as the eigenvectors and eigenvalues 
        ''' of the covariance matrix based on current parameter values. 
        ''' The settings of the ICOV, ICOR and IEIG variables determine which (if any) of 
        ''' these data are recorded on the matrix file. A setting of 1 for each of these variables 
        ''' will result in the corresponding data being recorded on the matrix file. On the other 
        ''' hand, a setting of 0 will result in the corresponding data not being recorded.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n130_ICOR As Integer

        ''' <summary>
        ''' At the end of each optimisation iteration PEST writes a “matrix file” containing the covariance 
        ''' and correlation coefficient matrices, as well as the eigenvectors and eigenvalues 
        ''' of the covariance matrix based on current parameter values. 
        ''' The settings of the ICOV, ICOR and IEIG variables determine which (if any) of 
        ''' these data are recorded on the matrix file. A setting of 1 for each of these variables 
        ''' will result in the corresponding data being recorded on the matrix file. On the other 
        ''' hand, a setting of 0 will result in the corresponding data not being recorded.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n131_IEIG As Integer
#End Region
#Region "Parameter groups : 2##"
        ''' <summary>
        ''' This is the parameter group name; it must be a maximum of twelve characters in length.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n200_PARGPNME As List(Of String)

        ''' <summary>
        ''' INCTYP is a character variable which can assume the values “relative”, “absolute” or “rel_to_max”.
        ''' If it is “relative”, the increment used for forward-difference calculation of derivatives with respect to 
        ''' any parameter belonging to the group is calculated as a fraction of the current value of that parameter; 
        ''' that fraction is provided as the real variable DERINC. However if INCTYP is “absolute” the parameter increment 
        ''' for parameters belonging to the group is fixed, being again provided as the variable DERINC. 
        ''' Alternatively, if INCTYP is “rel_to_max”, the increment for any group member is calculated as a fraction 
        ''' of the group member with highest absolute value, that fraction again being DERINC.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n201_INCTYP As String

        ''' <summary>
        ''' INCTYP is a character variable which can assume the values “relative”, “absolute” or “rel_to_max”.
        ''' If it is “relative”, the increment used for forward-difference calculation of derivatives with respect to 
        ''' any parameter belonging to the group is calculated as a fraction of the current value of that parameter; 
        ''' that fraction is provided as the real variable DERINC. However if INCTYP is “absolute” the parameter increment 
        ''' for parameters belonging to the group is fixed, being again provided as the variable DERINC. 
        ''' Alternatively, if INCTYP is “rel_to_max”, the increment for any group member is calculated as a fraction 
        ''' of the group member with highest absolute value, that fraction again being DERINC.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n202_DERINC As Single

        ''' <summary>
        ''' An absolute lower bound can be placed on parameter increments; 
        ''' the lower bound will be the same for all group members, and is provided as the input variable DERINCLB. 
        ''' Thus if a parameter value is currently 1000.0 and it belongs to a group for 
        ''' which INCTYP is “relative”, DERINC is 0.01, and DERINCLB is 15.0, the parameter 
        ''' increment will be 15.0 instead of 10.0 calculated on the basis of DERINC alone. 
        ''' If you do not wish to place a lower bound on parameter increments in this 
        ''' fashion, you should provide DERINCLB with a value of 0.0.
        ''' Note that if INCTYP is “absolute”, DERINCLB is ignored.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n203_DERINCLB As Single

        ''' <summary>
        ''' The character variable FORCEN (an abbreviation of “FORward/CENtral”) determines whether 
        ''' derivatives for group members are calculated using forward differences, one of the variants of the central difference method, 
        ''' of whether both alternatives are used in the course of an optimisation run. 
        ''' It must assume one of the values “always_2”, “always_3” or “switch”.
        ''' If FORCEN is set to “switch”, derivatives calculation for all adjustable group members 
        ''' will begin using the forward difference method, switching to the central method for the remainder of the 
        ''' estimation process on the iteration after the relative objective function reduction between successive optimisation 
        ''' iterations is less than PHIREDSWH, a value for which is supplied in the “control data” section of the PEST control file.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n204_FORCEN As String

        ''' <summary>
        ''' Whenever the central method is employed for derivatives calculation, 
        ''' DERINC is multiplied by DERINCMUL, no matter whether INCTYP is “absolute”, “relative” or “rel_to_max”, 
        ''' and whether FORCEN is “always_3” or “switch”. If you do not wish the increment to be increased, 
        ''' you must provide DERINCMUL with a value of 1.0. Alternatively, if for some reason you wish the increment 
        ''' to be reduced if three-point derivatives calculation is employed, you should provide DERINCMUL with a 
        ''' value of less than 1.0. Experience shows that a value between 1.0 and 2.0 is usually satisfactory.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n205_DERINCMUL As Single

        ''' <summary>
        ''' There are three variants of the central (ie. three-point) method of derivatives calculation.
        ''' If FORCEN for a particular parameter group is “always_3” or “switch”, you must inform 
        ''' PEST which three-point method to use. This is accomplished through the character 
        ''' variable DERMTHD which must be supplied as “parabolic”, “best_fit” or “outside_pts”.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n206_DERMTHD As String
#End Region

#Region "Parameter data : 3##"
        ''' <summary>
        ''' This is the parameter name. Each parameter name must be unique and 
        ''' of twelve characters or less in length; the name is case insensitive.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n300_PARNME As List(Of String)

        ''' <summary>
        ''' PARTRANS is a character variable which must assume one of four values, viz. “none”, “log”, “fixed” or “tied”.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n301_PARTRANS As String

        ''' <summary>
        ''' This character variable is used to designate whether an adjustable parameter is relative-limited or factor-limited.
        ''' PARCHGLIM must be provided with one of two possible values, viz. “relative” or “factor”. 
        ''' For tied or fixed parameters this variable has no significance.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n302_PARCHGLIM As String

        ''' <summary>
        ''' PARVAL1, a real variable, is a parameter’s initial value.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n303_PARVAL1 As List(Of Single)

        ''' <summary>
        ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n304_PARLBND As List(Of Single)

        ''' <summary>
        ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n305_PARUBND As List(Of Single)

        ''' <summary>
        ''' PARGP is the name of the group to which a parameter belongs. 
        ''' A parameter group name must be twelve characters or less in length and is case-insensitive.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n306_PARGP As List(Of String)

        ''' <summary>
        ''' Just before a parameter value is written to a model input file, 
        ''' it is multiplied by the real variable SCALE, after which the real variable OFFSET is added.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n307_SCALE As Single

        ''' <summary>
        ''' Just before a parameter value is written to a model input file, 
        ''' it is multiplied by the real variable SCALE, after which the real variable OFFSET is added.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n308_OFFSET As Single

        ''' <summary>
        ''' Unless using PEST’s external derivatives functionality, this variable should be set to 1.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n309_DERCOM As Integer

        ''' <summary>
        ''' The character variable PARTIED, must hold the name of the parameter to which the first-mentioned parameter is tied, 
        ''' ie. the “parent parameter” of the first-mentioned parameter. The parent parameter must not be a tied or fixed parameter itself.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n310_PARTIED As String
#End Region

        ' Observation groups : 4## /
        ''' <summary>
        ''' Observation group names must be of twelve characters or less in length and are case insensitive.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n400_OBGNME As List(Of String)

        '  Observation data : 5## /
        ''' <summary>
        ''' This is a character variable containing the observation name. An observation name must be twenty characters or less in length. 
        ''' Observation names are case-insensitive, but must be unique to each observation.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n500_OBSNME As Integer

        ''' <summary>
        ''' OBSVAL, a real variable, is the field or laboratory measurement corresponding to a model-generated observation. 
        ''' It is PEST’s role to minimise the difference between this number and
        ''' the corresponding model-calculated number (the difference being referred to as the “residual”) over all 
        ''' observations by adjusting parameter values until the sum of squared weighted residuals (ie. the objective function) is at a minimum.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n501_OBSVAL As List(Of String)

        ''' <summary>
        ''' This is the weight attached to each residual in the calculation of the objective function. 
        ''' An observation weight can be zero if you wish (meaning that the observation takes no 
        ''' part in the calculation of the objective function), but it must not be negative.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n502_WEIGHT As Single

        'Model command line : 6##
        ''' <summary>
        ''' The command line may be simply the name of an executable file, or it may be the name 
        ''' of a batch file containing a complex sequence of steps.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n600_ModelCommandLine As String

#Region "'Model input/output : 7##"
        ''' <summary>
        ''' TEMPFLE and INFLE are character variables. 
        ''' The first of these is the name of a PEST template file while the second is the name of the model input file 
        ''' to which the template file is matched.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n700_TEMPFLE As String

        ''' <summary>
        ''' TEMPFLE and INFLE are character variables. 
        ''' The first of these is the name of a PEST template file while the second is the name of the model input file 
        ''' to which the template file is matched.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n701_INFLE As String

        ''' <summary>
        ''' Instruction file name
        ''' </summary>
        ''' <remarks></remarks>
        Dim n702_INSFLE As String

        ''' <summary>
        ''' Model output file name.
        ''' </summary>
        ''' <remarks></remarks>
        Dim n703_OUTFLE As String
#End Region
    End Structure

    Public Structure ParameterDataVariables

        ''' <summary>
        ''' This is the parameter name. Each parameter name must be unique and 
        ''' of twelve characters or less in length; the name is case insensitive.
        ''' </summary>
        ''' <remarks></remarks>
        Dim PARNME As List(Of String)

        ''' <summary>
        ''' PARVAL1, a real variable, is a parameter’s initial value.
        ''' </summary>
        ''' <remarks></remarks>
        Dim PARVAL1 As List(Of Single)

        ''' <summary>
        ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
        ''' </summary>
        ''' <remarks></remarks>
        Dim PARLBND As List(Of Single)

        ''' <summary>
        ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
        ''' </summary>
        ''' <remarks></remarks>
        Dim PARUBND As List(Of Single)
    End Structure

    Public Structure PAR_RMF
        Dim NSLAVE As Integer
        Dim IFLETYP As Integer
        Dim WAIT As Single
        Dim PARLAM As Integer
        Dim RUNTIME As Single
        Dim FPathSlave As List(Of String)
    End Structure


    Public mParsPCF As PAR_PCF
    Public mdtQobsSeries As DataTable
    Public mWP As List(Of String)
    Public mParsChecked As ParameterChecked
    Public mFPNPestBatCopy As String
    Public mFPNPestBatRun As String
    Public mFPNPPestSlaveBat As List(Of String)
    Public mbShowConsole As Boolean
    Public mbPPEST As Boolean
    Public mParsRMF As PAR_RMF
    'Public mGRMProjectFileName As String
    'Dim psPEST As Diagnostics.Process
    'Dim psInfo As ProcessStartInfo
    'Public mPsPslave As List(Of Diagnostics.Process)
    Private mPsIDPSlave As List(Of Integer)
    Sub New()
        mdtQobsSeries = New DataTable
        mWP = New List(Of String)
        mFPNPPestSlaveBat = New List(Of String)
        mWPColumnIndex = 0
        InitializeParsChecked()
        InitializeParsInPCF()
        InitializeParsInRMF()
    End Sub

    Public Sub InitializeParsChecked()
        With mParsChecked
            .ISSRisChecked = False
            .MSLSisChecked = False
            .MSCBisChecked = False
            .MCWisChecked = False
            .CRCisChecked = False
            .IDSOisChecked = False
            .LCRCisChecked = False
            .SPisChecked = False
            .SWFSHisChecked = False
            .SHCisChecked = False
            .SDisChecked = False
        End With
    End Sub

    Private Sub InitializeParsInPCF()
        With mParsPCF
            .n100_RSTFLE = "restart"
            .n101_PESTMODE = "estimation"
            .n102_NPAR = 1
            .n103_NOBS = 1
            .n104_NPARGP = 0
            .n105_NPRIOR = 0
            .n106_NOBSGP = 1
            .n107_NTPLFLE = 1
            .n108_NINSFLE = 1
            .n109_PRECIS = "single"
            .n110_DPOINT = "point" 'or "nopoint"
            .n111_NUMCOM = 1
            .n112_JACFILE = 0
            .n113_MESSFILE = 0
            .n114_RLAMBDA1 = 5
            .n115_RLAMFAC = 2.0
            .n116_PHIRATSUF = 0.3
            .n117_PHIREDLAM = 0.01
            .n118_NUMLAM = 10
            .n119_RELPARMAX = 5.0
            .n120_FACPARMAX = 5.0
            .n121_FACORIG = 0.001
            .n122_PHIREDSWH = 0.1
            .n123_NOPTMAX = 30
            .n124_PHIREDSTP = 0.005
            .n125_NPHISTP = 4
            .n126_NPHINORED = 4
            .n127_RELPARSTP = 0.01
            .n128_NRELPAR = 4
            .n129_ICOV = 1
            .n130_ICOR = 1
            .n131_IEIG = 1
            .n200_PARGPNME = New List(Of String)
            .n201_INCTYP = "relative"
            .n202_DERINC = 0.01
            .n203_DERINCLB = 0.00001
            .n204_FORCEN = "switch"
            .n205_DERINCMUL = 2
            .n206_DERMTHD = "parabolic"
            .n300_PARNME = New List(Of String)
            .n301_PARTRANS = "none"
            .n302_PARCHGLIM = "factor"
            .n303_PARVAL1 = New List(Of Single)
            .n304_PARLBND = New List(Of Single)
            .n305_PARUBND = New List(Of Single)
            .n306_PARGP = New List(Of String)
            .n307_SCALE = 1
            .n308_OFFSET = 0
            .n309_DERCOM = 1
            .n400_OBGNME = New List(Of String)
            .n500_OBSNME = 0
            .n501_OBSVAL = New List(Of String)
            .n502_WEIGHT = 1
            .n600_ModelCommandLine = ""
            .n700_TEMPFLE = "ptf.pst"
            .n701_INFLE = ""
            .n702_INSFLE = "pif.pst"
            .n703_OUTFLE = ""
        End With
    End Sub

    Private Sub InitializeParsInRMF()
        With mParsRMF
            .NSLAVE = 4
            .IFLETYP = 1
            .WAIT = 1
            .PARLAM = 1
            .RUNTIME = 600
            .FPathSlave = New List(Of String)
        End With
    End Sub


    Public Function MakePControlFile_pcf(pcfFPN As String) As Boolean
        Try
            Dim pcfs As String = ""
            With mParsPCF
                pcfs = pcfs + String.Format("pcf") + vbCrLf
                pcfs = pcfs + String.Format("* control data") + vbCrLf
                pcfs = pcfs + String.Format("{0} {1}", .n100_RSTFLE, .n101_PESTMODE) + vbCrLf
                pcfs = pcfs + String.Format("{0} {1} {2} {3} {4}", .n102_NPAR, .n103_NOBS, .n104_NPARGP, _
                                            .n105_NPRIOR, .n106_NOBSGP) + vbCrLf
                pcfs = pcfs + String.Format("{0} {1} {2} {3} {4} {5} {6}", .n107_NTPLFLE, .n108_NINSFLE, _
                                            .n109_PRECIS, .n110_DPOINT, .n111_NUMCOM, .n112_JACFILE, .n113_MESSFILE) + vbCrLf
                pcfs = pcfs + String.Format("{0} {1} {2} {3} {4}", .n114_RLAMBDA1, .n115_RLAMFAC, _
                                            .n116_PHIRATSUF, .n117_PHIREDLAM, .n118_NUMLAM) + vbCrLf
                pcfs = pcfs + String.Format("{0} {1} {2}", .n119_RELPARMAX, .n120_FACPARMAX, _
                                        .n121_FACORIG) + vbCrLf
                pcfs = pcfs + String.Format("{0}", .n122_PHIREDSWH) + vbCrLf
                pcfs = pcfs + String.Format("{0} {1} {2} {3} {4} {5}", .n123_NOPTMAX, .n124_PHIREDSTP, _
                                .n125_NPHISTP, .n126_NPHINORED, .n127_RELPARSTP, .n128_NRELPAR) + vbCrLf
                pcfs = pcfs + String.Format("{0} {1} {2}", .n129_ICOV, .n130_ICOR, .n131_IEIG) + vbCrLf
                pcfs = pcfs + String.Format("* parameter groups") + vbCrLf
                For n As Integer = 0 To .n104_NPARGP - 1
                    pcfs = pcfs + String.Format("{0} {1} {2} {3} {4} {5} {6}", .n200_PARGPNME(n), .n201_INCTYP, _
                                           .n202_DERINC, .n203_DERINCLB, .n204_FORCEN, .n205_DERINCMUL, .n206_DERMTHD) + vbCrLf
                Next
                pcfs = pcfs + String.Format("* parameter data") + vbCrLf
                For n As Integer = 0 To .n104_NPARGP - 1
                    pcfs = pcfs + String.Format("{0} {1} {2} {3} {4} {5} {6} {7} {8} {9}", .n300_PARNME(n), .n301_PARTRANS, _
                                           .n302_PARCHGLIM, .n303_PARVAL1(n), .n304_PARLBND(n), .n305_PARUBND(n), _
                                           .n306_PARGP(n), .n307_SCALE, .n308_OFFSET, .n309_DERCOM) + vbCrLf
                Next
                pcfs = pcfs + String.Format("* observation groups") + vbCrLf
                For nog As Integer = 0 To .n400_OBGNME.Count - 1
                    pcfs = pcfs + String.Format("{0}", .n400_OBGNME(nog)) + vbCrLf
                Next
                pcfs = pcfs + String.Format("* observation data") + vbCrLf
                For no As Integer = 0 To .n103_NOBS - 1
                    .n500_OBSNME = no + 1
                    pcfs = pcfs + String.Format("o{0} {1} {2} {3}", .n500_OBSNME, .n501_OBSVAL(no), _
                                                .n502_WEIGHT, .n400_OBGNME(0)) + vbCrLf
                Next
                pcfs = pcfs + String.Format("* model command line") + vbCrLf
                pcfs = pcfs + String.Format("{0}", .n600_ModelCommandLine) + vbCrLf
                pcfs = pcfs + String.Format("* model input/output") + vbCrLf
                pcfs = pcfs + String.Format("{0} {1}", .n700_TEMPFLE, .n701_INFLE) + vbCrLf
                pcfs = pcfs + String.Format("{0} {1}", .n702_INSFLE, .n703_OUTFLE) + vbCrLf
                pcfs = pcfs + String.Format("* prior information") + vbCrLf
            End With
            System.IO.File.AppendAllText(pcfFPN, pcfs)
            Return True
        Catch ex As Exception
            System.Console.WriteLine("An error was occurred in making pcf file. ")
            Return False
        End Try
    End Function

    Public Function MakePInstructionFile_pif(pifFPN As String) As Boolean
        Try
            Dim wpname As String = mWP(0)
            Dim obsCount As Integer = mdtQobsSeries.Rows.Count
            Dim pifString As String
            Dim columnRange As String = ""
            '텝을 한문자로 인식할경우, 유량은 18열 부터 시작해서 8자리 기록, 
            '원래는 16, 18:25,  27:34, 36:43, 45:52 인데..
            '한문자를 2자리로 인식. 그러면, 32, 36:50, 54:68, 72:86, 90:102
            '그런데 날자와 첫번째 탭까지는 1자리로 인식 그러면, 16, 18:32, 34:48, 50:64 => 이게 최종 결론...

            columnRange = CStr(18 + 16 * (mWPColumnIndex - 1)) _
                       + ":" + CStr(18 + 16 * (mWPColumnIndex - 1) + 14)
            'columnRange = CStr(18 + 18 * (mWPColumnIndex - 1)) + _
            '             ":" + CStr(18 + 18 * (mWPColumnIndex - 1) + 16)

            pifString = "pif #" + vbCrLf
            pifString = pifString + String.Format("#[{0}]#", wpname) + vbCrLf
            For n As Integer = 0 To obsCount - 1
                pifString = pifString + String.Format("l{0} [o{1}]{2}", 1, n + 1, columnRange) + vbCrLf
            Next
            System.IO.File.AppendAllText(pifFPN, pifString)
            Return True
        Catch ex As Exception
            System.Console.WriteLine("An error was occurred in making pif file.")
            Return False
        End Try
    End Function

    Public Function MakePTemplateFile_ptf(ptfFPN As String) As Boolean
        Try
            Dim Lines() As String = System.IO.File.ReadAllLines(ptfFPN)
            For n As Integer = 0 To Lines.Length - 1
                If mParsChecked.ISSRisChecked = True AndAlso Lines(n).Contains("IniSaturation") = True Then
                    Lines(n) = "    <IniSaturation>#ISSR      #</IniSaturation>"
                End If
                If mParsChecked.MSLSisChecked = True AndAlso Lines(n).Contains("MinSlopeOF") = True Then
                    Lines(n) = "    <MinSlopeOF>#MSLS      #</MinSlopeOF>"
                End If
                If mParsChecked.MSCBisChecked = True AndAlso Lines(n).Contains("MinSlopeChBed") = True Then
                    Lines(n) = "    <MinSlopeChBed>#MSCB      #</MinSlopeChBed>"
                End If
                If mParsChecked.MCWisChecked = True AndAlso Lines(n).Contains("MinChBaseWidth") = True Then
                    Lines(n) = "    <MinChBaseWidth>#MCW       #</MinChBaseWidth>"
                End If
                If mParsChecked.CRCisChecked = True AndAlso Lines(n).Contains("ChRoughness") = True Then
                    Lines(n) = "    <ChRoughness>#CRC       #</ChRoughness>"
                End If
                If mParsChecked.IDSOisChecked = True AndAlso Lines(n).Contains("DryStreamOrder") = True Then
                    Lines(n) = "    <DryStreamOrder>#IDSO      #</DryStreamOrder>"
                End If
                If mParsChecked.LCRCisChecked = True AndAlso Lines(n).Contains("CalCoefLCRoughness") = True Then
                    Lines(n) = "    <CalCoefLCRoughness>#LCRC      #</CalCoefLCRoughness>"
                End If
                If mParsChecked.SPisChecked = True AndAlso Lines(n).Contains("CalCoefPorosity") = True Then
                    Lines(n) = "    <CalCoefPorosity>#SP        #</CalCoefPorosity>"
                End If
                If mParsChecked.SWFSHisChecked = True AndAlso Lines(n).Contains("CalCoefWFSuctionHead") = True Then
                    Lines(n) = "    <CalCoefWFSuctionHead>#SWFSH     #</CalCoefWFSuctionHead>"
                End If
                If mParsChecked.SHCisChecked = True AndAlso Lines(n).Contains("CalCoefHydraulicK") = True Then
                    Lines(n) = "    <CalCoefHydraulicK>#SHC       #</CalCoefHydraulicK>"
                End If
                If mParsChecked.SDisChecked = True AndAlso Lines(n).Contains("CalCoefSoilDepth") = True Then
                    Lines(n) = "    <CalCoefSoilDepth>#SD        #</CalCoefSoilDepth>"
                End If
            Next
            Dim newLines(Lines.Length) As String
            newLines(0) = "ptf #"

            For n As Integer = 1 To Lines.Length
                newLines(n) = Lines(n - 1)
            Next
            System.IO.File.WriteAllLines(ptfFPN, newLines)
            Return True
        Catch ex As Exception
            System.Console.WriteLine("An error was occurred in making ptf file.")
            Return False
        End Try
    End Function

    Public Function MakePPRunManageFile(rmfFPN As String) As Boolean
        Try
            Dim rmfs As String = ""
            With mParsRMF
                rmfs = rmfs + String.Format("prf") + vbCrLf
                rmfs = rmfs + String.Format("{0} {1} {2} {3}", .NSLAVE, .IFLETYP, .WAIT, .PARLAM) + vbCrLf

                rmfs = rmfs + String.Format("'MainMachine' .\") + vbCrLf
                For n As Integer = 1 To .NSLAVE - 1
                    rmfs = rmfs + String.Format("'Pslave{0}' {1}{2}", n, .FPathSlave(n), "\") + vbCrLf
                Next
                Dim lt As String = ""
                For n As Integer = 0 To .NSLAVE - 1
                    lt = lt + CStr(.RUNTIME) + " "
                Next
                rmfs = rmfs + Trim(lt) + vbCrLf
                For n As Integer = 0 To .NSLAVE - 1
                    rmfs = rmfs + Path.Combine(.FPathSlave(n), _
                                      Path.GetFileName(mParsPCF.n701_INFLE)) + vbCrLf
                    rmfs = rmfs + Path.Combine(.FPathSlave(n), _
                                      Path.GetFileName(mParsPCF.n703_OUTFLE)) + vbCrLf
                Next
            End With
            System.IO.File.AppendAllText(rmfFPN, rmfs)
            Return True
        Catch ex As Exception
            System.Console.WriteLine("An error was occurred in making rmf file.")
            Return False
        End Try
    End Function

    Public Shared Function InitializeParameterDataVariables(pdataVars As cPEST.ParameterDataVariables) As cPEST.ParameterDataVariables
        With pdataVars
            .PARNME = New List(Of String)
            .PARVAL1 = New List(Of Single)
            .PARLBND = New List(Of Single)
            .PARUBND = New List(Of Single)
        End With
        Return pdataVars
    End Function

    Public Sub RunPEST()
        Dim ts As New ThreadStart(AddressOf RunPESTInner)
        Dim th As New Thread(ts)
        th.Start()
    End Sub

    Private Sub RunPESTInner()
        If mbPPEST = False Then
            Call RunBatchCopy()
        End If
        Dim psPEST As New Diagnostics.Process
        Dim psInfo As New ProcessStartInfo
        psInfo.FileName = mFPNPestBatRun
        If mbShowConsole = True Then
            psInfo.WindowStyle = ProcessWindowStyle.Normal
        Else
            psInfo.WindowStyle = ProcessWindowStyle.Hidden
        End If
        psPEST.StartInfo = psInfo
        psPEST.Start()
        psPEST.WaitForExit()
        psPEST.Dispose()
        If mbPPEST = True Then Call CloseSlaveProcess()
        Thread.Sleep(2000) ' 이건 프로세스 종료 완료 잠깐 시간지연, 관련 실행파일 삭제를 위해 
        RaiseEvent PESTEnded(Me)
    End Sub

    Public Sub RunPSlave()
        Dim ts As New ThreadStart(AddressOf RunPSlaveInner)
        Dim th As New Thread(ts)
        th.Start()
    End Sub

    Public Sub RunPSlaveInner()
        Call RunBatchCopy()
        Call RunSlave()
    End Sub

    Private Sub RunBatchCopy()
        Dim psPEST As New Diagnostics.Process
        Dim psInfo As New ProcessStartInfo
        psInfo.FileName = mFPNPestBatCopy
        psInfo.WindowStyle = ProcessWindowStyle.Hidden
        psPEST.StartInfo = psInfo
        psPEST.Start()
        psPEST.WaitForExit()
        psPEST.Dispose()
    End Sub

    Private Sub RunSlave()
        'mPsPslave = New List(Of Diagnostics.Process)
        mPsIDPSlave = New List(Of Integer)
        If mbPPEST = True Then
            '이건 slave 실행 시키는 부분
            For n As Integer = 0 To mParsRMF.NSLAVE - 1
                Dim psinfo_tmp As New ProcessStartInfo
                psinfo_tmp.FileName = mFPNPPestSlaveBat(n)
                psinfo_tmp.WindowStyle = ProcessWindowStyle.Normal
                Dim ps_tmp As New Diagnostics.Process
                ps_tmp.StartInfo = psinfo_tmp
                ps_tmp.Start()
                'mPsPslave.Add(ps_tmp)
                'mPsPslave(n).Start()
                mPsIDPSlave.Add(ps_tmp.Id)
                'ps_tmp.Dispose()
            Next
        End If
    End Sub

    Private Sub CloseSlaveProcess()
        Try
            For n As Integer = 0 To mPsIDPSlave.Count - 1
                Dim ps As Process = Process.GetProcessById(mPsIDPSlave(n))
                ps.CloseMainWindow() '이렇게 해야 grm 실행중인 창도 강제로 닫힌다.
                'ps.Kill() '이걸로 하면 grm 실행중인창은 작업관리자에서 process는 없어지는데, 창은 안닫히고 계속 실행된다.
                ps.Close()
                ps.Dispose()
            Next
        Catch ex As Exception
            Throw ex
        End Try
        'Try '여기는 혹시 모르니.. 확인차원에서..
        '    Dim PSes As Process() = Process.GetProcessesByName("pslave")
        '    For Each aps As Process In PSes
        '        aps.CloseMainWindow()
        '        'aps.Kill()
        '        aps.Close()
        '        aps.Dispose()
        '    Next
        'Catch ex As Exception
        '    Throw ex
        'End Try
    End Sub
End Class
