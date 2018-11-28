using System;
using System.Collections.Generic;
using System.Threading;
using System.Diagnostics;
using System.IO;
using System.Data;

namespace GRMCore
{
    public class cPEST
    {
        public event PESTEndedEventHandler PESTEnded;
        public delegate void PESTEndedEventHandler(cPEST sender);

        public const string FILE_NAME_PEST_CONTROL_FILE = "pcf.pst";
        public const string FILE_NAME_PEST_INSTRUCTION_FILE = "pif.pst";
        public const string FILE_NAME_PEST_TEMPLATE_FILE = "ptf.pst";
        public const string FILE_NAME_PPEST_RUN_MANAGEMENT_FILE = "pcf.rmf";
        public const string FILE_NAME_BATCH_CPOY_PPEST = "PPESTCopy.bat";
        public const string FILE_NAME_BATCH_CPOY_PEST = "PPESTCopy.bat";
        public const string FILE_NAME_BATCH_RUN_PEST = "PPESTR.bat";
        public const string FILE_NAME_BATCH_RUN_PPEST = "PESTR.bat";
        public const string FILE_NAME_BATCH_RUN_PSLAVE = "PSLAVER.bat";
        public const string FILE_NAME_PPEST_EXE = "ppest.exe";
        public const string FILE_NAME_PEST_EXE = "pest.exe";
        public const string FILE_NAME_PSLAVE_EXE = "pslave.exe";
        /// <summary>
        ///   GRM 출력 파일에서 계산 결과를 읽을 watchpoint column 순서  
        ///   1부터 시작
        ///   </summary>
        ///   <remarks></remarks>
        public int mWPColumnIndex;
        public enum PestParGroupName
        {
            ISSR,
            MSLS,
            MSCB,
            MCW,
            CRC,
            IDSO,
            LCRC,
            SP,
            SWFSH,
            SHC,
            SD
        }

        public struct ParameterChecked
        {
            public bool ISSRisChecked;
            public bool MSLSisChecked;
            public bool MSCBisChecked;
            public bool MCWisChecked;
            public bool CRCisChecked;
            public bool IDSOisChecked;
            public bool LCRCisChecked;
            public bool SPisChecked;
            public bool SWFSHisChecked;
            public bool SHCisChecked;
            public bool SDisChecked;
        }

        /// <summary>
        ///   Control data : 1## /
        ///   Parameter groups : 2## /
        ///   Parameter data : 3## /
        ///   Observation groups : 4## /
        ///   Observation data : 5## /
        ///   Model command line : 6## /
        ///   Model input/output : 7## /
        ///   Prior information : 8## /
        ///   </summary>
        ///   <remarks></remarks>
        public struct PAR_PCF
        {
            /// <summary>
            ///         ''' If it takes the value “restart”, PEST will dump the contents of many of its data arrays to a binary file 
            ///         ''' (named case.rst where case is the current case name) at the beginning of each optimisation iteration; 
            ///         ''' this allows PEST to be restarted later if execution is prematurely terminated.
            ///         ''' If the RSTFLE variable is set to “norestart”, PEST will not intermittently dump 
            ///         ''' its array or Jacobian data; hence a later re-commencement of execution after premature termination is impossible.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n100_RSTFLE;

            /// <summary>
            ///         ''' If set to “estimation” PEST will run in parameter estimation mode (its traditional mode of operation); 
            ///         ''' if set to “prediction” PEST will run in predictive analysis mode; if set to “regularisation” PEST will run in regularisation mode.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n101_PESTMODE;

            /// <summary>
            ///         ''' Total number of parameters used for the current PEST case, including adjustable, fixed and tied parameters; NPAR must be supplied as an integer.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n102_NPAR;

            /// <summary>
            ///         ''' Represents the total number of observations used in the current case.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n103_NOBS;

            /// <summary>
            ///         ''' The number of parameter groups; parameter groups are discussed in detail below. NPARGP is an integer variable.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n104_NPARGP;

            /// <summary>
            ///         ''' NPRIOR is the number of articles of prior information that you wish to include in the parameter estimation process. 
            ///         ''' If there are no articles of prior information, NPRIOR must be zero.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n105_NPRIOR;

            /// <summary>
            ///         ''' NOBSGP is the number of observation groups used in the current case.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n106_NOBSGP;

            /// <summary>
            ///         ''' The number of model input files which contain parameters; PEST must write each of these files prior to a model run.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n107_NTPLFLE;

            /// <summary>
            ///         ''' This is the number of instruction files. There must be one instruction file for each model output file 
            ///         ''' containing model-generated observations which PEST uses in the determination of the objective function.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n108_NINSFLE;

            /// <summary>
            ///         ''' If it is supplied to PEST as “single”, PEST writes parameters to model input files using single precision protocol; 
            ///         ''' ie. parameter values will never be greater than 13 characters in length 
            ///         ''' (even if the parameter space allows for a greater length) and the exponentiation character is “e”. 
            ///         ''' If PRECIS is supplied as “double”, parameter values are written to model input files using double 
            ///         ''' precision protocol; the maximum parameter value length is 23 characters and the exponentiation symbol is “d”.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n109_PRECIS;

            /// <summary>
            ///         ''' If DPOINT is provided with the value “nopoint” PEST will 
            ///         ''' omit the decimal point from representations of parameter values 
            ///         '''on model input files if the decimal point is redundant, 
            ///         '''thus making room for the use of one extra significant figure. 
            ///         '''If DPOINT is supplied as “point”, PEST will ensure that the decimal point is always present.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n110_DPOINT;

