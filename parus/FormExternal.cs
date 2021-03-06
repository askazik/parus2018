﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace parus
{
    public partial class FormExternal : Form
    {
        public FormExternal()
        {
            InitializeComponent();
        }

        private void FormExternal_Load(object sender, EventArgs e)
        {
            textBoxIonogram.Text = Properties.Settings.Default.settinsExternal_Ionogram;
            textBoxObliqueIonogram.Text = Properties.Settings.Default.settinsExternal_ObliqueIonogram;
        }

        private void FormExternal_FormClosing(object sender, FormClosingEventArgs e)
        {
            Properties.Settings.Default.settinsExternal_Ionogram = textBoxIonogram.Text;
            Properties.Settings.Default.settinsExternal_ObliqueIonogram = textBoxObliqueIonogram.Text;
            Properties.Settings.Default.Save();
        }

        private void button_Click(object sender, EventArgs e)
        {
            var bt = sender as Button;

            if (openFileDialog.ShowDialog() == DialogResult.Cancel)
                return;
            // получаем выбранный файл
            string filename = openFileDialog.FileName;

            switch (Convert.ToInt32(bt.Tag))
            {
                case 0:
                    textBoxIonogram.Text = filename;
                    break;
                case 1:
                    textBoxObliqueIonogram.Text = filename;
                    break;
            }
        }
    }
}
