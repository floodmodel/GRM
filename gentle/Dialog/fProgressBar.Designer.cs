namespace gentle.Dialog
{
    partial class fProgressBar
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(fProgressBar));
            this.btStop = new System.Windows.Forms.Button();
            this.labGRMToolsPrograssBar = new System.Windows.Forms.Label();
            this.GRMPrograssBar = new System.Windows.Forms.ProgressBar();
            this.SuspendLayout();
            // 
            // btStop
            // 
            this.btStop.Location = new System.Drawing.Point(370, 7);
            this.btStop.Name = "btStop";
            this.btStop.Size = new System.Drawing.Size(75, 23);
            this.btStop.TabIndex = 5;
            this.btStop.Text = "Stop";
            this.btStop.UseVisualStyleBackColor = true;
            this.btStop.Click += new System.EventHandler(this.btStop_Click);
            // 
            // labGRMToolsPrograssBar
            // 
            this.labGRMToolsPrograssBar.AutoSize = true;
            this.labGRMToolsPrograssBar.Location = new System.Drawing.Point(22, 13);
            this.labGRMToolsPrograssBar.Name = "labGRMToolsPrograssBar";
            this.labGRMToolsPrograssBar.Size = new System.Drawing.Size(42, 12);
            this.labGRMToolsPrograssBar.TabIndex = 4;
            this.labGRMToolsPrograssBar.Text = "Label1";
            // 
            // GRMPrograssBar
            // 
            this.GRMPrograssBar.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(192)))), ((int)(((byte)(0)))));
            this.GRMPrograssBar.Location = new System.Drawing.Point(24, 36);
            this.GRMPrograssBar.Name = "GRMPrograssBar";
            this.GRMPrograssBar.Size = new System.Drawing.Size(421, 23);
            this.GRMPrograssBar.TabIndex = 3;
            // 
            // fProgressBar
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(466, 67);
            this.Controls.Add(this.btStop);
            this.Controls.Add(this.labGRMToolsPrograssBar);
            this.Controls.Add(this.GRMPrograssBar);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "fProgressBar";
            this.Text = "fProgressBar";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        internal System.Windows.Forms.Button btStop;
        internal System.Windows.Forms.Label labGRMToolsPrograssBar;
        internal System.Windows.Forms.ProgressBar GRMPrograssBar;
    }
}