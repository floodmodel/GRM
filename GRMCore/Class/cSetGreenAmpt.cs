using System;
using System.Data;

namespace GRMCore
{
    public class cSetGreenAmpt
    {
        public enum SoilTextureCode
        {
            C,
            CL,
            L,
            LS,
            S,
            SC,
            SCL,
            SiC,
            SiCL,
            SiL,
            SL,
            USER,
            CONSTV,
            NULL
        }

        public Nullable<cGRM.FileOrConst> mSoilTextureDataType;
        public string mGridSoilTextureFPN;
        public string mSoilTextureVATFPN;
        public Dataset.GRMProject.GreenAmptParameterDataTable mdtGreenAmptInfo;
        public Nullable<double> mConstPorosity;
        public Nullable<double> mConstEffectivePorosity;
        public Nullable<double> mConstWFS;
        public Nullable<double> mConstHydraulicCond;

        public cSetGreenAmpt()
        {
        }

        public void SetValues(Dataset.GRMProject prjdb)
        {
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];
            if (mSoilTextureDataType.HasValue)
            {
                {
                    row.SoilTextureDataType = mSoilTextureDataType.Value.ToString();
                    if (mSoilTextureDataType.Value == cGRM.FileOrConst.File)
                    {
                        row.SoilTextureFile = mGridSoilTextureFPN;
                        row.SoilTextureVATFile = mSoilTextureVATFPN;
                        row.SetConstantSoilPorosityNull();
                        row.SetConstantSoilEffPorosityNull();
                        row.SetConstantSoilWettingFrontSuctionHeadNull();
                        row.SetConstantSoilHydraulicConductivityNull();
                    }
                    else
                    {
                        row.SetSoilTextureFileNull();
                        row.SetSoilTextureVATFileNull();
                        row.ConstantSoilPorosity = System.Convert.ToString(mConstPorosity.Value);
                        row.ConstantSoilEffPorosity = System.Convert.ToString(mConstEffectivePorosity.Value);
                        row.ConstantSoilWettingFrontSuctionHead = System.Convert.ToString(mConstWFS.Value);
                        row.ConstantSoilHydraulicConductivity = System.Convert.ToString(mConstHydraulicCond.Value);
                    }
                }
            }

            if (mSoilTextureDataType.Equals(cGRM.FileOrConst.File))
            {
                mdtGreenAmptInfo.AcceptChanges();
                foreach (DataRow r in mdtGreenAmptInfo.Rows)
                    r.SetAdded();
            }
        }

        public void GetValues(Dataset.GRMProject prjdb)
        {
            mSoilTextureDataType = null;
            mSoilTextureVATFPN = null;
            mConstPorosity = null;
            mConstEffectivePorosity = null;
            mConstWFS = null;
            mConstHydraulicCond = null;
            Dataset.GRMProject.ProjectSettingsRow row = (Dataset.GRMProject.ProjectSettingsRow)prjdb.ProjectSettings.Rows[0];
            if (!row.IsSoilTextureDataTypeNull())
            {
                if (row.SoilTextureDataType == cGRM.FileOrConst.File.ToString())
                {
                    mSoilTextureDataType = cGRM.FileOrConst.File;
                    mGridSoilTextureFPN = row.SoilTextureFile;
                    mSoilTextureVATFPN = row.SoilTextureVATFile;
                }
                else if (row.SoilTextureDataType == cGRM.FileOrConst.Constant.ToString())
                {
                    mSoilTextureDataType = cGRM.FileOrConst.Constant;
                    double v = 0;
                    if (double.TryParse(row.ConstantSoilPorosity, out v)) { mConstPorosity = v; }
                    if (double.TryParse(row.ConstantSoilEffPorosity, out v)) { mConstEffectivePorosity = v; }
                    if (double.TryParse(row.ConstantSoilWettingFrontSuctionHead, out v)) { mConstWFS = v; }
                    if (double.TryParse(row.ConstantSoilHydraulicConductivity, out v)) { mConstHydraulicCond = v; }
                }
                else
                { throw new InvalidOperationException(); }
            }
            if (mSoilTextureDataType.Equals(cGRM.FileOrConst.File))
            {
                mdtGreenAmptInfo = new Dataset.GRMProject.GreenAmptParameterDataTable();
                mdtGreenAmptInfo = prjdb.GreenAmptParameter;
            }
            else
                mdtGreenAmptInfo = null;
        }

        public bool IsSet
        {
            get
            {
                return mSoilTextureDataType.HasValue;
            }
        }

        public string GetSoilTextureName(int intGridValue)
        {
            if (mSoilTextureDataType.Equals(cGRM.FileOrConst.File))
            {
                DataRow[] rows = mdtGreenAmptInfo.Select(string.Format("GridValue = {0}", intGridValue));
                return rows[0]["GRMTextureE"].ToString();
            }
            else
                return "[CONST]";
        }

        public static SoilTextureCode GetSoilTextureCode(string inName)
        {
            switch (inName.Trim())
            {
                case nameof(SoilTextureCode.C):
                    {
                        return SoilTextureCode.C;
                    }

                case nameof(SoilTextureCode.CL):
                    {
                        return SoilTextureCode.CL;
                    }

                case nameof(SoilTextureCode.L):
                    {
                        return SoilTextureCode.L;
                    }

                case nameof(SoilTextureCode.LS):
                    {
                        return SoilTextureCode.LS;
                    }

                case nameof(SoilTextureCode.S):
                    {
                        return SoilTextureCode.S;
                    }

                case nameof(SoilTextureCode.SC):
                    {
                        return SoilTextureCode.SC;
                    }

                case nameof(SoilTextureCode.SCL):
                    {
                        return SoilTextureCode.SCL;
                    }

                case nameof(SoilTextureCode.SiC):
                    {
                        return SoilTextureCode.SiC;
                    }

                case nameof(SoilTextureCode.SiCL):
                    {
                        return SoilTextureCode.SiCL;
                    }

                case nameof(SoilTextureCode.SiL):
                    {
                        return SoilTextureCode.SiL;
                    }

                case nameof(SoilTextureCode.SL):
                    {
                        return SoilTextureCode.SL;
                    }

                case nameof(SoilTextureCode.USER):
                    {
                        return SoilTextureCode.USER;
                    }

                case nameof(SoilTextureCode.CONSTV):
                    {
                        return SoilTextureCode.CONSTV;
                    }

                case nameof(SoilTextureCode.NULL):
                    {
                        return SoilTextureCode.NULL;
                    }

                default:
                    {
                        return default(SoilTextureCode);
                    }
            }
        }
    }
}