            /// <summary>
            ///         '''n10_NUMCOM, n11_JACFILE, and n12_MESSFILE are used to control the manner 
            ///         '''in which PEST can obtain derivatives 
            ///         '''directly from the model if these are available; see Chapter 9. 
            ///         '''For normal operation these should be set at 1, 0 and 0 respectively. 
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n111_NUMCOM;

            /// <summary>
            ///         '''n10_NUMCOM, n11_JACFILE, and n12_MESSFILE are used to control the manner 
            ///         '''in which PEST can obtain derivatives 
            ///         '''directly from the model if these are available; see Chapter 9. 
            ///         '''For normal operation these should be set at 1, 0 and 0 respectively. 
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n112_JACFILE;

            /// <summary>
            ///         '''n10_NUMCOM, n11_JACFILE, and n12_MESSFILE are used to control the manner 
            ///         '''in which PEST can obtain derivatives 
            ///         '''directly from the model if these are available; see Chapter 9. 
            ///         '''For normal operation these should be set at 1, 0 and 0 respectively. 
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n113_MESSFILE;

            /// <summary>
            ///         ''' This real variable is the initial Marquardt lambda. 
            ///         ''' PEST attempts parameter improvement using a number of 
            ///         ''' different Marquardt lambdas during any one optimisation 
            ///         ''' iteration; however, in the course of the overall parameter 
            ///         ''' estimation process, the Marquardt lambda 
            ///         ''' generally gets smaller. An initial value of 1.0 to 10.0 is appropriate 
            ///         ''' for most models, though if PEST complains that the normal 
            ///         ''' matrix is not positive definite, you will need to provide 
            ///         ''' a higher initial Marquardt lambda.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n114_RLAMBDA1;

            /// <summary>
            ///         ''' RLAMFAC must be greater than 1.0; a value of 2.0 seems to work well on most occasions.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n115_RLAMFAC;

            /// <summary>
            ///         ''' PHIRATSUF (which stands for “phi ratio sufficient”) is a real variable for 
            ///         ''' which a value of 0.3 is often appropriate.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n116_PHIRATSUF;

            /// <summary>
            ///         ''' A suitable value for PHIREDLAM is often around 0.01. 
            ///         ''' If it is set too large, the criterion for moving on to the next optimisation 
            ///         ''' iteration is too easily met and PEST is not given the 
            ///         ''' opportunity of adjusting lambda to its optimal value for 
            ///         ''' that particular stage of the parameter estimation process. 
            ///         ''' On the other hand if PHIREDLAM is set too low, 
            ///         ''' PEST will test too many Marquardt lambdas on each 
            ///         ''' optimisation iteration when it would be better off starting a new iteration.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n117_PHIREDLAM;

            /// <summary>
            ///         ''' This integer variable places an upper limit on the number of lambdas 
            ///         ''' that PEST can test during any one optimisation iteration. 
            ///         ''' It should normally be set between 5 and 10; however 
            ///         ''' if RLAMBDA1 is set to zero (as may be wise when using singular 
            ///         ''' value decomposition) it must be set to 1
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n118_NUMLAM;

            /// <summary>
            ///         ''' RELPARMAX is the maximum relative change that 
            ///         ''' a parameter is allowed to undergo between optimisation 
            ///         ''' iterations, whereas FACPARMAX is the maximum factor 
            ///         ''' change that a parameter is allowed to undergo.
            ///         ''' If you are unsure of how to set these parameters, 
            ///         ''' a value of 5 for each of them is often suitable.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n119_RELPARMAX;

            /// <summary>
            ///         ''' RELPARMAX is the maximum relative change that 
            ///         ''' a parameter is allowed to undergo between optimisation 
            ///         ''' iterations, whereas FACPARMAX is the maximum factor 
            ///         ''' change that a parameter is allowed to undergo.
            ///         ''' If you are unsure of how to set these parameters, 
            ///         ''' a value of 5 for each of them is often suitable.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n120_FACPARMAX;

            /// <summary>
            ///         ''' If the absolute value of a parameter falls below FACORIG times 
            ///         ''' its original value, then FACORIG times its original value 
            ///         ''' is substituted for the denominator of equation 4.3. 
            ///         ''' For factor-limited parameters, a similar modification to equation 4.4 applies. 
            ///         ''' Thus the constraints that apply to a growth in absolute value 
            ///         ''' of a parameter are lifted when its absolute value has become less than FACORIG times 
            ///         ''' its original absolute value. 
            ///         ''' However, where PEST wishes to reduce the parameter’s absolute value even further, factor-limitations are not lifted; 
            ///         ''' nor are relative limitations lifted if RELPARMAX is less than 1. 
            ///         ''' FACORIG is not used to adjust limits for log-transformed parameters.
            ///         '''FACORIG must be greater than zero. A value of 0.001 is often suitable.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n121_FACORIG;

            /// <summary>
            ///         ''' A value of 0.1 is often suitable for PHIREDSWH. If it is set too high, PEST may make the switch 
            ///         ''' to three-point derivatives calculation before it needs to; the result will be that more model runs will 
            ///         ''' be required to fill the Jacobian matrix than are really needed at that stage of the estimation process. 
            ///         ''' If PHIREDSWH is set too low, PEST may waste an optimisation iteration or two in lowering the 
            ///         ''' objective function to a smaller extent than would have been possible if it had made an earlier 
            ///         ''' switch to central derivatives calculation. Note that PHIREDSWH should be set considerably 
            ///         ''' higher than the input variable PHIREDSTP which sets one of the termination criteria on the 
            ///         ''' basis of the relative objective function reduction between optimisation iterations.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n122_PHIREDSWH;

