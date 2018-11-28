using System;

namespace GRMCore
{
    public class cHydroCom
    {
        public static cGRM.GRMFlowDirectionD8 GetFlowDirection(int value, cGRM.FlowDirectionType FDType)
        {
            if (FDType == cGRM.FlowDirectionType.StartsFromNE)
            {
                switch (value)
                {
                    case 1:
                        {
                            return cGRM.GRMFlowDirectionD8.NE;
                        }
                    case 2:
                        {
                            return cGRM.GRMFlowDirectionD8.E;
                        }
                    case 4:
                        {
                            return cGRM.GRMFlowDirectionD8.SE;
                        }
                    case 8:
                        {
                            return cGRM.GRMFlowDirectionD8.S;
                        }
                    case 16:
                        {
                            return cGRM.GRMFlowDirectionD8.SW;
                        }
                    case 32:
                        {
                            return cGRM.GRMFlowDirectionD8.W;
                        }
                    case 64:
                        {
                            return cGRM.GRMFlowDirectionD8.NW;
                        }
                    case 128:
                        {
                            return cGRM.GRMFlowDirectionD8.N;
                        }
                    default:
                        {
                            return cGRM.GRMFlowDirectionD8.NONE;
                        }
                }
            }

            if (FDType == cGRM.FlowDirectionType.StartsFromN)
            {
                switch (value)
                {
                    case 1:
                        {
                            return cGRM.GRMFlowDirectionD8.N;
                        }
                    case 2:
                        {
                            return cGRM.GRMFlowDirectionD8.NE;
                        }
                    case 4:
                        {
                            return cGRM.GRMFlowDirectionD8.E;
                        }
                    case 8:
                        {
                            return cGRM.GRMFlowDirectionD8.SE;
                        }
                    case 16:
                        {
                            return cGRM.GRMFlowDirectionD8.S;
                        }
                    case 32:
                        {
                            return cGRM.GRMFlowDirectionD8.SW;
                        }
                    case 64:
                        {
                            return cGRM.GRMFlowDirectionD8.W;
                        }
                    case 128:
                        {
                            return cGRM.GRMFlowDirectionD8.NW;
                        }
                    default:
                        {
                            return cGRM.GRMFlowDirectionD8.NONE;
                        }
                }
            }
            if (FDType == cGRM.FlowDirectionType.StartsFromE)
            {
                switch (value)
                {
                    case 1:
                        {
                            return cGRM.GRMFlowDirectionD8.E;
                        }
                    case 2:
                        {
                            return cGRM.GRMFlowDirectionD8.SE;
                        }
                    case 4:
                        {
                            return cGRM.GRMFlowDirectionD8.S;
                        }
                    case 8:
                        {
                            return cGRM.GRMFlowDirectionD8.SW;
                        }
                    case 16:
                        {
                            return cGRM.GRMFlowDirectionD8.W;
                        }
                    case 32:
                        {
                            return cGRM.GRMFlowDirectionD8.NW;
                        }
                    case 64:
                        {
                            return cGRM.GRMFlowDirectionD8.N;
                        }
                    case 128:
                        {
                            return cGRM.GRMFlowDirectionD8.NE;
                        }
                    default:
                        {
                            return cGRM.GRMFlowDirectionD8.NONE;
                        }
                }
            }

            if (FDType == cGRM.FlowDirectionType.StartsFromE_TauDEM)
            {
                switch (value)
                {
                    case 1:
                        {
                            return cGRM.GRMFlowDirectionD8.E;
                        }
                    case 2:
                        {
                            return cGRM.GRMFlowDirectionD8.NE;
                        }
                    case 3:
                        {
                            return cGRM.GRMFlowDirectionD8.N;
                        }
                    case 4:
                        {
                            return cGRM.GRMFlowDirectionD8.NW;
                        }
                    case 5:
                        {
                            return cGRM.GRMFlowDirectionD8.W;
                        }
                    case 6:
                        {
                            return cGRM.GRMFlowDirectionD8.SW;
                        }
                    case 7:
                        {
                            return cGRM.GRMFlowDirectionD8.S;
                        }
                    case 8:
                        {
                            return cGRM.GRMFlowDirectionD8.SE;
                        }
                    default:
                        {
                            return cGRM.GRMFlowDirectionD8.NONE;
                        }
                }
            }
            return cGRM.GRMFlowDirectionD8.NONE;
        }


        public static int getDTsec(double cfln, double dx, double vMax, int dtPrint_min)
        {
            if (vMax <= 0)
                return sThisSimulation.dtMaxLimit_sec;
            double dtsecNext = cGRM.CONST_CFL_NUMBER * dx / (double)vMax;
            dtsecNext = System.Convert.ToDouble(Math.Truncate(dtsecNext));

            if (dtsecNext > sThisSimulation.dtMaxLimit_sec)
                dtsecNext = sThisSimulation.dtMaxLimit_sec;
            if (dtsecNext < sThisSimulation.dtMinLimit_sec)
                dtsecNext = sThisSimulation.dtMinLimit_sec;

            if (dtsecNext > (dtPrint_min * 60 / (double)2))
                dtsecNext = dtPrint_min * 60 / (double)2;

            return System.Convert.ToInt32(dtsecNext);
        }


        /// <summary>
        ///   기지의 x축의 두점 t1, t2에 대한 기지의 y 축 값 A, B를 이용해서 대상 x 축 값 tx에 대한 미지의 y축 값 X를 
        ///   계산하기 위한 함수. 이때 t2는 t1 보다 큰 값임.
        ///   기지의 두점 사이는 직선으로 가정하고, 그 기울기를 이용해서, 미지값을 보간하여 계산함
        ///   </summary>
        ///   <param name="A">t1에 대한 y축 값</param>
        ///   <param name="B">t2에 대한 y축 값</param>
        ///   <param name="interCoef"></param>
        ///   <returns></returns>
        ///   <remarks></remarks>
        public static double GetInterpolatedValueLinear(double A, double B, double interCoef)
        {
            try
            {
                double X;
                X = (B - A) * interCoef + A;
                return X;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return double.MinValue;
            }
        }
    }
}
