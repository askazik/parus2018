using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;
using System.Drawing.Drawing2D;
using System.IO;
using System.Xml;
using System.Diagnostics;

namespace parus
{
    public partial class FormMain : Form
    {
        IonogramReader curIonogram = null; // информация о текущей ионограмме

        public FormMain()
        {
            InitializeComponent();
        }

        private void toolStripMenuItemExit_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void toolStripMenuItemOpenDir_Click(object sender, EventArgs e)
        {
            string curDir = Properties.Settings.Default.settingsWorkingDirectory;

            if (!String.IsNullOrEmpty(curDir))
                folderBrowserDialog.SelectedPath = curDir;
            else
                curDir = System.IO.Directory.GetCurrentDirectory();
            folderBrowserDialog.SelectedPath = curDir;
            
            // Update the working directory if the user clicks OK 
            if (folderBrowserDialog.ShowDialog() == DialogResult.OK)
                curDir = folderBrowserDialog.SelectedPath;

            // Попробуем поменять путь
            fill_listboxIonograms(curDir);
        }

        // Заполнение списка выбора именами ионограмм.
        private void fill_listboxIonograms(string curDir)
        {
            System.IO.DirectoryInfo DI = new System.IO.DirectoryInfo(curDir);
            System.IO.FileInfo[] FI = DI.GetFiles("*.ion");

            if (FI.Length > 0)
            {
                Properties.Settings.Default.settingsWorkingDirectory = curDir;
                Properties.Settings.Default.Save();
                toolStripStatusLabelDirectory.Text = "Папка :" + curDir;

                listBoxIonograms.Items.Clear();
                for (int i = 0; i < FI.Length; ++i)
                    listBoxIonograms.Items.Add(FI[i].Name);
                listBoxIonograms.SelectedIndex = listBoxIonograms.Items.Count-1;
                listBoxIonograms.Focus();
            }
            else
            {
                // Displays the MessageBox.
                DialogResult result = MessageBox.Show(
                    "В выбранной папке " + curDir + 
                    " отсутствуют файлы ионограмм *.ion. Выберите для работы другую папку!",
                    "Отсутствуют ионограммы!", MessageBoxButtons.OK);
            }
        }

        private void toolStripMenuItemOpenFile_Click(object sender, EventArgs e)
        {
            string curDir = Properties.Settings.Default.settingsWorkingDirectory;

            if (!String.IsNullOrEmpty(curDir))
                openFileDialog.InitialDirectory = curDir;
            openFileDialog.ShowDialog();
        }

        private void toolStripMenuItemHelpAbout_Click(object sender, EventArgs e)
        {
            AboutBox ab = new AboutBox();
            ab.Show();
        }

        private void chartIonogram_Paint(object sender, PaintEventArgs e)
        {
            if (listBoxIonograms.Items.Count > 0)
            {
                string ionname = Properties.Settings.Default.settingsWorkingDirectory + "\\";

                if (curIonogram == null)
                {
                    ionname = Properties.Settings.Default.settingsWorkingDirectory + "\\" + this.listBoxIonograms.Items[0].ToString();
                    curIonogram = new IonogramReader(ionname);
                }

                ChartArea a = chartIonogram.ChartAreas[0];
                int x1 = (int)a.AxisX.ValueToPixelPosition(a.AxisX.Minimum)+a.AxisX.MajorGrid.LineWidth;
                int x2 = (int)a.AxisX.ValueToPixelPosition(a.AxisX.Maximum)+2 * a.AxisX.MajorGrid.LineWidth;
                int y1 = (int)a.AxisY.ValueToPixelPosition(a.AxisY.Maximum)+a.AxisY.MajorGrid.LineWidth;
                int y2 = (int)a.AxisY.ValueToPixelPosition(a.AxisY.Minimum)+2 * a.AxisY.MajorGrid.LineWidth;

                e.Graphics.InterpolationMode = InterpolationMode.NearestNeighbor;
                if (curIonogram.Bitmap_O != null)
                    e.Graphics.DrawImage(curIonogram.Bitmap_O, new Rectangle(x1, y1, x2 - x1, y2 - y1));
            }
        }

        private void listBoxIonograms_SelectedValueChanged(object sender, EventArgs e)
        {
            ListBox listbox = (ListBox)sender;
            string ionname = Properties.Settings.Default.settingsWorkingDirectory + "\\" + listbox.SelectedItem.ToString();
            curIonogram = new IonogramReader(ionname);

            chartIonogram.Invalidate();

            chartIonogram.Titles["TitleTimeIonogram"].Text = curIonogram.TimeString;
            chartIonogram.ChartAreas["ChartAreaIonogram"].AxisX.Minimum = curIonogram.Header.freq_min/1000.0;
            chartIonogram.ChartAreas["ChartAreaIonogram"].AxisX.Maximum = curIonogram.Header.freq_max/1000.0;
            chartIonogram.ChartAreas["ChartAreaIonogram"].AxisX.Interval = 1;

            chartIonogram.ChartAreas["ChartAreaIonogram"].AxisY.Minimum = (int)(curIonogram.Header.height_min/1000.0);
            chartIonogram.ChartAreas["ChartAreaIonogram"].AxisY.Maximum = (int)(curIonogram.Header.height_min +
                curIonogram.Header.height_step * (curIonogram.Header.count_height-1))/1000.0;
            chartIonogram.ChartAreas["ChartAreaIonogram"].AxisY.Interval = 100.0;
         }

        private void FormMain_Load(object sender, EventArgs e)
        {
            string curDir = Properties.Settings.Default.settingsWorkingDirectory;
            if (Directory.Exists(curDir))
                fill_listboxIonograms(curDir); // попробуем заполнить список ионограмм
        }