            /// <summary>
            ///         ''' NOPTMAX sets the maximum number of optimisation iterations that PEST is permitted to 
            ///         ''' undertake on a particular parameter estimation run. If you want to ensure that PEST termination 
            ///         ''' is triggered by other criteria, more indicative of parameter convergence to an optimal set or of the 
            ///         ''' futility of further processing, you should set this variable very high. 
            ///         ''' A value of 20 to 30 is often appropriate.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n123_NOPTMAX;

            /// <summary>
            ///         ''' PHIREDSTP is a real variable whereas NPHISTP is an integer variable.
            ///         ''' For many cases 0.005 and 4 are suitable values for PHIREDSTP and NPHISTP respectively.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n124_PHIREDSTP;

            /// <summary>
            ///         ''' PHIREDSTP is a real variable whereas NPHISTP is an integer variable.
            ///         ''' For many cases 0.005 and 4 are suitable values for PHIREDSTP and NPHISTP respectively.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n125_NPHISTP;

            /// <summary>
            ///         ''' If PEST has failed to lower the objective function over NPHINORED successive iterations, 
            ///         ''' it will terminate execution. NPHINORED is an integer variable; a value of 3 or 4 is often suitable.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n126_NPHINORED;

            /// <summary>
            ///         ''' If the magnitude of the maximum relative parameter change between 
            ///         ''' optimisation iterations is less than RELPARSTP over NRELPAR successive iterations, PEST will cease execution.
            ///         ''' RELPARSTP is a real variable for which a value of 0.01 is often suitable. 
            ///         ''' NRELPAR is an integer variable; a value of 3 or 4 is normally satisfactory.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n127_RELPARSTP;

            /// <summary>
            ///         ''' If the magnitude of the maximum relative parameter change between 
            ///         ''' optimisation iterations is less than RELPARSTP over NRELPAR successive iterations, PEST will cease execution.
            ///         ''' RELPARSTP is a real variable for which a value of 0.01 is often suitable. 
            ///         ''' NRELPAR is an integer variable; a value of 3 or 4 is normally satisfactory.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n128_NRELPAR;

            /// <summary>
            ///         ''' At the end of each optimisation iteration PEST writes a “matrix file” containing the covariance 
            ///         ''' and correlation coefficient matrices, as well as the eigenvectors and eigenvalues 
            ///         ''' of the covariance matrix based on current parameter values. 
            ///         ''' The settings of the ICOV, ICOR and IEIG variables determine which (if any) of 
            ///         ''' these data are recorded on the matrix file. A setting of 1 for each of these variables 
            ///         ''' will result in the corresponding data being recorded on the matrix file. On the other 
            ///         ''' hand, a setting of 0 will result in the corresponding data not being recorded.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n129_ICOV;

            /// <summary>
            ///         ''' At the end of each optimisation iteration PEST writes a “matrix file” containing the covariance 
            ///         ''' and correlation coefficient matrices, as well as the eigenvectors and eigenvalues 
            ///         ''' of the covariance matrix based on current parameter values. 
            ///         ''' The settings of the ICOV, ICOR and IEIG variables determine which (if any) of 
            ///         ''' these data are recorded on the matrix file. A setting of 1 for each of these variables 
            ///         ''' will result in the corresponding data being recorded on the matrix file. On the other 
            ///         ''' hand, a setting of 0 will result in the corresponding data not being recorded.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n130_ICOR;

            /// <summary>
            ///         ''' At the end of each optimisation iteration PEST writes a “matrix file” containing the covariance 
            ///         ''' and correlation coefficient matrices, as well as the eigenvectors and eigenvalues 
            ///         ''' of the covariance matrix based on current parameter values. 
            ///         ''' The settings of the ICOV, ICOR and IEIG variables determine which (if any) of 
            ///         ''' these data are recorded on the matrix file. A setting of 1 for each of these variables 
            ///         ''' will result in the corresponding data being recorded on the matrix file. On the other 
            ///         ''' hand, a setting of 0 will result in the corresponding data not being recorded.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n131_IEIG;
            /// <summary>
            ///         ''' This is the parameter group name; it must be a maximum of twelve characters in length.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<string> n200_PARGPNME;

            /// <summary>
            ///         ''' INCTYP is a character variable which can assume the values “relative”, “absolute” or “rel_to_max”.
            ///         ''' If it is “relative”, the increment used for forward-difference calculation of derivatives with respect to 
            ///         ''' any parameter belonging to the group is calculated as a fraction of the current value of that parameter; 
            ///         ''' that fraction is provided as the real variable DERINC. However if INCTYP is “absolute” the parameter increment 
            ///         ''' for parameters belonging to the group is fixed, being again provided as the variable DERINC. 
            ///         ''' Alternatively, if INCTYP is “rel_to_max”, the increment for any group member is calculated as a fraction 
            ///         ''' of the group member with highest absolute value, that fraction again being DERINC.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n201_INCTYP;

