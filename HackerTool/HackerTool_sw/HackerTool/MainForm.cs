using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Threading;
using System.Linq;

namespace HackerTool {
    public enum LockState_t { Disconnected, EnteringServiceCode, WaitingCommand, AskingNewCodeA, AskingNewCodeB, AskingNewServiceCode };
    public enum LockSide_t { SideA, SideB };

    public partial class MainForm : Form {
        // ============================= Variables =================================
        public HTool_t HTool;
        public Lock_t Lock;
        private List<string> LockMenu;
        // Iteration
        int CurrentX, CurrentY;
        Random RndValue = new Random();
        int[,] InstantTable, AccumTable;
        int IterationCount = 0;

        #region ============================= Init / deinit ============================
        public MainForm() {
            InitializeComponent();
            txtConsole.Clear();
            HTool = new HTool_t();
            Lock = new Lock_t() { Tool = HTool };
            HTool.ComPort = serialPort1;
            HTool.IConsole = txtConsole;
            FillMenu();
            // Iteration components
            // Setup Grid
            dgvTable.RowCount = 10;
            for (int i = 0; i <= 9; i++) dgvTable.Rows[i].Cells[0].Value = i.ToString();
            // Setup variables
            InstantTable = new int[10, 10];
            AccumTable = new int[10, 10];
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {
            // Stop iterations 
            if (BruteForcer.IsBusy) BruteForcer.CancelAsync();
            
            try {
                HTool.ComPort.Close();
                HTool.ComPort = null;
                serialPort1.Dispose();
            }
            catch (System.Exception ex) {
                HTool.LastError = ex.Message;
            }
            HTool.IsConnected = false;
        }

        private void MainForm_Paint(object sender, PaintEventArgs e) {
            dgvTable.ClearSelection();
        }
        #endregion

        private void TbStartValue_TextChanged(object sender, EventArgs e) {
            if(!UInt32.TryParse(tbStartValue.Text, out uint dummy)) tbStartValue.BackColor = Color.Red;
            else if(tbStartValue.Text.Length != 6) tbStartValue.BackColor = Color.Yellow;
            else tbStartValue.BackColor = Color.FromKnownColor(KnownColor.Window);
        }

        #region ========================= Buttons =========================
        private void BtnRescan_Click(object sender, EventArgs e) {
            txtConsole.AppendText("Scanning system for available ports..." + Environment.NewLine);
            HTool.IsConnected = false;
            Lock.State = LockState_t.Disconnected;
            txtConsole.ReadOnly = true;
            cbPorts.Items.Clear();
            serialPort1.Close();
            string[] SPorts = System.IO.Ports.SerialPort.GetPortNames();
            int NPorts = SPorts.Length;
            if (NPorts == 0) {
                txtConsole.AppendText("Ports not found, mate. No ports - no business.");
                return;
            }
            else {
                cbPorts.Items.AddRange(SPorts);
                cbPorts.SelectedIndex = 0;
                for(int i=0; i<NPorts; i++) txtConsole.AppendText(SPorts[i] + Environment.NewLine);
                txtConsole.AppendText("Ports found: " + NPorts.ToString() + Environment.NewLine);
            }
        }

        private void BtnConnect_Click(object sender, EventArgs e) {
            Lock.State = LockState_t.Disconnected;
            txtConsole.ReadOnly = true;

            if ((cbPorts.Items.Count == 0) || (cbPorts.SelectedItem.ToString() == "")) {
                txtConsole.AppendText("Port not selected, afraid to connect to nowhere" + Environment.NewLine);
                return;
            }
            // Try to connect
            txtConsole.AppendText("Trying to connect to " + cbPorts.SelectedItem.ToString() + Environment.NewLine);
            serialPort1.Close();
            serialPort1.PortName = cbPorts.SelectedItem.ToString();
            try {
                serialPort1.Open();
                // Try to ping device
//                if (HTool.Ping()) { // Device found
//                    txtConsole.AppendText("Tool is found and connected." + Environment.NewLine);
                    HTool.IsConnected = true;
                    txtConsole.AppendText("Trying to connect to lock" + Environment.NewLine + "Connect" + Environment.NewLine);
                    TryToConnectLock();
                    txtConsole.ReadOnly = false;
                    return; // All right, nothing to do here
/*                }
                else {
                    HTool.IsConnected = false;
                    serialPort1.Close();
                    txtConsole.AppendText("Silence answers our cries. Tool not found." + Environment.NewLine);
                }
                */
            }   //try
            catch (System.Exception ex) {
                txtConsole.AppendText(ex.Message + Environment.NewLine);
                serialPort1.Close();
            }
        }

        private void BtnStart_Click(object sender, EventArgs e) {
            if (BruteForcer.IsBusy) return;      // do not start second time
            if (Lock.State != LockState_t.EnteringServiceCode) return;
            if(!UInt32.TryParse(tbStartValue.Text, out uint dummy)) return;
            // Display start info
            txtConsole.AppendText("Password search started" + Environment.NewLine);
            BruteForcer.RunWorkerAsync();
        }
        private void BtnStop_Click(object sender, EventArgs e) {
            BruteForcer.CancelAsync();
            txtConsole.AppendText("Pass search eagerly cancelled." + Environment.NewLine);
        }

        private void BtnIterate_Click(object sender, EventArgs e) {
            if (timerIteration.Enabled) return;     // do not start second time
            if (Lock.State == LockState_t.Disconnected) {
                txtConsole.AppendText("Lock is not connected" + Environment.NewLine);
                return;
            }
            // Get current pair
            string S;
            if (rbtnP1.Checked) S = Lock.ServiceCode.Substring(0, 2);
            else if (rbtnP2.Checked) S = Lock.ServiceCode.Substring(2, 2);
            else S = Lock.ServiceCode.Substring(4, 2);
            int.TryParse(S.Substring(0, 1), out CurrentY);
            int.TryParse(S.Substring(1, 1), out CurrentX);
            // Start iteration
            txtConsole.AppendText("Iteration started" + Environment.NewLine);
            timerIteration.Enabled = true;
        }
        private void BtnStopIteration_Click(object sender, EventArgs e) {
            timerIteration.Enabled = false;
            progressBar1.Value = 0;
            txtConsole.AppendText("Iteration stopped." + Environment.NewLine);
        }
        private void BtnReset_Click(object sender, EventArgs e) {
            IterationCount = 0;
            for (int x = 0; x <= 9; x++) {
                for (int y = 0; y <= 9; y++) {
                    AccumTable[x, y] = 0;
                    dgvTable.Rows[y].Cells[x + 1].Style.BackColor = Color.White;
                }
            }
        }

        #endregion

        #region ============================== Serial RX handler =========================
        private void SerialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e) {
            // Read RX buffer
            int N = serialPort1.BytesToRead;
            if (N > 128) N = 128;
            char[] buf = new char[128];
            serialPort1.Read(buf, 0, N);
            serialPort1.DiscardInBuffer();  // clear what remained 
            string S = new string(buf);
            HTool.RXString += S.TrimEnd('\0');
        }
        #endregion

