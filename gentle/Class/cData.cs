using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace gentle
{
    public class cData
    {
        public enum DataType
        {
            DTByte,
            DTShort,
            DTInteger,
            DTSingle,
            DTDouble,
            None
        };

        public static cData.DataType GetDataTypeByName(string inType)
        {
            foreach (DataType dtype in Enum.GetValues(typeof(cData.DataType)))
            {
                if (dtype.ToString() == inType)
                {
                    return dtype;
                }
            }
            return cData.DataType.None;
        }
    }
    public struct CellPosition
    {
        public int x;
        public int y;
    }
}
