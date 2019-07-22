//using System.Drawing;
using System.Collections.Generic;

namespace GRMCore
{
    public class cSetChannel
    {
        //===================================================================
        public Dictionary<int, cSetCrossSection> CrossSections = new Dictionary<int, cSetCrossSection>();
        //public cSetCrossSection mCrossSection;

        public cSetChannel()
        {
            //mCrossSection = new cSetCSSingle();
        }

        public void GetValues(Dataset.GRMProject prjDB)
        {
            int rowCount = prjDB.ChannelSettings.Rows.Count;

            for (int nr = 0; nr < rowCount; nr++)
            {
                Dataset.GRMProject.ChannelSettingsRow row
                    = (Dataset.GRMProject.ChannelSettingsRow)prjDB.ChannelSettings.Rows[nr];
                cSetCrossSection aCrossSection;
                if (!row.IsCrossSectionTypeNull())
                {

                    if (row.CrossSectionType.ToString() == cSetCrossSection.CSTypeEnum.CSCompound.ToString())
                    {
                        aCrossSection = new cSetCSCompound();
                    }
                    else
                    {
                        aCrossSection = new cSetCSSingle();
                    }
                    aCrossSection.GetValues(row);
                    CrossSections.Add(row.WSID, aCrossSection);
                }
            }
        }

        public bool IsSet
        {
            get
            {
                if (CrossSections.Count > 0)
                { return true; }
                else
                { return false; }
            }
        }

        public void SetValues(Dataset.GRMProject prjDB)
        {
            if (IsSet)
            {
                int idx = 0;
                foreach (int k in CrossSections.Keys)
                {
                    CrossSections[k].SetValues(prjDB, idx);
                }
                idx++;
            }
        }

        //===================================================================


        ////===================================================================
        ////public Dictionary<int, cUserParameters> userPars = new Dictionary<int, cUserParameters>();

        ////public static readonly Color DEFAULT_USER_CHANNEL_WIDTH_COLOR = Color.Cyan;
        //public cSetCrossSection mCrossSection;
        ////public double mRightBankSlope;
        ////public double mLeftBankSlope;
        ////public cSetCrossSection.CSTypeEnum mCrossSectionType;
        ////public Color mCellColor;


        //public cSetChannel()
        //{
        //    //mRightBankSlope = 0;
        //    //mLeftBankSlope = 0;
        //    mCrossSection = new cSetCSSingle();
        //}

        //public void GetValues(Dataset.GRMProject prjDB)
        //{
        //    Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
        //    if (!row.IsCrossSectionTypeNull())
        //    {
        //        //double v = 0;
        //        //if (double.TryParse(row.BankSideSlopeLeft, out v) == true) { mLeftBankSlope = v; }
        //        //if (double.TryParse(row.BankSideSlopeRight, out v) == true) { mRightBankSlope = v; }
        //        if (row.CrossSectionType.ToString() == cSetCrossSection.CSTypeEnum.CSCompound.ToString())
        //        {
        //            mCrossSection = new cSetCSCompound();
        //        }
        //        else
        //        {
        //            mCrossSection = new cSetCSSingle();
        //        }
        //        mCrossSection.GetValues(prjDB);
        //    }
        //}


        //public bool IsSet
        //{
        //    get
        //    {
        //        if (mCrossSection.RightBankSlope > 0)
        //        { return true; }
        //        else
        //        { return false; }
        //    }
        //}


        //public void SetValues(Dataset.GRMProject prjDB)
        //{
        //    if (IsSet)
        //    {
        //        Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
        //        row.BankSideSlopeRight = mCrossSection.RightBankSlope.ToString();
        //        row.BankSideSlopeLeft = mCrossSection.LeftBankSlope.ToString();
        //        mCrossSection.SetValues(prjDB);
        //    }

        ////===================================================================



        //public Color CellColor
        //{
        //    get
        //    {
        //        return mCellColor;
        //    }
        //}
    }
}
