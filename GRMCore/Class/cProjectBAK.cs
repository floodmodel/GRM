using System;

namespace GRMCore
{
    public class cProjectBAK : ICloneable
    {
        public cCVAttribute[] CVs;
        public cSetWatchPoint watchPoint;
        public cFlowControl fcGrid;

        public cProjectBAK()
        {
            watchPoint = new cSetWatchPoint();
            fcGrid = new cFlowControl();
        }

        public void SetCloneUsingCurrentProject(cProject project)
        {
            CVs = project.CVs;
            watchPoint = project.watchPoint;
            fcGrid = project.fcGrid;
            Clone();
        }

        public cCVAttribute CV(int index)
        {
            return CVs[index];
        }

        public object Clone()
        {
            cProjectBAK clo = new cProjectBAK();
            clo.CVs = (cCVAttribute[])CVs.Clone();
            clo.watchPoint = (cSetWatchPoint)this.watchPoint.Clone();
            clo.fcGrid = (cFlowControl)this.fcGrid.Clone();
            CVs = clo.CVs;
            watchPoint = clo.watchPoint;
            fcGrid = clo.fcGrid;
            return clo;
        }
    }
}
