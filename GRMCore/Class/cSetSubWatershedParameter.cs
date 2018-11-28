
using System.Collections.Generic;

namespace GRMCore
{
    public class cSetSubWatershedParameter
    {
        public Dictionary<int, cUserParameters> userPars = new Dictionary<int, cUserParameters>();

        public cSetSubWatershedParameter()
        {
            userPars.Clear();
        }

        public void SetSubWSkeys(List<int> WSList)
        {
            foreach (int wsid in WSList)
            {
                if (userPars.ContainsKey(wsid) == false)
                {
                    cUserParameters upars = new cUserParameters();
                    userPars.Add(wsid, upars);
                }
            }
        }

        public bool IsUserSetSubWSexist
        {
            get
            {
                foreach (int k in userPars.Keys)
                {
                    if (userPars[k].isUserSet == true)
                        return true;
                }
                return false;
            }
        }

        public bool AllSubWSParametersAreSet
        {
            get
            {
                foreach (int k in userPars.Keys)
                {
                    if (userPars[k].iniSaturation == 0)
                        return false;
                }
                return true;
            }
        }

        public bool NowSubWSParameterIsSet(int WSID)
        {
            return !(userPars[WSID].iniSaturation == 0);
        }

        public bool IsSet
        {
            get
            {
                return !(userPars == null);
            }
        }

        public void RemoveWSParametersSet(int WSID)
        {
            userPars[WSID] = null;
        }


        public void GetValues(Dataset.GRMProject prjDB)
        {
            Dataset.GRMProject.SubWatershedSettingsDataTable dt = prjDB.SubWatershedSettings;
            if (dt.Rows.Count > 0)
            {
                foreach (Dataset.GRMProject.SubWatershedSettingsRow row in dt)
                {
                    cUserParameters upars = new cUserParameters();
                    if (!row.IsIDNull())
                    {
                        upars.wsid = row.ID;
                        upars.iniSaturation = row.IniSaturation;
                        upars.minSlopeOF = row.MinSlopeOF;
                        if (!row.IsUnsaturatedKTypeNull() && row.UnsaturatedKType.ToString() != "")
                        {
                            upars.UKType = cGRM.UnSaturatedKType.Linear.ToString();
                            if (row.UnsaturatedKType.ToString().ToLower() == cGRM.UnSaturatedKType.Linear.ToString().ToLower()) { upars.UKType = cGRM.UnSaturatedKType.Linear.ToString(); }
                            else if (row.UnsaturatedKType.ToString().ToLower() == cGRM.UnSaturatedKType.Exponential.ToString().ToLower()) { upars.UKType = cGRM.UnSaturatedKType.Exponential.ToString(); }
                            else if (row.UnsaturatedKType.ToString().ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower()) { upars.UKType = cGRM.UnSaturatedKType.Constant.ToString(); }

                        }
                        else
                        { upars.UKType = cGRM.UnSaturatedKType.Linear.ToString(); }

                        if (!row.IsCoefUnsaturatedKNull() && row.CoefUnsaturatedK != "")
                            upars.coefUK = System.Convert.ToDouble(row.CoefUnsaturatedK);
                        else
                            // set defalut value
                            upars.coefUK = 0.2;
                        if (upars.UKType.ToLower() == cGRM.UnSaturatedKType.Linear.ToString().ToLower()) { upars.coefUK = 0.2; }
                        else if (upars.UKType.ToLower() == cGRM.UnSaturatedKType.Exponential.ToString().ToLower()) { upars.coefUK = 6.4; }
                        else if (upars.UKType.ToLower() == cGRM.UnSaturatedKType.Constant.ToString().ToLower()) { upars.coefUK = 0.1; }
                    }
                    upars.minSlopeChBed = row.MinSlopeChBed;
                    upars.minChBaseWidth = row.MinChBaseWidth;
                    upars.chRoughness = row.ChRoughness;
                    upars.dryStreamOrder = row.DryStreamOrder;
                    upars.iniFlow = row.IniFlow;
                    upars.ccLCRoughness = row.CalCoefLCRoughness;
                    upars.ccPorosity = row.CalCoefPorosity;
                    upars.ccWFSuctionHead = row.CalCoefWFSuctionHead;
                    upars.ccHydraulicK = row.CalCoefHydraulicK;
                    upars.ccSoilDepth = row.CalCoefSoilDepth;
                    if (row.UserSet.ToString().ToLower() == "true")
                    { upars.isUserSet = true; }
                    else
                    { upars.isUserSet = false; }
                    userPars.Add(row.ID, upars);
                }
            }
        }

