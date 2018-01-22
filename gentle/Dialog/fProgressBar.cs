using System;
using System.Windows.Forms;

namespace gentle.Dialog
{
    public partial class fProgressBar : Form
    {
        public event StopProcessEventHandler StopProcess;
        public delegate void StopProcessEventHandler(fProgressBar sender);
        public fProgressBar()
        {
            InitializeComponent();
        }

        private void btStop_Click(object sender, EventArgs e)
        {
            if (StopProcess != null)
            {
                StopProcess(this);
            }
        }


    }
}