        void TryToConnectLock() {
            Lock.State = LockState_t.Disconnected;
            if (Lock.GetState()) {
                // Check if side is crashed
                if (Lock.SideIsCrashed()) {
                    txtConsole.AppendText("> ");
                    // Produce string of chaos
                    int N = RndValue.Next(54, 180);
                    for (int i = 0; i < N; i++) {
                        char c = Convert.ToChar(RndValue.Next(35, 126));
                        string S = "" + c;
                        txtConsole.AppendText(S);
                        if (!LockPing()) return;
                    }
                    txtConsole.AppendText(Environment.NewLine + "No reply" + Environment.NewLine);
                    return;
                } // if crashed
                else {
                    txtConsole.AppendText("Lock connected" + Environment.NewLine + "> Enter service code:" + Environment.NewLine);
                    Lock.State = LockState_t.EnteringServiceCode;
                }
            }
            else txtConsole.AppendText("No reply" + Environment.NewLine);
        }
        private bool LockPing() {
            if (Lock.Kick()) return true;
            else {
                txtConsole.AppendText("Disconnected" + Environment.NewLine);
                return false;
            }
        }

        #region ========================= Console =========================
        private void FillMenu() {
            LockMenu = new List<string> {
                "Open door",
                "Close door",
                "Display codes",
                "Change code A",
                "Change code B",
                "Change service code"
            };
        }
        private void PrintMenu() {
            txtConsole.AppendText("> Enter command:" + Environment.NewLine);
            foreach (string SItem in LockMenu) {
                txtConsole.AppendText("> * " + SItem + Environment.NewLine);
            }
        }