        public void SetValues(Dataset.GRMProject prjDB)
        {
            Dataset.GRMProject.SubWatershedSettingsDataTable dt = prjDB.SubWatershedSettings;
            if (!(AllSubWSParametersAreSet == false))
            {
                dt.Rows.Clear();
                foreach (int wsid in userPars.Keys)
                {
                    Dataset.GRMProject.SubWatershedSettingsRow newRow = dt.NewSubWatershedSettingsRow();
                    newRow.ID = wsid;
                    newRow.IniSaturation = userPars[wsid].iniSaturation;
                    newRow.MinSlopeOF = userPars[wsid].minSlopeOF;
                    newRow.UnsaturatedKType = userPars[wsid].UKType;
                    newRow.CoefUnsaturatedK = userPars[wsid].coefUK.ToString();
                    newRow.ChRoughness = userPars[wsid].chRoughness;
                    newRow.MinSlopeChBed = userPars[wsid].minSlopeChBed;
                    newRow.MinChBaseWidth = userPars[wsid].minChBaseWidth;
                    newRow.DryStreamOrder = userPars[wsid].dryStreamOrder;
                    newRow.IniFlow = userPars[wsid].iniFlow;
                    newRow.CalCoefLCRoughness = userPars[wsid].ccLCRoughness;
                    newRow.CalCoefPorosity = userPars[wsid].ccPorosity;
                    newRow.CalCoefWFSuctionHead = userPars[wsid].ccWFSuctionHead;
                    newRow.CalCoefHydraulicK = userPars[wsid].ccHydraulicK;
                    newRow.CalCoefSoilDepth = userPars[wsid].ccSoilDepth;
                    newRow.UserSet = userPars[wsid].isUserSet.ToString();
                    dt.Rows.Add(newRow);
                }
            }
        }


        public static void UpdateSubWSParametersForWSNetwork(cProject prj)
        {
            foreach (int wsid in prj.watershed.WSIDList)
            {
                if (prj.subWSPar.userPars[wsid].isUserSet == true)
                {
                    List<int> wsidToExclude = new List<int>();
                    foreach (int upsid in prj.WSNetwork.WSIDsAllUps(wsid))
                    {
                        if (prj.subWSPar.userPars[upsid].isUserSet == true)
                        {
                            if (!wsidToExclude.Contains(upsid))
                                wsidToExclude.Add(upsid);
                            foreach (int upupID in prj.WSNetwork.WSIDsAllUps(upsid))
                            {
                                if (!wsidToExclude.Contains(upupID))
                                    wsidToExclude.Add(upupID);
                            }
                        }
                    }

                    foreach (int upsid in prj.WSNetwork.WSIDsAllUps(wsid))
                    {
                        if (wsidToExclude.Contains(upsid) == false)
                            SetWSParametersWithAnotherWatershedParameterSet(prj, upsid, wsid);
                    }
                }
            }
        }


        private static bool SetWSParametersWithAnotherWatershedParameterSet(cProject prj, int TargetWSid, int ReferenceWSid)
        {
            Dictionary<int, cUserParameters> spars = new Dictionary<int, cUserParameters>();
            spars = prj.subWSPar.userPars;
            int rid = ReferenceWSid;
            prj.subWSPar.userPars[TargetWSid].iniSaturation = spars[rid].iniSaturation;
            prj.subWSPar.userPars[TargetWSid].minSlopeOF = spars[rid].minSlopeOF;
            prj.subWSPar.userPars[TargetWSid].UKType = spars[rid].UKType;
            prj.subWSPar.userPars[TargetWSid].coefUK = spars[rid].coefUK;
            prj.subWSPar.userPars[TargetWSid].minSlopeChBed = spars[rid].minSlopeChBed;
            prj.subWSPar.userPars[TargetWSid].minChBaseWidth = spars[rid].minChBaseWidth;
            prj.subWSPar.userPars[TargetWSid].chRoughness = spars[rid].chRoughness;
            prj.subWSPar.userPars[TargetWSid].dryStreamOrder = spars[rid].dryStreamOrder;
            prj.subWSPar.userPars[TargetWSid].ccLCRoughness = spars[rid].ccLCRoughness;
            prj.subWSPar.userPars[TargetWSid].ccPorosity = spars[rid].ccPorosity;
            prj.subWSPar.userPars[TargetWSid].ccWFSuctionHead = spars[rid].ccWFSuctionHead;
            prj.subWSPar.userPars[TargetWSid].ccHydraulicK = spars[rid].ccHydraulicK;
            prj.subWSPar.userPars[TargetWSid].ccSoilDepth = spars[rid].ccSoilDepth;
            if (prj.subWSPar.userPars[TargetWSid].isUserSet == false)
            { prj.subWSPar.userPars[TargetWSid].iniFlow = 0; }
            return true;
        }
    }
}
