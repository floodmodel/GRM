
namespace GRMCore
{
    public abstract class cSetCrossSection
    {
        public enum CSTypeEnum
        {
            CSSingle,
            CSCompound
        }

        public abstract CSTypeEnum CSType { get; }
        public abstract void GetValues(Dataset.GRMProject prjds);
        public abstract bool IsSet { get; }
        public abstract void SetValues(Dataset.GRMProject prjds);
    }

}