            /// <summary>
            ///         ''' INCTYP is a character variable which can assume the values “relative”, “absolute” or “rel_to_max”.
            ///         ''' If it is “relative”, the increment used for forward-difference calculation of derivatives with respect to 
            ///         ''' any parameter belonging to the group is calculated as a fraction of the current value of that parameter; 
            ///         ''' that fraction is provided as the real variable DERINC. However if INCTYP is “absolute” the parameter increment 
            ///         ''' for parameters belonging to the group is fixed, being again provided as the variable DERINC. 
            ///         ''' Alternatively, if INCTYP is “rel_to_max”, the increment for any group member is calculated as a fraction 
            ///         ''' of the group member with highest absolute value, that fraction again being DERINC.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n202_DERINC;

            /// <summary>
            ///         ''' An absolute lower bound can be placed on parameter increments; 
            ///         ''' the lower bound will be the same for all group members, and is provided as the input variable DERINCLB. 
            ///         ''' Thus if a parameter value is currently 1000.0 and it belongs to a group for 
            ///         ''' which INCTYP is “relative”, DERINC is 0.01, and DERINCLB is 15.0, the parameter 
            ///         ''' increment will be 15.0 instead of 10.0 calculated on the basis of DERINC alone. 
            ///         ''' If you do not wish to place a lower bound on parameter increments in this 
            ///         ''' fashion, you should provide DERINCLB with a value of 0.0.
            ///         ''' Note that if INCTYP is “absolute”, DERINCLB is ignored.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n203_DERINCLB;

            /// <summary>
            ///         ''' The character variable FORCEN (an abbreviation of “FORward/CENtral”) determines whether 
            ///         ''' derivatives for group members are calculated using forward differences, one of the variants of the central difference method, 
            ///         ''' of whether both alternatives are used in the course of an optimisation run. 
            ///         ''' It must assume one of the values “always_2”, “always_3” or “switch”.
            ///         ''' If FORCEN is set to “switch”, derivatives calculation for all adjustable group members 
            ///         ''' will begin using the forward difference method, switching to the central method for the remainder of the 
            ///         ''' estimation process on the iteration after the relative objective function reduction between successive optimisation 
            ///         ''' iterations is less than PHIREDSWH, a value for which is supplied in the “control data” section of the PEST control file.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n204_FORCEN;

            /// <summary>
            ///         ''' Whenever the central method is employed for derivatives calculation, 
            ///         ''' DERINC is multiplied by DERINCMUL, no matter whether INCTYP is “absolute”, “relative” or “rel_to_max”, 
            ///         ''' and whether FORCEN is “always_3” or “switch”. If you do not wish the increment to be increased, 
            ///         ''' you must provide DERINCMUL with a value of 1.0. Alternatively, if for some reason you wish the increment 
            ///         ''' to be reduced if three-point derivatives calculation is employed, you should provide DERINCMUL with a 
            ///         ''' value of less than 1.0. Experience shows that a value between 1.0 and 2.0 is usually satisfactory.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n205_DERINCMUL;

            /// <summary>
            ///         ''' There are three variants of the central (ie. three-point) method of derivatives calculation.
            ///         ''' If FORCEN for a particular parameter group is “always_3” or “switch”, you must inform 
            ///         ''' PEST which three-point method to use. This is accomplished through the character 
            ///         ''' variable DERMTHD which must be supplied as “parabolic”, “best_fit” or “outside_pts”.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n206_DERMTHD;

            /// <summary>
            ///         ''' This is the parameter name. Each parameter name must be unique and 
            ///         ''' of twelve characters or less in length; the name is case insensitive.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<string> n300_PARNME;

            /// <summary>
            ///         ''' PARTRANS is a character variable which must assume one of four values, viz. “none”, “log”, “fixed” or “tied”.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n301_PARTRANS;

            /// <summary>
            ///         ''' This character variable is used to designate whether an adjustable parameter is relative-limited or factor-limited.
            ///         ''' PARCHGLIM must be provided with one of two possible values, viz. “relative” or “factor”. 
            ///         ''' For tied or fixed parameters this variable has no significance.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n302_PARCHGLIM;

            /// <summary>
            ///         ''' PARVAL1, a real variable, is a parameter’s initial value.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<float> n303_PARVAL1;

            /// <summary>
            ///         ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<float> n304_PARLBND;

            /// <summary>
            ///         ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<float> n305_PARUBND;

            /// <summary>
            ///         ''' PARGP is the name of the group to which a parameter belongs. 
            ///         ''' A parameter group name must be twelve characters or less in length and is case-insensitive.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<string> n306_PARGP;

            /// <summary>
            ///         ''' Just before a parameter value is written to a model input file, 
            ///         ''' it is multiplied by the real variable SCALE, after which the real variable OFFSET is added.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n307_SCALE;

            /// <summary>
            ///         ''' Just before a parameter value is written to a model input file, 
            ///         ''' it is multiplied by the real variable SCALE, after which the real variable OFFSET is added.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n308_OFFSET;

            /// <summary>
            ///         ''' Unless using PEST’s external derivatives functionality, this variable should be set to 1.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n309_DERCOM;

            /// <summary>
            ///         ''' The character variable PARTIED, must hold the name of the parameter to which the first-mentioned parameter is tied, 
            ///         ''' ie. the “parent parameter” of the first-mentioned parameter. The parent parameter must not be a tied or fixed parameter itself.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n310_PARTIED;

            // Observation groups : 4## /
            /// <summary>
            ///         ''' Observation group names must be of twelve characters or less in length and are case insensitive.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<string> n400_OBGNME;

            // Observation data : 5## /
            /// <summary>
            ///         ''' This is a character variable containing the observation name. An observation name must be twenty characters or less in length. 
            ///         ''' Observation names are case-insensitive, but must be unique to each observation.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public int n500_OBSNME;

