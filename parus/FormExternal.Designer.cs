namespace parus
{
    partial class FormExternal
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormExternal));
            this.panel1 = new System.Windows.Forms.Panel();
            this.buttonCalibration = new System.Windows.Forms.Button();
            this.textBoxCalibration = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.buttonAmplitudes = new System.Windows.Forms.Button();
            this.textBoxAmplitudes = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.buttonIonogram = new System.Windows.Forms.Button();
            this.textBoxIonogram = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.panel1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.buttonCalibration);
            this.panel1.Controls.Add(this.textBoxCalibration);
            this.panel1.Controls.Add(this.label3);
            this.panel1.Controls.Add(this.buttonAmplitudes);
            this.panel1.Controls.Add(this.textBoxAmplitudes);
            this.panel1.Controls.Add(this.label2);
            this.panel1.Controls.Add(this.buttonIonogram);
            this.panel1.Controls.Add(this.textBoxIonogram);
            this.panel1.Controls.Add(this.label1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(501, 193);
            this.panel1.TabIndex = 0;
            // 
            // buttonCalibration
            // 
            this.buttonCalibration.Location = new System.Drawing.Point(456, 152);
            this.buttonCalibration.Name = "buttonCalibration";
            this.buttonCalibration.Size = new System.Drawing.Size(30, 20);
            this.buttonCalibration.TabIndex = 8;
            this.buttonCalibration.Tag = "2";
            this.buttonCalibration.Text = "...";
            this.buttonCalibration.UseVisualStyleBackColor = true;
            this.buttonCalibration.Click += new System.EventHandler(this.button_Click);
            // 
            // textBoxCalibration
            // 
            this.textBoxCalibration.Location = new System.Drawing.Point(18, 152);
            this.textBoxCalibration.Name = "textBoxCalibration";
            this.textBoxCalibration.Size = new System.Drawing.Size(432, 20);
            this.textBoxCalibration.TabIndex = 7;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(19, 130);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(129, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Программа калибровки";
            // 
            // buttonAmplitudes
            // 
            this.buttonAmplitudes.Location = new System.Drawing.Point(456, 95);
            this.buttonAmplitudes.Name = "buttonAmplitudes";
            this.buttonAmplitudes.Size = new System.Drawing.Size(30, 20);
            this.buttonAmplitudes.TabIndex = 5;
            this.buttonAmplitudes.Tag = "1";
            this.buttonAmplitudes.Text = "...";
            this.buttonAmplitudes.UseVisualStyleBackColor = true;
            this.buttonAmplitudes.Click += new System.EventHandler(this.button_Click);
            // 
            // textBoxAmplitudes
            // 
            this.textBoxAmplitudes.Location = new System.Drawing.Point(18, 95);
            this.textBoxAmplitudes.Name = "textBoxAmplitudes";
            this.textBoxAmplitudes.Size = new System.Drawing.Size(432, 20);
            this.textBoxAmplitudes.TabIndex = 4;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(19, 73);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(176, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Программа измерения амплитуд";
            // 
            // buttonIonogram
            // 
            this.buttonIonogram.Location = new System.Drawing.Point(456, 40);
            this.buttonIonogram.Name = "buttonIonogram";
            this.buttonIonogram.Size = new System.Drawing.Size(30, 20);
            this.buttonIonogram.TabIndex = 2;
            this.buttonIonogram.Tag = "0";
            this.buttonIonogram.Text = "...";
            this.buttonIonogram.UseVisualStyleBackColor = true;
            this.buttonIonogram.Click += new System.EventHandler(this.button_Click);
            // 
            // textBoxIonogram
            // 
            this.textBoxIonogram.Location = new System.Drawing.Point(18, 40);
            this.textBoxIonogram.Name = "textBoxIonogram";
            this.textBoxIonogram.Size = new System.Drawing.Size(432, 20);
            this.textBoxIonogram.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(19, 18);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(193, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Программа измерения ионограммы";
            // 
            // openFileDialog
            // 
            this.openFileDialog.Filter = "Программа (*.exe)|*.exe|Все файлы (*.*)|*.*";
            // 
            // FormExternal
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(501, 193);
            this.Controls.Add(this.panel1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "FormExternal";
            this.Text = "Внешние программы";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormExternal_FormClosing);
            this.Load += new System.EventHandler(this.FormExternal_Load);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button buttonIonogram;
        private System.Windows.Forms.TextBox textBoxIonogram;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button buttonCalibration;
        private System.Windows.Forms.TextBox textBoxCalibration;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Button buttonAmplitudes;
        private System.Windows.Forms.TextBox textBoxAmplitudes;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
    }
}