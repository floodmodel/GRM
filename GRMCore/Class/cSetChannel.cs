using System.Drawing;

namespace GRMCore
{
    public class cSetChannel
    {
        public static readonly Color DEFAULT_USER_CHANNEL_WIDTH_COLOR = Color.Cyan;
        public cSetCrossSection mCrossSection;
        public double mRightBankSlope;
        public double mLeftBankSlope;
        public cSetCrossSection.CSTypeEnum mCrossSectionType;
        public Color mCellColor;

        public cSetChannel()
        {
            mRightBankSlope = 0;
            mLeftBankSlope = 0;
            mCrossSection = new cSetCSSingle();
        }

        public void GetValues(Dataset.GRMProject prjDB)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
            if (!row.IsCrossSectionTypeNull())
            {
                double v = 0;
                if (double.TryParse(row.BankSideSlopeLeft, out v) == true) { mLeftBankSlope = v; }
                if (double.TryParse(row.BankSideSlopeRight, out v) == true) { mRightBankSlope = v; }
                if (row.CrossSectionType.ToString() == cSetCrossSection.CSTypeEnum.CSCompound.ToString())
                {
                    mCrossSection = new cSetCSCompound();
                }
                else
                {
                    mCrossSection = new cSetCSSingle();
                }
                mCrossSection.GetValues(prjDB);
            }
        }

        public bool IsSet
        {
            get
            {
                if (mRightBankSlope > 0)
                { return true; }
                else
                { return false; }
            }
        }

        public void SetValues(Dataset.GRMProject prjDB)
        {
            if (IsSet)
            {
                Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
                row.BankSideSlopeRight = mRightBankSlope.ToString();
                row.BankSideSlopeLeft = mLeftBankSlope.ToString();
                mCrossSection.SetValues(prjDB);
            }
        }

        public Color CellColor
        {
            get
            {
                return mCellColor;
            }
        }
    }
}
