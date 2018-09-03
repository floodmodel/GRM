using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Data;
using System.Windows.Forms;

namespace gentle
{
    public class cFile
    {
        public enum FilePattern
        {
            TEXTFILE,
            TIFFILE,
            ASCFILE,
            ALLFILE,
            OUT_Discharge
        };

        private string[] mSourceFileList;
        private List<string> mNotDupFileList;

        public static bool ConfirmDeleteFiles(List<string> FilePathNames)
        {
            bool bAlldeleted = false;
            int n = 0;
            while (!(bAlldeleted == true))
            {
                n += 1;
                foreach (string fpn in FilePathNames)
                {
                    if (File.Exists(fpn) == true)
                    {
                        File.Delete(fpn);
                    }
                }
                foreach (string fpn in FilePathNames)
                {
                    if (File.Exists(fpn) == false)
                    {
                        bAlldeleted = true;
                    }
                    else
                    {
                        bAlldeleted = false;
                        break;
                    }
                }
                if (n > 100)
                    return false;
            }
            return true;
        }


        public static bool ConfirmDeleteFiles(string FilePathNames)
        {
            bool bAlldeleted = false;
            int n = 0;
            while (!(bAlldeleted == true))
            {
                n += 1;
                if (File.Exists(FilePathNames))
                {
                    File.Delete(FilePathNames);
                }
                if (File.Exists(FilePathNames) == false)
                {
                    bAlldeleted = true;
                }
                else
                {
                    bAlldeleted = false;
                }
                if (n > 100)
                    return false;
            }
            return true;
        }

        public static bool ConfirmDeleteDirectory(List<string> DirectoryPath)
        {
            bool bAlldeleted = false;
            while (!(bAlldeleted == true))
            {
                foreach (string dp in DirectoryPath)
                {
                    if (Directory.Exists(dp))
                    {
                        Directory.Delete(dp, true);
                    }
                }
                foreach (string dp in DirectoryPath)
                {
                    if (Directory.Exists(dp) == false)
                    {
                        bAlldeleted = true;
                    }
                    else
                    {
                        bAlldeleted = false;
                        return false;
                    }
                }
            }
            return true;
        }


        public static bool ConfirmCreateDirectory(List<string> DirectoryPath)
        {
            bool bAllCreated = false;
            while (!(bAllCreated == true))
            {
                foreach (string dpn in DirectoryPath)
                {
                    Directory.CreateDirectory(dpn);
                }
                foreach (string dpn in DirectoryPath)
                {
                    if (Directory.Exists(dpn) == true)
                    {
                        bAllCreated = true;
                    }
                    else
                    {
                        bAllCreated = false;
                        return false;
                    }
                }
            }
            return true;
        }

        public static void RefreshOrderInDataTable(DataTable dt, string OrderColumeName)
        {
            for (int i = 0; i <= dt.Rows.Count - 1; i++)
            {
                dt.Rows[i][OrderColumeName] = i + 1;
            }
        }