        private bool CheckEnteredCode(string AInCode, out string AOutCode) {
            AOutCode = "";
            if (!LockPing()) return false; // blink led
            // Check if none
            if ((AInCode.Length == 0) || AInCode.Equals("None", StringComparison.OrdinalIgnoreCase)) {
                txtConsole.AppendText("> Empty code entered (not recommended)." + Environment.NewLine);
                AOutCode = "None";
                return true;
            }
            else {  // Check if ok
                AOutCode = AInCode;
                if ((AInCode.Length <= 4) && UInt32.TryParse(AInCode, out uint dummy)) {
                    return true;
                }
                else {
                    txtConsole.AppendText("> Bad code." + Environment.NewLine + "> Enter new code:" + Environment.NewLine);
                    return false;
                }
            }
        } // CheckEnteredCode

        private void ParseCmd(string SCmd) {
            // Tool is just connected. Need to connect lock.
            if ((Lock.State == LockState_t.Disconnected) && SCmd.Equals("connect", StringComparison.OrdinalIgnoreCase)) {
                TryToConnectLock();
            } // if connect

            // Entering service code
            else if (Lock.State == LockState_t.EnteringServiceCode) {
                if (!LockPing()) return; // blink led
                if (SCmd.Equals(Lock.ServiceCode)) {    // Check if code equal
                    txtConsole.AppendText("> Access is allowed." + Environment.NewLine);
                    PrintMenu();
                    Lock.State = LockState_t.WaitingCommand;
                }
                else txtConsole.AppendText("> Incorrect code, access denied." + Environment.NewLine + "> Enter service code:" + Environment.NewLine);
            } // if code

            // Handling menu
            else if (Lock.State == LockState_t.WaitingCommand) {
                int indx = LockMenu.FindIndex(S => S.Equals(SCmd, StringComparison.OrdinalIgnoreCase)); // Get indx of entered command
                switch (indx) {
                    case 0: // Open door
                        if (!Lock.Open()) txtConsole.AppendText("Disconnected" + Environment.NewLine);
                        else {
                            txtConsole.AppendText("> Door opened." + Environment.NewLine);
                            PrintMenu();
                        }
                        break;
                    case 1: // Close door
                        if (!Lock.Close()) txtConsole.AppendText("Disconnected" + Environment.NewLine);
                        else {
                            txtConsole.AppendText("> Door closed." + Environment.NewLine);
                            PrintMenu();
                        }
                        break;
                    case 2: // Display code
                        if (!LockPing()) return; // blink led
                        txtConsole.AppendText("> Code A: " + Lock.CodeA + Environment.NewLine + "> Code B: " + Lock.CodeB + Environment.NewLine);
                        PrintMenu();
                        break;
                    case 3: // Change code A
                        if (!LockPing()) return; // blink led
                        txtConsole.AppendText("> Enter new code A:" + Environment.NewLine);
                        Lock.State = LockState_t.AskingNewCodeA;
                        break;
                    case 4: // Change code B
                        if (!LockPing()) return; // blink led
                        txtConsole.AppendText("> Enter new code B:" + Environment.NewLine);
                        Lock.State = LockState_t.AskingNewCodeB;
                        break;
                    case 5: // Change service code
                        if (!LockPing()) return; // blink led
                        txtConsole.AppendText("> Enter new service code:" + Environment.NewLine);
                        Lock.State = LockState_t.AskingNewServiceCode;
                        break;
                    default:
                        if (!LockPing()) return; // blink led
                        txtConsole.AppendText("> Command not found." + Environment.NewLine);
                        PrintMenu();
                        break;
                } // switch 
            } // if WaitingCommand
            
            // Entering new code 
            else if (Lock.State == LockState_t.AskingNewCodeA) {
                if (!CheckEnteredCode(SCmd, out SCmd)) return;  // Bad code entered
                else {                                          // SCmd contains correct code
                    if (Lock.ChangeCodeA(SCmd)) {
                        txtConsole.AppendText("> Code A changed." + Environment.NewLine);
                        PrintMenu();
                        Lock.State = LockState_t.WaitingCommand;
                    }
                    else txtConsole.AppendText("Disconnected" + Environment.NewLine);
                }
            } // AskingNewCodeA
            else if (Lock.State == LockState_t.AskingNewCodeB) {
                if (!CheckEnteredCode(SCmd, out SCmd)) return;  // Bad code entered
                else {                                          // SCmd contains correct code
                    if (Lock.ChangeCodeB(SCmd)) {
                        txtConsole.AppendText("> Code B changed." + Environment.NewLine);
                        PrintMenu();
                        Lock.State = LockState_t.WaitingCommand;
                    }
                    else txtConsole.AppendText("Disconnected" + Environment.NewLine);
                }
            } // AskingNewCodeB

            // Entering new service code 
            else if (Lock.State == LockState_t.AskingNewServiceCode) {
                if((SCmd.Length == 6) && UInt32.TryParse(SCmd, out uint dummy)) {    // Check if code ok
                    if(Lock.ChangeServiceCode(SCmd)) {
                        txtConsole.AppendText("> Service code changed." + Environment.NewLine);
                        PrintMenu();
                        Lock.State = LockState_t.WaitingCommand;
                    }
                    else txtConsole.AppendText("Disconnected" + Environment.NewLine);
                }
                else if(LockPing()) txtConsole.AppendText("> Bad code." + Environment.NewLine + "> Enter new service code:" + Environment.NewLine);
            } // AskingNewServiceCode
        } // ParseCmd 