        private void toolStripButtonChangeDir_Click(object sender, EventArgs e)
        {
            toolStripMenuItemOpenDir_Click(sender, e);
        }

        private void toolStripButtonRefresh_Click(object sender, EventArgs e)
        {
            string curDir = Properties.Settings.Default.settingsWorkingDirectory;

            // Попробуем поменять путь
            fill_listboxIonograms(curDir);
        }

        private void chartIonogram_MouseMove(object sender, MouseEventArgs e)
        {
            var ca = sender as Chart;

            // If the mouse isn't on the plotting area, a datapoint, or gridline then exit
            HitTestResult htr = ca.HitTest(e.X, e.Y);
            if (htr.ChartElementType != ChartElementType.PlottingArea && htr.ChartElementType != ChartElementType.DataPoint && htr.ChartElementType != ChartElementType.Gridlines)
                return;

            double xCoord = ca.ChartAreas["ChartAreaIonogram"].AxisX.PixelPositionToValue(e.X);
            double yCoord = ca.ChartAreas["ChartAreaIonogram"].AxisY.PixelPositionToValue(e.Y);

            toolStripStatusLabelPolnt.Text = "Точка: f = " + Math.Round(xCoord, 2).ToString() + " кГц, h' = " + Math.Round(yCoord, 2).ToString() + " км";
        }

        private void toolStripButtonXmlOpen_Click(object sender, EventArgs e)
        {
            openFileDialog.Filter = "Файл конфигурации (*.xml)|*.xml|Все файлы (*.*)|*.*";
            if (openFileDialog.ShowDialog() == DialogResult.Cancel)
                return;
            // получаем выбранный файл
            string filename = openFileDialog.FileName;

            richTextBox_XML.LoadFile(filename,RichTextBoxStreamType.UnicodePlainText);
            HighlightColors.HighlightRTF(richTextBox_XML);

            Properties.Settings.Default.settingsXmlConfig = filename;
            Properties.Settings.Default.Save();
        }

        private void toolStripButtonXmlSave_Click(object sender, EventArgs e)
        {
            richTextBox_XML.SaveFile(Properties.Settings.Default.settingsXmlConfig, RichTextBoxStreamType.UnicodePlainText);
        }

        private void toolStripMenuExternal_Click(object sender, EventArgs e)
        {
            FormExternal fe = new FormExternal();
            fe.Show();
        }

        private void toolStripButtonExternal_Click(object sender, EventArgs e)
        {
            string filename = "";

            int waitTime = 0;
            XMLConfig xml = new XMLConfig();
            xml.Load(Properties.Settings.Default.settingsXmlConfig);

            var bt = sender as ToolStripButton;
            switch (Convert.ToInt32(bt.Tag))
            {
                case 0:
                    filename = Properties.Settings.Default.settinsExternal_Ionogram;
                    waitTime = xml.getIonogramMeasuringTime() + xml.getAmplitudesMeasuringTime();
                    break;
                case 1:
                    filename = Properties.Settings.Default.settinsExternal_ObliqueIonogram;
                    waitTime = xml.getIonogramMeasuringTime();
                    break;
            }

            Process iStartProcess = new Process(); // новый процесс
            iStartProcess.StartInfo.FileName = filename; // путь к запускаемому файлу
            iStartProcess.StartInfo.WorkingDirectory = Path.GetDirectoryName(filename);
            iStartProcess.StartInfo.UseShellExecute = false;
            iStartProcess.Start(); // запускаем программу
            iStartProcess.WaitForExit(waitTime); // эту строку указываем, если нам надо будет ждать завершения программы определённое время, пример: 2 мин. (указано в миллисекундах - 2 мин. * 60 сек. * 1000 м.сек.)
            iStartProcess.Close();

            // Изменим текущую папку просмотра результатов измерений
            switch (Convert.ToInt32(bt.Tag))
            {
                case 0:
                    fill_listboxIonograms(Path.GetDirectoryName(filename));
                    break;
                case 1:
                    //
                    break;
            }
        }

        private void toolStripButtonCronTabOpen_Click(object sender, EventArgs e)
        {
            openFileDialog.Filter = "Файл конфигурации Cron (*.tab)|*.tab|Все файлы (*.*)|*.*";
            if (openFileDialog.ShowDialog() == DialogResult.Cancel)
                return;
            // получаем выбранный файл
            string filename = openFileDialog.FileName;

            richTextBox_tab.LoadFile(filename, RichTextBoxStreamType.PlainText);

            Properties.Settings.Default.settingsCronConfig = filename;
            Properties.Settings.Default.Save();
        }

        private void toolStripButtonCronTabSave_Click(object sender, EventArgs e)
        {
            richTextBox_tab.SaveFile(Properties.Settings.Default.settingsCronConfig, RichTextBoxStreamType.PlainText);
        }

        private void toolStripXML_Paint(object sender, PaintEventArgs e)
        {
            string filename = Properties.Settings.Default.settingsXmlConfig;
            if (Properties.Settings.Default.settingsXmlConfig.Length > 0)
            {
                richTextBox_XML.LoadFile(filename, RichTextBoxStreamType.PlainText);
                HighlightColors.HighlightRTF(richTextBox_XML);
            }
            else
                toolStripButtonXmlOpen_Click(sender, e);
        }

        private void toolStripCron_Paint(object sender, PaintEventArgs e)
        {
            string filename = Properties.Settings.Default.settingsCronConfig;
            if (Properties.Settings.Default.settingsCronConfig.Length > 0)
                richTextBox_tab.LoadFile(filename, RichTextBoxStreamType.PlainText);
            else
                toolStripButtonCronTabOpen_Click(sender, e);
        }

    }
}