        public string[] GetFilesAndCheckDuplication(ListBox TargetListBox, bool bSelected, string NowDirPath, DataTable dtTarget, string ColNameFileName, string ColNameFilePath)
        {
            if (bSelected)
            {
                mSourceFileList = new string[TargetListBox.SelectedItems.Count];
                for (int i = 0; i <= mSourceFileList.Length - 1; i++)
                {
                    mSourceFileList[i] = Convert.ToString(TargetListBox.SelectedItems[i]);
                }
            }
            else
            {
                mSourceFileList = new string[TargetListBox.Items.Count];
                for (int i = 0; i <= mSourceFileList.Length - 1; i++)
                {
                    mSourceFileList[i] = Convert.ToString(TargetListBox.Items[i]);
                }
            }

            mNotDupFileList = new List<string>();
            foreach (string fileName in mSourceFileList)
            {
                string fullPath = Path.Combine(NowDirPath, fileName).ToLower();
                bool bFound = false;
                foreach (DataRow row in dtTarget.Rows)
                {
                    string aPath = Path.Combine(row[ColNameFilePath].ToString(), row[ColNameFileName].ToString()).ToLower();
                    if (fullPath == aPath)
                    {
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)
                    mNotDupFileList.Add(fileName);
            }
            return mNotDupFileList.ToArray();
        }

        public DialogResult DuplicationfileProcessMessage
        {
            get
            {
                if (mNotDupFileList.Count == 0)
                {
                    MessageBox.Show("All of the selected files are already added.", " Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return DialogResult.Cancel;
                }
                else if (mNotDupFileList.Count != mSourceFileList.Length)
                {
                    DialogResult mbr = MessageBox.Show("Some of the selected files are already added." + "\r\n" +
                        "Do you want to add not duplicated files(Skips duplicated files)?", "Message", MessageBoxButtons.OKCancel, MessageBoxIcon.Exclamation);
                    return mbr;
                }
                return DialogResult.Ignore;
            }
        }


        public static List<string> GetFileList(string folderPath, cFile.FilePattern filePattern)
        {
            List<string> lstFLselected = new List<string>();
            string[] lstFLallinSourcePath = Directory.GetFiles(folderPath);

            switch (filePattern)
            {
                case cFile.FilePattern.TEXTFILE:
                    foreach (string rfFN in lstFLallinSourcePath)
                    {
                        if (Path.GetExtension(rfFN).ToLower() == ".txt" || Path.GetExtension(rfFN).ToLower() == ".mrf")
                        {
                            lstFLselected.Add(Path.GetFileName(rfFN));
                        }
                    }
                    break;
                case cFile.FilePattern.TIFFILE:
                    foreach (string rfFN in lstFLallinSourcePath)
                    {
                        if (Path.GetExtension(rfFN).ToLower() == ".tif")
                        {
                            lstFLselected.Add(Path.GetFileName(rfFN));
                        }
                    }
                    break;
                case cFile.FilePattern.ASCFILE:
                    foreach (string rfFN in lstFLallinSourcePath)
                    {
                        if (Path.GetExtension(rfFN).ToLower() == ".asc")
                        {
                            lstFLselected.Add(Path.GetFileName(rfFN));
                        }
                    }
                    break;
                case cFile.FilePattern.OUT_Discharge:
                    foreach (string rfFN in lstFLallinSourcePath)
                    {
                        if (rfFN.Contains("discharge.out") == true)
                        {
                            lstFLselected.Add(Path.GetFileName(rfFN));
                        }
                    }
                    break;
                case cFile.FilePattern.ALLFILE:
                    foreach (string rfFN in lstFLallinSourcePath)
                    {
                        lstFLselected.Add(Path.GetFileName(rfFN));
                    }
                    break;

                default:
                    foreach (string rfFN in lstFLallinSourcePath)
                    {
                        if (rfFN.Contains(filePattern.ToString()) == true)
                        {
                            lstFLselected.Add(Path.GetFileName(rfFN));
                        }
                    }
                    break;
            }
            return lstFLselected;
        }

        public static List<string> GetFileList(string folderPath, string filePattern)
        {
            List<string> lstFLselected = new List<string>();
            string[] lstFLallinSourcePath = Directory.GetFiles(folderPath);
            if (filePattern != "")
            {
                foreach (string rfFN in lstFLallinSourcePath)
                {
                    if (rfFN.Contains(filePattern.ToString()) == true)
                    {
                        lstFLselected.Add(Path.GetFileName(rfFN));
                    }
                }
            }
            else
            {
                lstFLselected = lstFLallinSourcePath.ToList();
            }
            return lstFLselected;
        }


        public void AddToDataTable(string[] items, string SourceFilePath, DataTable dtTarget, string NameCol1, string NameCol2, string NameCol3)
        {
            Array.Sort(items);
            int intFileCount = 0;
            int intRecordCountOri = 0;
            intFileCount = items.Length;
            intRecordCountOri = dtTarget.Rows.Count;
            gentle.Dialog.fProgressBar ofrmPrograssBar = new gentle.Dialog.fProgressBar();
            ofrmPrograssBar.GRMPrograssBar.Maximum = intFileCount;
            ofrmPrograssBar.GRMPrograssBar.Style = ProgressBarStyle.Blocks;
            ofrmPrograssBar.labGRMToolsPrograssBar.Text = "Add 0/" + Convert.ToString(items.Length) + "files...";
            ofrmPrograssBar.Text = "Add files";
            ofrmPrograssBar.Show();
            System.Windows.Forms.Application.DoEvents();
            dtTarget.BeginLoadData();
            for (int i = 0; i <= items.Length - 1; i++)
            {
                string strFNameOnly = Path.GetFileName(items[i]);
                DataRow nr = dtTarget.NewRow();

                nr[dtTarget.Columns.IndexOf(NameCol1)] = intRecordCountOri + i + 1;
                nr[dtTarget.Columns.IndexOf(NameCol2)] = strFNameOnly;
                nr[dtTarget.Columns.IndexOf(NameCol3)] = SourceFilePath;
                dtTarget.Rows.Add(nr);
                ofrmPrograssBar.GRMPrograssBar.Value = i + 1;
                ofrmPrograssBar.labGRMToolsPrograssBar.Text = "Add " + Convert.ToString(i + 1) + "/" + Convert.ToString(intFileCount) + " files...";
                System.Windows.Forms.Application.DoEvents();
            }
            dtTarget.EndLoadData();
            ofrmPrograssBar.Close();
        }

        public static bool IsFileLocked(string filePath, int checkingDuration_sec)
        {
            bool isLocked = true;
            int iter_sec = 0;
            checkingDuration_sec = checkingDuration_sec * 10;
            while (isLocked == true && ((iter_sec < checkingDuration_sec) || (checkingDuration_sec == 0)))
            {
                try
                {
                    return false;
                }
                catch (IOException e)
                {
                    iter_sec += 1;
                    if (checkingDuration_sec != 0)
                    {
                        System.Threading.ManualResetEvent mre = new System.Threading.ManualResetEvent(false);
                        mre.WaitOne(100);
                    }
                    throw (e);
                }
            }
            return isLocked;
        }

        //public static string GetAbsolutePathOneUpper(string PathToChange)
        //{
        //    //string outPathName = "";

        //    try
        //    {
        //        int len = PathToChange.Length;
        //        string p = null;
        //        for (int n = len; n >= 1; n += -1)
        //        {
        //            p = PathToChange.Substring(n-1, 1);
        //            if (p == "\\")
        //            {
        //                PathToChange = PathToChange.Substring(1, n - 1);
        //                break; // TODO: might not be correct. Was : Exit For
        //            }
        //        }
        //        return PathToChange.Trim();
        //    }
        //    catch (Exception ex)
        //    {
        //        throw ex;
        //    }
        //}

        public static bool DeleteFileFriends(string sourceFPN)
        {
            try
            {
                string fnWOe = Path.GetFileNameWithoutExtension(sourceFPN);
                string fnOnly = Path.GetFileName(sourceFPN);
                string fp = Path.GetDirectoryName(sourceFPN);
                string[] files = Directory.GetFiles(fp);
                for (int n = 0; n <= files.Length - 1; n++)
                {
                    if (Path.GetFileNameWithoutExtension(files[n]) == fnWOe && files[n] != sourceFPN)
                    {
                        File.Delete(Path.Combine(fp, files[n]));
                    }
                }
                File.Delete(Path.Combine(fp, fnWOe + ".asc.aux.xml"));
                return true;
            }
            catch (Exception ex)
            {
                return false;
                throw ex;
            }
        }
    }
}