        private void Console_KeyUp(object sender, KeyEventArgs e) {
            if (e.KeyValue == 13) {    // Enter was pressed
                // Get entered string
                string S = txtConsole.Lines[txtConsole.Lines.Count()-2];
                ParseCmd(S);                
            }
        }
        #endregion

        #region ============= Brute force ==============
        int MaxRang = 5;
        string ICode;
        
        string SumCodes(int[] ACodeArr, int[] AShiftArr) {
            string S = "";
            for (int i = 0; i < 6; i++) S += ((ACodeArr[i] + AShiftArr[i] + 100) % 10).ToString();
            return S;
        }

        int ShiftRang(int[] AShiftArr) {
            int IMax = 0;
            for (int i = 0; i < 6; i++)
                if (Math.Abs(AShiftArr[i]) > IMax)
                    IMax = Math.Abs(AShiftArr[i]);
            return IMax;
        }

        int GetNextShiftDigit(int AX, int ARang) {
            if (AX == ARang) return 0;
            else if (AX < 0) return -AX;
            else return -AX - 1;
        }

        int[] GetNextShiftSimple(int[] AShiftArr, int ARang) {
            int[] ResultArr = new int[6];
            bool Over = true;
            int x;
            for (int i = 5; i >= 0; i--) {
                if (Over) {
                    x = GetNextShiftDigit(AShiftArr[i], ARang);
                    Over = (x == 0);
                }
                else x = AShiftArr[i]; 
                ResultArr[i] = x;
            }
            return ResultArr;
        }

