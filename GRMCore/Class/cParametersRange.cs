using System;
using System.Collections.Generic;
using System.Data;

namespace GRMCore
{
    public class cParametersRange
    {
        /// <summary>
        ///   GRM staticDB에서 매개변수를 검색하고, 모형에서 활용하기 위한 key 텍스트
        ///   </summary>
        ///   <remarks></remarks>
        public enum Name
        {
            BankSideSlope,
            CalibrationCoeffLandCoverRoughnessC,
            CalibrationCoeffSoilDepth,
            CalibrationCoeffSoilHydraulicC,
            CalibrationCoeffSoilPorosity,
            CalibrationCoeffSoilWettingFrontSH,
            ChannelRoughnessCoeff,
            ChannelWidth,
            ChannelWidthEquationParameterC,
            ChannelWidthEquationParameterD,
            ChannelWidthEquationParameterE,
            ComputationalTimeStep,
            Discharge,
            EfficitivePorosity,
            FlowDepth,
            HydraulicConductivity,
            ImperviousRatio,
            LandCoverRoughnessCoeff,
            Porosity,
            RainfallDuration,
            ReservoirStorage,
            ReservoirStorageRatio,
            SimulationDuration,
            Slope,
            SoilDepth,
            SoilSaturation,
            StreamOrder,
            TimeInterval,
            WettingFrontSuctionHead
        }


        private static SortedList<string, decimal> mMinValue = new SortedList<string, decimal>();
        private static SortedList<string, decimal> mMaxValue = new SortedList<string, decimal>();
        private static SortedList<string, bool> mIncludeMinValue = new SortedList<string, bool>();
        private static SortedList<string, bool> mIncludeMaxValue = new SortedList<string, bool>();
        private static Dataset.GRMStaticDB.ParametersRangeDataTable mdtParRange;


        public void GetValues(string fpnStaticXmlDB)
        {
            Dataset.GRMStaticDB db = new Dataset.GRMStaticDB();
            db.ReadXml(fpnStaticXmlDB);
            mdtParRange = db.ParametersRange;
        }
        public void SaveDB(Dataset.GRMStaticDB grmStaticDB)
        {
            Console.WriteLine("여기서 뭔가 해야함");
        }

        public decimal Min(Name ParName)
        {
                DataRow[] rows = mdtParRange.Select(string.Format("ParName = '{0}'", ParName.ToString()));
                return rows[0].Field<Decimal>("MinValue");
        }


        public decimal Max(Name ParName)
        {
                DataRow[] rows = mdtParRange.Select(string.Format("ParName = '{0}'", ParName.ToString()));
                return rows[0].Field<Decimal>("MaxValue");
        }

        public bool IncludeMin(Name ParName)
        {
                DataRow[] rows = mdtParRange.Select(string.Format("ParName = '{0}'", ParName.ToString()));
                return rows[0].Field<bool>("IncludeMinValue");
         }

        public bool IncludeMax(Name ParName)
        {
                DataRow[] rows = mdtParRange.Select(string.Format("ParName = '{0}'", ParName.ToString()));
                return rows[0].Field<bool>("IncludeMaxValue");
        }
    }
}
