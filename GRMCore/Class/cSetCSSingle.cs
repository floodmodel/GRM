using System;

namespace GRMCore
{
    public class cSetCSSingle : cSetCrossSection
    {
        public enum CSSingleChannelWidthType
        {
            CWEquation,
            CWGeneration
        }

        public Nullable<CSSingleChannelWidthType> mCSSingleWidthType;
        public double mCWEc;
        public double mCWEd;
        public double mCWEe;
        public double mMaxChannelWidthSingleCS;

        public override cSetCrossSection.CSTypeEnum CSType
        {
            get
            {
                return CSTypeEnum.CSSingle;
            }
        }

        public override void GetValues(Dataset.GRMProject.ChannelSettingsRow row)
        {
            //Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];

            if (row.SingleCSChannelWidthType == cSetCSSingle.CSSingleChannelWidthType.CWEquation.ToString())
            {
                mCSSingleWidthType = CSSingleChannelWidthType.CWEquation;
                mCWEc = System.Convert.ToDouble(row.ChannelWidthEQc);
                mCWEd = System.Convert.ToDouble(row.ChannelWidthEQd);
                mCWEe = System.Convert.ToDouble(row.ChannelWidthEQe);
            }
            else
            {
                mCSSingleWidthType = CSSingleChannelWidthType.CWGeneration;
                if (row.ChannelWidthMostDownStream != "")
                {
                    mMaxChannelWidthSingleCS = System.Convert.ToDouble(row.ChannelWidthMostDownStream);
                }
            }
            double v = 0;
            if (double.TryParse(row.BankSideSlopeLeft, out v) == true) { LeftBankSlope = v; }
            if (double.TryParse(row.BankSideSlopeRight, out v) == true) { RightBankSlope = v; }
        }

        public override bool IsSet
        {
            get
            {
                return mCSSingleWidthType.HasValue;
            }
        }
        public override void SetValues(Dataset.GRMProject prjDB, int rowidx)
        {
            Dataset.GRMProject.ChannelSettingsRow row =
                (Dataset.GRMProject.ChannelSettingsRow)prjDB.ChannelSettings.Rows[rowidx];
            row.CrossSectionType = CSTypeEnum.CSSingle.ToString();
            if (mCSSingleWidthType == cSetCSSingle.CSSingleChannelWidthType.CWEquation)
            {
                row.SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation.ToString();
                row.ChannelWidthEQc = System.Convert.ToString(mCWEc);
                row.ChannelWidthEQd = System.Convert.ToString(mCWEd);
                row.ChannelWidthEQe = System.Convert.ToString(mCWEe);
                row.ChannelWidthMostDownStream = null;
            }
            else if (mCSSingleWidthType == cSetCSSingle.CSSingleChannelWidthType.CWGeneration)
            {
                row.SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWGeneration.ToString();
                row.ChannelWidthMostDownStream = System.Convert.ToString(mMaxChannelWidthSingleCS);
                row.ChannelWidthEQc = null;
                row.ChannelWidthEQd = null;
                row.ChannelWidthEQe = null;
            }
            row.BankSideSlopeRight = RightBankSlope.ToString();
            row.BankSideSlopeLeft = LeftBankSlope.ToString();
            row.LowerRegionBaseWidth = null;
            row.LowerRegionHeight = null;
            row.UpperRegionBaseWidth = null;
            row.CompoundCSChannelWidthLimit = null;
        }


        //public override void GetValues(Dataset.GRMProject prjdb)
        //{
        //    Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];

        //    if (row.SingleCSChannelWidthType == cSetCSSingle.CSSingleChannelWidthType.CWEquation.ToString())
        //    {
        //        mCSSingleWidthType = CSSingleChannelWidthType.CWEquation;
        //        mCWEc = System.Convert.ToDouble(row.ChannelWidthEQc);
        //        mCWEd = System.Convert.ToDouble(row.ChannelWidthEQd);
        //        mCWEe = System.Convert.ToDouble(row.ChannelWidthEQe);
        //    }
        //    else
        //    {
        //        mCSSingleWidthType = CSSingleChannelWidthType.CWGeneration;
        //        if (row.ChannelWidthMostDownStream != "")
        //        {
        //            mMaxChannelWidthSingleCS = System.Convert.ToDouble(row.ChannelWidthMostDownStream);
        //        }
        //    }
        //    double v = 0;
        //    if (double.TryParse(row.BankSideSlopeLeft, out v) == true) { LeftBankSlope = v; }
        //    if (double.TryParse(row.BankSideSlopeRight, out v) == true) { RightBankSlope = v; }
        //}

        //public override void SetValues(Dataset.GRMProject prjDB)
        //{
        //    Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
        //    row.CrossSectionType = CSTypeEnum.CSSingle.ToString();
        //    if (mCSSingleWidthType == cSetCSSingle.CSSingleChannelWidthType.CWEquation)
        //    {
        //        row.SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWEquation.ToString();
        //        row.ChannelWidthEQc = System.Convert.ToString(mCWEc);
        //        row.ChannelWidthEQd = System.Convert.ToString(mCWEd);
        //        row.ChannelWidthEQe = System.Convert.ToString(mCWEe);
        //        row.ChannelWidthMostDownStream = null;
        //    }
        //    else if (mCSSingleWidthType == cSetCSSingle.CSSingleChannelWidthType.CWGeneration)
        //    {
        //        row.SingleCSChannelWidthType = cSetCSSingle.CSSingleChannelWidthType.CWGeneration.ToString();
        //        row.ChannelWidthMostDownStream = System.Convert.ToString(mMaxChannelWidthSingleCS);
        //        row.ChannelWidthEQc = null;
        //        row.ChannelWidthEQd = null;
        //        row.ChannelWidthEQe = null;
        //    }
        //    // .CompoundCSIniFlowDepth = Nothing
        //    row.LowerRegionBaseWidth = null;
        //    row.LowerRegionHeight = null;
        //    row.UpperRegionBaseWidth = null;
        //    row.CompoundCSChannelWidthLimit = null;
        //}
    }
}
