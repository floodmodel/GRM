using System;
using System.Data;

namespace GRMCore
{
    public class cSetSoilDepth
    {
        public enum SoilDepthCode
        {
            D,
            M,
            S,
            VD,
            VS,
            USER,
            CONSTV,
            NULL
        }

        public Nullable<cGRM.FileOrConst> mSoilDepthDataType;
        public string mGridSoilDepthFPN;
        public string mSoilDepthVATFPN;
        public Dataset.GRMProject.SoilDepthDataTable mdtSoilDepthInfo;
        public Nullable<double> mConstSoilDepth;

        public cSetSoilDepth()
        {
        }

        public void SetValues(Dataset.GRMProject prjdb)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];
            if (mSoilDepthDataType.HasValue)
            {
                row.SoilDepthDataType = mSoilDepthDataType.Value.ToString();
                switch (mSoilDepthDataType.Value)
                {
                    case cGRM.FileOrConst.File:
                        {
                            row.SoilDepthFile = mGridSoilDepthFPN;
                            row.SoilDepthVATFile = mSoilDepthVATFPN;
                            row.SetConstantSoilDepthNull();
                            break;
                        }

                    case cGRM.FileOrConst.Constant:
                        {
                            row.SetSoilDepthFileNull();
                            row.SetSoilDepthVATFileNull();
                            row.ConstantSoilDepth = System.Convert.ToString(mConstSoilDepth.Value);
                            break;
                        }
                }
            }
            if (mSoilDepthDataType.Equals(cGRM.FileOrConst.File))
            {
                mdtSoilDepthInfo.AcceptChanges();
                foreach (DataRow r in mdtSoilDepthInfo.Rows)
                {
                    r.SetAdded();
                }
            }
        }

        public void GetValues(Dataset.GRMProject prjdb)
        {
            mSoilDepthDataType = default(cGRM.FileOrConst?);
            mSoilDepthVATFPN = null;
            mConstSoilDepth = default(Single?);
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];
            if (!row.IsSoilDepthDataTypeNull())
            {
                if (row.SoilDepthDataType.ToLower() == cGRM.FileOrConst.File.ToString().ToLower())
                {
                    mSoilDepthDataType = cGRM.FileOrConst.File;
                    mGridSoilDepthFPN = row.SoilDepthFile;
                    mSoilDepthVATFPN = row.SoilDepthVATFile;
                }
                else if (row.SoilDepthDataType.ToLower() == cGRM.FileOrConst.Constant.ToString().ToLower())
                {
                    mSoilDepthDataType = cGRM.FileOrConst.Constant;
                    double v = 0;
                    if (double.TryParse(row.ConstantSoilDepth, out v) == true) { mConstSoilDepth = v; }
                }
                else
                {
                    throw new InvalidOperationException();
                }
            }
            if (mSoilDepthDataType.Equals(cGRM.FileOrConst.File))
            {
                mdtSoilDepthInfo = new Dataset.GRMProject.SoilDepthDataTable();
                mdtSoilDepthInfo = prjdb.SoilDepth;
            }
            else
            { mdtSoilDepthInfo = null; }
        }

        public bool IsSet
        {
            get
            {
                return mSoilDepthDataType.HasValue;
            }
        }

        public static SoilDepthCode GetSoilDepthCode(string inName)
        {
            switch (inName.Trim())
            {
                case nameof(SoilDepthCode.D):
                    {
                        return SoilDepthCode.D;
                    }

                case nameof(SoilDepthCode.M):
                    {
                        return SoilDepthCode.M;
                    }

                case nameof(SoilDepthCode.S):
                    {
                        return SoilDepthCode.S;
                    }

                case nameof(SoilDepthCode.VD):
                    {
                        return SoilDepthCode.VD;
                    }

                case nameof(SoilDepthCode.VS):
                    {
                        return SoilDepthCode.VS;
                    }

                case nameof(SoilDepthCode.USER):
                    {
                        return SoilDepthCode.USER;
                    }

                case nameof(SoilDepthCode.CONSTV):
                    {
                        return SoilDepthCode.CONSTV;
                    }

                case nameof(SoilDepthCode.NULL):
                    {
                        return SoilDepthCode.NULL;
                    }

                default:
                    {
                        return default(SoilDepthCode);
                    }
            }
        }
    }
}