        int[] GetNextShift(int[] AShiftArr) {
            int[] NewShiftArr;
            int IRang = ShiftRang(AShiftArr);
            if (IRang == 0) IRang = 1;
            NewShiftArr = GetNextShiftSimple(AShiftArr, IRang);
            int ReturnedRang = ShiftRang(NewShiftArr);
            if (ReturnedRang < IRang) {
                if (ReturnedRang == 0) {
                    if (IRang == MaxRang) return null;
                    IRang++;
                }
                NewShiftArr[5] = -IRang;
            }
            if (IRang == 5) 
                for (int i=0; i<6; i++) 
                    if (NewShiftArr[i] == -5) NewShiftArr[i] = 5;
            return NewShiftArr;
        }

        // Main cycle
        private void BruteForcer_DoWork(object sender, DoWorkEventArgs e) {
            //Lock.ServiceCode = "123456";    // DEBUG
            string StartCodeStr = tbStartValue.Text;
            int[] StartCodeArr = (from x in StartCodeStr select int.Parse(x.ToString())).ToArray();
            int[] ShiftArr = new int[6];
        
            while (true) {
                // Get next code
                ICode = SumCodes(StartCodeArr, ShiftArr);

                // Check if code already equals
                if (ICode.Equals(Lock.ServiceCode)) return;
                if (BruteForcer.CancellationPending) {
                    e.Cancel = true;
                    return;
                }

                BruteForcer.ReportProgress(0);
                Thread.Sleep(50);

                ShiftArr = GetNextShift(ShiftArr);
                if (ShiftArr == null) break;
            } // while 1
        }

        private void BruteForcer_ProgressChanged(object sender, ProgressChangedEventArgs e) {
            if (BruteForcer.CancellationPending) return;
            if (LockPing()) {
                // Display code
                tbStartValue.Text = ICode;
                txtConsole.AppendText(ICode + Environment.NewLine);
            }
            else BruteForcer.CancelAsync();
        }

        private void BruteForcer_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e) {
            if (e.Error != null) txtConsole.AppendText("Error: " + e.Error.Message + Environment.NewLine);
            else if (!e.Cancelled) {
                // Check if password is correct
                if (ICode.Equals(Lock.ServiceCode)) {
                    txtConsole.AppendText("Password found: " + ICode + Environment.NewLine);
                    txtConsole.AppendText("> Access is allowed." + Environment.NewLine);
                    PrintMenu();
                    Lock.State = LockState_t.WaitingCommand;
                }
                else txtConsole.AppendText("> Incorrect code, access denied." + Environment.NewLine + "> Enter service code:" + Environment.NewLine);
            }
        }

        #endregion

        #region ========================== Iteration ===========================
        double Dif(int a1, int a2) {
            int NoiseMagn = Lock.Complexity+1;
            if (NoiseMagn > 0) {
                a1 += RndValue.Next(-NoiseMagn, NoiseMagn);
                if (a1 > 9) a1 -= 10;
                if (a1 < 0) a1 += 10;
            }
            return Math.Min((a1 - a2 + 10) % 10, (a2 - a1 + 10) % 10);
        }

        double P(double dist) {
            int NoiseMagn = 4 - HTool.U; //trackBar1.Value;
            //            return (1-dist)*.1+NoiseMagn*.01;
            //return (1 - dist) / (1 + NoiseMagn);
            return (1 - dist) * (4 - NoiseMagn) / (double)Lock.Complexity/4.0 + NoiseMagn * .1 + .1;
        }        // Distance function: returns 0...255, 255 means equality
       
        int Distance(int x1, int x2, int y1, int y2) {
            double difx = Dif(x1, x2);
            double dify = Dif(y1, y2);
            double Distn = Math.Sqrt(difx * difx + dify * dify);
            int c2 = RndValue.NextDouble() < P(Distn / (5 * Math.Sqrt(2))) ? 1 : 0;    // Norm to 1
            return c2;
        }

