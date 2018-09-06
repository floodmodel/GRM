using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace gentle
{
    public class cCalculator
    {
        public static bool checkIsAlgebraicOperator(string inString)
        {
           switch (inString.Trim())
            {
                case "+":
                    return true;
                case "-":
                    return true;
                case "*":
                    return true;
                case "/":
                    return true;
                default:
                    return false;
            }
        }

        public static double[,] calculate2DArryUsing2TermAlgebra(string inOperator, bool is1ASC, bool is2ASC,
            bool is1ASCnodataAsZero, bool is2ASCnodataAsZero,
            double[,] asc1 = null, double[,] asc2 = null, double value1 = 0, double value2 = 0, double nodataValue = -9999)
        {
            double[,] resultArr = null;
            if (is1ASC == true)
            { resultArr = new double[asc1.GetLength(0), asc1.GetLength(1)]; }
            else
            { resultArr = new double[asc2.GetLength(0), asc2.GetLength(1)]; }

            int ny = resultArr.GetLength(1);
            int nx = resultArr.GetLength(0);
            ParallelOptions options = new ParallelOptions();
            options.MaxDegreeOfParallelism = Environment.ProcessorCount;
            //for (int y  = 0; y< ny; y++)
            //{
            Parallel.For(0, ny, options, delegate (int y)
            {
                for (int x = 0; x <= nx - 1; x++)
                {
                    double v1;
                    double v2;
                    if (is1ASC == true)
                    {
                        v1 = asc1[x, y];
                        if (is1ASCnodataAsZero == true && v1 == nodataValue) { v1 = 0; }
                    }
                    else
                    { v1 = value1; }

                    if (is2ASC == true)
                    {
                        v2 = asc2[x, y];
                        if (is2ASCnodataAsZero == true && v2 == nodataValue) { v2 = 0; }
                    }
                    else
                    { v2 = value2; }

                    bool goCal = true;
                    if (is1ASC == true && is1ASCnodataAsZero == false && v1 == nodataValue)
                    { goCal = false; }
                    if (is2ASC == true && is2ASCnodataAsZero == false && v2 == nodataValue)
                    { goCal = false; }

                    if (goCal == true)
                    {
                        resultArr[x, y] = algebraicCal(inOperator, v1, v2, nodataValue);
                    }
                    else
                    {
                        resultArr[x, y] = nodataValue;
                    }
                }
            });
        //}
            return resultArr;
        }

        public static double[,] calculate2DArryUsingCondition(string ConOperator, bool is1ASC, bool is2ASC, bool isTasc, bool isFasc,
            double[,] asc1 = null, double[,] asc2 = null, double[,] ascT = null, double[,] ascF = null,
            double value1 = 0, double value2 = 0, double valueT = 0, double valueF = 0,
            double nodataValue = -9999)
        {
            double[,] resultArr = null;
            if (is1ASC == true)
            { resultArr = new double[asc1.GetLength(0), asc1.GetLength(1)]; }
            else
            { resultArr = new double[asc2.GetLength(0), asc2.GetLength(1)]; }

            int ny = resultArr.GetLength(1);
            int nx = resultArr.GetLength(0);
            ParallelOptions options = new ParallelOptions();
            options.MaxDegreeOfParallelism = Environment.ProcessorCount;
            // For y As Integer = 0 To nRowy - 1
            //{ 
            Parallel.For(0, ny, options, delegate (int y)
            {
                for (int x = 0; x <= nx - 1; x++)
                {
                    double v1;
                    double v2;
                    double vT;
                    double vF;
                    if (is1ASC == true)
                    { v1 = asc1[x, y]; }
                    else
                    { v1 = value1; }

                    if (is2ASC == true)
                    { v2 = asc2[x, y]; }
                    else
                    { v2 = value2; }

                    if (isTasc == true)
                    { vT = ascT[x, y]; }
                    else
                    { vT = valueT; }

                    if (isFasc == true)
                    { vF = ascF[x, y]; }
                    else
                    { vF = valueF; }

                    resultArr[x, y] = cCalculator.conditionalCal(ConOperator, v1, v2, vT, vF, nodataValue);
                }
            });
            //}
            return resultArr;
        }


        public static double conditionalCal(string conditionString, double conValue1, double conValue2, double TrueValue, double FalseValue, double nodataValue)
        {
            double vout = 0;
            switch (conditionString)
            {
                case ">":
                    if (conValue1 > conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "<":
                    if (conValue1 < conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "=":
                    if (conValue1 == conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case ">=":
                    if (conValue1 >= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "=>":
                    if (conValue1 >= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "<=":
                    if (conValue1 <= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                case "=<":
                    if (conValue1 <= conValue2)
                    { vout = TrueValue; }
                    else
                    { vout = FalseValue; }
                    break;
                default:
                    vout = nodataValue;
                    break;
            }
            return vout;
        }

        public static double algebraicCal(string conditionString, double v1, double v2,  double nodataValue)
        {
            double vout = 0;
            switch (conditionString)
            {
                case "+":
                    vout = v1 + v2;
                    break;
                case "-":
                    vout = v1 - v2;
                    break;
                case "*":
                    vout = v1 * v2;
                    break;
                case "/":
                    if (v2 != 0)
                    { vout = v1 / v2; }
                    else
                    { vout = nodataValue; }
                    break;
                default:
                    vout = nodataValue;
                    break;
            }
            return vout;
        }
    }
}
