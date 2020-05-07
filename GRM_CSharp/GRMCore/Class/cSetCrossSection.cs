
namespace GRMCore
{
    public abstract class cSetCrossSection
    {
        public enum CSTypeEnum
        {
            CSSingle,
            CSCompound
        }

        public double RightBankSlope;
        public double LeftBankSlope;
        public abstract CSTypeEnum CSType { get; }
        public abstract void GetValues(Dataset.GRMProject.ChannelSettingsRow row);
        public abstract bool IsSet { get; }
        public abstract void SetValues(Dataset.GRMProject prjds, int rowIndex);
    }

}
