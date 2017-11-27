namespace parus
{
    partial class FormMain
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
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea2 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Series series2 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.DataPoint dataPoint7 = new System.Windows.Forms.DataVisualization.Charting.DataPoint(0D, 0D);
            System.Windows.Forms.DataVisualization.Charting.DataPoint dataPoint8 = new System.Windows.Forms.DataVisualization.Charting.DataPoint(1D, 1D);
            System.Windows.Forms.DataVisualization.Charting.DataPoint dataPoint9 = new System.Windows.Forms.DataVisualization.Charting.DataPoint(2D, 2D);
            System.Windows.Forms.DataVisualization.Charting.DataPoint dataPoint10 = new System.Windows.Forms.DataVisualization.Charting.DataPoint(3D, 2D);
            System.Windows.Forms.DataVisualization.Charting.DataPoint dataPoint11 = new System.Windows.Forms.DataVisualization.Charting.DataPoint(4D, 1D);
            System.Windows.Forms.DataVisualization.Charting.DataPoint dataPoint12 = new System.Windows.Forms.DataVisualization.Charting.DataPoint(5D, 0D);
            System.Windows.Forms.DataVisualization.Charting.Title title2 = new System.Windows.Forms.DataVisualization.Charting.Title();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
            this.tabControl = new System.Windows.Forms.TabControl();
            this.tabPageIonogram = new System.Windows.Forms.TabPage();
            this.chartIonogram = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.panelIonogramFiles = new System.Windows.Forms.Panel();
            this.listBoxIonograms = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tabPageAmplitudes = new System.Windows.Forms.TabPage();
            this.tabPageCalibration = new System.Windows.Forms.TabPage();
            this.tabPageParameters = new System.Windows.Forms.TabPage();
            this.panel1 = new System.Windows.Forms.Panel();
            this.richTextBox_XML = new System.Windows.Forms.RichTextBox();
            this.toolStripXML = new System.Windows.Forms.ToolStrip();
            this.toolStripButtonXmlOpen = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonXmlSave = new System.Windows.Forms.ToolStripButton();
            this.menuStripMain = new System.Windows.Forms.MenuStrip();
            this.toolStripMenuItemFile = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemOpenFile = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemOpenDir = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuExternal = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuItemExit = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemHelp = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemHelpContent = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItemHelpAbout = new System.Windows.Forms.ToolStripMenuItem();
            this.folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            this.openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabelDirectory = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStripStatusLabelPolnt = new System.Windows.Forms.ToolStripStatusLabel();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripButtonChangeDir = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonRefresh = new System.Windows.Forms.ToolStripButton();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripButtonIonogram = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonAmplitudes = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCalibration = new System.Windows.Forms.ToolStripButton();
            this.tabPageCron = new System.Windows.Forms.TabPage();
            this.toolStrip2 = new System.Windows.Forms.ToolStrip();
            this.toolStripButtonCronTabOpen = new System.Windows.Forms.ToolStripButton();
            this.toolStripButtonCronTabSave = new System.Windows.Forms.ToolStripButton();
            this.richTextBox_tab = new System.Windows.Forms.RichTextBox();
            this.tabControl.SuspendLayout();
            this.tabPageIonogram.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chartIonogram)).BeginInit();
            this.panelIonogramFiles.SuspendLayout();
            this.tabPageParameters.SuspendLayout();
            this.panel1.SuspendLayout();
            this.toolStripXML.SuspendLayout();
            this.menuStripMain.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.tabPageCron.SuspendLayout();
            this.toolStrip2.SuspendLayout();
            this.SuspendLayout();
            // 
            // tabControl
            // 
            this.tabControl.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.tabControl.Controls.Add(this.tabPageIonogram);
            this.tabControl.Controls.Add(this.tabPageAmplitudes);
            this.tabControl.Controls.Add(this.tabPageCalibration);
            this.tabControl.Controls.Add(this.tabPageParameters);
            this.tabControl.Controls.Add(this.tabPageCron);
            this.tabControl.Location = new System.Drawing.Point(0, 52);
            this.tabControl.Name = "tabControl";
            this.tabControl.SelectedIndex = 0;
            this.tabControl.ShowToolTips = true;
            this.tabControl.Size = new System.Drawing.Size(798, 358);
            this.tabControl.TabIndex = 0;
            // 
            // tabPageIonogram
            // 
            this.tabPageIonogram.Controls.Add(this.chartIonogram);
            this.tabPageIonogram.Controls.Add(this.panelIonogramFiles);
            this.tabPageIonogram.Location = new System.Drawing.Point(4, 22);
            this.tabPageIonogram.Name = "tabPageIonogram";
            this.tabPageIonogram.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageIonogram.Size = new System.Drawing.Size(790, 332);
            this.tabPageIonogram.TabIndex = 0;
            this.tabPageIonogram.Text = "Ионограмма";
            this.tabPageIonogram.UseVisualStyleBackColor = true;
            // 
            // chartIonogram
            // 
            chartArea2.AxisX.MajorGrid.LineDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.Dash;
            chartArea2.AxisX.Title = "Частота, МГц";
            chartArea2.AxisX.TitleFont = new System.Drawing.Font("Times New Roman", 12F);
            chartArea2.AxisX.ToolTip = "wertyu";
            chartArea2.AxisX2.LineDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.NotSet;
            chartArea2.AxisY.MajorGrid.LineDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.Dash;
            chartArea2.AxisY.Title = "Действующая высота, км";
            chartArea2.AxisY.TitleFont = new System.Drawing.Font("Times New Roman", 12F);
            chartArea2.AxisY2.LineDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.NotSet;
            chartArea2.BorderDashStyle = System.Windows.Forms.DataVisualization.Charting.ChartDashStyle.Solid;
            chartArea2.Name = "ChartAreaIonogram";
            this.chartIonogram.ChartAreas.Add(chartArea2);
            this.chartIonogram.Dock = System.Windows.Forms.DockStyle.Fill;
            this.chartIonogram.Location = new System.Drawing.Point(3, 3);
            this.chartIonogram.Name = "chartIonogram";
            series2.ChartArea = "ChartAreaIonogram";
            series2.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.FastPoint;
            series2.Color = System.Drawing.Color.Transparent;
            series2.Name = "Series1";
            series2.Points.Add(dataPoint7);
            series2.Points.Add(dataPoint8);
            series2.Points.Add(dataPoint9);
            series2.Points.Add(dataPoint10);
            series2.Points.Add(dataPoint11);
            series2.Points.Add(dataPoint12);
            this.chartIonogram.Series.Add(series2);
            this.chartIonogram.Size = new System.Drawing.Size(596, 326);
            this.chartIonogram.TabIndex = 5;
            this.chartIonogram.Text = "chart1";
            title2.Font = new System.Drawing.Font("Times New Roman", 15.75F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            title2.Name = "TitleTimeIonogram";
            this.chartIonogram.Titles.Add(title2);
            this.chartIonogram.Paint += new System.Windows.Forms.PaintEventHandler(this.chartIonogram_Paint);
            this.chartIonogram.MouseMove += new System.Windows.Forms.MouseEventHandler(this.chartIonogram_MouseMove);
            // 
            // panelIonogramFiles
            // 
            this.panelIonogramFiles.Controls.Add(this.listBoxIonograms);
            this.panelIonogramFiles.Controls.Add(this.label1);
            this.panelIonogramFiles.Dock = System.Windows.Forms.DockStyle.Right;
            this.panelIonogramFiles.Location = new System.Drawing.Point(599, 3);
            this.panelIonogramFiles.Name = "panelIonogramFiles";
            this.panelIonogramFiles.Size = new System.Drawing.Size(188, 326);
            this.panelIonogramFiles.TabIndex = 0;
            // 
            // listBoxIonograms
            // 
            this.listBoxIonograms.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listBoxIonograms.FormattingEnabled = true;
            this.listBoxIonograms.Location = new System.Drawing.Point(0, 13);
            this.listBoxIonograms.Margin = new System.Windows.Forms.Padding(10);
            this.listBoxIonograms.Name = "listBoxIonograms";
            this.listBoxIonograms.Size = new System.Drawing.Size(188, 313);
            this.listBoxIonograms.TabIndex = 1;
            this.listBoxIonograms.SelectedValueChanged += new System.EventHandler(this.listBoxIonograms_SelectedValueChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Dock = System.Windows.Forms.DockStyle.Top;
            this.label1.Location = new System.Drawing.Point(0, 0);
            this.label1.Margin = new System.Windows.Forms.Padding(10);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(160, 13);
            this.label1.TabIndex = 0;
            this.label1.Text = "Ионограммы в рабочей папке";
            // 
            // tabPageAmplitudes
            // 
            this.tabPageAmplitudes.Location = new System.Drawing.Point(4, 22);
            this.tabPageAmplitudes.Name = "tabPageAmplitudes";
            this.tabPageAmplitudes.Size = new System.Drawing.Size(790, 332);
            this.tabPageAmplitudes.TabIndex = 2;
            this.tabPageAmplitudes.Text = "Амплитуды";
            this.tabPageAmplitudes.UseVisualStyleBackColor = true;
            // 
            // tabPageCalibration
            // 
            this.tabPageCalibration.Location = new System.Drawing.Point(4, 22);
            this.tabPageCalibration.Name = "tabPageCalibration";
            this.tabPageCalibration.Size = new System.Drawing.Size(790, 332);
            this.tabPageCalibration.TabIndex = 3;
            this.tabPageCalibration.Text = "Калибровка";
            this.tabPageCalibration.UseVisualStyleBackColor = true;
            // 
            // tabPageParameters
            // 
            this.tabPageParameters.Controls.Add(this.panel1);
            this.tabPageParameters.Controls.Add(this.toolStripXML);
            this.tabPageParameters.Location = new System.Drawing.Point(4, 22);
            this.tabPageParameters.Name = "tabPageParameters";
            this.tabPageParameters.Padding = new System.Windows.Forms.Padding(3);
            this.tabPageParameters.Size = new System.Drawing.Size(790, 332);
            this.tabPageParameters.TabIndex = 1;
            this.tabPageParameters.Text = "Параметры зондирования";
            this.tabPageParameters.UseVisualStyleBackColor = true;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.richTextBox_XML);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 28);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(784, 301);
            this.panel1.TabIndex = 2;
            // 
            // richTextBox_XML
            // 
            this.richTextBox_XML.Dock = System.Windows.Forms.DockStyle.Fill;
            this.richTextBox_XML.Font = new System.Drawing.Font("Microsoft Sans Serif", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.richTextBox_XML.Location = new System.Drawing.Point(0, 0);
            this.richTextBox_XML.Name = "richTextBox_XML";
            this.richTextBox_XML.Size = new System.Drawing.Size(784, 301);
            this.richTextBox_XML.TabIndex = 1;
            this.richTextBox_XML.Text = "";
            // 
            // toolStripXML
            // 
            this.toolStripXML.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonXmlOpen,
            this.toolStripButtonXmlSave});
            this.toolStripXML.Location = new System.Drawing.Point(3, 3);
            this.toolStripXML.Name = "toolStripXML";
            this.toolStripXML.Size = new System.Drawing.Size(784, 25);
            this.toolStripXML.TabIndex = 1;
            this.toolStripXML.Text = "toolStrip2";
            // 
            // toolStripButtonXmlOpen
            // 
            this.toolStripButtonXmlOpen.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonXmlOpen.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonXmlOpen.Image")));
            this.toolStripButtonXmlOpen.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonXmlOpen.Name = "toolStripButtonXmlOpen";
            this.toolStripButtonXmlOpen.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonXmlOpen.Text = "toolStripButtonXmlOpen";
            this.toolStripButtonXmlOpen.ToolTipText = "Открыть xml-файл конфигурации измерений\r\n";
            this.toolStripButtonXmlOpen.Click += new System.EventHandler(this.toolStripButtonXmlOpen_Click);
            // 
            // toolStripButtonXmlSave
            // 
            this.toolStripButtonXmlSave.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonXmlSave.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonXmlSave.Image")));
            this.toolStripButtonXmlSave.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonXmlSave.Name = "toolStripButtonXmlSave";
            this.toolStripButtonXmlSave.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonXmlSave.Text = "toolStripButton2";
            this.toolStripButtonXmlSave.ToolTipText = "Сохранить xml-файл конфигурации измерений";
            this.toolStripButtonXmlSave.Click += new System.EventHandler(this.toolStripButtonXmlSave_Click);
            // 
            // menuStripMain
            // 
            this.menuStripMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemFile,
            this.toolStripMenuItemHelp});
            this.menuStripMain.Location = new System.Drawing.Point(0, 0);
            this.menuStripMain.Name = "menuStripMain";
            this.menuStripMain.Size = new System.Drawing.Size(798, 24);
            this.menuStripMain.TabIndex = 1;
            this.menuStripMain.Text = "menuStrip";
            // 
            // toolStripMenuItemFile
            // 
            this.toolStripMenuItemFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemOpenFile,
            this.toolStripMenuItemOpenDir,
            this.toolStripSeparator1,
            this.toolStripMenuExternal,
            this.toolStripSeparator2,
            this.toolStripMenuItemExit});
            this.toolStripMenuItemFile.Name = "toolStripMenuItemFile";
            this.toolStripMenuItemFile.Size = new System.Drawing.Size(48, 20);
            this.toolStripMenuItemFile.Text = "Файл";
            // 
            // toolStripMenuItemOpenFile
            // 
            this.toolStripMenuItemOpenFile.Name = "toolStripMenuItemOpenFile";
            this.toolStripMenuItemOpenFile.Size = new System.Drawing.Size(194, 22);
            this.toolStripMenuItemOpenFile.Text = "Открыть файл...";
            this.toolStripMenuItemOpenFile.Click += new System.EventHandler(this.toolStripMenuItemOpenFile_Click);
            // 
            // toolStripMenuItemOpenDir
            // 
            this.toolStripMenuItemOpenDir.Name = "toolStripMenuItemOpenDir";
            this.toolStripMenuItemOpenDir.Size = new System.Drawing.Size(194, 22);
            this.toolStripMenuItemOpenDir.Text = "Рабочая папка...";
            this.toolStripMenuItemOpenDir.Click += new System.EventHandler(this.toolStripMenuItemOpenDir_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(191, 6);
            // 
            // toolStripMenuExternal
            // 
            this.toolStripMenuExternal.Name = "toolStripMenuExternal";
            this.toolStripMenuExternal.Size = new System.Drawing.Size(194, 22);
            this.toolStripMenuExternal.Text = "Внешние программы";
            this.toolStripMenuExternal.Click += new System.EventHandler(this.toolStripMenuExternal_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(191, 6);
            // 
            // toolStripMenuItemExit
            // 
            this.toolStripMenuItemExit.Name = "toolStripMenuItemExit";
            this.toolStripMenuItemExit.Size = new System.Drawing.Size(194, 22);
            this.toolStripMenuItemExit.Text = "Выход";
            this.toolStripMenuItemExit.Click += new System.EventHandler(this.toolStripMenuItemExit_Click);
            // 
            // toolStripMenuItemHelp
            // 
            this.toolStripMenuItemHelp.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItemHelpContent,
            this.toolStripMenuItemHelpAbout});
            this.toolStripMenuItemHelp.Name = "toolStripMenuItemHelp";
            this.toolStripMenuItemHelp.Size = new System.Drawing.Size(65, 20);
            this.toolStripMenuItemHelp.Text = "Справка";
            // 
            // toolStripMenuItemHelpContent
            // 
            this.toolStripMenuItemHelpContent.Name = "toolStripMenuItemHelpContent";
            this.toolStripMenuItemHelpContent.ShortcutKeys = System.Windows.Forms.Keys.F1;
            this.toolStripMenuItemHelpContent.Size = new System.Drawing.Size(162, 22);
            this.toolStripMenuItemHelpContent.Text = "Содержание";
            // 
            // toolStripMenuItemHelpAbout
            // 
            this.toolStripMenuItemHelpAbout.Name = "toolStripMenuItemHelpAbout";
            this.toolStripMenuItemHelpAbout.Size = new System.Drawing.Size(162, 22);
            this.toolStripMenuItemHelpAbout.Text = "О программе";
            this.toolStripMenuItemHelpAbout.Click += new System.EventHandler(this.toolStripMenuItemHelpAbout_Click);
            // 
            // folderBrowserDialog
            // 
            this.folderBrowserDialog.Description = "Рабочая папка";
            // 
            // openFileDialog
            // 
            this.openFileDialog.Filter = "Ионограммы (*.ion)|*.ion|Амплитуды (*.frq)|*.frq|Все файлы (*.*)|*.*";
            this.openFileDialog.Title = "Открыть файл";
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabelDirectory,
            this.toolStripStatusLabelPolnt});
            this.statusStrip.Location = new System.Drawing.Point(0, 411);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(798, 24);
            this.statusStrip.TabIndex = 3;
            this.statusStrip.Text = "statusStrip1";
            // 
            // toolStripStatusLabelDirectory
            // 
            this.toolStripStatusLabelDirectory.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
            this.toolStripStatusLabelDirectory.Name = "toolStripStatusLabelDirectory";
            this.toolStripStatusLabelDirectory.Size = new System.Drawing.Size(51, 19);
            this.toolStripStatusLabelDirectory.Text = "Папка: ";
            // 
            // toolStripStatusLabelPolnt
            // 
            this.toolStripStatusLabelPolnt.BorderSides = System.Windows.Forms.ToolStripStatusLabelBorderSides.Right;
            this.toolStripStatusLabelPolnt.Name = "toolStripStatusLabelPolnt";
            this.toolStripStatusLabelPolnt.Size = new System.Drawing.Size(50, 19);
            this.toolStripStatusLabelPolnt.Text = "Точка: ";
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonChangeDir,
            this.toolStripButtonRefresh,
            this.toolStripSeparator3,
            this.toolStripButtonIonogram,
            this.toolStripButtonAmplitudes,
            this.toolStripButtonCalibration});
            this.toolStrip1.Location = new System.Drawing.Point(0, 24);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(798, 25);
            this.toolStrip1.TabIndex = 4;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // toolStripButtonChangeDir
            // 
            this.toolStripButtonChangeDir.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonChangeDir.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonChangeDir.Image")));
            this.toolStripButtonChangeDir.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonChangeDir.Name = "toolStripButtonChangeDir";
            this.toolStripButtonChangeDir.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonChangeDir.Text = "toolStripButton1";
            this.toolStripButtonChangeDir.ToolTipText = "Изменить рабочую папку";
            this.toolStripButtonChangeDir.Click += new System.EventHandler(this.toolStripButtonChangeDir_Click);
            // 
            // toolStripButtonRefresh
            // 
            this.toolStripButtonRefresh.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonRefresh.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonRefresh.Image")));
            this.toolStripButtonRefresh.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonRefresh.Name = "toolStripButtonRefresh";
            this.toolStripButtonRefresh.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonRefresh.Text = "toolStripButton1";
            this.toolStripButtonRefresh.ToolTipText = "Обновить список файлов в папке";
            this.toolStripButtonRefresh.Click += new System.EventHandler(this.toolStripButtonRefresh_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(6, 25);
            // 
            // toolStripButtonIonogram
            // 
            this.toolStripButtonIonogram.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonIonogram.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonIonogram.Image")));
            this.toolStripButtonIonogram.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonIonogram.Name = "toolStripButtonIonogram";
            this.toolStripButtonIonogram.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonIonogram.Tag = "0";
            this.toolStripButtonIonogram.Text = "Измерение ионограммы";
            this.toolStripButtonIonogram.Click += new System.EventHandler(this.toolStripButtonExternal_Click);
            // 
            // toolStripButtonAmplitudes
            // 
            this.toolStripButtonAmplitudes.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonAmplitudes.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonAmplitudes.Image")));
            this.toolStripButtonAmplitudes.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonAmplitudes.Name = "toolStripButtonAmplitudes";
            this.toolStripButtonAmplitudes.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonAmplitudes.Tag = "1";
            this.toolStripButtonAmplitudes.Text = "Измерение амплитуд";
            this.toolStripButtonAmplitudes.Click += new System.EventHandler(this.toolStripButtonExternal_Click);
            // 
            // toolStripButtonCalibration
            // 
            this.toolStripButtonCalibration.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCalibration.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCalibration.Image")));
            this.toolStripButtonCalibration.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCalibration.Name = "toolStripButtonCalibration";
            this.toolStripButtonCalibration.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCalibration.Tag = "3";
            this.toolStripButtonCalibration.Text = "Калибровка";
            this.toolStripButtonCalibration.Click += new System.EventHandler(this.toolStripButtonExternal_Click);
            // 
            // tabPageCron
            // 
            this.tabPageCron.Controls.Add(this.richTextBox_tab);
            this.tabPageCron.Controls.Add(this.toolStrip2);
            this.tabPageCron.Location = new System.Drawing.Point(4, 22);
            this.tabPageCron.Name = "tabPageCron";
            this.tabPageCron.Size = new System.Drawing.Size(790, 332);
            this.tabPageCron.TabIndex = 4;
            this.tabPageCron.Text = "Cron.tab";
            this.tabPageCron.UseVisualStyleBackColor = true;
            // 
            // toolStrip2
            // 
            this.toolStrip2.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripButtonCronTabOpen,
            this.toolStripButtonCronTabSave});
            this.toolStrip2.Location = new System.Drawing.Point(0, 0);
            this.toolStrip2.Name = "toolStrip2";
            this.toolStrip2.Size = new System.Drawing.Size(790, 25);
            this.toolStrip2.TabIndex = 2;
            this.toolStrip2.Text = "toolStrip2";
            // 
            // toolStripButtonCronTabOpen
            // 
            this.toolStripButtonCronTabOpen.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCronTabOpen.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCronTabOpen.Image")));
            this.toolStripButtonCronTabOpen.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCronTabOpen.Name = "toolStripButtonCronTabOpen";
            this.toolStripButtonCronTabOpen.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCronTabOpen.Text = "toolStripButtonXmlOpen";
            this.toolStripButtonCronTabOpen.ToolTipText = "Открыть xml-файл конфигурации измерений\r\n";
            this.toolStripButtonCronTabOpen.Click += new System.EventHandler(this.toolStripButtonCronTabOpen_Click);
            // 
            // toolStripButtonCronTabSave
            // 
            this.toolStripButtonCronTabSave.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.toolStripButtonCronTabSave.Image = ((System.Drawing.Image)(resources.GetObject("toolStripButtonCronTabSave.Image")));
            this.toolStripButtonCronTabSave.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.toolStripButtonCronTabSave.Name = "toolStripButtonCronTabSave";
            this.toolStripButtonCronTabSave.Size = new System.Drawing.Size(23, 22);
            this.toolStripButtonCronTabSave.Text = "toolStripButton2";
            this.toolStripButtonCronTabSave.ToolTipText = "Сохранить xml-файл конфигурации измерений";
            this.toolStripButtonCronTabSave.Click += new System.EventHandler(this.toolStripButtonCronTabSave_Click);
            // 
            // richTextBox_tab
            // 
            this.richTextBox_tab.Dock = System.Windows.Forms.DockStyle.Fill;
            this.richTextBox_tab.Font = new System.Drawing.Font("Microsoft Sans Serif", 11F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.richTextBox_tab.Location = new System.Drawing.Point(0, 25);
            this.richTextBox_tab.Name = "richTextBox_tab";
            this.richTextBox_tab.Size = new System.Drawing.Size(790, 307);
            this.richTextBox_tab.TabIndex = 3;
            this.richTextBox_tab.Text = "";
            // 
            // FormMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(798, 435);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.tabControl);
            this.Controls.Add(this.menuStripMain);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStripMain;
            this.Name = "FormMain";
            this.Text = "Интерфейс к ионозонду \"Парус\" НИИФ ЮФУ";
            this.Load += new System.EventHandler(this.FormMain_Load);
            this.tabControl.ResumeLayout(false);
            this.tabPageIonogram.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chartIonogram)).EndInit();
            this.panelIonogramFiles.ResumeLayout(false);
            this.panelIonogramFiles.PerformLayout();
            this.tabPageParameters.ResumeLayout(false);
            this.tabPageParameters.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.toolStripXML.ResumeLayout(false);
            this.toolStripXML.PerformLayout();
            this.menuStripMain.ResumeLayout(false);
            this.menuStripMain.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.tabPageCron.ResumeLayout(false);
            this.tabPageCron.PerformLayout();
            this.toolStrip2.ResumeLayout(false);
            this.toolStrip2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TabControl tabControl;
        private System.Windows.Forms.TabPage tabPageIonogram;
        private System.Windows.Forms.TabPage tabPageParameters;
        private System.Windows.Forms.MenuStrip menuStripMain;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemFile;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemOpenFile;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemOpenDir;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemExit;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemHelp;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemHelpContent;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItemHelpAbout;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog;
        private System.Windows.Forms.OpenFileDialog openFileDialog;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabelDirectory;
        private System.Windows.Forms.DataVisualization.Charting.Chart chartIonogram;
        private System.Windows.Forms.Panel panelIonogramFiles;
        private System.Windows.Forms.ListBox listBoxIonograms;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton toolStripButtonChangeDir;
        private System.Windows.Forms.ToolStripButton toolStripButtonRefresh;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabelPolnt;
        private System.Windows.Forms.ToolStrip toolStripXML;
        private System.Windows.Forms.ToolStripButton toolStripButtonXmlOpen;
        private System.Windows.Forms.ToolStripButton toolStripButtonXmlSave;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.RichTextBox richTextBox_XML;
        private System.Windows.Forms.TabPage tabPageAmplitudes;
        private System.Windows.Forms.TabPage tabPageCalibration;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuExternal;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripButton toolStripButtonIonogram;
        private System.Windows.Forms.ToolStripButton toolStripButtonAmplitudes;
        private System.Windows.Forms.ToolStripButton toolStripButtonCalibration;
        private System.Windows.Forms.TabPage tabPageCron;
        private System.Windows.Forms.ToolStrip toolStrip2;
        private System.Windows.Forms.ToolStripButton toolStripButtonCronTabOpen;
        private System.Windows.Forms.ToolStripButton toolStripButtonCronTabSave;
        private System.Windows.Forms.RichTextBox richTextBox_tab;
    }
}