        // Fill instant table
        void FillInstantTable() {
            for (int y = 0; y <= 9; y++) {
                for (int x = 0; x <= 9; x++) {
                    InstantTable[x, y] = Distance(x, CurrentX, y, CurrentY);
                }
            }
        }

        private void TimerIteration_Tick(object sender, EventArgs e) {
            // Get U
            if (!HTool.GetU()) {
                txtConsole.AppendText("Measurement failure" + Environment.NewLine);
                timerIteration.Enabled = false;
                return;
            }
            progressBar1.Value = HTool.U;
            //textBox3.Text = HTool.U.ToString();

            IterationCount++;
            // Autoreset if needed
            if (cbAutoreset.Checked && (IterationCount >= numericUpDown2.Value)) {
                BtnReset_Click(sender, null);
                return;
            }
            lblIterationCounter.Text = "Iteration: " + IterationCount.ToString();
            FillInstantTable();
            // Accumulate values
            double MaxValue = 0;
            for (int y = 0; y <= 9; y++) {
                for (int x = 0; x <= 9; x++) {
                    AccumTable[x, y] += InstantTable[x, y];
                    if (AccumTable[x, y] > MaxValue) MaxValue = AccumTable[x, y];
                }
            }
            // Fill grid with normalized colors
            double NormValue;
            for (int y = 0; y <= 9; y++) {
                for (int x = 0; x <= 9; x++) {
                    NormValue = Math.Round(255 * (AccumTable[x, y] / MaxValue));
                    dgvTable.Rows[y].Cells[x + 1].Style.BackColor = Color.FromArgb(0, (int)NormValue, (int)NormValue);
                }
            }
        }

