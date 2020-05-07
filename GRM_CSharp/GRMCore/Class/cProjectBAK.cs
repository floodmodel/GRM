using System;

namespace GRMCore
{
    public class cProjectBAK : ICloneable
    {
        public cCVAttribute[] CVs;
        public cSetWatchPoint watchPoint;
        public cFlowControl fcGrid;
        public bool isSet = false;
        private cProject mprj;

        public cProjectBAK()
        {
            CVs = new cCVAttribute[cProject .Current .CVs .Length ];
            watchPoint = new cSetWatchPoint();
            fcGrid = new cFlowControl();
        }

        public void SetCloneUsingCurrentProject(cProject project)
        {
            //CVs = project.CVs;
            //watchPoint = project.watchPoint;
            //fcGrid = project.fcGrid;
            mprj = project;
            Clone();
            isSet = true;
        }

        public cCVAttribute CV(int index)
        {
            return CVs[index];
        }

        public object Clone()
        {
            cProjectBAK clo = new cProjectBAK();
            clo.CVs = new cCVAttribute[mprj.CVs.Length];
            for(int i=0;i<clo.CVs .Length;i++)
            {
                clo.CVs[i] = (cCVAttribute)mprj.CVs[i].Clone();
            }
            clo.watchPoint = (cSetWatchPoint)mprj.watchPoint.Clone();
            clo.fcGrid = (cFlowControl)mprj.fcGrid.Clone();
            CVs = clo.CVs;
            watchPoint = clo.watchPoint;
            fcGrid = clo.fcGrid;

            return clo;
        }
    }
}