            /// <summary>
            ///         ''' OBSVAL, a real variable, is the field or laboratory measurement corresponding to a model-generated observation. 
            ///         ''' It is PEST’s role to minimise the difference between this number and
            ///         ''' the corresponding model-calculated number (the difference being referred to as the “residual”) over all 
            ///         ''' observations by adjusting parameter values until the sum of squared weighted residuals (ie. the objective function) is at a minimum.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<string> n501_OBSVAL;

            /// <summary>
            ///         ''' This is the weight attached to each residual in the calculation of the objective function. 
            ///         ''' An observation weight can be zero if you wish (meaning that the observation takes no 
            ///         ''' part in the calculation of the objective function), but it must not be negative.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public double n502_WEIGHT;

            // Model command line : 6##
            /// <summary>
            ///         ''' The command line may be simply the name of an executable file, or it may be the name 
            ///         ''' of a batch file containing a complex sequence of steps.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n600_ModelCommandLine;

            /// <summary>
            ///         ''' TEMPFLE and INFLE are character variables. 
            ///         ''' The first of these is the name of a PEST template file while the second is the name of the model input file 
            ///         ''' to which the template file is matched.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n700_TEMPFLE;

            /// <summary>
            ///         ''' TEMPFLE and INFLE are character variables. 
            ///         ''' The first of these is the name of a PEST template file while the second is the name of the model input file 
            ///         ''' to which the template file is matched.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n701_INFLE;

            /// <summary>
            ///         ''' Instruction file name
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n702_INSFLE;

            /// <summary>
            ///         ''' Model output file name.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public string n703_OUTFLE;
        }

        public struct ParameterDataVariables
        {

            /// <summary>
            ///         ''' This is the parameter name. Each parameter name must be unique and 
            ///         ''' of twelve characters or less in length; the name is case insensitive.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<string> PARNME;

            /// <summary>
            ///         ''' PARVAL1, a real variable, is a parameter’s initial value.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<float> PARVAL1;

            /// <summary>
            ///         ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<float> PARLBND;

            /// <summary>
            ///         ''' PARLBND and PARUBND real variables represent a parameter’s lower and upper bounds respectively.
            ///         ''' </summary>
            ///         ''' <remarks></remarks>
            public List<float> PARUBND;
        }

        public struct PAR_RMF
        {
            public int NSLAVE;
            public int IFLETYP;
            public double WAIT;
            public int PARLAM;
            public double RUNTIME;
            public List<string> FPathSlave;
        }


        public PAR_PCF mParsPCF;
        public DataTable mdtQobsSeries;
        public List<string> mWP;
        public ParameterChecked mParsChecked;
        public string mFPNPestBatCopy;
        public string mFPNPestBatRun;
        public List<string> mFPNPPestSlaveBat;
        public bool mbShowConsole;
        public bool mbPPEST;
        public PAR_RMF mParsRMF;
        private List<int> mPsIDPSlave;
        public cPEST()
        {
            mdtQobsSeries = new DataTable();
            mWP = new List<string>();
            mFPNPPestSlaveBat = new List<string>();
            mWPColumnIndex = 0;
            InitializeParsChecked();
            InitializeParsInPCF();
            InitializeParsInRMF();
        }

        public void InitializeParsChecked()
        {
            mParsChecked.ISSRisChecked = false;
            mParsChecked.MSLSisChecked = false;
            mParsChecked.MSCBisChecked = false;
            mParsChecked.MCWisChecked = false;
            mParsChecked.CRCisChecked = false;
            mParsChecked.IDSOisChecked = false;
            mParsChecked.LCRCisChecked = false;
            mParsChecked.SPisChecked = false;
            mParsChecked.SWFSHisChecked = false;
            mParsChecked.SHCisChecked = false;
            mParsChecked.SDisChecked = false;
        }

