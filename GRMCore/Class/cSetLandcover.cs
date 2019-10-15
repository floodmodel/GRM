using System;
using System.Data;

namespace GRMCore
{
    public class cSetLandcover
    {
        public enum LandCoverCode
        {
            WATR,
            URBN,
            BARE,
            WTLD,
            GRSS,
            FRST,
            AGRL,
            USER,
            CONSTV,
            NULL
        }

        public Nullable<cGRM.FileOrConst> mLandCoverDataType;
        public string mGridLandCoverFPN;
        public string mLandCoverVATFPN;
        public Dataset.GRMProject.LandCoverDataTable mdtLandCoverInfo;
        public Nullable<double> mConstRoughnessCoefficient;
        public Nullable<double> mConstImperviousRatio;

        public cSetLandcover()
        {
        }

        public void SetValues(Dataset.GRMProject prjDB)
        {
            if (mLandCoverDataType.HasValue)
            {
                Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
                row.LandCoverDataType = mLandCoverDataType.Value.ToString();
                switch (mLandCoverDataType.Value)
                {
                    case cGRM.FileOrConst.File:
                        {
                            row.LandCoverFile = mGridLandCoverFPN;
                            row.LandCoverVATFile = mLandCoverVATFPN;
                            row.SetConstantRoughnessCoeffNull();
                            row.SetConstantImperviousRatioNull();
                            break;
                        }

                    case cGRM.FileOrConst.Constant:
                        {
                            row.SetLandCoverFileNull();
                            row.SetLandCoverVATFileNull();
                            row.ConstantRoughnessCoeff = System.Convert.ToString(mConstRoughnessCoefficient.Value);
                            row.ConstantImperviousRatio = System.Convert.ToString(mConstImperviousRatio.Value);
                            break;
                        }
                }
                if (mLandCoverDataType.Equals(cGRM.FileOrConst.File))
                {
                    mdtLandCoverInfo.AcceptChanges();
                    foreach (DataRow r in mdtLandCoverInfo.Rows)
                    {
                        r.SetAdded();
                    }
                }
            }
        }

        public void GetValues(Dataset.GRMProject prjDB)
        {
            mLandCoverDataType = default(cGRM.FileOrConst?);
            mLandCoverVATFPN = null;
            mConstRoughnessCoefficient = default(Single?);
            mConstImperviousRatio = default(Single?);
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjDB.ProjectSettings.Rows[0];
            if (!row.IsLandCoverDataTypeNull())
            {
                if (row.LandCoverDataType.ToLower() == cGRM.FileOrConst.File.ToString().ToLower())
                {
                    mLandCoverDataType = cGRM.FileOrConst.File;
                    mGridLandCoverFPN = row.LandCoverFile;
                    mLandCoverVATFPN = row.LandCoverVATFile;
                }
                else if (row.LandCoverDataType.ToLower() == cGRM.FileOrConst.Constant.ToString().ToLower())
                {
                    mLandCoverDataType = cGRM.FileOrConst.Constant;
                    double v = 0;
                    if (double.TryParse(row.ConstantImperviousRatio, out v)) { mConstImperviousRatio = v; }
                    else if (double.TryParse(row.ConstantRoughnessCoeff, out v)) { mConstRoughnessCoefficient = v; }
                }
                else
                {
                    throw new InvalidOperationException();
                }
            }

            if (mLandCoverDataType.Equals(cGRM.FileOrConst.File))
            {
                mdtLandCoverInfo = new Dataset.GRMProject.LandCoverDataTable();
                mdtLandCoverInfo = prjDB.LandCover;
            }
            else
            { mdtLandCoverInfo = null; }
        }

        public bool IsSet
        {
            get
            {
                return mLandCoverDataType.HasValue;
            }
        }

        public string GetLandCoverName(int intGridValue)
        {
            if (mLandCoverDataType.Equals(cGRM.FileOrConst.File))
            {
                DataRow[] rows = mdtLandCoverInfo.Select("GridValue = " + intGridValue);
                return rows[0]["GRMLandCoverE"].ToString();
            }
            else
            {
                return "[CONST]";
            }
        }

        public static LandCoverCode GetLandCoverCode(string inName)
        {
            switch (inName.Trim())
            {
                case nameof(LandCoverCode.AGRL):
                    {
                        return cSetLandcover.LandCoverCode.AGRL;
                    }

                case nameof(LandCoverCode.BARE):
                    {
                        return cSetLandcover.LandCoverCode.BARE;
                    }

                case nameof(LandCoverCode.FRST):
                    {
                        return LandCoverCode.FRST;
                    }

                case nameof(LandCoverCode.GRSS):
                    {
                        return LandCoverCode.GRSS;
                    }

                case nameof(LandCoverCode.URBN):
                    {
                        return LandCoverCode.URBN;
                    }

                case nameof(LandCoverCode.WATR):
                    {
                        return LandCoverCode.WATR;
                    }

                case nameof(LandCoverCode.WTLD):
                    {
                        return LandCoverCode.WTLD;
                    }

                case nameof(LandCoverCode.USER):
                    {
                        return LandCoverCode.USER;
                    }

                case nameof(LandCoverCode.CONSTV):
                    {
                        return LandCoverCode.CONSTV;
                    }

                default:
                    {
                        return default(LandCoverCode);
                    }
            }
        }
    }
}
