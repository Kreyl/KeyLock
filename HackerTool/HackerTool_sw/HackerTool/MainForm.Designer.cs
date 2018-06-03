namespace HackerTool
{
    partial class MainForm
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.txtConsole = new System.Windows.Forms.TextBox();
            this.serialPort1 = new System.IO.Ports.SerialPort(this.components);
            this.panel1 = new System.Windows.Forms.Panel();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.dgvTable = new System.Windows.Forms.DataGridView();
            this.cl = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl0 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl1 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl2 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl3 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl4 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl5 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl6 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl7 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl8 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.cl9 = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.panel2 = new System.Windows.Forms.Panel();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.label2 = new System.Windows.Forms.Label();
            this.numericUpDown2 = new System.Windows.Forms.NumericUpDown();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.cbAutoreset = new System.Windows.Forms.CheckBox();
            this.lblIterationCounter = new System.Windows.Forms.Label();
            this.textBox3 = new System.Windows.Forms.TextBox();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.btnReset = new System.Windows.Forms.Button();
            this.btnStopIteration = new System.Windows.Forms.Button();
            this.btnIterate = new System.Windows.Forms.Button();
            this.rbtnP3 = new System.Windows.Forms.RadioButton();
            this.rbtnP2 = new System.Windows.Forms.RadioButton();
            this.rbtnP1 = new System.Windows.Forms.RadioButton();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.btnConnect = new System.Windows.Forms.Button();
            this.btnRescan = new System.Windows.Forms.Button();
            this.cbPorts = new System.Windows.Forms.ComboBox();
            this.gbPassSearch = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.tbStartValue = new System.Windows.Forms.TextBox();
            this.btnStop = new System.Windows.Forms.Button();
            this.btnStart = new System.Windows.Forms.Button();
            this.timerIteration = new System.Windows.Forms.Timer(this.components);
            this.BruteForcer = new System.ComponentModel.BackgroundWorker();
            this.groupBox2.SuspendLayout();
            this.panel1.SuspendLayout();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgvTable)).BeginInit();
            this.panel2.SuspendLayout();
            this.groupBox4.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.gbPassSearch.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.txtConsole);
            this.groupBox2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox2.Location = new System.Drawing.Point(0, 311);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(587, 194);
            this.groupBox2.TabIndex = 3;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Console";
            // 
            // txtConsole
            // 
            this.txtConsole.BackColor = System.Drawing.Color.Black;
            this.txtConsole.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txtConsole.Font = new System.Drawing.Font("Courier New", 9F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.txtConsole.ForeColor = System.Drawing.Color.DeepSkyBlue;
            this.txtConsole.Location = new System.Drawing.Point(3, 16);
            this.txtConsole.Multiline = true;
            this.txtConsole.Name = "txtConsole";
            this.txtConsole.ReadOnly = true;
            this.txtConsole.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.txtConsole.Size = new System.Drawing.Size(581, 175);
            this.txtConsole.TabIndex = 0;
            this.txtConsole.Text = "Console";
            this.txtConsole.KeyUp += new System.Windows.Forms.KeyEventHandler(this.Console_KeyUp);
            // 
            // serialPort1
            // 
            this.serialPort1.DataReceived += new System.IO.Ports.SerialDataReceivedEventHandler(this.SerialPort1_DataReceived);
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.groupBox3);
            this.panel1.Controls.Add(this.panel2);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
            this.panel1.Location = new System.Drawing.Point(0, 0);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(587, 311);
            this.panel1.TabIndex = 2;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.dgvTable);
            this.groupBox3.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBox3.Location = new System.Drawing.Point(280, 0);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(307, 311);
            this.groupBox3.TabIndex = 6;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Analyzer";
            // 
            // dgvTable
            // 
            this.dgvTable.AllowUserToAddRows = false;
            this.dgvTable.AllowUserToDeleteRows = false;
            this.dgvTable.AllowUserToResizeColumns = false;
            this.dgvTable.AllowUserToResizeRows = false;
            this.dgvTable.ClipboardCopyMode = System.Windows.Forms.DataGridViewClipboardCopyMode.Disable;
            this.dgvTable.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvTable.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.cl,
            this.cl0,
            this.cl1,
            this.cl2,
            this.cl3,
            this.cl4,
            this.cl5,
            this.cl6,
            this.cl7,
            this.cl8,
            this.cl9});
            this.dgvTable.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dgvTable.Enabled = false;
            this.dgvTable.Location = new System.Drawing.Point(3, 16);
            this.dgvTable.Name = "dgvTable";
            this.dgvTable.ReadOnly = true;
            this.dgvTable.RowHeadersVisible = false;
            this.dgvTable.RowTemplate.Height = 27;
            this.dgvTable.ScrollBars = System.Windows.Forms.ScrollBars.None;
            this.dgvTable.ShowCellErrors = false;
            this.dgvTable.ShowCellToolTips = false;
            this.dgvTable.ShowEditingIcon = false;
            this.dgvTable.ShowRowErrors = false;
            this.dgvTable.Size = new System.Drawing.Size(301, 292);
            this.dgvTable.TabIndex = 2;
            this.dgvTable.TabStop = false;
            // 
            // cl
            // 
            this.cl.Frozen = true;
            this.cl.HeaderText = "";
            this.cl.Name = "cl";
            this.cl.ReadOnly = true;
            this.cl.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.cl.Width = 27;
            // 
            // cl0
            // 
            this.cl0.HeaderText = "0";
            this.cl0.Name = "cl0";
            this.cl0.ReadOnly = true;
            this.cl0.Resizable = System.Windows.Forms.DataGridViewTriState.False;
            this.cl0.Width = 27;
            // 
            // cl1
            // 
            this.cl1.HeaderText = "1";
            this.cl1.Name = "cl1";
            this.cl1.ReadOnly = true;
            this.cl1.Width = 27;
            // 
            // cl2
            // 
            this.cl2.HeaderText = "2";
            this.cl2.Name = "cl2";
            this.cl2.ReadOnly = true;
            this.cl2.Width = 27;
            // 
            // cl3
            // 
            this.cl3.HeaderText = "3";
            this.cl3.Name = "cl3";
            this.cl3.ReadOnly = true;
            this.cl3.Width = 27;
            // 
            // cl4
            // 
            this.cl4.HeaderText = "4";
            this.cl4.Name = "cl4";
            this.cl4.ReadOnly = true;
            this.cl4.Width = 27;
            // 
            // cl5
            // 
            this.cl5.HeaderText = "5";
            this.cl5.Name = "cl5";
            this.cl5.ReadOnly = true;
            this.cl5.Width = 27;
            // 
            // cl6
            // 
            this.cl6.HeaderText = "6";
            this.cl6.Name = "cl6";
            this.cl6.ReadOnly = true;
            this.cl6.Width = 27;
            // 
            // cl7
            // 
            this.cl7.HeaderText = "7";
            this.cl7.Name = "cl7";
            this.cl7.ReadOnly = true;
            this.cl7.Width = 27;
            // 
            // cl8
            // 
            this.cl8.HeaderText = "8";
            this.cl8.Name = "cl8";
            this.cl8.ReadOnly = true;
            this.cl8.Width = 27;
            // 
            // cl9
            // 
            this.cl9.HeaderText = "9";
            this.cl9.Name = "cl9";
            this.cl9.ReadOnly = true;
            this.cl9.Width = 27;
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.groupBox4);
            this.panel2.Controls.Add(this.groupBox1);
            this.panel2.Controls.Add(this.gbPassSearch);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Left;
            this.panel2.Location = new System.Drawing.Point(0, 0);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(280, 311);
            this.panel2.TabIndex = 5;
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.label2);
            this.groupBox4.Controls.Add(this.numericUpDown2);
            this.groupBox4.Controls.Add(this.progressBar1);
            this.groupBox4.Controls.Add(this.cbAutoreset);
            this.groupBox4.Controls.Add(this.lblIterationCounter);
            this.groupBox4.Controls.Add(this.textBox3);
            this.groupBox4.Controls.Add(this.textBox2);
            this.groupBox4.Controls.Add(this.textBox1);
            this.groupBox4.Controls.Add(this.btnReset);
            this.groupBox4.Controls.Add(this.btnStopIteration);
            this.groupBox4.Controls.Add(this.btnIterate);
            this.groupBox4.Controls.Add(this.rbtnP3);
            this.groupBox4.Controls.Add(this.rbtnP2);
            this.groupBox4.Controls.Add(this.rbtnP1);
            this.groupBox4.Location = new System.Drawing.Point(12, 158);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(262, 147);
            this.groupBox4.TabIndex = 6;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Stethoscope";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(159, 103);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(49, 13);
            this.label2.TabIndex = 12;
            this.label2.Text = "iterations";
            // 
            // numericUpDown2
            // 
            this.numericUpDown2.Location = new System.Drawing.Point(98, 101);
            this.numericUpDown2.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numericUpDown2.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDown2.Name = "numericUpDown2";
            this.numericUpDown2.Size = new System.Drawing.Size(55, 20);
            this.numericUpDown2.TabIndex = 11;
            this.numericUpDown2.Value = new decimal(new int[] {
            20,
            0,
            0,
            0});
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(117, 126);
            this.progressBar1.Maximum = 4;
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(139, 15);
            this.progressBar1.Step = 1;
            this.progressBar1.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
            this.progressBar1.TabIndex = 7;
            this.progressBar1.Visible = false;
            // 
            // cbAutoreset
            // 
            this.cbAutoreset.AutoSize = true;
            this.cbAutoreset.Location = new System.Drawing.Point(15, 102);
            this.cbAutoreset.Name = "cbAutoreset";
            this.cbAutoreset.Size = new System.Drawing.Size(83, 17);
            this.cbAutoreset.TabIndex = 10;
            this.cbAutoreset.Text = "Reset every";
            this.cbAutoreset.UseVisualStyleBackColor = true;
            // 
            // lblIterationCounter
            // 
            this.lblIterationCounter.AutoSize = true;
            this.lblIterationCounter.Location = new System.Drawing.Point(12, 122);
            this.lblIterationCounter.Name = "lblIterationCounter";
            this.lblIterationCounter.Size = new System.Drawing.Size(48, 13);
            this.lblIterationCounter.TabIndex = 9;
            this.lblIterationCounter.Text = "Iteration:";
            // 
            // textBox3
            // 
            this.textBox3.Location = new System.Drawing.Point(73, 67);
            this.textBox3.Name = "textBox3";
            this.textBox3.Size = new System.Drawing.Size(78, 20);
            this.textBox3.TabIndex = 8;
            // 
            // textBox2
            // 
            this.textBox2.Location = new System.Drawing.Point(73, 44);
            this.textBox2.Name = "textBox2";
            this.textBox2.Size = new System.Drawing.Size(78, 20);
            this.textBox2.TabIndex = 7;
            // 
            // textBox1
            // 
            this.textBox1.Location = new System.Drawing.Point(73, 22);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(78, 20);
            this.textBox1.TabIndex = 6;
            // 
            // btnReset
            // 
            this.btnReset.Image = global::HackerTool.Properties.Resources.grafic_hide_all;
            this.btnReset.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnReset.Location = new System.Drawing.Point(169, 77);
            this.btnReset.Name = "btnReset";
            this.btnReset.Size = new System.Drawing.Size(75, 23);
            this.btnReset.TabIndex = 5;
            this.btnReset.Text = "Reset";
            this.btnReset.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.btnReset.UseVisualStyleBackColor = true;
            this.btnReset.Click += new System.EventHandler(this.BtnReset_Click);
            // 
            // btnStopIteration
            // 
            this.btnStopIteration.Image = global::HackerTool.Properties.Resources.stop_16x16;
            this.btnStopIteration.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnStopIteration.Location = new System.Drawing.Point(169, 48);
            this.btnStopIteration.Name = "btnStopIteration";
            this.btnStopIteration.Size = new System.Drawing.Size(75, 23);
            this.btnStopIteration.TabIndex = 4;
            this.btnStopIteration.Text = "Stop";
            this.btnStopIteration.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.btnStopIteration.UseVisualStyleBackColor = true;
            this.btnStopIteration.Click += new System.EventHandler(this.BtnStopIteration_Click);
            // 
            // btnIterate
            // 
            this.btnIterate.Image = global::HackerTool.Properties.Resources.start;
            this.btnIterate.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnIterate.Location = new System.Drawing.Point(169, 19);
            this.btnIterate.Name = "btnIterate";
            this.btnIterate.Size = new System.Drawing.Size(75, 23);
            this.btnIterate.TabIndex = 3;
            this.btnIterate.Text = "Start";
            this.btnIterate.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.btnIterate.UseVisualStyleBackColor = true;
            this.btnIterate.Click += new System.EventHandler(this.BtnIterate_Click);
            // 
            // rbtnP3
            // 
            this.rbtnP3.AutoSize = true;
            this.rbtnP3.Location = new System.Drawing.Point(15, 68);
            this.rbtnP3.Name = "rbtnP3";
            this.rbtnP3.Size = new System.Drawing.Size(49, 17);
            this.rbtnP3.TabIndex = 2;
            this.rbtnP3.Text = "Pair3";
            this.rbtnP3.UseVisualStyleBackColor = true;
            // 
            // rbtnP2
            // 
            this.rbtnP2.AutoSize = true;
            this.rbtnP2.Location = new System.Drawing.Point(15, 45);
            this.rbtnP2.Name = "rbtnP2";
            this.rbtnP2.Size = new System.Drawing.Size(49, 17);
            this.rbtnP2.TabIndex = 1;
            this.rbtnP2.Text = "Pair2";
            this.rbtnP2.UseVisualStyleBackColor = true;
            // 
            // rbtnP1
            // 
            this.rbtnP1.AutoSize = true;
            this.rbtnP1.Checked = true;
            this.rbtnP1.Location = new System.Drawing.Point(15, 22);
            this.rbtnP1.Name = "rbtnP1";
            this.rbtnP1.Size = new System.Drawing.Size(52, 17);
            this.rbtnP1.TabIndex = 0;
            this.rbtnP1.TabStop = true;
            this.rbtnP1.Text = "Pair 1";
            this.rbtnP1.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.btnConnect);
            this.groupBox1.Controls.Add(this.btnRescan);
            this.groupBox1.Controls.Add(this.cbPorts);
            this.groupBox1.Location = new System.Drawing.Point(12, 3);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(262, 68);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Port settings";
            // 
            // btnConnect
            // 
            this.btnConnect.Image = global::HackerTool.Properties.Resources.connect2;
            this.btnConnect.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnConnect.Location = new System.Drawing.Point(169, 30);
            this.btnConnect.Name = "btnConnect";
            this.btnConnect.Size = new System.Drawing.Size(75, 26);
            this.btnConnect.TabIndex = 2;
            this.btnConnect.Text = "Connect";
            this.btnConnect.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.btnConnect.UseVisualStyleBackColor = true;
            this.btnConnect.Click += new System.EventHandler(this.BtnConnect_Click);
            // 
            // btnRescan
            // 
            this.btnRescan.Image = global::HackerTool.Properties.Resources.count;
            this.btnRescan.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnRescan.Location = new System.Drawing.Point(17, 30);
            this.btnRescan.Name = "btnRescan";
            this.btnRescan.Size = new System.Drawing.Size(75, 26);
            this.btnRescan.TabIndex = 1;
            this.btnRescan.Text = "Rescan";
            this.btnRescan.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.btnRescan.UseVisualStyleBackColor = true;
            this.btnRescan.Click += new System.EventHandler(this.BtnRescan_Click);
            // 
            // cbPorts
            // 
            this.cbPorts.FormattingEnabled = true;
            this.cbPorts.Location = new System.Drawing.Point(98, 34);
            this.cbPorts.Name = "cbPorts";
            this.cbPorts.Size = new System.Drawing.Size(65, 21);
            this.cbPorts.TabIndex = 0;
            // 
            // gbPassSearch
            // 
            this.gbPassSearch.Controls.Add(this.label1);
            this.gbPassSearch.Controls.Add(this.tbStartValue);
            this.gbPassSearch.Controls.Add(this.btnStop);
            this.gbPassSearch.Controls.Add(this.btnStart);
            this.gbPassSearch.Location = new System.Drawing.Point(12, 72);
            this.gbPassSearch.Name = "gbPassSearch";
            this.gbPassSearch.Size = new System.Drawing.Size(262, 80);
            this.gbPassSearch.TabIndex = 4;
            this.gbPassSearch.TabStop = false;
            this.gbPassSearch.Text = "Brute force";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(61, 13);
            this.label1.TabIndex = 3;
            this.label1.Text = "Start value:";
            // 
            // tbStartValue
            // 
            this.tbStartValue.Location = new System.Drawing.Point(73, 19);
            this.tbStartValue.Name = "tbStartValue";
            this.tbStartValue.Size = new System.Drawing.Size(50, 20);
            this.tbStartValue.TabIndex = 2;
            this.tbStartValue.Text = "555555";
            this.tbStartValue.TextChanged += new System.EventHandler(this.TbStartValue_TextChanged);
            // 
            // btnStop
            // 
            this.btnStop.Image = global::HackerTool.Properties.Resources.stop_16x16;
            this.btnStop.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnStop.Location = new System.Drawing.Point(169, 45);
            this.btnStop.Name = "btnStop";
            this.btnStop.Size = new System.Drawing.Size(75, 23);
            this.btnStop.TabIndex = 1;
            this.btnStop.Text = "Stop";
            this.btnStop.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.btnStop.UseVisualStyleBackColor = true;
            this.btnStop.Click += new System.EventHandler(this.BtnStop_Click);
            // 
            // btnStart
            // 
            this.btnStart.Image = global::HackerTool.Properties.Resources.start;
            this.btnStart.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.btnStart.Location = new System.Drawing.Point(169, 16);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(75, 23);
            this.btnStart.TabIndex = 0;
            this.btnStart.Text = "Start";
            this.btnStart.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.btnStart.UseVisualStyleBackColor = true;
            this.btnStart.Click += new System.EventHandler(this.BtnStart_Click);
            // 
            // timerIteration
            // 
            this.timerIteration.Interval = 27;
            this.timerIteration.Tick += new System.EventHandler(this.TimerIteration_Tick);
            // 
            // BruteForcer
            // 
            this.BruteForcer.WorkerReportsProgress = true;
            this.BruteForcer.WorkerSupportsCancellation = true;
            this.BruteForcer.DoWork += new System.ComponentModel.DoWorkEventHandler(this.BruteForcer_DoWork);
            this.BruteForcer.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.BruteForcer_ProgressChanged);
            this.BruteForcer.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.BruteForcer_RunWorkerCompleted);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(587, 505);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.panel1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximumSize = new System.Drawing.Size(878, 10000);
            this.MinimumSize = new System.Drawing.Size(603, 450);
            this.Name = "MainForm";
            this.Text = "10ck uP";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MainForm_FormClosing);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.MainForm_Paint);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.panel1.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.dgvTable)).EndInit();
            this.panel2.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDown2)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.gbPassSearch.ResumeLayout(false);
            this.gbPassSearch.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.TextBox txtConsole;
        private System.IO.Ports.SerialPort serialPort1;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Panel panel2;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.Button btnRescan;
        private System.Windows.Forms.ComboBox cbPorts;
        private System.Windows.Forms.GroupBox gbPassSearch;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbStartValue;
        private System.Windows.Forms.Button btnStop;
        private System.Windows.Forms.Button btnStart;
        private System.Windows.Forms.DataGridView dgvTable;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl0;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl1;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl2;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl3;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl4;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl5;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl6;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl7;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl8;
        private System.Windows.Forms.DataGridViewTextBoxColumn cl9;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Button btnReset;
        private System.Windows.Forms.Button btnStopIteration;
        private System.Windows.Forms.Button btnIterate;
        private System.Windows.Forms.RadioButton rbtnP3;
        private System.Windows.Forms.RadioButton rbtnP2;
        private System.Windows.Forms.RadioButton rbtnP1;
        private System.Windows.Forms.TextBox textBox3;
        private System.Windows.Forms.TextBox textBox2;
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.Timer timerIteration;
        private System.Windows.Forms.Label lblIterationCounter;
        private System.ComponentModel.BackgroundWorker BruteForcer;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.CheckBox cbAutoreset;
        private System.Windows.Forms.NumericUpDown numericUpDown2;
        private System.Windows.Forms.Label label2;

    }
}