        #endregion
    }


    #region ========================= Classes =========================
    public class Lock_t {
        public string ServiceCode = "Empty", CodeA = "Empty", CodeB = "Empty";  // will never equal
        public int Complexity;
        public LockState_t State;
        public HTool_t Tool;
        public LockSide_t Side;

        public bool SideIsCrashed() {
            return (((Side == LockSide_t.SideA) && CodeA.StartsWith("-")) || ((Side == LockSide_t.SideB) && CodeB.StartsWith("-")));
        }

        #region ==== Lock commands ====
        public bool GetState() {
            LockState_t IState = State;     // Save current state
            State = LockState_t.Disconnected;
            if (!Tool.SendCmd("S")) return false;
            if (!Tool.WaitAnswer()) return false;
            // Check what received
            if (!Tool.RXString.StartsWith("S:")) return false;
            // Get service code, two lock codes and complexity from state reply
            string[] sl = Tool.RXString.Substring(2).Split(',');    // Split reply to chunks
            if (sl.Count() != 4) return false;                      // Must be 4 chunks
            ServiceCode = sl[0];
            CodeA = sl[1];
            CodeB = sl[2];
            if (!Int32.TryParse(sl[3], out Complexity)) return false;
            // === Now get side which tool is connected to ===
            //if (!Tool.SendCmd("G")) return false;
            //if (!Tool.WaitAnswer()) return false;
            // Check what received
            //if (Tool.RXString.StartsWith("G:A"))
                Side = LockSide_t.SideA;
            //else if (Tool.RXString.StartsWith("G:B")) Side = LockSide_t.SideB;
            //else return false;
            State = IState;     // Restore state
            return true;
        }

        public bool Kick() {
            LockState_t IState = State;     // Save current state
            State = LockState_t.Disconnected;
            for(int i = 0; i < 7; i++) {
                Thread.Sleep(10);
                if(!Tool.SendCmd("K")) continue;
                if(!Tool.WaitAnswer()) continue;
                // Check what received
                if(Tool.RXString.StartsWith("K")) {
                    State = IState;     // Restore state
                    return true;
                }
            }
            return false;
        }

        public bool Open() {
            LockState_t IState = State;     // Save current state
            State = LockState_t.Disconnected;
            if (!Tool.SendCmd("O")) return false;
            if (!Tool.WaitAnswer()) return false;
            // Check what received
            if (!Tool.RXString.StartsWith("O")) return false;
            State = IState;     // Restore state
            return true;
        }
        public bool Close() {
            LockState_t IState = State;     // Save current state
            State = LockState_t.Disconnected;
            if (!Tool.SendCmd("C")) return false;
            if (!Tool.WaitAnswer()) return false;
            // Check what received
            if (!Tool.RXString.StartsWith("C")) return false;
            State = IState;     // Restore state
            return true;
        }
        
        public bool ChangeCodeA(string ANewCode) {
            LockState_t IState = State;     // Save current state
            State = LockState_t.Disconnected;
            if (!Tool.SendCmd("A:" + ANewCode)) return false;
            if (!Tool.WaitAnswer()) return false;
            // Check what received
            if (Tool.RXString.StartsWith("A")) {
                CodeA = ANewCode;
                State = IState;     // Restore state
                return true;
            }
            else return false;
        }
        public bool ChangeCodeB(string ANewCode) {
            LockState_t IState = State;     // Save current state
            State = LockState_t.Disconnected;
            if (!Tool.SendCmd("B:" + ANewCode)) return false;
            if (!Tool.WaitAnswer()) return false;
            // Check what received
            if (Tool.RXString.StartsWith("B")) {
                CodeB = ANewCode;
                State = IState;     // Restore state
                return true;
            }
            else return false;
        }
        public bool ChangeServiceCode(string ANewCode) {
            LockState_t IState = State;     // Save current state
            State = LockState_t.Disconnected;
            if (!Tool.SendCmd("V:" + ANewCode)) return false;
            if (!Tool.WaitAnswer()) return false;
            // Check what received
            if (Tool.RXString.StartsWith("V")) {
                ServiceCode = ANewCode;
                State = IState;     // Restore state
                return true;
            }
            else return false;
        }

        #endregion
    }

    public class HTool_t {
        public string RXString;
        public bool IsConnected;
        public int U;
        public System.IO.Ports.SerialPort ComPort;
        public TextBox IConsole;
        public string LastError;

        // Constructor
        public HTool_t() {
            IsConnected = false;
        }

        #region ======== Tool commands ========
        public bool SendCmd(string ACmd) {
            RXString = "";
            ACmd += '\r';
            byte[] buf = Encoding.ASCII.GetBytes(ACmd);
            try {
                ComPort.Write(buf, 0, ACmd.Length);
                return true;
            }
            catch (Exception ex) {
                if (ComPort != null) ComPort.Close();
                IsConnected = false;
                LastError = ex.Message;
                return false;
            }
        }
        public bool WaitAnswer() {
            for (int i = 0; i < 10; i++) {
                Thread.Sleep(20);
                if (RXString.EndsWith("\r")) {
                    RXString = RXString.TrimEnd('\r');
                    return true;
                }
            }
            IsConnected = false;
            return false;
        }

        public bool Ping() {
            if (!SendCmd("P")) return false;
            if (!WaitAnswer()) return false;
            // Check what received
            IConsole.AppendText(RXString);
            IConsole.AppendText(Environment.NewLine);
            return RXString.StartsWith("HackerTool");
        }

        public bool GetU() {
            /*if (!SendCmd("U")) return false;
            if (!WaitAnswer()) return false;
            // Check what received
            if (!RXString.StartsWith("U:")) return false;   // something wrong came
            string SU = RXString.Substring(2);
            if (!Int32.TryParse(SU, out U)) return false;   // failed to convert string to int
            // Normalize U
            U = (U / 100);
            U = (U < 16) ? 0 : (U - 15);
            if (U > 18) */
                U = 18;
            U = (int)Math.Round(U / 4.5);
            // Kick lock to signal we are here
            for(int i = 0; i < 7; i++) {
                if(!SendCmd("K")) continue;
                if(WaitAnswer()) return true;
            }
            return false;
        }
    
        #endregion
        
    } // HTool_t
    #endregion
}
