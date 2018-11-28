using System;

namespace GRMCore
{
    public class cSetCSCompound : cSetCrossSection
    {

        /// <summary>
        /// 저수부의 하폭[m]
        /// </summary>
        /// <remarks></remarks>
        public double mLowerRegionBaseWidth;

        /// <summary>
        /// 하상으로 부터 고수부가 시작되는 부분까지의 높이[m]
        /// </summary>
        /// <remarks></remarks>
        public double mLowerRegionHeight;

        /// <summary>
        /// 고수부의 하폭[m]
        /// </summary>
        /// <remarks></remarks>
        public double mUpperRegionBaseWidth;

        /// <summary>
        /// 이거 보다 큰 하폭에서만 복단면 적용한다.[m]
        /// </summary>
        /// <remarks></remarks>
        public double mCompoundCSCriteriaChannelWidth;

        public override cSetCrossSection.CSTypeEnum CSType
        {
            get
            {
                return CSTypeEnum.CSCompound;
            }
        }

        public override void GetValues(Dataset.GRMProject prjDB)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
            mLowerRegionBaseWidth = System.Convert.ToDouble(row.LowerRegionBaseWidth);
            mLowerRegionHeight = System.Convert.ToDouble(row.LowerRegionHeight);
            mUpperRegionBaseWidth = System.Convert.ToDouble(row.UpperRegionBaseWidth);
            mCompoundCSCriteriaChannelWidth = System.Convert.ToDouble(row.CompoundCSChannelWidthLimit);
            throw new NotImplementedException();
        }

        public override bool IsSet
        {
            get
            {
                return mLowerRegionBaseWidth != 0;
            }
        }

        public override void SetValues(Dataset.GRMProject prjDB)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
            row.CrossSectionType = CSTypeEnum.CSCompound.ToString();
            row.LowerRegionBaseWidth = System.Convert.ToString(mLowerRegionBaseWidth);
            row.LowerRegionHeight = System.Convert.ToString(mLowerRegionHeight);
            row.UpperRegionBaseWidth = System.Convert.ToString(mUpperRegionBaseWidth);
            row.CompoundCSChannelWidthLimit = System.Convert.ToString(mCompoundCSCriteriaChannelWidth);
            row.SingleCSChannelWidthType = null;
            row.ChannelWidthEQc = null;
            row.ChannelWidthEQd = null;
            row.ChannelWidthEQe = null;
            row.ChannelWidthMostDownStream = null;
            throw new NotImplementedException();
        }
    }
}