        private void InitializeParsInPCF()
        {
            mParsPCF.n100_RSTFLE = "restart";
            mParsPCF.n101_PESTMODE = "estimation";
            mParsPCF.n102_NPAR = 1;
            mParsPCF.n103_NOBS = 1;
            mParsPCF.n104_NPARGP = 0;
            mParsPCF.n105_NPRIOR = 0;
            mParsPCF.n106_NOBSGP = 1;
            mParsPCF.n107_NTPLFLE = 1;
            mParsPCF.n108_NINSFLE = 1;
            mParsPCF.n109_PRECIS = "single";
            mParsPCF.n110_DPOINT = "point"; // or "nopoint"
            mParsPCF.n111_NUMCOM = 1;
            mParsPCF.n112_JACFILE = 0;
            mParsPCF.n113_MESSFILE = 0;
            mParsPCF.n114_RLAMBDA1 = 5;
            mParsPCF.n115_RLAMFAC = 2.0;
            mParsPCF.n116_PHIRATSUF = 0.3;
            mParsPCF.n117_PHIREDLAM = 0.01;
            mParsPCF.n118_NUMLAM = 10;
            mParsPCF.n119_RELPARMAX = 5.0;
            mParsPCF.n120_FACPARMAX = 5.0;
            mParsPCF.n121_FACORIG = 0.001;
            mParsPCF.n122_PHIREDSWH = 0.1;
            mParsPCF.n123_NOPTMAX = 30;
            mParsPCF.n124_PHIREDSTP = 0.005;
            mParsPCF.n125_NPHISTP = 4;
            mParsPCF.n126_NPHINORED = 4;
            mParsPCF.n127_RELPARSTP = 0.01;
            mParsPCF.n128_NRELPAR = 4;
            mParsPCF.n129_ICOV = 1;
            mParsPCF.n130_ICOR = 1;
            mParsPCF.n131_IEIG = 1;
            mParsPCF.n200_PARGPNME = new List<string>();
            mParsPCF.n201_INCTYP = "relative";
            mParsPCF.n202_DERINC = 0.01;
            mParsPCF.n203_DERINCLB = 0.00001;
            mParsPCF.n204_FORCEN = "switch";
            mParsPCF.n205_DERINCMUL = 2;
            mParsPCF.n206_DERMTHD = "parabolic";
            mParsPCF.n300_PARNME = new List<string>();
            mParsPCF.n301_PARTRANS = "none";
            mParsPCF.n302_PARCHGLIM = "factor";
            mParsPCF.n303_PARVAL1 = new List<float>();
            mParsPCF.n304_PARLBND = new List<float>();
            mParsPCF.n305_PARUBND = new List<float>();
            mParsPCF.n306_PARGP = new List<string>();
            mParsPCF.n307_SCALE = 1;
            mParsPCF.n308_OFFSET = 0;
            mParsPCF.n309_DERCOM = 1;
            mParsPCF.n400_OBGNME = new List<string>();
            mParsPCF.n500_OBSNME = 0;
            mParsPCF.n501_OBSVAL = new List<string>();
            mParsPCF.n502_WEIGHT = 1;
            mParsPCF.n600_ModelCommandLine = "";
            mParsPCF.n700_TEMPFLE = "ptf.pst";
            mParsPCF.n701_INFLE = "";
            mParsPCF.n702_INSFLE = "pif.pst";
            mParsPCF.n703_OUTFLE = "";
        }

        private void InitializeParsInRMF()
        {
            mParsRMF.NSLAVE = 4;
            mParsRMF.IFLETYP = 1;
            mParsRMF.WAIT = 1;
            mParsRMF.PARLAM = 1;
            mParsRMF.RUNTIME = 600;
            mParsRMF.FPathSlave = new List<string>();
        }


        public bool MakePControlFile_pcf(string pcfFPN)
        {
            try
            {
                string pcfs = "";
                pcfs = pcfs + string.Format("pcf") + "\r\n";
                pcfs = pcfs + string.Format("* control data") + "\r\n";
                pcfs = pcfs + string.Format("{0} {1}", mParsPCF.n100_RSTFLE, mParsPCF.n101_PESTMODE) + "\r\n";
                pcfs = pcfs + string.Format("{0} {1} {2} {3} {4}", mParsPCF.n102_NPAR, mParsPCF.n103_NOBS, mParsPCF.n104_NPARGP, mParsPCF.n105_NPRIOR, mParsPCF.n106_NOBSGP) + "\r\n";
                pcfs = pcfs + string.Format("{0} {1} {2} {3} {4} {5} {6}", mParsPCF.n107_NTPLFLE, mParsPCF.n108_NINSFLE, mParsPCF.n109_PRECIS, mParsPCF.n110_DPOINT, mParsPCF.n111_NUMCOM, mParsPCF.n112_JACFILE, mParsPCF.n113_MESSFILE) + "\r\n";
                pcfs = pcfs + string.Format("{0} {1} {2} {3} {4}", mParsPCF.n114_RLAMBDA1, mParsPCF.n115_RLAMFAC, mParsPCF.n116_PHIRATSUF, mParsPCF.n117_PHIREDLAM, mParsPCF.n118_NUMLAM) + "\r\n";
                pcfs = pcfs + string.Format("{0} {1} {2}", mParsPCF.n119_RELPARMAX, mParsPCF.n120_FACPARMAX, mParsPCF.n121_FACORIG) + "\r\n";
                pcfs = pcfs + string.Format("{0}", mParsPCF.n122_PHIREDSWH) + "\r\n";
                pcfs = pcfs + string.Format("{0} {1} {2} {3} {4} {5}", mParsPCF.n123_NOPTMAX, mParsPCF.n124_PHIREDSTP, mParsPCF.n125_NPHISTP, mParsPCF.n126_NPHINORED, mParsPCF.n127_RELPARSTP, mParsPCF.n128_NRELPAR) + "\r\n";
                pcfs = pcfs + string.Format("{0} {1} {2}", mParsPCF.n129_ICOV, mParsPCF.n130_ICOR, mParsPCF.n131_IEIG) + "\r\n";
                pcfs = pcfs + string.Format("* parameter groups") + "\r\n";
                for (int n = 0; n < mParsPCF.n104_NPARGP; n++)
                {
                    pcfs = pcfs + string.Format("{0} {1} {2} {3} {4} {5} {6}", mParsPCF.n200_PARGPNME[n], mParsPCF.n201_INCTYP, mParsPCF.n202_DERINC, mParsPCF.n203_DERINCLB, 
                        mParsPCF.n204_FORCEN, mParsPCF.n205_DERINCMUL, mParsPCF.n206_DERMTHD) + "\r\n";
                }
                pcfs = pcfs + string.Format("* parameter data") + "\r\n";
                for (int n = 0; n < mParsPCF.n104_NPARGP; n++)
                {
                    pcfs = pcfs + string.Format("{0} {1} {2} {3} {4} {5} {6} {7} {8} {9}", mParsPCF.n300_PARNME[n], mParsPCF.n301_PARTRANS, mParsPCF.n302_PARCHGLIM, mParsPCF.n303_PARVAL1[n],
                        mParsPCF.n304_PARLBND[n], mParsPCF.n305_PARUBND[n], mParsPCF.n306_PARGP[n], mParsPCF.n307_SCALE, mParsPCF.n308_OFFSET, mParsPCF.n309_DERCOM) + "\r\n";
                }
                pcfs = pcfs + string.Format("* observation groups") + "\r\n";
                for (int nog = 0; nog < mParsPCF.n400_OBGNME.Count; nog++)
                {
                    pcfs = pcfs + string.Format("{0}", mParsPCF.n400_OBGNME[nog]) + "\r\n";
                }
                pcfs = pcfs + string.Format("* observation data") + "\r\n";
                for (int no = 0; no < mParsPCF.n103_NOBS; no++)
                {
                    mParsPCF.n500_OBSNME = no + 1;
                    pcfs = pcfs + string.Format("o{0} {1} {2} {3}", mParsPCF.n500_OBSNME, mParsPCF.n501_OBSVAL[no], mParsPCF.n502_WEIGHT, mParsPCF.n400_OBGNME[0]) + "\r\n";
                }
                pcfs = pcfs + string.Format("* model command line") + "\r\n";
                pcfs = pcfs + string.Format("{0}", mParsPCF.n600_ModelCommandLine) + "\r\n";
                pcfs = pcfs + string.Format("* model input/output") + "\r\n";
                pcfs = pcfs + string.Format("{0} {1}", mParsPCF.n700_TEMPFLE, mParsPCF.n701_INFLE) + "\r\n";
                pcfs = pcfs + string.Format("{0} {1}", mParsPCF.n702_INSFLE, mParsPCF.n703_OUTFLE) + "\r\n";
                pcfs = pcfs + string.Format("* prior information") + "\r\n";
                System.IO.File.AppendAllText(pcfFPN, pcfs);
                return true;
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("An error was occurred in making pcf file. ");
                System.Console.WriteLine(ex.ToString());
                return false;
            }
        }

