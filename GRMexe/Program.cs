
using System.IO;
using System.Text.RegularExpressions;
using gentle;
using System.Text;
using System;
using System.Linq;
using System.Diagnostics;
using System.Collections.Generic;
using GRMCore;


namespace GRMexe
{
    public class Program
    {

        private GRMCore.cSimulator mSimulator;
        private string mMessage;
        private int mSimDurationHour;
        private GRMCore.cRasterOutput mRaterFileOutput;
        private bool mbCreateDistributionFiles = false;
        private string projectPath;
        private string mFileInfoLogExe;
        private string mFileInfoLogCore;
        private string mFileInfoLogGentle;
        private string mFileInfos;
        //static 

        static void Main(string[] args)
        {
            Program MainRun = new Program();
            MainRun.RunGRM(args);
        }


        private void RunGRM(string[] args)
        {
            try
            {
                System.Reflection.Assembly aExe = System.Reflection.Assembly.GetExecutingAssembly();
                string fp = Path.GetDirectoryName(aExe.Location);
                string fpnCore = Path.Combine(fp, "GRMCore.dll");
                string fpnGentle = Path.Combine(fp, "gentle.dll");
                projectPath = fp;
                //System.Reflection.Assembly aCore = System.Reflection.Assembly.LoadFrom(fpnCore);
                //System.Reflection.AssemblyName anExe = aExe.GetName();
                //System.Reflection.AssemblyName anCore = aCore.GetName();


                FileVersionInfo fviExe = FileVersionInfo.GetVersionInfo(aExe.Location);
                FileVersionInfo fviCore = FileVersionInfo.GetVersionInfo(fpnCore);
                FileVersionInfo fviGentle = FileVersionInfo.GetVersionInfo(fpnGentle);
                //string fvExe = anExe.Version.Major.ToString() + "."
                //    + anExe.Version.Minor.ToString() + "." + anExe.Version.Build.ToString();
                //string fvCore = anCore.Version.Major.ToString() + "."
                //    + anCore.Version.Minor.ToString() + "." + anCore.Version.Build.ToString();
                string fvExe = fviExe.FileVersion.ToString();
                string fvCore = fviCore.FileVersion.ToString();
                string fvGentle = fviGentle.FileVersion.ToString();
                FileInfo fiExe = new FileInfo(aExe.Location);
                FileInfo fiCore = new FileInfo(fpnCore);
                FileInfo fiGentle = new FileInfo(fpnGentle);
                //mFileInfoLogExe = string.Format("{0} v{1}. Built in {2}",
                //    anExe.Name, fvExe, fiExe.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
                //mFileInfoLogCore = string.Format("{0} v{1}. Built in {2}",
                //    anCore.Name, fvCore, fiCore.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
                mFileInfoLogExe = string.Format("{0} v{1}. Built in {2}",
                   "GRM.exe", fvExe, fiExe.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
                mFileInfoLogCore = string.Format("{0} v{1}. Built in {2}",
                   "GRMCore.dll", fvCore, fiCore.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
                mFileInfoLogGentle = string.Format("{0} v{1}. Built in {2}",
                    "gentle.dll", fvGentle, fiGentle.LastWriteTime.ToString("yyyy-MM-dd HH:mm"));
                Console.Write(mFileInfoLogExe + "\r\n");
                Console.Write(mFileInfoLogCore + "\r\n");
                Console.Write(mFileInfoLogGentle + "\r\n");

                string prjFPN = "";

                if (args.Length == 0)
                {
                    Console.Write("G2D project file was not entered or invalid arguments. ");
                    GRMconsoleHelp();
                    return;
                }

                if (args.Length == 1)
                {
                    if (args[0] == "/?")
                    {
                        GRMconsoleHelp();
                        return;
                    }
                    if (File.Exists(args[0].Trim()) == true)
                    {
                        prjFPN = args[0].Trim();

                        StartSingleRun(prjFPN, false);
                        return;
                    }
                    else
                    {
                        Console.WriteLine("Current project file is inavlid!!!");
                        waitUserKey();
                        return;
                    }
                }

                if (args.Length == 2)
                {
                    string arg0 = args[0].Trim();
                    string arg1 = args[1].Trim();
                    if (arg0.ToLower() == "/f" || arg0.ToLower() == "/fd")
                    {
                        if (Directory.Exists(arg1))
                        {
                            string[] gmpFiles = GetGMPFileList(arg1);
                            if (gmpFiles.Length == 0)
                            {
                                Console.WriteLine("There is no GRM project file in this directory.");
                                waitUserKey();
                                return;
                            }
                            bool bDelete = false;
                            switch (arg0.ToLower())
                            {
                                case "/f":
                                    {
                                        bDelete = false;
                                        break;
                                    }

                                case "/fd":
                                    {
                                        bDelete = true;
                                        break;
                                    }
                            }

                            for (int n = 0, loopTo = gmpFiles.Length - 1; n <= loopTo; n++)
                            {
                                if (File.Exists(gmpFiles[n]) == true)
                                {
                                    mMessage = string.Format("Total progress: {0}/{1} ({2}%). ", n + 1,
                                        gmpFiles.Length, ((n + 1) / (double)gmpFiles.Length * 100).ToString("#0.00"));
                                    StartSingleRun(gmpFiles[n], bDelete);
                                }
                                else
                                    Console.WriteLine(string.Format("{0} file is not exist.", gmpFiles[n]));
                                if ((n + 1) % 100 == 0)
                                { Console.Clear(); }
                            }
                            return;
                        }
                        else
                        {
                            Console.WriteLine("Project folder is invalid!!");
                            waitUserKey();
                            return;
                        }
                    }
                    else if (File.Exists(arg0) == true)
                    {
                        StartSingleRun(arg0, false);
                        return;
                    }
                }
                Console.WriteLine("Invalid command. Use /? argument for help!!!");
                waitUserKey();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex);
                waitUserKey();
            }
        }

        private void StartSingleRun(string currentPrjFPN, bool bDeleteFilesExceptQ = false)
        {
            //    ''여기서 셀 정보를 미리 알 수 있다.
            //'Dim WSFPN As String = "D:/Nakdong/watershed/ND_Watershed.asc"
            //'Dim SlopeFPN As String = "D:/Nakdong/watershed/ND_Slope.asc"
            //'Dim FdirFPN As String = "D:/Nakdong/watershed/ND_Fdr.asc"
            //'Dim FacFPN As String = "D:/Nakdong/watershed/ND_Fac.asc"
            //'Dim streamFPN As String = "D:/Nakdong/watershed/ND_stream.asc"
            //'Dim lcFPN As String = "D:/Nakdong/watershed/ND_lc.asc"
            //'Dim stFPN As String = "D:/Nakdong/watershed/ND_stexture.asc"
            //'Dim sdFPN As String = "D:/Nakdong/watershed/ND_sdepth.asc"

            //'Dim wsinfo As New cGetWatershedInfo(cGRM.FlowDirectionType.StartsFromE_TauDEM.ToString, WSFPN, SlopeFPN, FdirFPN, FacFPN, streamFPN, lcFPN, stFPN, sdFPN,,)
            //    ''Dim cc As Integer = wsinfo.cellCountInWatershed
            //''Dim a As Integer = wsinfo.WSIDsAll.Count
            //''Dim aa As List(Of Integer) = wsinfo.upStreamWSIDs(1)
            //'Dim cs As Single = wsinfo.cellSize

            List<string> wpNames = new List<string>();
            if (Path.GetDirectoryName(currentPrjFPN) == "")
            {
                currentPrjFPN = Path.Combine(projectPath, currentPrjFPN);
            }
            string fpnLog = Path.Combine(Path.GetDirectoryName(currentPrjFPN), Path.GetFileNameWithoutExtension(currentPrjFPN) + ".log");
            if (File.Exists(fpnLog) == true)
            { File.Delete(fpnLog); }

            try
            {
                cProject.OpenProject(currentPrjFPN, false);
                cGRM.writelogAndConsole(mFileInfoLogExe, true, false);
                cGRM.writelogAndConsole(mFileInfoLogCore, true, false);
                cGRM.writelogAndConsole(mFileInfoLogGentle, true, false);
                //'cGRM.writelogAndConsole(mFileInfos, true, false)
                cProject.ValidateProjectFile(cProject.Current);
                mSimDurationHour = (int)(cProject.Current.generalSimulEnv.mSimDurationHOUR);
                if (cProject.Current.SetupModelParametersAfterProjectFileWasOpened() == false)
                {
                    cGRM.writelogAndConsole("GRM setup was failed !!!", true, true);
                    return;
                }

                if (cOutPutControl.CreateNewOutputFiles(cProject.Current, false) == false)// 기존의 single 이벤트에서의 출력파일을 지운다.
                {
                    cGRM.writelogAndConsole("Making new output files were failed !!!", true, true);
                }


                //''여기서 셀 정보를 미리 알 수 있다.
                //'Dim wsinfo As New cGetWatershedInfo(currentPrjFPN)
                //'Dim mdwsid As List(Of Integer) = wsinfo.mostDownStreamWSIDs
                //'Dim uswsid As List(Of Integer) = wsinfo.

                //'Dim slp As Single = wsinfo.subwatershedPars(1).minSlopeChBed
                //'Dim ukk As String = wsinfo.subwatershedPars(1).UKType
                //'Dim cc As Integer = wsinfo.cellCountInWatershed
                //'wsinfo.UpdateAllSubWatershedParametersUsingNetwork()

                //'Dim aa As Single = wsinfo.subwatershedPars(2).coefUK
                //'Dim a As Integer = 1
                //''혹은 아래의 방법
                //'Dim WSFPN As String = cProject.Current.Watershed.mFPN_watershed
                //'Dim SlopeFPN As String = cProject.Current.Watershed.mFPN_slope
                //'Dim FdirFPN As String = cProject.Current.Watershed.mFPN_fdir
                //'Dim FacFPN As String = cProject.Current.Watershed.mFPN_fac
                //'Dim streamFPN As String = cProject.Current.Watershed.mFPN_stream
                //'Dim lcFPN As String = cProject.Current.Landcover.mGridLandCoverFPN
                //'Dim stFPN As String = cProject.Current.GreenAmpt.mGridSoilTextureFPN
                //'Dim sdFPN As String = cProject.Current.SoilDepth.mGridSoilDepthFPN
                //'wsinfo.SetOneSWSParametersAndUpdateAllSWSUsingNetwork(1, 1, 0.0001, 0.0001, 30, 0.045, 0, 1, 1, 1, 1, 1, true, 30)
                //'Dim v As Single = wsinfo.grmPrj.SubWSPar.userPars(1).iniSaturation
                //'Dim vv As Single = wsinfo.subwatershedPars(1).iniSaturation
                //'Dim x As Integer = wsinfo.mostDownStreamCellArrayXColPosition
                //'Dim y As Integer = wsinfo.mostDownStreamCellArrayYRowPosition
                //'Dim isIn As Boolean = wsinfo.IsInWatershedArea(x, y)
                //'Dim array() As String = wsinfo.allCellsInUpstreamArea(x, y)
                //'Dim fd As String = wsinfo.flowDirection(x, y)
                //'Dim lcv As Integer = wsinfo.landCoverValue(x, y)
                //'Dim wsCount As Integer = wsinfo.WScount
                //'Dim intStreamValue As Integer = wsinfo.streamValue(x, y)

                if (cProject.Current.generalSimulEnv.mbCreateASCFile == true ||
                    cProject.Current.generalSimulEnv.mbCreateImageFile == true)
                {
                    mbCreateDistributionFiles = true;
                    mRaterFileOutput = new cRasterOutput(cProject.Current);
                }

                cGRM.writelogAndConsole(currentPrjFPN + " -> Model setup completed.", true, true);
                foreach (GRMCore.Dataset.GRMProject.WatchPointsRow row in cProject.Current.watchPoint.mdtWatchPointInfo)
                {
                    wpNames.Add(row.Name);
                }

                mSimulator = new cSimulator();
                mSimulator.SimulationStep += new cSimulator.SimulationStepEventHandler(simulationStep);
                mSimulator.SimulationComplete += new cSimulator.SimulationCompleteEventHandler(simulationCompleted);
                mSimulator.SimulationRaiseError += new cSimulator.SimulationRaiseErrorEventHandler(simulationRaiseError);
                mSimulator.MakeRasterOutput += new cSimulator.MakeRasterOutputEventHandler(makeRasterOutput);
                mSimulator.SimulateSingleEvent(cProject.Current);
                //'삭제 대상
                //'IO.File.AppendAllText(cThisSimulation.tmp_InfoFile, String.Format("{0}", currentPrjFPN + vbTab))
                //'IO.File.AppendAllText(cThisSimulation.tmp_InfoFile, String.Format("{0}{1}{2}{3}{4}{5}", cThisSimulation.MaxDegreeOfParallelism, vbTab, cThisSimulation.tmp_24H_RunTime, vbTab, cThisSimulation.tmp_48H_RunTime, vbCrLf))
                //''''''''''''''''''''''''''''
                //'cProject.Current.SaveProject()
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                cGRM.writelogAndConsole(String.Format("Starting GRM project ({0}) is failed !!!", currentPrjFPN), true, true);
                throw (ex);
            }

            try
            {
                if (bDeleteFilesExceptQ == true)
                {
                    string prjPathOnly = Path.GetDirectoryName(currentPrjFPN);
                    string prjNameOnlyWithoutExtsion = Path.GetFileNameWithoutExtension(currentPrjFPN);
                    string fNameTmp = "";
                    File.Delete(currentPrjFPN);
                    fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "Depth.out");
                    File.Delete(fNameTmp);
                    fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "RFGrid.out");
                    File.Delete(fNameTmp);
                    fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "RFUpMean.out");
                    File.Delete(fNameTmp);
                    foreach (string wpname in wpNames)
                    {
                        fNameTmp = Path.Combine(prjPathOnly, prjNameOnlyWithoutExtsion + "WP_" + wpname + ".out");
                        File.Delete(fNameTmp);
                    }

                    File.Delete(currentPrjFPN);

                }
                cProject.Current.Dispose();
                GC.Collect();
            }
            catch (Exception ex)
            {
                Console.WriteLine(String.Format("ERROR : [{0}] project could not be deleted!!!", currentPrjFPN));
                throw (ex);
            }
        }


        private string[] GetGMPFileList(string path)

        {

            List<string> fileList = new List<string>();
            NaturalComparer naCom = new NaturalComparer();
            fileList = ((string[])Directory.GetFiles(path, "*.gmp")).ToList();
            fileList.Sort(naCom);
            return fileList.ToArray();
        }


        private void waitUserKey()
        {
            Console.Write("Press any key to continue..");
            Console.ReadKey();
        }

        private static void GRMconsoleHelp()
        {
            Console.WriteLine();
            Console.WriteLine("GRM v2018.09.");
            Console.WriteLine("Usage : GRM [Current project file full path and name to simulate]");
            Console.WriteLine();
            Console.WriteLine("**사용법");
            Console.WriteLine("1. GRM 모형의 project 파일(.gmp)과 입력자료(지형공간자료, 강우, flow control 시계열 자료)를 준비한다.");
            Console.WriteLine("2. 모델링 대상 프로젝트 이름을 스위치(argument)로 넣고 실행한다.");
            Console.WriteLine("4. Console에서 grm.exe [argument] 로 실행한다.");
            Console.WriteLine("5. ** 주의사항 : 장기간 모의할 경우, 컴퓨터 업데이트로 인해, 종료될 수 있으니, 네트워크 차단, 자동업데이트 하지 않음으로 설정한다.");
            Console.WriteLine("6. [argument]");
            Console.WriteLine("- /?");
            Console.WriteLine("       도움말");
            Console.WriteLine("- 프로젝트 파일경로와 이름");
            Console.WriteLine("        grm을 파일단위로 실행시킨다.");
            Console.WriteLine("        이때 full path, name을 넣어야 하지만, ");
            Console.WriteLine("        대상 프로젝트 파일이 grm.exe 파일과 동일한 폴더에 있을 경우에는, path는 입력하지 않아도 된다.");
            Console.WriteLine("        또한 대상 프로젝트 이름과 경로에 공백이 포함될 경우 큰따옴표로 묶어서 입력한다.");
            Console.WriteLine("         ** 예문(grm.exe가 d://GRMrun에 있을 경우)");
            Console.WriteLine("             - grm.exe와 다른 폴더에 프로젝트 파일이 있을 경우");
            Console.WriteLine("               d://GRMrun>grm D://GRMTest//TestProject\test.gmp");
            Console.WriteLine("             - grm.exe와 같은 폴더에 프로젝트 파일이 있을 경우");
            Console.WriteLine("               d://GRMrun>grm test.gmp");
            Console.WriteLine("- /f 폴더경로");
            Console.WriteLine("         grm을 폴더 단위로 실행시킨다.");
            Console.WriteLine("          ** 예문 : grm /f d://GRMrun//TestProject");
            Console.WriteLine("- /fd 폴더경로");
            Console.WriteLine("         grm을 폴더 단위로 실행시킨다.");
            Console.WriteLine("         유량 모의결과인 *discharge.out을 제외한 파일을 지운다(*.gmp, *Depth.out, 등등...)");
            Console.WriteLine("          ** 예문 : grm /fd d://GRMrun//TestProject");
        }


        private void simulationStep(int elapsedMinutes)
        {
            double nowStep;
            string currentProgress = "";
            double simDur_min = mSimDurationHour * 60.0;
            nowStep = elapsedMinutes / simDur_min * 100;
            currentProgress = nowStep.ToString("#0");
            Console.Write("\r" + "Current progress: " + currentProgress + "%. " + mMessage);
            cGRM.writelogAndConsole(string.Format("Current step (min) : {0} / {1}", elapsedMinutes, simDur_min), cGRM.bwriteLog, false);
        }

        private void simulationCompleted()
        {
            cGRM.writelogAndConsole("Simulation was completed!!", true, true);
        }

        private void simulationRaiseError(cSimulator.SimulationErrors simulError, object erroData)
        {
            Console.WriteLine("Simulation error!!!");
            Console.WriteLine(simulError.ToString());
            Console.WriteLine(erroData.ToString());
        }

        private void makeRasterOutput(int nowTtoPrint_MIN)
        {
            if (mbCreateDistributionFiles == true)
                mRaterFileOutput.MakeDistributionFiles(nowTtoPrint_MIN, 
                    mRaterFileOutput.ImgWidth, mRaterFileOutput.ImgHeight, true);
        }
    }
}