        public bool MakePInstructionFile_pif(string pifFPN)
        {
            try
            {
                string wpname = mWP[0];
                int obsCount = mdtQobsSeries.Rows.Count;
                string pifString;
                string columnRange = "";
                // 날자와 첫번째 탭까지는 1자리로 인식 그러면, 16, 18:32, 34:48, 50:64 => 이게 최종 결론...

                columnRange = System.Convert.ToString(18 + 16 * (mWPColumnIndex - 1))
                           + ":" + System.Convert.ToString(18 + 16 * (mWPColumnIndex - 1) + 14);
                pifString = "pif #" + "\r\n";
                pifString = pifString + string.Format("#[{0}]#", wpname) + "\r\n";
                for (int n = 0; n < obsCount; n++)
                    pifString = pifString + string.Format("l{0} [o{1}]{2}", 1, n + 1, columnRange) + "\r\n";
                System.IO.File.AppendAllText(pifFPN, pifString);
                return true;
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("An error was occurred in making pif file.");
                System.Console.WriteLine(ex.ToString());
                return false;
            }
        }

        public bool MakePTemplateFile_ptf(string ptfFPN)
        {
            try
            {
                string[] Lines = System.IO.File.ReadAllLines(ptfFPN);
                for (int n = 0; n < Lines.Length; n++)
                {
                    if (mParsChecked.ISSRisChecked == true && Lines[n].Contains("IniSaturation") == true)
                        Lines[n] = "    <IniSaturation>#ISSR      #</IniSaturation>";
                    if (mParsChecked.MSLSisChecked == true && Lines[n].Contains("MinSlopeOF") == true)
                        Lines[n] = "    <MinSlopeOF>#MSLS      #</MinSlopeOF>";
                    if (mParsChecked.MSCBisChecked == true && Lines[n].Contains("MinSlopeChBed") == true)
                        Lines[n] = "    <MinSlopeChBed>#MSCB      #</MinSlopeChBed>";
                    if (mParsChecked.MCWisChecked == true && Lines[n].Contains("MinChBaseWidth") == true)
                        Lines[n] = "    <MinChBaseWidth>#MCW       #</MinChBaseWidth>";
                    if (mParsChecked.CRCisChecked == true && Lines[n].Contains("ChRoughness") == true)
                        Lines[n] = "    <ChRoughness>#CRC       #</ChRoughness>";
                    if (mParsChecked.IDSOisChecked == true && Lines[n].Contains("DryStreamOrder") == true)
                        Lines[n] = "    <DryStreamOrder>#IDSO      #</DryStreamOrder>";
                    if (mParsChecked.LCRCisChecked == true && Lines[n].Contains("CalCoefLCRoughness") == true)
                        Lines[n] = "    <CalCoefLCRoughness>#LCRC      #</CalCoefLCRoughness>";
                    if (mParsChecked.SPisChecked == true && Lines[n].Contains("CalCoefPorosity") == true)
                        Lines[n] = "    <CalCoefPorosity>#SP        #</CalCoefPorosity>";
                    if (mParsChecked.SWFSHisChecked == true && Lines[n].Contains("CalCoefWFSuctionHead") == true)
                        Lines[n] = "    <CalCoefWFSuctionHead>#SWFSH     #</CalCoefWFSuctionHead>";
                    if (mParsChecked.SHCisChecked == true && Lines[n].Contains("CalCoefHydraulicK") == true)
                        Lines[n] = "    <CalCoefHydraulicK>#SHC       #</CalCoefHydraulicK>";
                    if (mParsChecked.SDisChecked == true && Lines[n].Contains("CalCoefSoilDepth") == true)
                        Lines[n] = "    <CalCoefSoilDepth>#SD        #</CalCoefSoilDepth>";
                }
                string[] newLines = new string[Lines.Length + 1];
                newLines[0] = "ptf #";
                for (int n = 0; n < Lines.Length; n++)
                    newLines[n+1] = Lines[n];
                System.IO.File.WriteAllLines(ptfFPN, newLines);
                return true;
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("An error was occurred in making ptf file.");
                System.Console.WriteLine(ex.ToString());
                return false;
            }
        }

        public bool MakePPRunManageFile(string rmfFPN)
        {
            try
            {
                string rmfs = "";
                rmfs = rmfs + string.Format("prf") + "\r\n";
                rmfs = rmfs + string.Format("{0} {1} {2} {3}", mParsRMF.NSLAVE, mParsRMF.IFLETYP, mParsRMF.WAIT, mParsRMF.PARLAM) + "\r\n";
                rmfs = rmfs + string.Format(@"'MainMachine' .\") + "\r\n";
                for (int n = 1; n < mParsRMF.NSLAVE; n++)
                    rmfs = rmfs + string.Format("'Pslave{0}' {1}{2}", n, mParsRMF.FPathSlave[n], @"\") + "\r\n";
                string it = "";
                for (int n = 0; n < mParsRMF.NSLAVE; n++)
                    it = it + System.Convert.ToString(mParsRMF.RUNTIME) + " ";
                rmfs = rmfs + it.Trim() + "\r\n";
                for (int n = 0; n < mParsRMF.NSLAVE; n++)
                {
                    rmfs = rmfs + Path.Combine(mParsRMF.FPathSlave[n], Path.GetFileName(mParsPCF.n701_INFLE)) + "\r\n";
                    rmfs = rmfs + Path.Combine(mParsRMF.FPathSlave[n], Path.GetFileName(mParsPCF.n703_OUTFLE)) + "\r\n";
                }
                System.IO.File.AppendAllText(rmfFPN, rmfs);
                return true;
            }
            catch (Exception ex)
            {
                System.Console.WriteLine("An error was occurred in making rmf file.");
                System.Console.WriteLine(ex.ToString());
                return false;
            }
        }

        public static cPEST.ParameterDataVariables InitializeParameterDataVariables(cPEST.ParameterDataVariables pdataVars)
        {
            pdataVars.PARNME = new List<string>();
            pdataVars.PARVAL1 = new List<float>();
            pdataVars.PARLBND = new List<float>();
            pdataVars.PARUBND = new List<float>();
            return pdataVars;
        }

        public void RunPEST()
        {
            ThreadStart ts = new ThreadStart(RunPESTInner);
            Thread th = new Thread(ts);
            th.Start();
        }

        private void RunPESTInner()
        {
            if (mbPPEST == false)
                RunBatchCopy();
            System.Diagnostics.Process psPEST = new System.Diagnostics.Process();
            ProcessStartInfo psInfo = new ProcessStartInfo();
            psInfo.FileName = mFPNPestBatRun;
            if (mbShowConsole == true)
                psInfo.WindowStyle = ProcessWindowStyle.Normal;
            else
                psInfo.WindowStyle = ProcessWindowStyle.Hidden;
            psPEST.StartInfo = psInfo;
            psPEST.Start();
            psPEST.WaitForExit();
            psPEST.Dispose();
            if (mbPPEST == true)
                CloseSlaveProcess();
            Thread.Sleep(2000); // 프로세스 종료 완료 잠깐 시간지연, 관련 실행파일 삭제를 위해 
            PESTEnded(this);
        }

        public void RunPSlave()
        {
            ThreadStart ts = new ThreadStart(RunPSlaveInner);
            Thread th = new Thread(ts);
            th.Start();
        }

        public void RunPSlaveInner()
        {
            RunBatchCopy();
            RunSlave();
        }

        private void RunBatchCopy()
        {
            System.Diagnostics.Process psPEST = new System.Diagnostics.Process();
            ProcessStartInfo psInfo = new ProcessStartInfo();
            psInfo.FileName = mFPNPestBatCopy;
            psInfo.WindowStyle = ProcessWindowStyle.Hidden;
            psPEST.StartInfo = psInfo;
            psPEST.Start();
            psPEST.WaitForExit();
            psPEST.Dispose();
        }

        private void RunSlave()
        {
            mPsIDPSlave = new List<int>();
            if (mbPPEST == true)
            {
                for (int n = 0; n < mParsRMF.NSLAVE; n++)
                {
                    ProcessStartInfo psinfo_tmp = new ProcessStartInfo();
                    psinfo_tmp.FileName = mFPNPPestSlaveBat[n];
                    psinfo_tmp.WindowStyle = ProcessWindowStyle.Normal;
                    System.Diagnostics.Process ps_tmp = new System.Diagnostics.Process();
                    ps_tmp.StartInfo = psinfo_tmp;
                    ps_tmp.Start();
                    mPsIDPSlave.Add(ps_tmp.Id);
                }
            }
        }

        private void CloseSlaveProcess()
        {
            try
            {
                for (int n = 0; n < mPsIDPSlave.Count; n++)
                {
                    Process ps = Process.GetProcessById(mPsIDPSlave[n]);
                    ps.CloseMainWindow(); // grm 실행중인 창도 강제로 닫힌다.
                    ps.Close();
                    ps.Dispose();
                }
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }
    }
}
