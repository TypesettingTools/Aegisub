/*
SSATool - A collection of utilities for Advanced Substation Alpha
Copyright (C) 2007 Dan Donovan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; ONLY under version 2

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/



using System;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.Drawing;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml;
using System.Collections.Generic;

namespace SSATool {
    /// <summary>
    /// Summary description for Form1.
    /// </summary>
    public class Form1 : System.Windows.Forms.Form {
        public static Form1 FormMain;


        #region Form Info
        public System.Windows.Forms.MainMenu mainMenu1;
        public System.Windows.Forms.MenuItem menuItem1;
        public System.Windows.Forms.MenuItem menuItem5;
        public System.Windows.Forms.MenuItem menuItem8;
        public System.Windows.Forms.MenuItem menuItem10;
        public System.Windows.Forms.MenuItem menuItem13;
        public System.Windows.Forms.MenuItem menuItem15;
        public System.Windows.Forms.MenuItem menuItem16;
        public System.Windows.Forms.OpenFileDialog openFileDialog1;
        public System.Windows.Forms.CheckedListBox lstEffectOpts;
        public System.Windows.Forms.MenuItem menuOpen;
        public System.Windows.Forms.MenuItem menuOpenSJIS;
        public System.Windows.Forms.MenuItem menuSave;
        public System.Windows.Forms.MenuItem menuSaveAs;
        public System.Windows.Forms.MenuItem menuLoadProj;
        public System.Windows.Forms.MenuItem menuSaveProj;
        public System.Windows.Forms.MenuItem menuExit;
        public System.Windows.Forms.MenuItem menuLoadEffects;
        public System.Windows.Forms.MenuItem menuItem3;
        public System.Windows.Forms.MenuItem menuShowEffectEditor;
        public System.Windows.Forms.FontDialog fontDialog1;
        public System.Windows.Forms.MenuItem menuFont;
        public System.Windows.Forms.MenuItem menuItem2;
        public System.Windows.Forms.MenuItem menuGoTo;
        public System.Windows.Forms.MenuItem menuNewLineBeginning;
        public System.Windows.Forms.MenuItem menuNewLineEnd;
        public System.Windows.Forms.MenuItem menuNewLineAbove;
        public System.Windows.Forms.MenuItem menuNewLineBelow;
        public System.Windows.Forms.MenuItem menuMoveLineUp;
        public System.Windows.Forms.MenuItem menuMoveLineDown;
        public System.Windows.Forms.MenuItem menuChangeLine;
        public System.Windows.Forms.MenuItem menuRemoveLine;
        public System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
        public System.Windows.Forms.ColumnHeader columnHeader11;
        private IContainer components;
        public MenuItem menuItem4;
        public MenuItem menuItem6;
        public MenuItem menuPrecisioncs;
        public MenuItem menuPrecisionms;
        public MenuItem menuItem7;
        public MenuItem menuItem9;
        public TabPage tabPage6;
        public Button button2;
        public Button button1;
        public TextBox textBox2;
        public Button button4;
        public Button button3;
        public CheckBox checkBox1;
        public ComboBox comboBox1;
        public Label label55;
        public Button button6;
        public MaskedTextBox maskedTextBox1;
        public CheckBox checkBox2;
        public TreeView treeView1;
        public MenuItem menuItem11;
        public MenuItem menuUndo;
        public MenuItem menuRedo;
        public MenuItem menuOpenEUCJP;
        public SplitContainer splitContainer1;
        public ListViewES listSSA;
        public ColumnHeader columnHeader14;
        public TabControl tabMain;
        public TabPage tabPage1;
        public GroupBox groupBox12;
        public ComboBox cmbFR;
        public Label label54;
        public GroupBox groupBox4;
        public Button cmdSearchReplace;
        public TextBox txtSearchReplaceWith;
        public TextBox txtSearchFor;
        public Label label12;
        public Label label11;
        public GroupBox groupBox3;
        public Button cmdChangeLastKLen;
        public Button cmdRemoveDups;
        public Button cmdStrip;
        public GroupBox groupBox2;
        public Button cmdShift;
        public CheckBox chkShiftNoNeg;
        public CheckBox chkShiftEnd;
        public CheckBox chkShiftStart;
        public TabControl tabShiftType;
        public TabPage tabPage9;
        public MaskedTextBox maskedTextShiftTime;
        public RadioButton radTShiftBack;
        public RadioButton radTShiftForward;
        public Label label3;
        public TabPage tabPage10;
        public RadioButton radFShiftBackward;
        public RadioButton radFShiftForward;
        public TextBox txtShiftFrames;
        public Label label4;
        public TabPage tabPage11;
        public MaskedTextBox maskedTextTDShiftNew;
        public MaskedTextBox maskedTextTDShiftOld;
        public Label label7;
        public Label label6;
        public TabPage tabPage12;
        public Label label9;
        public Label label8;
        public TextBox txtFDShiftNew;
        public TextBox txtFDShiftOld;
        public TabPage tabPage2;
        public GroupBox groupBox11;
        public Button cmdTimecodesDeapply;
        public Button cmdTimecodesApply;
        public Button cmdTimecodesBrowse;
        public TextBox textTimecodeFile;
        public GroupBox groupBox10;
        public MaskedTextBox txtLeadOut;
        public MaskedTextBox txtLeadIn;
        public CheckBox chkLeadOut;
        public CheckBox chkLeadIn;
        public Button cmdLead;
        public CheckBox chkLeadKaraEmptyK;
        public CheckBox chkLeadKaraOnly;
        public GroupBox groupBox6;
        public CheckBox chkScalePlayRes;
        public TextBox txtScaleYDenominator;
        public Label label50;
        public TextBox txtScaleYNumerator;
        public Label label51;
        public Button cmdScale;
        public TextBox txtScaleXDenominator;
        public Label label26;
        public TextBox txtScaleXNumerator;
        public Label label25;
        public GroupBox groupBox5;
        public Button cmdLenBasedK;
        public TextBox txtLenBasedKThreshold;
        public Label label13;
        public TabPage tabPage3;
        public Button cmdAddEffect;
        public TreeView treeKaraoke;
        public Button cmdNewLayer;
        public Button cmdKaraDel;
        public Button cmdDoEffects;
        public Button cmdKaraDup;
        public Panel panelLayer;
        public CheckBox chkSyllablePerLine;
        public GroupBox groupBox16;
        public CheckBox chkKaraAddText;
        public CheckBox chkKaraAddClosingBracket;
        public CheckBox chkKaraAddASSA;
        public CheckBox chkKaraAddK;
        public TextBox textLayerRepetitions;
        public Label label49;
        public CheckBox chkAddOnce;
        public CheckBox chkAddAnyway;
        public CheckBox chkLayerPerSyllable;
        public Button cmdDelLayerCondition;
        public Button cmdNewLayerCondition;
        public ListViewES lstLayerConditions;
        public ColumnHeader columnHeader4;
        public ColumnHeader columnHeader5;
        public ColumnHeader columnHeader6;
        public Panel panelEffect;
        public TabControl tabControl1;
        public TabPage tabPage7;
        public ListView lstEffectOptions;
        public ColumnHeader lstoOptionName;
        public ColumnHeader lstoOptionValue;
        public Label label2;
        public TabPage tabPage8;
        public ListViewES lstEffectConditions;
        public ColumnHeader columnHeader1;
        public ColumnHeader columnHeader2;
        public ColumnHeader columnHeader3;
        public Button cmdDelEffectCondition;
        public Button cmdAddEffectCondition;
        public TabPage tabPage4;
        public Label label16;
        public Label label18;
        public Label label15;
        public Label label14;
        public Button cmdGradMoveBefore;
        public TextBox txtGradOut;
        public TextBox txtGradIn;
        public TextBox txtGradBefore;
        public TabPage tabPage14;
        public Label label42;
        public TextBox txtBlurRadV;
        public GroupBox groupBlurAffect;
        public CheckBox chkBlur4a;
        public CheckBox chkBlur3a;
        public CheckBox chkBlur2a;
        public CheckBox chkBlur1a;
        public GroupBox groupBox9;
        public RadioButton radBlurExp;
        public RadioButton radBlurLog;
        public RadioButton radBlurPoly;
        public GroupBox groupBlurSub;
        public CheckBox chkBlur2aSub;
        public CheckBox chkBlur4aSub;
        public CheckBox chkBlur3aSub;
        public CheckBox chkBlur1aSub;
        public Label label38;
        public TextBox txtBlurBase;
        public TextBox txtBlurPos;
        public Label label36;
        public Button cmdBlurListToInput;
        public Button cmdBlurInsertBefore;
        public Button cmdBlurInsertAfter;
        public Button cmdBlurReplaceLine;
        public Button cmdBlurMoveBefore;
        public Label label35;
        public TextBox txtBlurBefore;
        public TextBox txtBlurRadH;
        public Label label34;
        public Label label32;
        public Label label31;
        public TextBox txtBlurOut;
        public TextBox txtBlurIn;
        public GroupBox groupBox8;
        public RadioButton radBlurGlow;
        public RadioButton radBlurHV;
        public Button cmdDoBlur;
        public TextBox txtBlurEndAlpha;
        public TextBox txtBlurStartAlpha;
        public Label label30;
        public Label label29;
        public TextBox txtBlurLines;
        public Label label28;
        public TabPage tabPage16;
        public MaskedTextBox maskedTextTransformTime;
        public Button cmdTransformListToInput;
        public Button cmdTransformInsertBefore;
        public Button cmdTransformInsertAfter;
        public Button cmdTransformReplaceLine;
        public Label label45;
        public TextBox txtTransformPrecision;
        public Label label1;
        public ListViewES listTransformVars;
        public ColumnHeader columnHeader13;
        public TextBox txtTransformCode;
        public Label label47;
        public Label label46;
        public Button cmdTransformDelVar;
        public Button cmdTransformNewVar;
        public Button cmdDoTransform;
        public Label label44;
        public Button cmdTransformDelTime;
        public Button cmdTransformAddTime;
        public ListBox listTransformTimes;
        public Label label43;
        public TextBox txtTransformOut;
        public TabPage tabPage13;
        public ListBox lstErrors;
        public Label label27;
        public Button cmdCheckErrors;
        public TabPage tabPage15;
        public Label labelFontNum;
        public Button cmdFontSearchFolder;
        public Label label40;
        public Button cmdFindFonts;
        public Label label39;
        public ListView lstFonts;
        public ColumnHeader columnHeader7;
        public ColumnHeader columnHeader8;
        public ColumnHeader columnHeader9;
        public TabPage tabPage5;
        public GroupBox groupBox19;
        public TextBox textBox1;
        public GroupBox groupBox18;
        public Label lblRegroupDestIndex;
        public Label label10;
        public Label lblRegroupSourceIndex;
        public Label label5;
        public GroupBox groupBox17;
        public ListView listRegroupPairs;
        public ColumnHeader columnHeader10;
        public ColumnHeader columnHeader12;
        public GroupBox groupBox15;
        public Button cmdRegroupStart;
        public Button cmdRegroupAcceptLine;
        public Button cmdRegroupUnlinkLast;
        public Button cmdRegroupGoBack;
        public Button cmdRegroupSkipDestLine;
        public Button cmdRegroupSkipSourceLine;
        public Button cmdRegroupLink;
        public GroupBox groupBox14;
        public TextBox txtRegroupSource;
        public TextBox txtRegroupDest;
        public GroupBox groupBox13;
        public ComboBox cmbRegroupDest;
        public ComboBox cmbRegroupSource;
        public Label label53;
        public Label label52;
        public TabPage tabPage17;
        public GroupBox groupBox22;
        public ComboBox cmbNBStyle;
        public Label label59;
        public Button cmdNBCopyStyles;
        public Button cmdNBInsertBefore;
        public Button cmdNBInsertAfter;
        public Button cmdNBReplaceLine;
        public Button cmdNoteBox;
        public GroupBox groupBox21;
        public TextBox textNBStyles;
        public TextBox textNBOut;
        public GroupBox groupBox20;
        public Button cmdNBGetTimes;
        public MaskedTextBox maskedTextNBEnd;
        public Label label58;
        public MaskedTextBox maskedTextNBStart;
        public Label label57;
        public TextBox textNotebox2;
        public TextBox textNotebox1;
        public Label label56;
        public Label label48;
        public GroupBox groupBox1;
        public Label labelSelLine;
        public Label label41;
        public Button cmdReloadFile;
        public CheckedListBox lstStyles;
        public Button cmdSSAInvertSel;
        public Button cmdSSADeselall;
        public Button cmdSSASelall;
        public Button cmdDeselAll;
        public Button cmdSelAll;
        public Button cmdNBReparse;
        private CheckBox chkSRCaseSensitive;
        private TextBox textNBDesc;
        public Label label17;
        private Panel panelGradient;
        public GroupBox groupBox7;
        public RadioButton radGradExp;
        public RadioButton radGradLog;
        public RadioButton radGradPoly;
        public TextBox txtGradMirror;
        public Label label33;
        public Label label37;
        public TextBox txtGradBase;
        public CheckBox chkGrad4c;
        public CheckBox chkGrad3c;
        public CheckBox chkGrad2c;
        public CheckBox chkGrad1c;
        public Button cmdGradListToInput;
        public TextBox txtGradEndBGR;
        public TextBox txtGradStartBGR;
        public TextBox txtGradOffset;
        public TextBox txtGradEndPos;
        public TextBox txtGradStartPos;
        public TextBox txtGradCLines;
        public Label label24;
        public Label label23;
        public Label label22;
        public Label label21;
        public Label label20;
        public Label label19;
        public Button cmdGradInsertBefore;
        public Button cmdGradInsertAfter;
        public Button cmdGradReplaceLine;
        public Button cmdGradDoGrad;
        private Panel panelBlur;
        public MenuItem menuClearUndoRedo;

        #endregion
        protected override void Dispose(bool disposing) {
            base.Dispose(disposing);
        }
        #region Windows Form Designer generated code
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.mainMenu1 = new System.Windows.Forms.MainMenu(this.components);
            this.menuItem1 = new System.Windows.Forms.MenuItem();
            this.menuOpen = new System.Windows.Forms.MenuItem();
            this.menuOpenSJIS = new System.Windows.Forms.MenuItem();
            this.menuOpenEUCJP = new System.Windows.Forms.MenuItem();
            this.menuSave = new System.Windows.Forms.MenuItem();
            this.menuSaveAs = new System.Windows.Forms.MenuItem();
            this.menuItem5 = new System.Windows.Forms.MenuItem();
            this.menuLoadProj = new System.Windows.Forms.MenuItem();
            this.menuSaveProj = new System.Windows.Forms.MenuItem();
            this.menuItem8 = new System.Windows.Forms.MenuItem();
            this.menuExit = new System.Windows.Forms.MenuItem();
            this.menuItem11 = new System.Windows.Forms.MenuItem();
            this.menuUndo = new System.Windows.Forms.MenuItem();
            this.menuRedo = new System.Windows.Forms.MenuItem();
            this.menuClearUndoRedo = new System.Windows.Forms.MenuItem();
            this.menuItem2 = new System.Windows.Forms.MenuItem();
            this.menuItem7 = new System.Windows.Forms.MenuItem();
            this.menuNewLineBeginning = new System.Windows.Forms.MenuItem();
            this.menuNewLineEnd = new System.Windows.Forms.MenuItem();
            this.menuNewLineAbove = new System.Windows.Forms.MenuItem();
            this.menuNewLineBelow = new System.Windows.Forms.MenuItem();
            this.menuItem9 = new System.Windows.Forms.MenuItem();
            this.menuMoveLineUp = new System.Windows.Forms.MenuItem();
            this.menuMoveLineDown = new System.Windows.Forms.MenuItem();
            this.menuGoTo = new System.Windows.Forms.MenuItem();
            this.menuChangeLine = new System.Windows.Forms.MenuItem();
            this.menuRemoveLine = new System.Windows.Forms.MenuItem();
            this.menuItem10 = new System.Windows.Forms.MenuItem();
            this.menuFont = new System.Windows.Forms.MenuItem();
            this.menuItem4 = new System.Windows.Forms.MenuItem();
            this.menuItem6 = new System.Windows.Forms.MenuItem();
            this.menuPrecisioncs = new System.Windows.Forms.MenuItem();
            this.menuPrecisionms = new System.Windows.Forms.MenuItem();
            this.menuItem13 = new System.Windows.Forms.MenuItem();
            this.menuShowEffectEditor = new System.Windows.Forms.MenuItem();
            this.menuItem15 = new System.Windows.Forms.MenuItem();
            this.menuItem16 = new System.Windows.Forms.MenuItem();
            this.menuItem3 = new System.Windows.Forms.MenuItem();
            this.menuLoadEffects = new System.Windows.Forms.MenuItem();
            this.tabPage6 = new System.Windows.Forms.TabPage();
            this.treeView1 = new System.Windows.Forms.TreeView();
            this.maskedTextBox1 = new System.Windows.Forms.MaskedTextBox();
            this.checkBox2 = new System.Windows.Forms.CheckBox();
            this.button6 = new System.Windows.Forms.Button();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.label55 = new System.Windows.Forms.Label();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.button4 = new System.Windows.Forms.Button();
            this.button3 = new System.Windows.Forms.Button();
            this.button2 = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.columnHeader11 = new System.Windows.Forms.ColumnHeader();
            this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
            this.lstEffectOpts = new System.Windows.Forms.CheckedListBox();
            this.fontDialog1 = new System.Windows.Forms.FontDialog();
            this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.labelSelLine = new System.Windows.Forms.Label();
            this.label41 = new System.Windows.Forms.Label();
            this.cmdReloadFile = new System.Windows.Forms.Button();
            this.lstStyles = new System.Windows.Forms.CheckedListBox();
            this.cmdSSAInvertSel = new System.Windows.Forms.Button();
            this.cmdSSADeselall = new System.Windows.Forms.Button();
            this.cmdSSASelall = new System.Windows.Forms.Button();
            this.cmdDeselAll = new System.Windows.Forms.Button();
            this.cmdSelAll = new System.Windows.Forms.Button();
            this.listSSA = new SSATool.ListViewES();
            this.columnHeader14 = new System.Windows.Forms.ColumnHeader();
            this.tabMain = new System.Windows.Forms.TabControl();
            this.tabPage1 = new System.Windows.Forms.TabPage();
            this.groupBox12 = new System.Windows.Forms.GroupBox();
            this.cmbFR = new System.Windows.Forms.ComboBox();
            this.label54 = new System.Windows.Forms.Label();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.chkSRCaseSensitive = new System.Windows.Forms.CheckBox();
            this.cmdSearchReplace = new System.Windows.Forms.Button();
            this.txtSearchReplaceWith = new System.Windows.Forms.TextBox();
            this.txtSearchFor = new System.Windows.Forms.TextBox();
            this.label12 = new System.Windows.Forms.Label();
            this.label11 = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.cmdChangeLastKLen = new System.Windows.Forms.Button();
            this.cmdRemoveDups = new System.Windows.Forms.Button();
            this.cmdStrip = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.cmdShift = new System.Windows.Forms.Button();
            this.chkShiftNoNeg = new System.Windows.Forms.CheckBox();
            this.chkShiftEnd = new System.Windows.Forms.CheckBox();
            this.chkShiftStart = new System.Windows.Forms.CheckBox();
            this.tabShiftType = new System.Windows.Forms.TabControl();
            this.tabPage9 = new System.Windows.Forms.TabPage();
            this.maskedTextShiftTime = new System.Windows.Forms.MaskedTextBox();
            this.radTShiftBack = new System.Windows.Forms.RadioButton();
            this.radTShiftForward = new System.Windows.Forms.RadioButton();
            this.label3 = new System.Windows.Forms.Label();
            this.tabPage10 = new System.Windows.Forms.TabPage();
            this.radFShiftBackward = new System.Windows.Forms.RadioButton();
            this.radFShiftForward = new System.Windows.Forms.RadioButton();
            this.txtShiftFrames = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.tabPage11 = new System.Windows.Forms.TabPage();
            this.maskedTextTDShiftNew = new System.Windows.Forms.MaskedTextBox();
            this.maskedTextTDShiftOld = new System.Windows.Forms.MaskedTextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.tabPage12 = new System.Windows.Forms.TabPage();
            this.label9 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.txtFDShiftNew = new System.Windows.Forms.TextBox();
            this.txtFDShiftOld = new System.Windows.Forms.TextBox();
            this.tabPage2 = new System.Windows.Forms.TabPage();
            this.groupBox11 = new System.Windows.Forms.GroupBox();
            this.cmdTimecodesDeapply = new System.Windows.Forms.Button();
            this.cmdTimecodesApply = new System.Windows.Forms.Button();
            this.cmdTimecodesBrowse = new System.Windows.Forms.Button();
            this.textTimecodeFile = new System.Windows.Forms.TextBox();
            this.groupBox10 = new System.Windows.Forms.GroupBox();
            this.txtLeadOut = new System.Windows.Forms.MaskedTextBox();
            this.txtLeadIn = new System.Windows.Forms.MaskedTextBox();
            this.chkLeadOut = new System.Windows.Forms.CheckBox();
            this.chkLeadIn = new System.Windows.Forms.CheckBox();
            this.cmdLead = new System.Windows.Forms.Button();
            this.chkLeadKaraEmptyK = new System.Windows.Forms.CheckBox();
            this.chkLeadKaraOnly = new System.Windows.Forms.CheckBox();
            this.groupBox6 = new System.Windows.Forms.GroupBox();
            this.chkScalePlayRes = new System.Windows.Forms.CheckBox();
            this.txtScaleYDenominator = new System.Windows.Forms.TextBox();
            this.label50 = new System.Windows.Forms.Label();
            this.txtScaleYNumerator = new System.Windows.Forms.TextBox();
            this.label51 = new System.Windows.Forms.Label();
            this.cmdScale = new System.Windows.Forms.Button();
            this.txtScaleXDenominator = new System.Windows.Forms.TextBox();
            this.label26 = new System.Windows.Forms.Label();
            this.txtScaleXNumerator = new System.Windows.Forms.TextBox();
            this.label25 = new System.Windows.Forms.Label();
            this.groupBox5 = new System.Windows.Forms.GroupBox();
            this.cmdLenBasedK = new System.Windows.Forms.Button();
            this.txtLenBasedKThreshold = new System.Windows.Forms.TextBox();
            this.label13 = new System.Windows.Forms.Label();
            this.tabPage3 = new System.Windows.Forms.TabPage();
            this.cmdAddEffect = new System.Windows.Forms.Button();
            this.treeKaraoke = new System.Windows.Forms.TreeView();
            this.cmdNewLayer = new System.Windows.Forms.Button();
            this.cmdKaraDel = new System.Windows.Forms.Button();
            this.cmdDoEffects = new System.Windows.Forms.Button();
            this.cmdKaraDup = new System.Windows.Forms.Button();
            this.panelEffect = new System.Windows.Forms.Panel();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabPage7 = new System.Windows.Forms.TabPage();
            this.lstEffectOptions = new System.Windows.Forms.ListView();
            this.lstoOptionName = new System.Windows.Forms.ColumnHeader();
            this.lstoOptionValue = new System.Windows.Forms.ColumnHeader();
            this.label2 = new System.Windows.Forms.Label();
            this.tabPage8 = new System.Windows.Forms.TabPage();
            this.lstEffectConditions = new SSATool.ListViewES();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.cmdDelEffectCondition = new System.Windows.Forms.Button();
            this.cmdAddEffectCondition = new System.Windows.Forms.Button();
            this.panelLayer = new System.Windows.Forms.Panel();
            this.chkSyllablePerLine = new System.Windows.Forms.CheckBox();
            this.groupBox16 = new System.Windows.Forms.GroupBox();
            this.chkKaraAddText = new System.Windows.Forms.CheckBox();
            this.chkKaraAddClosingBracket = new System.Windows.Forms.CheckBox();
            this.chkKaraAddASSA = new System.Windows.Forms.CheckBox();
            this.chkKaraAddK = new System.Windows.Forms.CheckBox();
            this.textLayerRepetitions = new System.Windows.Forms.TextBox();
            this.label49 = new System.Windows.Forms.Label();
            this.chkAddOnce = new System.Windows.Forms.CheckBox();
            this.chkAddAnyway = new System.Windows.Forms.CheckBox();
            this.chkLayerPerSyllable = new System.Windows.Forms.CheckBox();
            this.cmdDelLayerCondition = new System.Windows.Forms.Button();
            this.cmdNewLayerCondition = new System.Windows.Forms.Button();
            this.lstLayerConditions = new SSATool.ListViewES();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
            this.tabPage4 = new System.Windows.Forms.TabPage();
            this.label17 = new System.Windows.Forms.Label();
            this.panelGradient = new System.Windows.Forms.Panel();
            this.txtGradCLines = new System.Windows.Forms.TextBox();
            this.cmdGradDoGrad = new System.Windows.Forms.Button();
            this.groupBox7 = new System.Windows.Forms.GroupBox();
            this.radGradExp = new System.Windows.Forms.RadioButton();
            this.radGradLog = new System.Windows.Forms.RadioButton();
            this.radGradPoly = new System.Windows.Forms.RadioButton();
            this.cmdGradReplaceLine = new System.Windows.Forms.Button();
            this.txtGradMirror = new System.Windows.Forms.TextBox();
            this.cmdGradInsertAfter = new System.Windows.Forms.Button();
            this.label33 = new System.Windows.Forms.Label();
            this.cmdGradInsertBefore = new System.Windows.Forms.Button();
            this.label37 = new System.Windows.Forms.Label();
            this.label19 = new System.Windows.Forms.Label();
            this.txtGradBase = new System.Windows.Forms.TextBox();
            this.label20 = new System.Windows.Forms.Label();
            this.chkGrad4c = new System.Windows.Forms.CheckBox();
            this.label21 = new System.Windows.Forms.Label();
            this.chkGrad3c = new System.Windows.Forms.CheckBox();
            this.label22 = new System.Windows.Forms.Label();
            this.chkGrad2c = new System.Windows.Forms.CheckBox();
            this.label23 = new System.Windows.Forms.Label();
            this.chkGrad1c = new System.Windows.Forms.CheckBox();
            this.label24 = new System.Windows.Forms.Label();
            this.txtGradStartPos = new System.Windows.Forms.TextBox();
            this.cmdGradListToInput = new System.Windows.Forms.Button();
            this.txtGradEndPos = new System.Windows.Forms.TextBox();
            this.txtGradEndBGR = new System.Windows.Forms.TextBox();
            this.txtGradOffset = new System.Windows.Forms.TextBox();
            this.txtGradStartBGR = new System.Windows.Forms.TextBox();
            this.label16 = new System.Windows.Forms.Label();
            this.label18 = new System.Windows.Forms.Label();
            this.label15 = new System.Windows.Forms.Label();
            this.label14 = new System.Windows.Forms.Label();
            this.cmdGradMoveBefore = new System.Windows.Forms.Button();
            this.txtGradOut = new System.Windows.Forms.TextBox();
            this.txtGradIn = new System.Windows.Forms.TextBox();
            this.txtGradBefore = new System.Windows.Forms.TextBox();
            this.tabPage14 = new System.Windows.Forms.TabPage();
            this.panelBlur = new System.Windows.Forms.Panel();
            this.groupBlurAffect = new System.Windows.Forms.GroupBox();
            this.chkBlur4a = new System.Windows.Forms.CheckBox();
            this.chkBlur3a = new System.Windows.Forms.CheckBox();
            this.chkBlur2a = new System.Windows.Forms.CheckBox();
            this.chkBlur1a = new System.Windows.Forms.CheckBox();
            this.label42 = new System.Windows.Forms.Label();
            this.label28 = new System.Windows.Forms.Label();
            this.txtBlurRadV = new System.Windows.Forms.TextBox();
            this.txtBlurLines = new System.Windows.Forms.TextBox();
            this.label29 = new System.Windows.Forms.Label();
            this.groupBox9 = new System.Windows.Forms.GroupBox();
            this.radBlurExp = new System.Windows.Forms.RadioButton();
            this.radBlurLog = new System.Windows.Forms.RadioButton();
            this.radBlurPoly = new System.Windows.Forms.RadioButton();
            this.label30 = new System.Windows.Forms.Label();
            this.groupBlurSub = new System.Windows.Forms.GroupBox();
            this.chkBlur2aSub = new System.Windows.Forms.CheckBox();
            this.chkBlur4aSub = new System.Windows.Forms.CheckBox();
            this.chkBlur3aSub = new System.Windows.Forms.CheckBox();
            this.chkBlur1aSub = new System.Windows.Forms.CheckBox();
            this.txtBlurStartAlpha = new System.Windows.Forms.TextBox();
            this.label38 = new System.Windows.Forms.Label();
            this.txtBlurEndAlpha = new System.Windows.Forms.TextBox();
            this.txtBlurBase = new System.Windows.Forms.TextBox();
            this.cmdDoBlur = new System.Windows.Forms.Button();
            this.txtBlurPos = new System.Windows.Forms.TextBox();
            this.groupBox8 = new System.Windows.Forms.GroupBox();
            this.radBlurGlow = new System.Windows.Forms.RadioButton();
            this.radBlurHV = new System.Windows.Forms.RadioButton();
            this.label36 = new System.Windows.Forms.Label();
            this.label34 = new System.Windows.Forms.Label();
            this.cmdBlurListToInput = new System.Windows.Forms.Button();
            this.txtBlurRadH = new System.Windows.Forms.TextBox();
            this.cmdBlurInsertBefore = new System.Windows.Forms.Button();
            this.cmdBlurReplaceLine = new System.Windows.Forms.Button();
            this.cmdBlurInsertAfter = new System.Windows.Forms.Button();
            this.cmdBlurMoveBefore = new System.Windows.Forms.Button();
            this.label35 = new System.Windows.Forms.Label();
            this.txtBlurBefore = new System.Windows.Forms.TextBox();
            this.label32 = new System.Windows.Forms.Label();
            this.label31 = new System.Windows.Forms.Label();
            this.txtBlurOut = new System.Windows.Forms.TextBox();
            this.txtBlurIn = new System.Windows.Forms.TextBox();
            this.tabPage16 = new System.Windows.Forms.TabPage();
            this.maskedTextTransformTime = new System.Windows.Forms.MaskedTextBox();
            this.cmdTransformListToInput = new System.Windows.Forms.Button();
            this.cmdTransformInsertBefore = new System.Windows.Forms.Button();
            this.cmdTransformInsertAfter = new System.Windows.Forms.Button();
            this.cmdTransformReplaceLine = new System.Windows.Forms.Button();
            this.label45 = new System.Windows.Forms.Label();
            this.txtTransformPrecision = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.listTransformVars = new SSATool.ListViewES();
            this.columnHeader13 = new System.Windows.Forms.ColumnHeader();
            this.txtTransformCode = new System.Windows.Forms.TextBox();
            this.label47 = new System.Windows.Forms.Label();
            this.label46 = new System.Windows.Forms.Label();
            this.cmdTransformDelVar = new System.Windows.Forms.Button();
            this.cmdTransformNewVar = new System.Windows.Forms.Button();
            this.cmdDoTransform = new System.Windows.Forms.Button();
            this.label44 = new System.Windows.Forms.Label();
            this.cmdTransformDelTime = new System.Windows.Forms.Button();
            this.cmdTransformAddTime = new System.Windows.Forms.Button();
            this.listTransformTimes = new System.Windows.Forms.ListBox();
            this.label43 = new System.Windows.Forms.Label();
            this.txtTransformOut = new System.Windows.Forms.TextBox();
            this.tabPage13 = new System.Windows.Forms.TabPage();
            this.lstErrors = new System.Windows.Forms.ListBox();
            this.label27 = new System.Windows.Forms.Label();
            this.cmdCheckErrors = new System.Windows.Forms.Button();
            this.tabPage15 = new System.Windows.Forms.TabPage();
            this.labelFontNum = new System.Windows.Forms.Label();
            this.cmdFontSearchFolder = new System.Windows.Forms.Button();
            this.label40 = new System.Windows.Forms.Label();
            this.cmdFindFonts = new System.Windows.Forms.Button();
            this.label39 = new System.Windows.Forms.Label();
            this.lstFonts = new System.Windows.Forms.ListView();
            this.columnHeader7 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader8 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader9 = new System.Windows.Forms.ColumnHeader();
            this.tabPage5 = new System.Windows.Forms.TabPage();
            this.groupBox19 = new System.Windows.Forms.GroupBox();
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.groupBox18 = new System.Windows.Forms.GroupBox();
            this.lblRegroupDestIndex = new System.Windows.Forms.Label();
            this.label10 = new System.Windows.Forms.Label();
            this.lblRegroupSourceIndex = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.groupBox17 = new System.Windows.Forms.GroupBox();
            this.listRegroupPairs = new System.Windows.Forms.ListView();
            this.columnHeader10 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader12 = new System.Windows.Forms.ColumnHeader();
            this.groupBox15 = new System.Windows.Forms.GroupBox();
            this.cmdRegroupStart = new System.Windows.Forms.Button();
            this.cmdRegroupAcceptLine = new System.Windows.Forms.Button();
            this.cmdRegroupUnlinkLast = new System.Windows.Forms.Button();
            this.cmdRegroupGoBack = new System.Windows.Forms.Button();
            this.cmdRegroupSkipDestLine = new System.Windows.Forms.Button();
            this.cmdRegroupSkipSourceLine = new System.Windows.Forms.Button();
            this.cmdRegroupLink = new System.Windows.Forms.Button();
            this.groupBox14 = new System.Windows.Forms.GroupBox();
            this.txtRegroupSource = new System.Windows.Forms.TextBox();
            this.txtRegroupDest = new System.Windows.Forms.TextBox();
            this.groupBox13 = new System.Windows.Forms.GroupBox();
            this.cmbRegroupDest = new System.Windows.Forms.ComboBox();
            this.cmbRegroupSource = new System.Windows.Forms.ComboBox();
            this.label53 = new System.Windows.Forms.Label();
            this.label52 = new System.Windows.Forms.Label();
            this.tabPage17 = new System.Windows.Forms.TabPage();
            this.cmdNBReparse = new System.Windows.Forms.Button();
            this.groupBox22 = new System.Windows.Forms.GroupBox();
            this.textNBDesc = new System.Windows.Forms.TextBox();
            this.cmbNBStyle = new System.Windows.Forms.ComboBox();
            this.label59 = new System.Windows.Forms.Label();
            this.cmdNBCopyStyles = new System.Windows.Forms.Button();
            this.cmdNBInsertBefore = new System.Windows.Forms.Button();
            this.cmdNBInsertAfter = new System.Windows.Forms.Button();
            this.cmdNBReplaceLine = new System.Windows.Forms.Button();
            this.cmdNoteBox = new System.Windows.Forms.Button();
            this.groupBox21 = new System.Windows.Forms.GroupBox();
            this.textNBStyles = new System.Windows.Forms.TextBox();
            this.textNBOut = new System.Windows.Forms.TextBox();
            this.groupBox20 = new System.Windows.Forms.GroupBox();
            this.cmdNBGetTimes = new System.Windows.Forms.Button();
            this.maskedTextNBEnd = new System.Windows.Forms.MaskedTextBox();
            this.label58 = new System.Windows.Forms.Label();
            this.maskedTextNBStart = new System.Windows.Forms.MaskedTextBox();
            this.label57 = new System.Windows.Forms.Label();
            this.textNotebox2 = new System.Windows.Forms.TextBox();
            this.textNotebox1 = new System.Windows.Forms.TextBox();
            this.label56 = new System.Windows.Forms.Label();
            this.label48 = new System.Windows.Forms.Label();
            this.tabPage6.SuspendLayout();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabMain.SuspendLayout();
            this.tabPage1.SuspendLayout();
            this.groupBox12.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.tabShiftType.SuspendLayout();
            this.tabPage9.SuspendLayout();
            this.tabPage10.SuspendLayout();
            this.tabPage11.SuspendLayout();
            this.tabPage12.SuspendLayout();
            this.tabPage2.SuspendLayout();
            this.groupBox11.SuspendLayout();
            this.groupBox10.SuspendLayout();
            this.groupBox6.SuspendLayout();
            this.groupBox5.SuspendLayout();
            this.tabPage3.SuspendLayout();
            this.panelEffect.SuspendLayout();
            this.tabControl1.SuspendLayout();
            this.tabPage7.SuspendLayout();
            this.tabPage8.SuspendLayout();
            this.panelLayer.SuspendLayout();
            this.groupBox16.SuspendLayout();
            this.tabPage4.SuspendLayout();
            this.panelGradient.SuspendLayout();
            this.groupBox7.SuspendLayout();
            this.tabPage14.SuspendLayout();
            this.panelBlur.SuspendLayout();
            this.groupBlurAffect.SuspendLayout();
            this.groupBox9.SuspendLayout();
            this.groupBlurSub.SuspendLayout();
            this.groupBox8.SuspendLayout();
            this.tabPage16.SuspendLayout();
            this.tabPage13.SuspendLayout();
            this.tabPage15.SuspendLayout();
            this.tabPage5.SuspendLayout();
            this.groupBox19.SuspendLayout();
            this.groupBox18.SuspendLayout();
            this.groupBox17.SuspendLayout();
            this.groupBox15.SuspendLayout();
            this.groupBox14.SuspendLayout();
            this.groupBox13.SuspendLayout();
            this.tabPage17.SuspendLayout();
            this.groupBox22.SuspendLayout();
            this.groupBox21.SuspendLayout();
            this.groupBox20.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu1
            // 
            this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem1,
            this.menuItem11,
            this.menuItem2,
            this.menuItem10,
            this.menuItem13});
            // 
            // menuItem1
            // 
            this.menuItem1.Index = 0;
            this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuOpen,
            this.menuOpenSJIS,
            this.menuOpenEUCJP,
            this.menuSave,
            this.menuSaveAs,
            this.menuItem5,
            this.menuLoadProj,
            this.menuSaveProj,
            this.menuItem8,
            this.menuExit});
            resources.ApplyResources(this.menuItem1, "menuItem1");
            // 
            // menuOpen
            // 
            this.menuOpen.Index = 0;
            resources.ApplyResources(this.menuOpen, "menuOpen");
            this.menuOpen.Click += new System.EventHandler(this.menuOpen_Click);
            // 
            // menuOpenSJIS
            // 
            this.menuOpenSJIS.Index = 1;
            resources.ApplyResources(this.menuOpenSJIS, "menuOpenSJIS");
            this.menuOpenSJIS.Click += new System.EventHandler(this.menuOpenSJIS_Click);
            // 
            // menuOpenEUCJP
            // 
            this.menuOpenEUCJP.Index = 2;
            resources.ApplyResources(this.menuOpenEUCJP, "menuOpenEUCJP");
            this.menuOpenEUCJP.Click += new System.EventHandler(this.menuOpenEUCJP_Click);
            // 
            // menuSave
            // 
            this.menuSave.Index = 3;
            resources.ApplyResources(this.menuSave, "menuSave");
            this.menuSave.Click += new System.EventHandler(this.menuSave_Click);
            // 
            // menuSaveAs
            // 
            this.menuSaveAs.Index = 4;
            resources.ApplyResources(this.menuSaveAs, "menuSaveAs");
            this.menuSaveAs.Click += new System.EventHandler(this.menuSaveAs_Click);
            // 
            // menuItem5
            // 
            this.menuItem5.Index = 5;
            resources.ApplyResources(this.menuItem5, "menuItem5");
            // 
            // menuLoadProj
            // 
            this.menuLoadProj.Index = 6;
            resources.ApplyResources(this.menuLoadProj, "menuLoadProj");
            this.menuLoadProj.Click += new System.EventHandler(this.menuLoadProj_Click);
            // 
            // menuSaveProj
            // 
            this.menuSaveProj.Index = 7;
            resources.ApplyResources(this.menuSaveProj, "menuSaveProj");
            this.menuSaveProj.Click += new System.EventHandler(this.menuSaveProj_Click);
            // 
            // menuItem8
            // 
            this.menuItem8.Index = 8;
            resources.ApplyResources(this.menuItem8, "menuItem8");
            // 
            // menuExit
            // 
            this.menuExit.Index = 9;
            resources.ApplyResources(this.menuExit, "menuExit");
            this.menuExit.Click += new System.EventHandler(this.menuExit_Click);
            // 
            // menuItem11
            // 
            this.menuItem11.Index = 1;
            this.menuItem11.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuUndo,
            this.menuRedo,
            this.menuClearUndoRedo});
            resources.ApplyResources(this.menuItem11, "menuItem11");
            // 
            // menuUndo
            // 
            resources.ApplyResources(this.menuUndo, "menuUndo");
            this.menuUndo.Index = 0;
            this.menuUndo.Click += new System.EventHandler(this.menuUndo_Click);
            // 
            // menuRedo
            // 
            resources.ApplyResources(this.menuRedo, "menuRedo");
            this.menuRedo.Index = 1;
            this.menuRedo.Click += new System.EventHandler(this.menuRedo_Click);
            // 
            // menuClearUndoRedo
            // 
            this.menuClearUndoRedo.Index = 2;
            resources.ApplyResources(this.menuClearUndoRedo, "menuClearUndoRedo");
            this.menuClearUndoRedo.Click += new System.EventHandler(this.menuClearUndoRedo_Click);
            // 
            // menuItem2
            // 
            this.menuItem2.Index = 2;
            this.menuItem2.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem7,
            this.menuItem9,
            this.menuGoTo,
            this.menuChangeLine,
            this.menuRemoveLine});
            resources.ApplyResources(this.menuItem2, "menuItem2");
            // 
            // menuItem7
            // 
            this.menuItem7.Index = 0;
            this.menuItem7.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuNewLineBeginning,
            this.menuNewLineEnd,
            this.menuNewLineAbove,
            this.menuNewLineBelow});
            resources.ApplyResources(this.menuItem7, "menuItem7");
            // 
            // menuNewLineBeginning
            // 
            this.menuNewLineBeginning.Index = 0;
            resources.ApplyResources(this.menuNewLineBeginning, "menuNewLineBeginning");
            this.menuNewLineBeginning.Click += new System.EventHandler(this.menuNewLineBeginning_Click);
            // 
            // menuNewLineEnd
            // 
            this.menuNewLineEnd.Index = 1;
            resources.ApplyResources(this.menuNewLineEnd, "menuNewLineEnd");
            this.menuNewLineEnd.Click += new System.EventHandler(this.menuNewLineEnd_Click);
            // 
            // menuNewLineAbove
            // 
            this.menuNewLineAbove.Index = 2;
            resources.ApplyResources(this.menuNewLineAbove, "menuNewLineAbove");
            this.menuNewLineAbove.Click += new System.EventHandler(this.menuNewLineAbove_Click);
            // 
            // menuNewLineBelow
            // 
            this.menuNewLineBelow.Index = 3;
            resources.ApplyResources(this.menuNewLineBelow, "menuNewLineBelow");
            this.menuNewLineBelow.Click += new System.EventHandler(this.menuNewLineBelow_Click);
            // 
            // menuItem9
            // 
            this.menuItem9.Index = 1;
            this.menuItem9.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuMoveLineUp,
            this.menuMoveLineDown});
            resources.ApplyResources(this.menuItem9, "menuItem9");
            // 
            // menuMoveLineUp
            // 
            this.menuMoveLineUp.Index = 0;
            resources.ApplyResources(this.menuMoveLineUp, "menuMoveLineUp");
            this.menuMoveLineUp.Click += new System.EventHandler(this.menuMoveLineUp_Click);
            // 
            // menuMoveLineDown
            // 
            this.menuMoveLineDown.Index = 1;
            resources.ApplyResources(this.menuMoveLineDown, "menuMoveLineDown");
            this.menuMoveLineDown.Click += new System.EventHandler(this.menuMoveLineDown_Click);
            // 
            // menuGoTo
            // 
            this.menuGoTo.Index = 2;
            resources.ApplyResources(this.menuGoTo, "menuGoTo");
            this.menuGoTo.Click += new System.EventHandler(this.menuGoTo_Click);
            // 
            // menuChangeLine
            // 
            this.menuChangeLine.Index = 3;
            resources.ApplyResources(this.menuChangeLine, "menuChangeLine");
            this.menuChangeLine.Click += new System.EventHandler(this.menuChangeLine_Click);
            // 
            // menuRemoveLine
            // 
            this.menuRemoveLine.Index = 4;
            resources.ApplyResources(this.menuRemoveLine, "menuRemoveLine");
            this.menuRemoveLine.Click += new System.EventHandler(this.menuRemoveLine_Click);
            // 
            // menuItem10
            // 
            this.menuItem10.Index = 3;
            this.menuItem10.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuFont,
            this.menuItem4,
            this.menuItem6});
            resources.ApplyResources(this.menuItem10, "menuItem10");
            // 
            // menuFont
            // 
            this.menuFont.Index = 0;
            resources.ApplyResources(this.menuFont, "menuFont");
            this.menuFont.Click += new System.EventHandler(this.menuFont_Click);
            // 
            // menuItem4
            // 
            this.menuItem4.Index = 1;
            resources.ApplyResources(this.menuItem4, "menuItem4");
            // 
            // menuItem6
            // 
            this.menuItem6.Index = 2;
            this.menuItem6.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuPrecisioncs,
            this.menuPrecisionms});
            resources.ApplyResources(this.menuItem6, "menuItem6");
            // 
            // menuPrecisioncs
            // 
            this.menuPrecisioncs.Checked = true;
            this.menuPrecisioncs.Index = 0;
            resources.ApplyResources(this.menuPrecisioncs, "menuPrecisioncs");
            this.menuPrecisioncs.Click += new System.EventHandler(this.menuPrecisioncs_Click);
            // 
            // menuPrecisionms
            // 
            this.menuPrecisionms.Index = 1;
            resources.ApplyResources(this.menuPrecisionms, "menuPrecisionms");
            this.menuPrecisionms.Click += new System.EventHandler(this.menuPrecisionms_Click);
            // 
            // menuItem13
            // 
            this.menuItem13.Index = 4;
            this.menuItem13.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuShowEffectEditor,
            this.menuItem15,
            this.menuItem3,
            this.menuLoadEffects});
            resources.ApplyResources(this.menuItem13, "menuItem13");
            // 
            // menuShowEffectEditor
            // 
            this.menuShowEffectEditor.Index = 0;
            resources.ApplyResources(this.menuShowEffectEditor, "menuShowEffectEditor");
            this.menuShowEffectEditor.Click += new System.EventHandler(this.menuShowEffectsEditor_Click);
            // 
            // menuItem15
            // 
            this.menuItem15.Index = 1;
            this.menuItem15.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.menuItem16});
            resources.ApplyResources(this.menuItem15, "menuItem15");
            // 
            // menuItem16
            // 
            this.menuItem16.Index = 0;
            resources.ApplyResources(this.menuItem16, "menuItem16");
            this.menuItem16.Click += new System.EventHandler(this.menuFilter_Click);
            // 
            // menuItem3
            // 
            this.menuItem3.Index = 2;
            resources.ApplyResources(this.menuItem3, "menuItem3");
            // 
            // menuLoadEffects
            // 
            this.menuLoadEffects.Index = 3;
            resources.ApplyResources(this.menuLoadEffects, "menuLoadEffects");
            this.menuLoadEffects.Click += new System.EventHandler(this.menuLoadEffects_Click);
            // 
            // tabPage6
            // 
            this.tabPage6.Controls.Add(this.treeView1);
            this.tabPage6.Controls.Add(this.maskedTextBox1);
            this.tabPage6.Controls.Add(this.checkBox2);
            this.tabPage6.Controls.Add(this.button6);
            this.tabPage6.Controls.Add(this.comboBox1);
            this.tabPage6.Controls.Add(this.label55);
            this.tabPage6.Controls.Add(this.checkBox1);
            this.tabPage6.Controls.Add(this.button4);
            this.tabPage6.Controls.Add(this.button3);
            this.tabPage6.Controls.Add(this.button2);
            this.tabPage6.Controls.Add(this.button1);
            this.tabPage6.Controls.Add(this.textBox2);
            resources.ApplyResources(this.tabPage6, "tabPage6");
            this.tabPage6.Name = "tabPage6";
            this.tabPage6.UseVisualStyleBackColor = true;
            // 
            // treeView1
            // 
            this.treeView1.LineColor = System.Drawing.Color.Empty;
            resources.ApplyResources(this.treeView1, "treeView1");
            this.treeView1.Name = "treeView1";
            // 
            // maskedTextBox1
            // 
            resources.ApplyResources(this.maskedTextBox1, "maskedTextBox1");
            this.maskedTextBox1.Name = "maskedTextBox1";
            // 
            // checkBox2
            // 
            resources.ApplyResources(this.checkBox2, "checkBox2");
            this.checkBox2.Name = "checkBox2";
            this.checkBox2.UseVisualStyleBackColor = true;
            // 
            // button6
            // 
            resources.ApplyResources(this.button6, "button6");
            this.button6.Name = "button6";
            this.button6.UseVisualStyleBackColor = true;
            // 
            // comboBox1
            // 
            this.comboBox1.FormattingEnabled = true;
            resources.ApplyResources(this.comboBox1, "comboBox1");
            this.comboBox1.Name = "comboBox1";
            // 
            // label55
            // 
            resources.ApplyResources(this.label55, "label55");
            this.label55.Name = "label55";
            // 
            // checkBox1
            // 
            resources.ApplyResources(this.checkBox1, "checkBox1");
            this.checkBox1.Checked = true;
            this.checkBox1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.UseVisualStyleBackColor = true;
            // 
            // button4
            // 
            resources.ApplyResources(this.button4, "button4");
            this.button4.Name = "button4";
            this.button4.UseVisualStyleBackColor = true;
            // 
            // button3
            // 
            resources.ApplyResources(this.button3, "button3");
            this.button3.Name = "button3";
            this.button3.UseVisualStyleBackColor = true;
            // 
            // button2
            // 
            resources.ApplyResources(this.button2, "button2");
            this.button2.Name = "button2";
            this.button2.UseCompatibleTextRendering = true;
            this.button2.UseVisualStyleBackColor = true;
            // 
            // button1
            // 
            resources.ApplyResources(this.button1, "button1");
            this.button1.Name = "button1";
            this.button1.UseCompatibleTextRendering = true;
            this.button1.UseVisualStyleBackColor = true;
            // 
            // textBox2
            // 
            resources.ApplyResources(this.textBox2, "textBox2");
            this.textBox2.Name = "textBox2";
            // 
            // columnHeader11
            // 
            resources.ApplyResources(this.columnHeader11, "columnHeader11");
            // 
            // lstEffectOpts
            // 
            resources.ApplyResources(this.lstEffectOpts, "lstEffectOpts");
            this.lstEffectOpts.MultiColumn = true;
            this.lstEffectOpts.Name = "lstEffectOpts";
            // 
            // fontDialog1
            // 
            this.fontDialog1.AllowVerticalFonts = false;
            this.fontDialog1.ShowEffects = false;
            // 
            // folderBrowserDialog1
            // 
            this.folderBrowserDialog1.ShowNewFolderButton = false;
            // 
            // splitContainer1
            // 
            resources.ApplyResources(this.splitContainer1, "splitContainer1");
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.groupBox1);
            this.splitContainer1.Panel1.Controls.Add(this.listSSA);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.tabMain);
            this.splitContainer1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.splitContainer1_MouseDown);
            this.splitContainer1.SplitterMoved += new System.Windows.Forms.SplitterEventHandler(this.splitterMoved);
            this.splitContainer1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.splitContainer1_MouseUp);
            this.splitContainer1.SizeChanged += new System.EventHandler(this.Resize_Handler);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.labelSelLine);
            this.groupBox1.Controls.Add(this.label41);
            this.groupBox1.Controls.Add(this.cmdReloadFile);
            this.groupBox1.Controls.Add(this.lstStyles);
            this.groupBox1.Controls.Add(this.cmdSSAInvertSel);
            this.groupBox1.Controls.Add(this.cmdSSADeselall);
            this.groupBox1.Controls.Add(this.cmdSSASelall);
            this.groupBox1.Controls.Add(this.cmdDeselAll);
            this.groupBox1.Controls.Add(this.cmdSelAll);
            resources.ApplyResources(this.groupBox1, "groupBox1");
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.TabStop = false;
            // 
            // labelSelLine
            // 
            resources.ApplyResources(this.labelSelLine, "labelSelLine");
            this.labelSelLine.Name = "labelSelLine";
            // 
            // label41
            // 
            resources.ApplyResources(this.label41, "label41");
            this.label41.Name = "label41";
            // 
            // cmdReloadFile
            // 
            resources.ApplyResources(this.cmdReloadFile, "cmdReloadFile");
            this.cmdReloadFile.Name = "cmdReloadFile";
            this.cmdReloadFile.UseCompatibleTextRendering = true;
            this.cmdReloadFile.Click += new System.EventHandler(this.cmdReloadFile_Click);
            // 
            // lstStyles
            // 
            resources.ApplyResources(this.lstStyles, "lstStyles");
            this.lstStyles.Name = "lstStyles";
            this.lstStyles.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.lstStyles_ItemCheck);
            // 
            // cmdSSAInvertSel
            // 
            resources.ApplyResources(this.cmdSSAInvertSel, "cmdSSAInvertSel");
            this.cmdSSAInvertSel.Name = "cmdSSAInvertSel";
            this.cmdSSAInvertSel.UseCompatibleTextRendering = true;
            // 
            // cmdSSADeselall
            // 
            resources.ApplyResources(this.cmdSSADeselall, "cmdSSADeselall");
            this.cmdSSADeselall.Name = "cmdSSADeselall";
            this.cmdSSADeselall.UseCompatibleTextRendering = true;
            this.cmdSSADeselall.Click += new System.EventHandler(this.cmdSSADeselall_Click);
            // 
            // cmdSSASelall
            // 
            resources.ApplyResources(this.cmdSSASelall, "cmdSSASelall");
            this.cmdSSASelall.Name = "cmdSSASelall";
            this.cmdSSASelall.UseCompatibleTextRendering = true;
            this.cmdSSASelall.Click += new System.EventHandler(this.cmdSSASelall_Click);
            // 
            // cmdDeselAll
            // 
            resources.ApplyResources(this.cmdDeselAll, "cmdDeselAll");
            this.cmdDeselAll.Name = "cmdDeselAll";
            this.cmdDeselAll.UseCompatibleTextRendering = true;
            this.cmdDeselAll.Click += new System.EventHandler(this.cmdDeselAll_Click);
            // 
            // cmdSelAll
            // 
            resources.ApplyResources(this.cmdSelAll, "cmdSelAll");
            this.cmdSelAll.Name = "cmdSelAll";
            this.cmdSelAll.UseCompatibleTextRendering = true;
            this.cmdSelAll.Click += new System.EventHandler(this.cmdSelAll_Click);
            // 
            // listSSA
            // 
            this.listSSA.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader14});
            this.listSSA.FullRowSelect = true;
            this.listSSA.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listSSA.HideSelection = false;
            this.listSSA.LabelEdit = true;
            resources.ApplyResources(this.listSSA, "listSSA");
            this.listSSA.Name = "listSSA";
            this.listSSA.OwnerDraw = true;
            this.listSSA.UseCompatibleStateImageBehavior = false;
            this.listSSA.View = System.Windows.Forms.View.Details;
            this.listSSA.VirtualMode = true;
            this.listSSA.DrawItem += new System.Windows.Forms.DrawListViewItemEventHandler(this.listSSA_DrawItem);
            this.listSSA.DoubleClick += new System.EventHandler(this.listSSA_DoubleClick);
            this.listSSA.VirtualItemsSelectionRangeChanged += new System.Windows.Forms.ListViewVirtualItemsSelectionRangeChangedEventHandler(this.listSSA_VirtualItemsSelectionRangeChanged);
            this.listSSA.OnScroll += new System.Windows.Forms.ScrollEventHandler(this.listSSA_OnScroll);
            this.listSSA.SelectedIndexChanged += new System.EventHandler(this.listSSA_SelectedIndexChanged);
            this.listSSA.MouseUp += new System.Windows.Forms.MouseEventHandler(this.listSSA_MouseUp);
            this.listSSA.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.listSSA_ItemCheck);
            this.listSSA.RetrieveVirtualItem += new System.Windows.Forms.RetrieveVirtualItemEventHandler(this.listSSA_RetrieveVirtualItem);
            this.listSSA.ItemSelectionChanged += new System.Windows.Forms.ListViewItemSelectionChangedEventHandler(this.listSSA_ItemSelectionChanged);
            this.listSSA.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.listSSA_KeyPress);
            this.listSSA.KeyUp += new System.Windows.Forms.KeyEventHandler(this.listSSA_KeyUp);
            // 
            // columnHeader14
            // 
            resources.ApplyResources(this.columnHeader14, "columnHeader14");
            // 
            // tabMain
            // 
            this.tabMain.Controls.Add(this.tabPage1);
            this.tabMain.Controls.Add(this.tabPage2);
            this.tabMain.Controls.Add(this.tabPage3);
            this.tabMain.Controls.Add(this.tabPage4);
            this.tabMain.Controls.Add(this.tabPage14);
            this.tabMain.Controls.Add(this.tabPage16);
            this.tabMain.Controls.Add(this.tabPage13);
            this.tabMain.Controls.Add(this.tabPage15);
            this.tabMain.Controls.Add(this.tabPage5);
            this.tabMain.Controls.Add(this.tabPage17);
            resources.ApplyResources(this.tabMain, "tabMain");
            this.tabMain.Name = "tabMain";
            this.tabMain.SelectedIndex = 0;
            this.tabMain.Resize += new System.EventHandler(this.tabMain_Resize);
            // 
            // tabPage1
            // 
            this.tabPage1.Controls.Add(this.groupBox12);
            this.tabPage1.Controls.Add(this.groupBox4);
            this.tabPage1.Controls.Add(this.groupBox3);
            this.tabPage1.Controls.Add(this.groupBox2);
            resources.ApplyResources(this.tabPage1, "tabPage1");
            this.tabPage1.Name = "tabPage1";
            this.tabPage1.UseVisualStyleBackColor = true;
            // 
            // groupBox12
            // 
            this.groupBox12.Controls.Add(this.cmbFR);
            this.groupBox12.Controls.Add(this.label54);
            resources.ApplyResources(this.groupBox12, "groupBox12");
            this.groupBox12.Name = "groupBox12";
            this.groupBox12.TabStop = false;
            // 
            // cmbFR
            // 
            this.cmbFR.FormattingEnabled = true;
            this.cmbFR.Items.AddRange(new object[] {
            resources.GetString("cmbFR.Items"),
            resources.GetString("cmbFR.Items1"),
            resources.GetString("cmbFR.Items2"),
            resources.GetString("cmbFR.Items3"),
            resources.GetString("cmbFR.Items4"),
            resources.GetString("cmbFR.Items5"),
            resources.GetString("cmbFR.Items6"),
            resources.GetString("cmbFR.Items7")});
            resources.ApplyResources(this.cmbFR, "cmbFR");
            this.cmbFR.Name = "cmbFR";
            // 
            // label54
            // 
            resources.ApplyResources(this.label54, "label54");
            this.label54.Name = "label54";
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.chkSRCaseSensitive);
            this.groupBox4.Controls.Add(this.cmdSearchReplace);
            this.groupBox4.Controls.Add(this.txtSearchReplaceWith);
            this.groupBox4.Controls.Add(this.txtSearchFor);
            this.groupBox4.Controls.Add(this.label12);
            this.groupBox4.Controls.Add(this.label11);
            resources.ApplyResources(this.groupBox4, "groupBox4");
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.TabStop = false;
            // 
            // chkSRCaseSensitive
            // 
            resources.ApplyResources(this.chkSRCaseSensitive, "chkSRCaseSensitive");
            this.chkSRCaseSensitive.Name = "chkSRCaseSensitive";
            this.chkSRCaseSensitive.UseVisualStyleBackColor = true;
            // 
            // cmdSearchReplace
            // 
            resources.ApplyResources(this.cmdSearchReplace, "cmdSearchReplace");
            this.cmdSearchReplace.Name = "cmdSearchReplace";
            this.cmdSearchReplace.Click += new System.EventHandler(this.cmdSearchReplace_Click);
            // 
            // txtSearchReplaceWith
            // 
            resources.ApplyResources(this.txtSearchReplaceWith, "txtSearchReplaceWith");
            this.txtSearchReplaceWith.Name = "txtSearchReplaceWith";
            // 
            // txtSearchFor
            // 
            resources.ApplyResources(this.txtSearchFor, "txtSearchFor");
            this.txtSearchFor.Name = "txtSearchFor";
            // 
            // label12
            // 
            resources.ApplyResources(this.label12, "label12");
            this.label12.Name = "label12";
            // 
            // label11
            // 
            resources.ApplyResources(this.label11, "label11");
            this.label11.Name = "label11";
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.cmdChangeLastKLen);
            this.groupBox3.Controls.Add(this.cmdRemoveDups);
            this.groupBox3.Controls.Add(this.cmdStrip);
            resources.ApplyResources(this.groupBox3, "groupBox3");
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.TabStop = false;
            // 
            // cmdChangeLastKLen
            // 
            resources.ApplyResources(this.cmdChangeLastKLen, "cmdChangeLastKLen");
            this.cmdChangeLastKLen.Name = "cmdChangeLastKLen";
            this.cmdChangeLastKLen.Click += new System.EventHandler(this.cmdChangeLastKLen_Click);
            // 
            // cmdRemoveDups
            // 
            resources.ApplyResources(this.cmdRemoveDups, "cmdRemoveDups");
            this.cmdRemoveDups.Name = "cmdRemoveDups";
            this.cmdRemoveDups.Click += new System.EventHandler(this.cmdRemoveDups_Click);
            // 
            // cmdStrip
            // 
            resources.ApplyResources(this.cmdStrip, "cmdStrip");
            this.cmdStrip.Name = "cmdStrip";
            this.cmdStrip.Click += new System.EventHandler(this.cmdStrip_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.cmdShift);
            this.groupBox2.Controls.Add(this.chkShiftNoNeg);
            this.groupBox2.Controls.Add(this.chkShiftEnd);
            this.groupBox2.Controls.Add(this.chkShiftStart);
            this.groupBox2.Controls.Add(this.tabShiftType);
            resources.ApplyResources(this.groupBox2, "groupBox2");
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.TabStop = false;
            // 
            // cmdShift
            // 
            resources.ApplyResources(this.cmdShift, "cmdShift");
            this.cmdShift.Name = "cmdShift";
            this.cmdShift.Click += new System.EventHandler(this.cmdShift_Click);
            // 
            // chkShiftNoNeg
            // 
            this.chkShiftNoNeg.Checked = true;
            this.chkShiftNoNeg.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkShiftNoNeg, "chkShiftNoNeg");
            this.chkShiftNoNeg.Name = "chkShiftNoNeg";
            // 
            // chkShiftEnd
            // 
            this.chkShiftEnd.Checked = true;
            this.chkShiftEnd.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkShiftEnd, "chkShiftEnd");
            this.chkShiftEnd.Name = "chkShiftEnd";
            // 
            // chkShiftStart
            // 
            this.chkShiftStart.Checked = true;
            this.chkShiftStart.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkShiftStart, "chkShiftStart");
            this.chkShiftStart.Name = "chkShiftStart";
            // 
            // tabShiftType
            // 
            resources.ApplyResources(this.tabShiftType, "tabShiftType");
            this.tabShiftType.Controls.Add(this.tabPage9);
            this.tabShiftType.Controls.Add(this.tabPage10);
            this.tabShiftType.Controls.Add(this.tabPage11);
            this.tabShiftType.Controls.Add(this.tabPage12);
            this.tabShiftType.Name = "tabShiftType";
            this.tabShiftType.SelectedIndex = 0;
            // 
            // tabPage9
            // 
            this.tabPage9.Controls.Add(this.maskedTextShiftTime);
            this.tabPage9.Controls.Add(this.radTShiftBack);
            this.tabPage9.Controls.Add(this.radTShiftForward);
            this.tabPage9.Controls.Add(this.label3);
            resources.ApplyResources(this.tabPage9, "tabPage9");
            this.tabPage9.Name = "tabPage9";
            // 
            // maskedTextShiftTime
            // 
            resources.ApplyResources(this.maskedTextShiftTime, "maskedTextShiftTime");
            this.maskedTextShiftTime.Name = "maskedTextShiftTime";
            this.maskedTextShiftTime.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // radTShiftBack
            // 
            resources.ApplyResources(this.radTShiftBack, "radTShiftBack");
            this.radTShiftBack.Name = "radTShiftBack";
            // 
            // radTShiftForward
            // 
            this.radTShiftForward.Checked = true;
            resources.ApplyResources(this.radTShiftForward, "radTShiftForward");
            this.radTShiftForward.Name = "radTShiftForward";
            this.radTShiftForward.TabStop = true;
            // 
            // label3
            // 
            resources.ApplyResources(this.label3, "label3");
            this.label3.Name = "label3";
            // 
            // tabPage10
            // 
            this.tabPage10.Controls.Add(this.radFShiftBackward);
            this.tabPage10.Controls.Add(this.radFShiftForward);
            this.tabPage10.Controls.Add(this.txtShiftFrames);
            this.tabPage10.Controls.Add(this.label4);
            resources.ApplyResources(this.tabPage10, "tabPage10");
            this.tabPage10.Name = "tabPage10";
            // 
            // radFShiftBackward
            // 
            resources.ApplyResources(this.radFShiftBackward, "radFShiftBackward");
            this.radFShiftBackward.Name = "radFShiftBackward";
            // 
            // radFShiftForward
            // 
            this.radFShiftForward.Checked = true;
            resources.ApplyResources(this.radFShiftForward, "radFShiftForward");
            this.radFShiftForward.Name = "radFShiftForward";
            this.radFShiftForward.TabStop = true;
            // 
            // txtShiftFrames
            // 
            resources.ApplyResources(this.txtShiftFrames, "txtShiftFrames");
            this.txtShiftFrames.Name = "txtShiftFrames";
            // 
            // label4
            // 
            resources.ApplyResources(this.label4, "label4");
            this.label4.Name = "label4";
            // 
            // tabPage11
            // 
            this.tabPage11.Controls.Add(this.maskedTextTDShiftNew);
            this.tabPage11.Controls.Add(this.maskedTextTDShiftOld);
            this.tabPage11.Controls.Add(this.label7);
            this.tabPage11.Controls.Add(this.label6);
            resources.ApplyResources(this.tabPage11, "tabPage11");
            this.tabPage11.Name = "tabPage11";
            // 
            // maskedTextTDShiftNew
            // 
            resources.ApplyResources(this.maskedTextTDShiftNew, "maskedTextTDShiftNew");
            this.maskedTextTDShiftNew.Name = "maskedTextTDShiftNew";
            this.maskedTextTDShiftNew.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // maskedTextTDShiftOld
            // 
            resources.ApplyResources(this.maskedTextTDShiftOld, "maskedTextTDShiftOld");
            this.maskedTextTDShiftOld.Name = "maskedTextTDShiftOld";
            this.maskedTextTDShiftOld.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // label7
            // 
            resources.ApplyResources(this.label7, "label7");
            this.label7.Name = "label7";
            // 
            // label6
            // 
            resources.ApplyResources(this.label6, "label6");
            this.label6.Name = "label6";
            // 
            // tabPage12
            // 
            this.tabPage12.Controls.Add(this.label9);
            this.tabPage12.Controls.Add(this.label8);
            this.tabPage12.Controls.Add(this.txtFDShiftNew);
            this.tabPage12.Controls.Add(this.txtFDShiftOld);
            resources.ApplyResources(this.tabPage12, "tabPage12");
            this.tabPage12.Name = "tabPage12";
            // 
            // label9
            // 
            resources.ApplyResources(this.label9, "label9");
            this.label9.Name = "label9";
            // 
            // label8
            // 
            resources.ApplyResources(this.label8, "label8");
            this.label8.Name = "label8";
            // 
            // txtFDShiftNew
            // 
            resources.ApplyResources(this.txtFDShiftNew, "txtFDShiftNew");
            this.txtFDShiftNew.Name = "txtFDShiftNew";
            // 
            // txtFDShiftOld
            // 
            resources.ApplyResources(this.txtFDShiftOld, "txtFDShiftOld");
            this.txtFDShiftOld.Name = "txtFDShiftOld";
            // 
            // tabPage2
            // 
            this.tabPage2.Controls.Add(this.groupBox11);
            this.tabPage2.Controls.Add(this.groupBox10);
            this.tabPage2.Controls.Add(this.groupBox6);
            this.tabPage2.Controls.Add(this.groupBox5);
            resources.ApplyResources(this.tabPage2, "tabPage2");
            this.tabPage2.Name = "tabPage2";
            this.tabPage2.UseVisualStyleBackColor = true;
            // 
            // groupBox11
            // 
            this.groupBox11.Controls.Add(this.cmdTimecodesDeapply);
            this.groupBox11.Controls.Add(this.cmdTimecodesApply);
            this.groupBox11.Controls.Add(this.cmdTimecodesBrowse);
            this.groupBox11.Controls.Add(this.textTimecodeFile);
            resources.ApplyResources(this.groupBox11, "groupBox11");
            this.groupBox11.Name = "groupBox11";
            this.groupBox11.TabStop = false;
            // 
            // cmdTimecodesDeapply
            // 
            resources.ApplyResources(this.cmdTimecodesDeapply, "cmdTimecodesDeapply");
            this.cmdTimecodesDeapply.Name = "cmdTimecodesDeapply";
            this.cmdTimecodesDeapply.UseVisualStyleBackColor = true;
            this.cmdTimecodesDeapply.Click += new System.EventHandler(this.cmdTimecodesDeapply_Click);
            // 
            // cmdTimecodesApply
            // 
            resources.ApplyResources(this.cmdTimecodesApply, "cmdTimecodesApply");
            this.cmdTimecodesApply.Name = "cmdTimecodesApply";
            this.cmdTimecodesApply.UseVisualStyleBackColor = true;
            this.cmdTimecodesApply.Click += new System.EventHandler(this.cmdTimecodesApply_Click);
            // 
            // cmdTimecodesBrowse
            // 
            resources.ApplyResources(this.cmdTimecodesBrowse, "cmdTimecodesBrowse");
            this.cmdTimecodesBrowse.Name = "cmdTimecodesBrowse";
            this.cmdTimecodesBrowse.UseCompatibleTextRendering = true;
            this.cmdTimecodesBrowse.UseVisualStyleBackColor = true;
            this.cmdTimecodesBrowse.Click += new System.EventHandler(this.cmdTimecodesBrowse_Click);
            // 
            // textTimecodeFile
            // 
            resources.ApplyResources(this.textTimecodeFile, "textTimecodeFile");
            this.textTimecodeFile.Name = "textTimecodeFile";
            // 
            // groupBox10
            // 
            this.groupBox10.Controls.Add(this.txtLeadOut);
            this.groupBox10.Controls.Add(this.txtLeadIn);
            this.groupBox10.Controls.Add(this.chkLeadOut);
            this.groupBox10.Controls.Add(this.chkLeadIn);
            this.groupBox10.Controls.Add(this.cmdLead);
            this.groupBox10.Controls.Add(this.chkLeadKaraEmptyK);
            this.groupBox10.Controls.Add(this.chkLeadKaraOnly);
            resources.ApplyResources(this.groupBox10, "groupBox10");
            this.groupBox10.Name = "groupBox10";
            this.groupBox10.TabStop = false;
            // 
            // txtLeadOut
            // 
            resources.ApplyResources(this.txtLeadOut, "txtLeadOut");
            this.txtLeadOut.Name = "txtLeadOut";
            this.txtLeadOut.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // txtLeadIn
            // 
            resources.ApplyResources(this.txtLeadIn, "txtLeadIn");
            this.txtLeadIn.Name = "txtLeadIn";
            this.txtLeadIn.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // chkLeadOut
            // 
            resources.ApplyResources(this.chkLeadOut, "chkLeadOut");
            this.chkLeadOut.Name = "chkLeadOut";
            this.chkLeadOut.UseCompatibleTextRendering = true;
            this.chkLeadOut.UseVisualStyleBackColor = true;
            // 
            // chkLeadIn
            // 
            resources.ApplyResources(this.chkLeadIn, "chkLeadIn");
            this.chkLeadIn.Name = "chkLeadIn";
            this.chkLeadIn.UseCompatibleTextRendering = true;
            this.chkLeadIn.UseVisualStyleBackColor = true;
            // 
            // cmdLead
            // 
            resources.ApplyResources(this.cmdLead, "cmdLead");
            this.cmdLead.Name = "cmdLead";
            this.cmdLead.UseVisualStyleBackColor = true;
            this.cmdLead.Click += new System.EventHandler(this.cmdLead_Click);
            // 
            // chkLeadKaraEmptyK
            // 
            resources.ApplyResources(this.chkLeadKaraEmptyK, "chkLeadKaraEmptyK");
            this.chkLeadKaraEmptyK.Name = "chkLeadKaraEmptyK";
            this.chkLeadKaraEmptyK.UseVisualStyleBackColor = true;
            // 
            // chkLeadKaraOnly
            // 
            resources.ApplyResources(this.chkLeadKaraOnly, "chkLeadKaraOnly");
            this.chkLeadKaraOnly.Name = "chkLeadKaraOnly";
            this.chkLeadKaraOnly.UseVisualStyleBackColor = true;
            // 
            // groupBox6
            // 
            this.groupBox6.Controls.Add(this.chkScalePlayRes);
            this.groupBox6.Controls.Add(this.txtScaleYDenominator);
            this.groupBox6.Controls.Add(this.label50);
            this.groupBox6.Controls.Add(this.txtScaleYNumerator);
            this.groupBox6.Controls.Add(this.label51);
            this.groupBox6.Controls.Add(this.cmdScale);
            this.groupBox6.Controls.Add(this.txtScaleXDenominator);
            this.groupBox6.Controls.Add(this.label26);
            this.groupBox6.Controls.Add(this.txtScaleXNumerator);
            this.groupBox6.Controls.Add(this.label25);
            resources.ApplyResources(this.groupBox6, "groupBox6");
            this.groupBox6.Name = "groupBox6";
            this.groupBox6.TabStop = false;
            // 
            // chkScalePlayRes
            // 
            resources.ApplyResources(this.chkScalePlayRes, "chkScalePlayRes");
            this.chkScalePlayRes.Name = "chkScalePlayRes";
            this.chkScalePlayRes.UseVisualStyleBackColor = true;
            // 
            // txtScaleYDenominator
            // 
            resources.ApplyResources(this.txtScaleYDenominator, "txtScaleYDenominator");
            this.txtScaleYDenominator.Name = "txtScaleYDenominator";
            // 
            // label50
            // 
            resources.ApplyResources(this.label50, "label50");
            this.label50.Name = "label50";
            // 
            // txtScaleYNumerator
            // 
            resources.ApplyResources(this.txtScaleYNumerator, "txtScaleYNumerator");
            this.txtScaleYNumerator.Name = "txtScaleYNumerator";
            // 
            // label51
            // 
            resources.ApplyResources(this.label51, "label51");
            this.label51.Name = "label51";
            // 
            // cmdScale
            // 
            resources.ApplyResources(this.cmdScale, "cmdScale");
            this.cmdScale.Name = "cmdScale";
            this.cmdScale.Click += new System.EventHandler(this.cmdScale_Click);
            // 
            // txtScaleXDenominator
            // 
            resources.ApplyResources(this.txtScaleXDenominator, "txtScaleXDenominator");
            this.txtScaleXDenominator.Name = "txtScaleXDenominator";
            // 
            // label26
            // 
            resources.ApplyResources(this.label26, "label26");
            this.label26.Name = "label26";
            // 
            // txtScaleXNumerator
            // 
            resources.ApplyResources(this.txtScaleXNumerator, "txtScaleXNumerator");
            this.txtScaleXNumerator.Name = "txtScaleXNumerator";
            // 
            // label25
            // 
            resources.ApplyResources(this.label25, "label25");
            this.label25.Name = "label25";
            // 
            // groupBox5
            // 
            this.groupBox5.Controls.Add(this.cmdLenBasedK);
            this.groupBox5.Controls.Add(this.txtLenBasedKThreshold);
            this.groupBox5.Controls.Add(this.label13);
            resources.ApplyResources(this.groupBox5, "groupBox5");
            this.groupBox5.Name = "groupBox5";
            this.groupBox5.TabStop = false;
            // 
            // cmdLenBasedK
            // 
            resources.ApplyResources(this.cmdLenBasedK, "cmdLenBasedK");
            this.cmdLenBasedK.Name = "cmdLenBasedK";
            this.cmdLenBasedK.Click += new System.EventHandler(this.cmdLenBasedK_Click);
            // 
            // txtLenBasedKThreshold
            // 
            resources.ApplyResources(this.txtLenBasedKThreshold, "txtLenBasedKThreshold");
            this.txtLenBasedKThreshold.Name = "txtLenBasedKThreshold";
            // 
            // label13
            // 
            resources.ApplyResources(this.label13, "label13");
            this.label13.Name = "label13";
            // 
            // tabPage3
            // 
            this.tabPage3.Controls.Add(this.cmdAddEffect);
            this.tabPage3.Controls.Add(this.treeKaraoke);
            this.tabPage3.Controls.Add(this.cmdNewLayer);
            this.tabPage3.Controls.Add(this.cmdKaraDel);
            this.tabPage3.Controls.Add(this.cmdDoEffects);
            this.tabPage3.Controls.Add(this.cmdKaraDup);
            this.tabPage3.Controls.Add(this.panelEffect);
            this.tabPage3.Controls.Add(this.panelLayer);
            resources.ApplyResources(this.tabPage3, "tabPage3");
            this.tabPage3.Name = "tabPage3";
            this.tabPage3.UseVisualStyleBackColor = true;
            // 
            // cmdAddEffect
            // 
            resources.ApplyResources(this.cmdAddEffect, "cmdAddEffect");
            this.cmdAddEffect.Name = "cmdAddEffect";
            this.cmdAddEffect.Click += new System.EventHandler(this.cmdAddEffect_Click);
            // 
            // treeKaraoke
            // 
            this.treeKaraoke.AllowDrop = true;
            this.treeKaraoke.CheckBoxes = true;
            this.treeKaraoke.HideSelection = false;
            resources.ApplyResources(this.treeKaraoke, "treeKaraoke");
            this.treeKaraoke.ItemHeight = 18;
            this.treeKaraoke.LabelEdit = true;
            this.treeKaraoke.Name = "treeKaraoke";
            this.treeKaraoke.ShowPlusMinus = false;
            this.treeKaraoke.ShowRootLines = false;
            this.treeKaraoke.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.treeKaraoke_AfterCheck);
            this.treeKaraoke.DragDrop += new System.Windows.Forms.DragEventHandler(this.treeKaraoke_DragDrop);
            this.treeKaraoke.AfterLabelEdit += new System.Windows.Forms.NodeLabelEditEventHandler(this.treeKaraoke_AfterLabelEdit);
            this.treeKaraoke.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeKaraoke_AfterSelect);
            this.treeKaraoke.DragEnter += new System.Windows.Forms.DragEventHandler(this.treeKaraoke_DragEnter);
            this.treeKaraoke.ItemDrag += new System.Windows.Forms.ItemDragEventHandler(this.treeKaraoke_ItemDrag);
            // 
            // cmdNewLayer
            // 
            resources.ApplyResources(this.cmdNewLayer, "cmdNewLayer");
            this.cmdNewLayer.Name = "cmdNewLayer";
            this.cmdNewLayer.Click += new System.EventHandler(this.cmdNewLayer_Click);
            // 
            // cmdKaraDel
            // 
            resources.ApplyResources(this.cmdKaraDel, "cmdKaraDel");
            this.cmdKaraDel.Name = "cmdKaraDel";
            this.cmdKaraDel.Click += new System.EventHandler(this.cmdKaraDel_Click);
            // 
            // cmdDoEffects
            // 
            resources.ApplyResources(this.cmdDoEffects, "cmdDoEffects");
            this.cmdDoEffects.Name = "cmdDoEffects";
            this.cmdDoEffects.Click += new System.EventHandler(this.cmdDoEffects_Click);
            // 
            // cmdKaraDup
            // 
            resources.ApplyResources(this.cmdKaraDup, "cmdKaraDup");
            this.cmdKaraDup.Name = "cmdKaraDup";
            this.cmdKaraDup.Click += new System.EventHandler(this.cmdKaraDup_Click);
            // 
            // panelEffect
            // 
            this.panelEffect.Controls.Add(this.tabControl1);
            resources.ApplyResources(this.panelEffect, "panelEffect");
            this.panelEffect.Name = "panelEffect";
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabPage7);
            this.tabControl1.Controls.Add(this.tabPage8);
            resources.ApplyResources(this.tabControl1, "tabControl1");
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            // 
            // tabPage7
            // 
            this.tabPage7.Controls.Add(this.lstEffectOptions);
            this.tabPage7.Controls.Add(this.label2);
            resources.ApplyResources(this.tabPage7, "tabPage7");
            this.tabPage7.Name = "tabPage7";
            // 
            // lstEffectOptions
            // 
            resources.ApplyResources(this.lstEffectOptions, "lstEffectOptions");
            this.lstEffectOptions.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.lstoOptionName,
            this.lstoOptionValue});
            this.lstEffectOptions.FullRowSelect = true;
            this.lstEffectOptions.GridLines = true;
            this.lstEffectOptions.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.lstEffectOptions.MultiSelect = false;
            this.lstEffectOptions.Name = "lstEffectOptions";
            this.lstEffectOptions.UseCompatibleStateImageBehavior = false;
            this.lstEffectOptions.View = System.Windows.Forms.View.Details;
            this.lstEffectOptions.Click += new System.EventHandler(this.lstEffectOptions_Click);
            // 
            // lstoOptionName
            // 
            resources.ApplyResources(this.lstoOptionName, "lstoOptionName");
            // 
            // lstoOptionValue
            // 
            resources.ApplyResources(this.lstoOptionValue, "lstoOptionValue");
            // 
            // label2
            // 
            resources.ApplyResources(this.label2, "label2");
            this.label2.Name = "label2";
            // 
            // tabPage8
            // 
            this.tabPage8.Controls.Add(this.lstEffectConditions);
            this.tabPage8.Controls.Add(this.cmdDelEffectCondition);
            this.tabPage8.Controls.Add(this.cmdAddEffectCondition);
            resources.ApplyResources(this.tabPage8, "tabPage8");
            this.tabPage8.Name = "tabPage8";
            // 
            // lstEffectConditions
            // 
            this.lstEffectConditions.AutoArrange = false;
            this.lstEffectConditions.CheckBoxes = true;
            this.lstEffectConditions.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3});
            this.lstEffectConditions.FullRowSelect = true;
            this.lstEffectConditions.GridLines = true;
            this.lstEffectConditions.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.lstEffectConditions.HideSelection = false;
            this.lstEffectConditions.LabelEdit = true;
            resources.ApplyResources(this.lstEffectConditions, "lstEffectConditions");
            this.lstEffectConditions.MultiSelect = false;
            this.lstEffectConditions.Name = "lstEffectConditions";
            this.lstEffectConditions.UseCompatibleStateImageBehavior = false;
            this.lstEffectConditions.View = System.Windows.Forms.View.Details;
            this.lstEffectConditions.SubItemClicked += new SSATool.ListViewES.LabelSubEditEventHandler(this.lstEffectConditions_SubItemClicked);
            this.lstEffectConditions.SubItemEndEditing += new SSATool.ListViewES.LabelSubEditEndEventHandler(this.lstEffectConditions_SubItemEndEditing);
            // 
            // columnHeader1
            // 
            resources.ApplyResources(this.columnHeader1, "columnHeader1");
            // 
            // columnHeader2
            // 
            resources.ApplyResources(this.columnHeader2, "columnHeader2");
            // 
            // columnHeader3
            // 
            resources.ApplyResources(this.columnHeader3, "columnHeader3");
            // 
            // cmdDelEffectCondition
            // 
            resources.ApplyResources(this.cmdDelEffectCondition, "cmdDelEffectCondition");
            this.cmdDelEffectCondition.Name = "cmdDelEffectCondition";
            this.cmdDelEffectCondition.Click += new System.EventHandler(this.cmdDelEffectCondition_Click);
            // 
            // cmdAddEffectCondition
            // 
            resources.ApplyResources(this.cmdAddEffectCondition, "cmdAddEffectCondition");
            this.cmdAddEffectCondition.Name = "cmdAddEffectCondition";
            this.cmdAddEffectCondition.Click += new System.EventHandler(this.cmdAddEffectCondition_Click);
            // 
            // panelLayer
            // 
            this.panelLayer.Controls.Add(this.chkSyllablePerLine);
            this.panelLayer.Controls.Add(this.groupBox16);
            this.panelLayer.Controls.Add(this.textLayerRepetitions);
            this.panelLayer.Controls.Add(this.label49);
            this.panelLayer.Controls.Add(this.chkAddOnce);
            this.panelLayer.Controls.Add(this.chkAddAnyway);
            this.panelLayer.Controls.Add(this.chkLayerPerSyllable);
            this.panelLayer.Controls.Add(this.cmdDelLayerCondition);
            this.panelLayer.Controls.Add(this.cmdNewLayerCondition);
            this.panelLayer.Controls.Add(this.lstLayerConditions);
            resources.ApplyResources(this.panelLayer, "panelLayer");
            this.panelLayer.Name = "panelLayer";
            // 
            // chkSyllablePerLine
            // 
            resources.ApplyResources(this.chkSyllablePerLine, "chkSyllablePerLine");
            this.chkSyllablePerLine.Name = "chkSyllablePerLine";
            this.chkSyllablePerLine.UseVisualStyleBackColor = true;
            this.chkSyllablePerLine.CheckedChanged += new System.EventHandler(this.chkSyllablePerLine_CheckedChanged);
            // 
            // groupBox16
            // 
            this.groupBox16.Controls.Add(this.chkKaraAddText);
            this.groupBox16.Controls.Add(this.chkKaraAddClosingBracket);
            this.groupBox16.Controls.Add(this.chkKaraAddASSA);
            this.groupBox16.Controls.Add(this.chkKaraAddK);
            resources.ApplyResources(this.groupBox16, "groupBox16");
            this.groupBox16.Name = "groupBox16";
            this.groupBox16.TabStop = false;
            // 
            // chkKaraAddText
            // 
            resources.ApplyResources(this.chkKaraAddText, "chkKaraAddText");
            this.chkKaraAddText.Checked = true;
            this.chkKaraAddText.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkKaraAddText.Name = "chkKaraAddText";
            this.chkKaraAddText.UseVisualStyleBackColor = true;
            this.chkKaraAddText.CheckedChanged += new System.EventHandler(this.chkKaraAddText_CheckedChanged);
            // 
            // chkKaraAddClosingBracket
            // 
            resources.ApplyResources(this.chkKaraAddClosingBracket, "chkKaraAddClosingBracket");
            this.chkKaraAddClosingBracket.Checked = true;
            this.chkKaraAddClosingBracket.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkKaraAddClosingBracket.Name = "chkKaraAddClosingBracket";
            this.chkKaraAddClosingBracket.UseVisualStyleBackColor = true;
            this.chkKaraAddClosingBracket.CheckedChanged += new System.EventHandler(this.chkKaraAddClosingBracket_CheckedChanged);
            // 
            // chkKaraAddASSA
            // 
            resources.ApplyResources(this.chkKaraAddASSA, "chkKaraAddASSA");
            this.chkKaraAddASSA.Checked = true;
            this.chkKaraAddASSA.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkKaraAddASSA.Name = "chkKaraAddASSA";
            this.chkKaraAddASSA.UseVisualStyleBackColor = true;
            this.chkKaraAddASSA.CheckedChanged += new System.EventHandler(this.chkKaraAddASSA_CheckedChanged);
            // 
            // chkKaraAddK
            // 
            resources.ApplyResources(this.chkKaraAddK, "chkKaraAddK");
            this.chkKaraAddK.Checked = true;
            this.chkKaraAddK.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chkKaraAddK.Name = "chkKaraAddK";
            this.chkKaraAddK.UseVisualStyleBackColor = true;
            this.chkKaraAddK.CheckedChanged += new System.EventHandler(this.chkKaraAddK_CheckedChanged);
            // 
            // textLayerRepetitions
            // 
            resources.ApplyResources(this.textLayerRepetitions, "textLayerRepetitions");
            this.textLayerRepetitions.Name = "textLayerRepetitions";
            this.textLayerRepetitions.Leave += new System.EventHandler(this.textLayerRepetitions_Leave);
            this.textLayerRepetitions.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textLayerRepetitions_KeyPress);
            // 
            // label49
            // 
            resources.ApplyResources(this.label49, "label49");
            this.label49.Name = "label49";
            this.label49.UseCompatibleTextRendering = true;
            // 
            // chkAddOnce
            // 
            resources.ApplyResources(this.chkAddOnce, "chkAddOnce");
            this.chkAddOnce.Name = "chkAddOnce";
            this.chkAddOnce.UseCompatibleTextRendering = true;
            this.chkAddOnce.CheckedChanged += new System.EventHandler(this.chkAddOnce_CheckedChanged);
            // 
            // chkAddAnyway
            // 
            resources.ApplyResources(this.chkAddAnyway, "chkAddAnyway");
            this.chkAddAnyway.Name = "chkAddAnyway";
            this.chkAddAnyway.UseCompatibleTextRendering = true;
            this.chkAddAnyway.CheckedChanged += new System.EventHandler(this.chkAddAnyway_CheckedChanged);
            // 
            // chkLayerPerSyllable
            // 
            resources.ApplyResources(this.chkLayerPerSyllable, "chkLayerPerSyllable");
            this.chkLayerPerSyllable.Name = "chkLayerPerSyllable";
            this.chkLayerPerSyllable.UseCompatibleTextRendering = true;
            this.chkLayerPerSyllable.CheckedChanged += new System.EventHandler(this.chkLayerPerSyllable_CheckedChanged);
            // 
            // cmdDelLayerCondition
            // 
            resources.ApplyResources(this.cmdDelLayerCondition, "cmdDelLayerCondition");
            this.cmdDelLayerCondition.Name = "cmdDelLayerCondition";
            this.cmdDelLayerCondition.Click += new System.EventHandler(this.cmdDelLayerCondition_Click);
            // 
            // cmdNewLayerCondition
            // 
            resources.ApplyResources(this.cmdNewLayerCondition, "cmdNewLayerCondition");
            this.cmdNewLayerCondition.Name = "cmdNewLayerCondition";
            this.cmdNewLayerCondition.Click += new System.EventHandler(this.cmdNewLayerCondition_Click);
            // 
            // lstLayerConditions
            // 
            this.lstLayerConditions.AutoArrange = false;
            this.lstLayerConditions.CheckBoxes = true;
            this.lstLayerConditions.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader4,
            this.columnHeader5,
            this.columnHeader6});
            resources.ApplyResources(this.lstLayerConditions, "lstLayerConditions");
            this.lstLayerConditions.FullRowSelect = true;
            this.lstLayerConditions.GridLines = true;
            this.lstLayerConditions.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.lstLayerConditions.HideSelection = false;
            this.lstLayerConditions.LabelEdit = true;
            this.lstLayerConditions.MultiSelect = false;
            this.lstLayerConditions.Name = "lstLayerConditions";
            this.lstLayerConditions.UseCompatibleStateImageBehavior = false;
            this.lstLayerConditions.View = System.Windows.Forms.View.Details;
            this.lstLayerConditions.SubItemClicked += new SSATool.ListViewES.LabelSubEditEventHandler(this.lstLayerConditions_SubItemClicked);
            this.lstLayerConditions.ItemCheck += new System.Windows.Forms.ItemCheckEventHandler(this.lstLayerConditions_ItemCheck);
            this.lstLayerConditions.SubItemEndEditing += new SSATool.ListViewES.LabelSubEditEndEventHandler(this.lstLayerConditions_SubItemEndEditing);
            // 
            // columnHeader4
            // 
            resources.ApplyResources(this.columnHeader4, "columnHeader4");
            // 
            // columnHeader5
            // 
            resources.ApplyResources(this.columnHeader5, "columnHeader5");
            // 
            // columnHeader6
            // 
            resources.ApplyResources(this.columnHeader6, "columnHeader6");
            // 
            // tabPage4
            // 
            this.tabPage4.Controls.Add(this.label17);
            this.tabPage4.Controls.Add(this.panelGradient);
            this.tabPage4.Controls.Add(this.label16);
            this.tabPage4.Controls.Add(this.label18);
            this.tabPage4.Controls.Add(this.label15);
            this.tabPage4.Controls.Add(this.label14);
            this.tabPage4.Controls.Add(this.cmdGradMoveBefore);
            this.tabPage4.Controls.Add(this.txtGradOut);
            this.tabPage4.Controls.Add(this.txtGradIn);
            this.tabPage4.Controls.Add(this.txtGradBefore);
            resources.ApplyResources(this.tabPage4, "tabPage4");
            this.tabPage4.Name = "tabPage4";
            this.tabPage4.UseVisualStyleBackColor = true;
            // 
            // label17
            // 
            resources.ApplyResources(this.label17, "label17");
            this.label17.Name = "label17";
            // 
            // panelGradient
            // 
            this.panelGradient.Controls.Add(this.txtGradCLines);
            this.panelGradient.Controls.Add(this.cmdGradDoGrad);
            this.panelGradient.Controls.Add(this.groupBox7);
            this.panelGradient.Controls.Add(this.cmdGradReplaceLine);
            this.panelGradient.Controls.Add(this.txtGradMirror);
            this.panelGradient.Controls.Add(this.cmdGradInsertAfter);
            this.panelGradient.Controls.Add(this.label33);
            this.panelGradient.Controls.Add(this.cmdGradInsertBefore);
            this.panelGradient.Controls.Add(this.label37);
            this.panelGradient.Controls.Add(this.label19);
            this.panelGradient.Controls.Add(this.txtGradBase);
            this.panelGradient.Controls.Add(this.label20);
            this.panelGradient.Controls.Add(this.chkGrad4c);
            this.panelGradient.Controls.Add(this.label21);
            this.panelGradient.Controls.Add(this.chkGrad3c);
            this.panelGradient.Controls.Add(this.label22);
            this.panelGradient.Controls.Add(this.chkGrad2c);
            this.panelGradient.Controls.Add(this.label23);
            this.panelGradient.Controls.Add(this.chkGrad1c);
            this.panelGradient.Controls.Add(this.label24);
            this.panelGradient.Controls.Add(this.txtGradStartPos);
            this.panelGradient.Controls.Add(this.cmdGradListToInput);
            this.panelGradient.Controls.Add(this.txtGradEndPos);
            this.panelGradient.Controls.Add(this.txtGradEndBGR);
            this.panelGradient.Controls.Add(this.txtGradOffset);
            this.panelGradient.Controls.Add(this.txtGradStartBGR);
            resources.ApplyResources(this.panelGradient, "panelGradient");
            this.panelGradient.Name = "panelGradient";
            // 
            // txtGradCLines
            // 
            resources.ApplyResources(this.txtGradCLines, "txtGradCLines");
            this.txtGradCLines.Name = "txtGradCLines";
            // 
            // cmdGradDoGrad
            // 
            resources.ApplyResources(this.cmdGradDoGrad, "cmdGradDoGrad");
            this.cmdGradDoGrad.Name = "cmdGradDoGrad";
            this.cmdGradDoGrad.Click += new System.EventHandler(this.cmdGradDoGrad_Click);
            // 
            // groupBox7
            // 
            this.groupBox7.Controls.Add(this.radGradExp);
            this.groupBox7.Controls.Add(this.radGradLog);
            this.groupBox7.Controls.Add(this.radGradPoly);
            resources.ApplyResources(this.groupBox7, "groupBox7");
            this.groupBox7.Name = "groupBox7";
            this.groupBox7.TabStop = false;
            // 
            // radGradExp
            // 
            resources.ApplyResources(this.radGradExp, "radGradExp");
            this.radGradExp.Name = "radGradExp";
            this.radGradExp.UseCompatibleTextRendering = true;
            // 
            // radGradLog
            // 
            resources.ApplyResources(this.radGradLog, "radGradLog");
            this.radGradLog.Name = "radGradLog";
            this.radGradLog.UseCompatibleTextRendering = true;
            // 
            // radGradPoly
            // 
            this.radGradPoly.Checked = true;
            resources.ApplyResources(this.radGradPoly, "radGradPoly");
            this.radGradPoly.Name = "radGradPoly";
            this.radGradPoly.TabStop = true;
            this.radGradPoly.UseCompatibleTextRendering = true;
            // 
            // cmdGradReplaceLine
            // 
            resources.ApplyResources(this.cmdGradReplaceLine, "cmdGradReplaceLine");
            this.cmdGradReplaceLine.Name = "cmdGradReplaceLine";
            this.cmdGradReplaceLine.UseCompatibleTextRendering = true;
            this.cmdGradReplaceLine.Click += new System.EventHandler(this.cmdGradReplaceLine_Click);
            // 
            // txtGradMirror
            // 
            resources.ApplyResources(this.txtGradMirror, "txtGradMirror");
            this.txtGradMirror.Name = "txtGradMirror";
            // 
            // cmdGradInsertAfter
            // 
            resources.ApplyResources(this.cmdGradInsertAfter, "cmdGradInsertAfter");
            this.cmdGradInsertAfter.Name = "cmdGradInsertAfter";
            this.cmdGradInsertAfter.UseCompatibleTextRendering = true;
            this.cmdGradInsertAfter.Click += new System.EventHandler(this.cmdGradInsertAfter_Click);
            // 
            // label33
            // 
            resources.ApplyResources(this.label33, "label33");
            this.label33.Name = "label33";
            // 
            // cmdGradInsertBefore
            // 
            resources.ApplyResources(this.cmdGradInsertBefore, "cmdGradInsertBefore");
            this.cmdGradInsertBefore.Name = "cmdGradInsertBefore";
            this.cmdGradInsertBefore.UseCompatibleTextRendering = true;
            this.cmdGradInsertBefore.Click += new System.EventHandler(this.cmdGradInsertBefore_Click);
            // 
            // label37
            // 
            resources.ApplyResources(this.label37, "label37");
            this.label37.Name = "label37";
            // 
            // label19
            // 
            resources.ApplyResources(this.label19, "label19");
            this.label19.Name = "label19";
            // 
            // txtGradBase
            // 
            resources.ApplyResources(this.txtGradBase, "txtGradBase");
            this.txtGradBase.Name = "txtGradBase";
            // 
            // label20
            // 
            resources.ApplyResources(this.label20, "label20");
            this.label20.Name = "label20";
            // 
            // chkGrad4c
            // 
            resources.ApplyResources(this.chkGrad4c, "chkGrad4c");
            this.chkGrad4c.Name = "chkGrad4c";
            // 
            // label21
            // 
            resources.ApplyResources(this.label21, "label21");
            this.label21.Name = "label21";
            // 
            // chkGrad3c
            // 
            resources.ApplyResources(this.chkGrad3c, "chkGrad3c");
            this.chkGrad3c.Name = "chkGrad3c";
            // 
            // label22
            // 
            resources.ApplyResources(this.label22, "label22");
            this.label22.Name = "label22";
            // 
            // chkGrad2c
            // 
            resources.ApplyResources(this.chkGrad2c, "chkGrad2c");
            this.chkGrad2c.Name = "chkGrad2c";
            // 
            // label23
            // 
            resources.ApplyResources(this.label23, "label23");
            this.label23.Name = "label23";
            // 
            // chkGrad1c
            // 
            this.chkGrad1c.Checked = true;
            this.chkGrad1c.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkGrad1c, "chkGrad1c");
            this.chkGrad1c.Name = "chkGrad1c";
            // 
            // label24
            // 
            resources.ApplyResources(this.label24, "label24");
            this.label24.Name = "label24";
            // 
            // txtGradStartPos
            // 
            resources.ApplyResources(this.txtGradStartPos, "txtGradStartPos");
            this.txtGradStartPos.Name = "txtGradStartPos";
            // 
            // cmdGradListToInput
            // 
            resources.ApplyResources(this.cmdGradListToInput, "cmdGradListToInput");
            this.cmdGradListToInput.Name = "cmdGradListToInput";
            this.cmdGradListToInput.UseCompatibleTextRendering = true;
            this.cmdGradListToInput.Click += new System.EventHandler(this.cmdGradListToInput_Click);
            // 
            // txtGradEndPos
            // 
            resources.ApplyResources(this.txtGradEndPos, "txtGradEndPos");
            this.txtGradEndPos.Name = "txtGradEndPos";
            // 
            // txtGradEndBGR
            // 
            resources.ApplyResources(this.txtGradEndBGR, "txtGradEndBGR");
            this.txtGradEndBGR.Name = "txtGradEndBGR";
            // 
            // txtGradOffset
            // 
            resources.ApplyResources(this.txtGradOffset, "txtGradOffset");
            this.txtGradOffset.Name = "txtGradOffset";
            // 
            // txtGradStartBGR
            // 
            resources.ApplyResources(this.txtGradStartBGR, "txtGradStartBGR");
            this.txtGradStartBGR.Name = "txtGradStartBGR";
            // 
            // label16
            // 
            resources.ApplyResources(this.label16, "label16");
            this.label16.Name = "label16";
            // 
            // label18
            // 
            resources.ApplyResources(this.label18, "label18");
            this.label18.Name = "label18";
            // 
            // label15
            // 
            resources.ApplyResources(this.label15, "label15");
            this.label15.Name = "label15";
            // 
            // label14
            // 
            resources.ApplyResources(this.label14, "label14");
            this.label14.Name = "label14";
            // 
            // cmdGradMoveBefore
            // 
            resources.ApplyResources(this.cmdGradMoveBefore, "cmdGradMoveBefore");
            this.cmdGradMoveBefore.Name = "cmdGradMoveBefore";
            this.cmdGradMoveBefore.UseCompatibleTextRendering = true;
            this.cmdGradMoveBefore.UseMnemonic = false;
            this.cmdGradMoveBefore.Click += new System.EventHandler(this.cmdGradMoveBefore_Click);
            // 
            // txtGradOut
            // 
            this.txtGradOut.AcceptsReturn = true;
            resources.ApplyResources(this.txtGradOut, "txtGradOut");
            this.txtGradOut.Name = "txtGradOut";
            // 
            // txtGradIn
            // 
            resources.ApplyResources(this.txtGradIn, "txtGradIn");
            this.txtGradIn.Name = "txtGradIn";
            // 
            // txtGradBefore
            // 
            resources.ApplyResources(this.txtGradBefore, "txtGradBefore");
            this.txtGradBefore.Name = "txtGradBefore";
            // 
            // tabPage14
            // 
            this.tabPage14.Controls.Add(this.panelBlur);
            this.tabPage14.Controls.Add(this.cmdBlurMoveBefore);
            this.tabPage14.Controls.Add(this.label35);
            this.tabPage14.Controls.Add(this.txtBlurBefore);
            this.tabPage14.Controls.Add(this.label32);
            this.tabPage14.Controls.Add(this.label31);
            this.tabPage14.Controls.Add(this.txtBlurOut);
            this.tabPage14.Controls.Add(this.txtBlurIn);
            resources.ApplyResources(this.tabPage14, "tabPage14");
            this.tabPage14.Name = "tabPage14";
            this.tabPage14.UseVisualStyleBackColor = true;
            // 
            // panelBlur
            // 
            this.panelBlur.Controls.Add(this.groupBlurAffect);
            this.panelBlur.Controls.Add(this.label42);
            this.panelBlur.Controls.Add(this.label28);
            this.panelBlur.Controls.Add(this.txtBlurRadV);
            this.panelBlur.Controls.Add(this.txtBlurLines);
            this.panelBlur.Controls.Add(this.label29);
            this.panelBlur.Controls.Add(this.groupBox9);
            this.panelBlur.Controls.Add(this.label30);
            this.panelBlur.Controls.Add(this.groupBlurSub);
            this.panelBlur.Controls.Add(this.txtBlurStartAlpha);
            this.panelBlur.Controls.Add(this.label38);
            this.panelBlur.Controls.Add(this.txtBlurEndAlpha);
            this.panelBlur.Controls.Add(this.txtBlurBase);
            this.panelBlur.Controls.Add(this.cmdDoBlur);
            this.panelBlur.Controls.Add(this.txtBlurPos);
            this.panelBlur.Controls.Add(this.groupBox8);
            this.panelBlur.Controls.Add(this.label36);
            this.panelBlur.Controls.Add(this.label34);
            this.panelBlur.Controls.Add(this.cmdBlurListToInput);
            this.panelBlur.Controls.Add(this.txtBlurRadH);
            this.panelBlur.Controls.Add(this.cmdBlurInsertBefore);
            this.panelBlur.Controls.Add(this.cmdBlurReplaceLine);
            this.panelBlur.Controls.Add(this.cmdBlurInsertAfter);
            resources.ApplyResources(this.panelBlur, "panelBlur");
            this.panelBlur.Name = "panelBlur";
            // 
            // groupBlurAffect
            // 
            this.groupBlurAffect.Controls.Add(this.chkBlur4a);
            this.groupBlurAffect.Controls.Add(this.chkBlur3a);
            this.groupBlurAffect.Controls.Add(this.chkBlur2a);
            this.groupBlurAffect.Controls.Add(this.chkBlur1a);
            resources.ApplyResources(this.groupBlurAffect, "groupBlurAffect");
            this.groupBlurAffect.Name = "groupBlurAffect";
            this.groupBlurAffect.TabStop = false;
            // 
            // chkBlur4a
            // 
            resources.ApplyResources(this.chkBlur4a, "chkBlur4a");
            this.chkBlur4a.Name = "chkBlur4a";
            this.chkBlur4a.UseCompatibleTextRendering = true;
            // 
            // chkBlur3a
            // 
            this.chkBlur3a.Checked = true;
            this.chkBlur3a.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkBlur3a, "chkBlur3a");
            this.chkBlur3a.Name = "chkBlur3a";
            this.chkBlur3a.UseCompatibleTextRendering = true;
            // 
            // chkBlur2a
            // 
            resources.ApplyResources(this.chkBlur2a, "chkBlur2a");
            this.chkBlur2a.Name = "chkBlur2a";
            this.chkBlur2a.UseCompatibleTextRendering = true;
            // 
            // chkBlur1a
            // 
            resources.ApplyResources(this.chkBlur1a, "chkBlur1a");
            this.chkBlur1a.Name = "chkBlur1a";
            this.chkBlur1a.UseCompatibleTextRendering = true;
            // 
            // label42
            // 
            resources.ApplyResources(this.label42, "label42");
            this.label42.Name = "label42";
            // 
            // label28
            // 
            resources.ApplyResources(this.label28, "label28");
            this.label28.Name = "label28";
            // 
            // txtBlurRadV
            // 
            resources.ApplyResources(this.txtBlurRadV, "txtBlurRadV");
            this.txtBlurRadV.Name = "txtBlurRadV";
            // 
            // txtBlurLines
            // 
            resources.ApplyResources(this.txtBlurLines, "txtBlurLines");
            this.txtBlurLines.Name = "txtBlurLines";
            // 
            // label29
            // 
            resources.ApplyResources(this.label29, "label29");
            this.label29.Name = "label29";
            // 
            // groupBox9
            // 
            this.groupBox9.Controls.Add(this.radBlurExp);
            this.groupBox9.Controls.Add(this.radBlurLog);
            this.groupBox9.Controls.Add(this.radBlurPoly);
            resources.ApplyResources(this.groupBox9, "groupBox9");
            this.groupBox9.Name = "groupBox9";
            this.groupBox9.TabStop = false;
            // 
            // radBlurExp
            // 
            resources.ApplyResources(this.radBlurExp, "radBlurExp");
            this.radBlurExp.Name = "radBlurExp";
            this.radBlurExp.UseCompatibleTextRendering = true;
            // 
            // radBlurLog
            // 
            resources.ApplyResources(this.radBlurLog, "radBlurLog");
            this.radBlurLog.Name = "radBlurLog";
            this.radBlurLog.UseCompatibleTextRendering = true;
            // 
            // radBlurPoly
            // 
            this.radBlurPoly.Checked = true;
            resources.ApplyResources(this.radBlurPoly, "radBlurPoly");
            this.radBlurPoly.Name = "radBlurPoly";
            this.radBlurPoly.TabStop = true;
            this.radBlurPoly.UseCompatibleTextRendering = true;
            // 
            // label30
            // 
            resources.ApplyResources(this.label30, "label30");
            this.label30.Name = "label30";
            // 
            // groupBlurSub
            // 
            this.groupBlurSub.Controls.Add(this.chkBlur2aSub);
            this.groupBlurSub.Controls.Add(this.chkBlur4aSub);
            this.groupBlurSub.Controls.Add(this.chkBlur3aSub);
            this.groupBlurSub.Controls.Add(this.chkBlur1aSub);
            resources.ApplyResources(this.groupBlurSub, "groupBlurSub");
            this.groupBlurSub.Name = "groupBlurSub";
            this.groupBlurSub.TabStop = false;
            // 
            // chkBlur2aSub
            // 
            this.chkBlur2aSub.Checked = true;
            this.chkBlur2aSub.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkBlur2aSub, "chkBlur2aSub");
            this.chkBlur2aSub.Name = "chkBlur2aSub";
            this.chkBlur2aSub.UseCompatibleTextRendering = true;
            // 
            // chkBlur4aSub
            // 
            this.chkBlur4aSub.Checked = true;
            this.chkBlur4aSub.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkBlur4aSub, "chkBlur4aSub");
            this.chkBlur4aSub.Name = "chkBlur4aSub";
            this.chkBlur4aSub.UseCompatibleTextRendering = true;
            // 
            // chkBlur3aSub
            // 
            resources.ApplyResources(this.chkBlur3aSub, "chkBlur3aSub");
            this.chkBlur3aSub.Name = "chkBlur3aSub";
            this.chkBlur3aSub.UseCompatibleTextRendering = true;
            // 
            // chkBlur1aSub
            // 
            this.chkBlur1aSub.Checked = true;
            this.chkBlur1aSub.CheckState = System.Windows.Forms.CheckState.Checked;
            resources.ApplyResources(this.chkBlur1aSub, "chkBlur1aSub");
            this.chkBlur1aSub.Name = "chkBlur1aSub";
            this.chkBlur1aSub.UseCompatibleTextRendering = true;
            // 
            // txtBlurStartAlpha
            // 
            resources.ApplyResources(this.txtBlurStartAlpha, "txtBlurStartAlpha");
            this.txtBlurStartAlpha.Name = "txtBlurStartAlpha";
            // 
            // label38
            // 
            resources.ApplyResources(this.label38, "label38");
            this.label38.Name = "label38";
            // 
            // txtBlurEndAlpha
            // 
            resources.ApplyResources(this.txtBlurEndAlpha, "txtBlurEndAlpha");
            this.txtBlurEndAlpha.Name = "txtBlurEndAlpha";
            // 
            // txtBlurBase
            // 
            resources.ApplyResources(this.txtBlurBase, "txtBlurBase");
            this.txtBlurBase.Name = "txtBlurBase";
            // 
            // cmdDoBlur
            // 
            resources.ApplyResources(this.cmdDoBlur, "cmdDoBlur");
            this.cmdDoBlur.Name = "cmdDoBlur";
            this.cmdDoBlur.Click += new System.EventHandler(this.cmdDoBlur_Click);
            // 
            // txtBlurPos
            // 
            resources.ApplyResources(this.txtBlurPos, "txtBlurPos");
            this.txtBlurPos.Name = "txtBlurPos";
            // 
            // groupBox8
            // 
            this.groupBox8.Controls.Add(this.radBlurGlow);
            this.groupBox8.Controls.Add(this.radBlurHV);
            resources.ApplyResources(this.groupBox8, "groupBox8");
            this.groupBox8.Name = "groupBox8";
            this.groupBox8.TabStop = false;
            // 
            // radBlurGlow
            // 
            resources.ApplyResources(this.radBlurGlow, "radBlurGlow");
            this.radBlurGlow.Name = "radBlurGlow";
            this.radBlurGlow.UseCompatibleTextRendering = true;
            this.radBlurGlow.CheckedChanged += new System.EventHandler(this.radBlurGlow_CheckedChanged);
            // 
            // radBlurHV
            // 
            this.radBlurHV.Checked = true;
            resources.ApplyResources(this.radBlurHV, "radBlurHV");
            this.radBlurHV.Name = "radBlurHV";
            this.radBlurHV.TabStop = true;
            this.radBlurHV.UseCompatibleTextRendering = true;
            this.radBlurHV.CheckedChanged += new System.EventHandler(this.radBlurHV_CheckedChanged);
            // 
            // label36
            // 
            resources.ApplyResources(this.label36, "label36");
            this.label36.Name = "label36";
            // 
            // label34
            // 
            resources.ApplyResources(this.label34, "label34");
            this.label34.Name = "label34";
            // 
            // cmdBlurListToInput
            // 
            resources.ApplyResources(this.cmdBlurListToInput, "cmdBlurListToInput");
            this.cmdBlurListToInput.Name = "cmdBlurListToInput";
            this.cmdBlurListToInput.Click += new System.EventHandler(this.cmdBlurListToInput_Click);
            // 
            // txtBlurRadH
            // 
            resources.ApplyResources(this.txtBlurRadH, "txtBlurRadH");
            this.txtBlurRadH.Name = "txtBlurRadH";
            // 
            // cmdBlurInsertBefore
            // 
            resources.ApplyResources(this.cmdBlurInsertBefore, "cmdBlurInsertBefore");
            this.cmdBlurInsertBefore.Name = "cmdBlurInsertBefore";
            this.cmdBlurInsertBefore.Click += new System.EventHandler(this.cmdBlurInsertBefore_Click);
            // 
            // cmdBlurReplaceLine
            // 
            resources.ApplyResources(this.cmdBlurReplaceLine, "cmdBlurReplaceLine");
            this.cmdBlurReplaceLine.Name = "cmdBlurReplaceLine";
            this.cmdBlurReplaceLine.Click += new System.EventHandler(this.cmdBlurReplaceLine_Click);
            // 
            // cmdBlurInsertAfter
            // 
            resources.ApplyResources(this.cmdBlurInsertAfter, "cmdBlurInsertAfter");
            this.cmdBlurInsertAfter.Name = "cmdBlurInsertAfter";
            this.cmdBlurInsertAfter.Click += new System.EventHandler(this.cmdBlurInsertAfter_Click);
            // 
            // cmdBlurMoveBefore
            // 
            resources.ApplyResources(this.cmdBlurMoveBefore, "cmdBlurMoveBefore");
            this.cmdBlurMoveBefore.Name = "cmdBlurMoveBefore";
            this.cmdBlurMoveBefore.UseCompatibleTextRendering = true;
            this.cmdBlurMoveBefore.Click += new System.EventHandler(this.cmdBlurMoveBefore_Click);
            // 
            // label35
            // 
            resources.ApplyResources(this.label35, "label35");
            this.label35.Name = "label35";
            // 
            // txtBlurBefore
            // 
            resources.ApplyResources(this.txtBlurBefore, "txtBlurBefore");
            this.txtBlurBefore.Name = "txtBlurBefore";
            // 
            // label32
            // 
            resources.ApplyResources(this.label32, "label32");
            this.label32.Name = "label32";
            // 
            // label31
            // 
            resources.ApplyResources(this.label31, "label31");
            this.label31.Name = "label31";
            // 
            // txtBlurOut
            // 
            resources.ApplyResources(this.txtBlurOut, "txtBlurOut");
            this.txtBlurOut.Name = "txtBlurOut";
            // 
            // txtBlurIn
            // 
            resources.ApplyResources(this.txtBlurIn, "txtBlurIn");
            this.txtBlurIn.Name = "txtBlurIn";
            // 
            // tabPage16
            // 
            this.tabPage16.Controls.Add(this.maskedTextTransformTime);
            this.tabPage16.Controls.Add(this.cmdTransformListToInput);
            this.tabPage16.Controls.Add(this.cmdTransformInsertBefore);
            this.tabPage16.Controls.Add(this.cmdTransformInsertAfter);
            this.tabPage16.Controls.Add(this.cmdTransformReplaceLine);
            this.tabPage16.Controls.Add(this.label45);
            this.tabPage16.Controls.Add(this.txtTransformPrecision);
            this.tabPage16.Controls.Add(this.label1);
            this.tabPage16.Controls.Add(this.listTransformVars);
            this.tabPage16.Controls.Add(this.txtTransformCode);
            this.tabPage16.Controls.Add(this.label47);
            this.tabPage16.Controls.Add(this.label46);
            this.tabPage16.Controls.Add(this.cmdTransformDelVar);
            this.tabPage16.Controls.Add(this.cmdTransformNewVar);
            this.tabPage16.Controls.Add(this.cmdDoTransform);
            this.tabPage16.Controls.Add(this.label44);
            this.tabPage16.Controls.Add(this.cmdTransformDelTime);
            this.tabPage16.Controls.Add(this.cmdTransformAddTime);
            this.tabPage16.Controls.Add(this.listTransformTimes);
            this.tabPage16.Controls.Add(this.label43);
            this.tabPage16.Controls.Add(this.txtTransformOut);
            resources.ApplyResources(this.tabPage16, "tabPage16");
            this.tabPage16.Name = "tabPage16";
            this.tabPage16.UseVisualStyleBackColor = true;
            // 
            // maskedTextTransformTime
            // 
            resources.ApplyResources(this.maskedTextTransformTime, "maskedTextTransformTime");
            this.maskedTextTransformTime.Name = "maskedTextTransformTime";
            this.maskedTextTransformTime.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // cmdTransformListToInput
            // 
            resources.ApplyResources(this.cmdTransformListToInput, "cmdTransformListToInput");
            this.cmdTransformListToInput.Name = "cmdTransformListToInput";
            this.cmdTransformListToInput.Click += new System.EventHandler(this.cmdTransformListToInput_Click);
            // 
            // cmdTransformInsertBefore
            // 
            resources.ApplyResources(this.cmdTransformInsertBefore, "cmdTransformInsertBefore");
            this.cmdTransformInsertBefore.Name = "cmdTransformInsertBefore";
            this.cmdTransformInsertBefore.Click += new System.EventHandler(this.cmdTransformInsertBefore_Click);
            // 
            // cmdTransformInsertAfter
            // 
            resources.ApplyResources(this.cmdTransformInsertAfter, "cmdTransformInsertAfter");
            this.cmdTransformInsertAfter.Name = "cmdTransformInsertAfter";
            this.cmdTransformInsertAfter.Click += new System.EventHandler(this.cmdTransformInsertAfter_Click);
            // 
            // cmdTransformReplaceLine
            // 
            resources.ApplyResources(this.cmdTransformReplaceLine, "cmdTransformReplaceLine");
            this.cmdTransformReplaceLine.Name = "cmdTransformReplaceLine";
            this.cmdTransformReplaceLine.Click += new System.EventHandler(this.cmdTransformReplaceLine_Click);
            // 
            // label45
            // 
            resources.ApplyResources(this.label45, "label45");
            this.label45.Name = "label45";
            // 
            // txtTransformPrecision
            // 
            resources.ApplyResources(this.txtTransformPrecision, "txtTransformPrecision");
            this.txtTransformPrecision.Name = "txtTransformPrecision";
            // 
            // label1
            // 
            resources.ApplyResources(this.label1, "label1");
            this.label1.Name = "label1";
            // 
            // listTransformVars
            // 
            this.listTransformVars.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader13});
            this.listTransformVars.GridLines = true;
            this.listTransformVars.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            this.listTransformVars.LabelEdit = true;
            resources.ApplyResources(this.listTransformVars, "listTransformVars");
            this.listTransformVars.Name = "listTransformVars";
            this.listTransformVars.UseCompatibleStateImageBehavior = false;
            this.listTransformVars.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader13
            // 
            resources.ApplyResources(this.columnHeader13, "columnHeader13");
            // 
            // txtTransformCode
            // 
            resources.ApplyResources(this.txtTransformCode, "txtTransformCode");
            this.txtTransformCode.Name = "txtTransformCode";
            // 
            // label47
            // 
            resources.ApplyResources(this.label47, "label47");
            this.label47.Name = "label47";
            // 
            // label46
            // 
            resources.ApplyResources(this.label46, "label46");
            this.label46.Name = "label46";
            // 
            // cmdTransformDelVar
            // 
            resources.ApplyResources(this.cmdTransformDelVar, "cmdTransformDelVar");
            this.cmdTransformDelVar.Name = "cmdTransformDelVar";
            this.cmdTransformDelVar.UseCompatibleTextRendering = true;
            this.cmdTransformDelVar.Click += new System.EventHandler(this.cmdTransformDelVar_Click);
            // 
            // cmdTransformNewVar
            // 
            resources.ApplyResources(this.cmdTransformNewVar, "cmdTransformNewVar");
            this.cmdTransformNewVar.Name = "cmdTransformNewVar";
            this.cmdTransformNewVar.UseCompatibleTextRendering = true;
            this.cmdTransformNewVar.Click += new System.EventHandler(this.cmdTransformNewVar_Click);
            // 
            // cmdDoTransform
            // 
            resources.ApplyResources(this.cmdDoTransform, "cmdDoTransform");
            this.cmdDoTransform.Name = "cmdDoTransform";
            this.cmdDoTransform.Click += new System.EventHandler(this.cmdDoTransform_Click);
            // 
            // label44
            // 
            resources.ApplyResources(this.label44, "label44");
            this.label44.Name = "label44";
            // 
            // cmdTransformDelTime
            // 
            resources.ApplyResources(this.cmdTransformDelTime, "cmdTransformDelTime");
            this.cmdTransformDelTime.Name = "cmdTransformDelTime";
            this.cmdTransformDelTime.UseCompatibleTextRendering = true;
            this.cmdTransformDelTime.Click += new System.EventHandler(this.cmdTransformDelTime_Click);
            // 
            // cmdTransformAddTime
            // 
            resources.ApplyResources(this.cmdTransformAddTime, "cmdTransformAddTime");
            this.cmdTransformAddTime.Name = "cmdTransformAddTime";
            this.cmdTransformAddTime.UseCompatibleTextRendering = true;
            this.cmdTransformAddTime.Click += new System.EventHandler(this.cmdTransformAddTime_Click);
            // 
            // listTransformTimes
            // 
            resources.ApplyResources(this.listTransformTimes, "listTransformTimes");
            this.listTransformTimes.Name = "listTransformTimes";
            // 
            // label43
            // 
            resources.ApplyResources(this.label43, "label43");
            this.label43.Name = "label43";
            // 
            // txtTransformOut
            // 
            resources.ApplyResources(this.txtTransformOut, "txtTransformOut");
            this.txtTransformOut.Name = "txtTransformOut";
            // 
            // tabPage13
            // 
            this.tabPage13.Controls.Add(this.lstErrors);
            this.tabPage13.Controls.Add(this.label27);
            this.tabPage13.Controls.Add(this.cmdCheckErrors);
            resources.ApplyResources(this.tabPage13, "tabPage13");
            this.tabPage13.Name = "tabPage13";
            this.tabPage13.UseVisualStyleBackColor = true;
            // 
            // lstErrors
            // 
            resources.ApplyResources(this.lstErrors, "lstErrors");
            this.lstErrors.Name = "lstErrors";
            this.lstErrors.DoubleClick += new System.EventHandler(this.lstErrors_DoubleClick);
            // 
            // label27
            // 
            resources.ApplyResources(this.label27, "label27");
            this.label27.Name = "label27";
            // 
            // cmdCheckErrors
            // 
            resources.ApplyResources(this.cmdCheckErrors, "cmdCheckErrors");
            this.cmdCheckErrors.Name = "cmdCheckErrors";
            this.cmdCheckErrors.Click += new System.EventHandler(this.cmdCheckErrors_Click);
            // 
            // tabPage15
            // 
            this.tabPage15.Controls.Add(this.labelFontNum);
            this.tabPage15.Controls.Add(this.cmdFontSearchFolder);
            this.tabPage15.Controls.Add(this.label40);
            this.tabPage15.Controls.Add(this.cmdFindFonts);
            this.tabPage15.Controls.Add(this.label39);
            this.tabPage15.Controls.Add(this.lstFonts);
            resources.ApplyResources(this.tabPage15, "tabPage15");
            this.tabPage15.Name = "tabPage15";
            this.tabPage15.UseVisualStyleBackColor = true;
            // 
            // labelFontNum
            // 
            resources.ApplyResources(this.labelFontNum, "labelFontNum");
            this.labelFontNum.Name = "labelFontNum";
            // 
            // cmdFontSearchFolder
            // 
            resources.ApplyResources(this.cmdFontSearchFolder, "cmdFontSearchFolder");
            this.cmdFontSearchFolder.Name = "cmdFontSearchFolder";
            this.cmdFontSearchFolder.Click += new System.EventHandler(this.cmdFontSearchFolder_Click);
            // 
            // label40
            // 
            resources.ApplyResources(this.label40, "label40");
            this.label40.Name = "label40";
            // 
            // cmdFindFonts
            // 
            resources.ApplyResources(this.cmdFindFonts, "cmdFindFonts");
            this.cmdFindFonts.Name = "cmdFindFonts";
            this.cmdFindFonts.Click += new System.EventHandler(this.cmdFindFonts_Click);
            // 
            // label39
            // 
            resources.ApplyResources(this.label39, "label39");
            this.label39.Name = "label39";
            // 
            // lstFonts
            // 
            this.lstFonts.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader7,
            this.columnHeader8,
            this.columnHeader9});
            this.lstFonts.FullRowSelect = true;
            this.lstFonts.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
            resources.ApplyResources(this.lstFonts, "lstFonts");
            this.lstFonts.Name = "lstFonts";
            this.lstFonts.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.lstFonts.UseCompatibleStateImageBehavior = false;
            this.lstFonts.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader7
            // 
            resources.ApplyResources(this.columnHeader7, "columnHeader7");
            // 
            // columnHeader8
            // 
            resources.ApplyResources(this.columnHeader8, "columnHeader8");
            // 
            // columnHeader9
            // 
            resources.ApplyResources(this.columnHeader9, "columnHeader9");
            // 
            // tabPage5
            // 
            this.tabPage5.Controls.Add(this.groupBox19);
            this.tabPage5.Controls.Add(this.groupBox18);
            this.tabPage5.Controls.Add(this.groupBox17);
            this.tabPage5.Controls.Add(this.groupBox15);
            this.tabPage5.Controls.Add(this.groupBox14);
            this.tabPage5.Controls.Add(this.groupBox13);
            resources.ApplyResources(this.tabPage5, "tabPage5");
            this.tabPage5.Name = "tabPage5";
            this.tabPage5.UseVisualStyleBackColor = true;
            // 
            // groupBox19
            // 
            this.groupBox19.Controls.Add(this.textBox1);
            resources.ApplyResources(this.groupBox19, "groupBox19");
            this.groupBox19.Name = "groupBox19";
            this.groupBox19.TabStop = false;
            // 
            // textBox1
            // 
            this.textBox1.BackColor = System.Drawing.SystemColors.Control;
            this.textBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
            resources.ApplyResources(this.textBox1, "textBox1");
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            // 
            // groupBox18
            // 
            this.groupBox18.Controls.Add(this.lblRegroupDestIndex);
            this.groupBox18.Controls.Add(this.label10);
            this.groupBox18.Controls.Add(this.lblRegroupSourceIndex);
            this.groupBox18.Controls.Add(this.label5);
            resources.ApplyResources(this.groupBox18, "groupBox18");
            this.groupBox18.Name = "groupBox18";
            this.groupBox18.TabStop = false;
            // 
            // lblRegroupDestIndex
            // 
            resources.ApplyResources(this.lblRegroupDestIndex, "lblRegroupDestIndex");
            this.lblRegroupDestIndex.Name = "lblRegroupDestIndex";
            // 
            // label10
            // 
            resources.ApplyResources(this.label10, "label10");
            this.label10.Name = "label10";
            // 
            // lblRegroupSourceIndex
            // 
            resources.ApplyResources(this.lblRegroupSourceIndex, "lblRegroupSourceIndex");
            this.lblRegroupSourceIndex.Name = "lblRegroupSourceIndex";
            // 
            // label5
            // 
            resources.ApplyResources(this.label5, "label5");
            this.label5.Name = "label5";
            // 
            // groupBox17
            // 
            this.groupBox17.Controls.Add(this.listRegroupPairs);
            resources.ApplyResources(this.groupBox17, "groupBox17");
            this.groupBox17.Name = "groupBox17";
            this.groupBox17.TabStop = false;
            // 
            // listRegroupPairs
            // 
            this.listRegroupPairs.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader10,
            this.columnHeader12});
            this.listRegroupPairs.FullRowSelect = true;
            this.listRegroupPairs.GridLines = true;
            this.listRegroupPairs.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            resources.ApplyResources(this.listRegroupPairs, "listRegroupPairs");
            this.listRegroupPairs.MultiSelect = false;
            this.listRegroupPairs.Name = "listRegroupPairs";
            this.listRegroupPairs.UseCompatibleStateImageBehavior = false;
            this.listRegroupPairs.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader10
            // 
            resources.ApplyResources(this.columnHeader10, "columnHeader10");
            // 
            // columnHeader12
            // 
            resources.ApplyResources(this.columnHeader12, "columnHeader12");
            // 
            // groupBox15
            // 
            this.groupBox15.Controls.Add(this.cmdRegroupStart);
            this.groupBox15.Controls.Add(this.cmdRegroupAcceptLine);
            this.groupBox15.Controls.Add(this.cmdRegroupUnlinkLast);
            this.groupBox15.Controls.Add(this.cmdRegroupGoBack);
            this.groupBox15.Controls.Add(this.cmdRegroupSkipDestLine);
            this.groupBox15.Controls.Add(this.cmdRegroupSkipSourceLine);
            this.groupBox15.Controls.Add(this.cmdRegroupLink);
            resources.ApplyResources(this.groupBox15, "groupBox15");
            this.groupBox15.Name = "groupBox15";
            this.groupBox15.TabStop = false;
            // 
            // cmdRegroupStart
            // 
            resources.ApplyResources(this.cmdRegroupStart, "cmdRegroupStart");
            this.cmdRegroupStart.Name = "cmdRegroupStart";
            this.cmdRegroupStart.UseVisualStyleBackColor = true;
            this.cmdRegroupStart.Click += new System.EventHandler(this.cmdRegroupStart_Click);
            // 
            // cmdRegroupAcceptLine
            // 
            resources.ApplyResources(this.cmdRegroupAcceptLine, "cmdRegroupAcceptLine");
            this.cmdRegroupAcceptLine.Name = "cmdRegroupAcceptLine";
            this.cmdRegroupAcceptLine.UseCompatibleTextRendering = true;
            this.cmdRegroupAcceptLine.UseVisualStyleBackColor = true;
            this.cmdRegroupAcceptLine.Click += new System.EventHandler(this.cmdRegroupAcceptLine_Click);
            // 
            // cmdRegroupUnlinkLast
            // 
            resources.ApplyResources(this.cmdRegroupUnlinkLast, "cmdRegroupUnlinkLast");
            this.cmdRegroupUnlinkLast.Name = "cmdRegroupUnlinkLast";
            this.cmdRegroupUnlinkLast.UseCompatibleTextRendering = true;
            this.cmdRegroupUnlinkLast.UseVisualStyleBackColor = true;
            this.cmdRegroupUnlinkLast.Click += new System.EventHandler(this.cmdRegroupUnlinkLast_Click);
            // 
            // cmdRegroupGoBack
            // 
            resources.ApplyResources(this.cmdRegroupGoBack, "cmdRegroupGoBack");
            this.cmdRegroupGoBack.Name = "cmdRegroupGoBack";
            this.cmdRegroupGoBack.UseCompatibleTextRendering = true;
            this.cmdRegroupGoBack.UseVisualStyleBackColor = true;
            this.cmdRegroupGoBack.Click += new System.EventHandler(this.cmdRegroupGoBack_Click);
            // 
            // cmdRegroupSkipDestLine
            // 
            resources.ApplyResources(this.cmdRegroupSkipDestLine, "cmdRegroupSkipDestLine");
            this.cmdRegroupSkipDestLine.Name = "cmdRegroupSkipDestLine";
            this.cmdRegroupSkipDestLine.UseCompatibleTextRendering = true;
            this.cmdRegroupSkipDestLine.UseVisualStyleBackColor = true;
            this.cmdRegroupSkipDestLine.Click += new System.EventHandler(this.cmdRegroupSkipDestLine_Click);
            // 
            // cmdRegroupSkipSourceLine
            // 
            resources.ApplyResources(this.cmdRegroupSkipSourceLine, "cmdRegroupSkipSourceLine");
            this.cmdRegroupSkipSourceLine.Name = "cmdRegroupSkipSourceLine";
            this.cmdRegroupSkipSourceLine.UseCompatibleTextRendering = true;
            this.cmdRegroupSkipSourceLine.UseVisualStyleBackColor = true;
            this.cmdRegroupSkipSourceLine.Click += new System.EventHandler(this.cmdRegroupSkipSourceLine_Click);
            // 
            // cmdRegroupLink
            // 
            resources.ApplyResources(this.cmdRegroupLink, "cmdRegroupLink");
            this.cmdRegroupLink.Name = "cmdRegroupLink";
            this.cmdRegroupLink.UseCompatibleTextRendering = true;
            this.cmdRegroupLink.UseVisualStyleBackColor = true;
            this.cmdRegroupLink.Click += new System.EventHandler(this.cmdRegroupLink_Click);
            // 
            // groupBox14
            // 
            this.groupBox14.Controls.Add(this.txtRegroupSource);
            this.groupBox14.Controls.Add(this.txtRegroupDest);
            resources.ApplyResources(this.groupBox14, "groupBox14");
            this.groupBox14.Name = "groupBox14";
            this.groupBox14.TabStop = false;
            // 
            // txtRegroupSource
            // 
            this.txtRegroupSource.BackColor = System.Drawing.SystemColors.Control;
            this.txtRegroupSource.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            resources.ApplyResources(this.txtRegroupSource, "txtRegroupSource");
            this.txtRegroupSource.HideSelection = false;
            this.txtRegroupSource.Name = "txtRegroupSource";
            this.txtRegroupSource.ReadOnly = true;
            this.txtRegroupSource.ShortcutsEnabled = false;
            // 
            // txtRegroupDest
            // 
            this.txtRegroupDest.BackColor = System.Drawing.SystemColors.Control;
            this.txtRegroupDest.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            resources.ApplyResources(this.txtRegroupDest, "txtRegroupDest");
            this.txtRegroupDest.HideSelection = false;
            this.txtRegroupDest.Name = "txtRegroupDest";
            this.txtRegroupDest.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtRegroupDest_KeyPress);
            this.txtRegroupDest.KeyDown += new System.Windows.Forms.KeyEventHandler(this.txtRegroupDest_KeyDown);
            // 
            // groupBox13
            // 
            this.groupBox13.Controls.Add(this.cmbRegroupDest);
            this.groupBox13.Controls.Add(this.cmbRegroupSource);
            this.groupBox13.Controls.Add(this.label53);
            this.groupBox13.Controls.Add(this.label52);
            resources.ApplyResources(this.groupBox13, "groupBox13");
            this.groupBox13.Name = "groupBox13";
            this.groupBox13.TabStop = false;
            // 
            // cmbRegroupDest
            // 
            this.cmbRegroupDest.FormattingEnabled = true;
            resources.ApplyResources(this.cmbRegroupDest, "cmbRegroupDest");
            this.cmbRegroupDest.Name = "cmbRegroupDest";
            // 
            // cmbRegroupSource
            // 
            this.cmbRegroupSource.FormattingEnabled = true;
            resources.ApplyResources(this.cmbRegroupSource, "cmbRegroupSource");
            this.cmbRegroupSource.Name = "cmbRegroupSource";
            // 
            // label53
            // 
            resources.ApplyResources(this.label53, "label53");
            this.label53.Name = "label53";
            // 
            // label52
            // 
            resources.ApplyResources(this.label52, "label52");
            this.label52.Name = "label52";
            // 
            // tabPage17
            // 
            this.tabPage17.Controls.Add(this.cmdNBReparse);
            this.tabPage17.Controls.Add(this.groupBox22);
            this.tabPage17.Controls.Add(this.cmdNBCopyStyles);
            this.tabPage17.Controls.Add(this.cmdNBInsertBefore);
            this.tabPage17.Controls.Add(this.cmdNBInsertAfter);
            this.tabPage17.Controls.Add(this.cmdNBReplaceLine);
            this.tabPage17.Controls.Add(this.cmdNoteBox);
            this.tabPage17.Controls.Add(this.groupBox21);
            this.tabPage17.Controls.Add(this.groupBox20);
            resources.ApplyResources(this.tabPage17, "tabPage17");
            this.tabPage17.Name = "tabPage17";
            this.tabPage17.UseVisualStyleBackColor = true;
            // 
            // cmdNBReparse
            // 
            resources.ApplyResources(this.cmdNBReparse, "cmdNBReparse");
            this.cmdNBReparse.Name = "cmdNBReparse";
            this.cmdNBReparse.Click += new System.EventHandler(this.cmdNBReparse_Click);
            // 
            // groupBox22
            // 
            this.groupBox22.Controls.Add(this.textNBDesc);
            this.groupBox22.Controls.Add(this.cmbNBStyle);
            this.groupBox22.Controls.Add(this.label59);
            resources.ApplyResources(this.groupBox22, "groupBox22");
            this.groupBox22.Name = "groupBox22";
            this.groupBox22.TabStop = false;
            // 
            // textNBDesc
            // 
            this.textNBDesc.BackColor = System.Drawing.SystemColors.Control;
            this.textNBDesc.BorderStyle = System.Windows.Forms.BorderStyle.None;
            resources.ApplyResources(this.textNBDesc, "textNBDesc");
            this.textNBDesc.Name = "textNBDesc";
            // 
            // cmbNBStyle
            // 
            this.cmbNBStyle.FormattingEnabled = true;
            resources.ApplyResources(this.cmbNBStyle, "cmbNBStyle");
            this.cmbNBStyle.Name = "cmbNBStyle";
            this.cmbNBStyle.SelectedIndexChanged += new System.EventHandler(this.cmbNBStyle_SelectedIndexChanged);
            // 
            // label59
            // 
            resources.ApplyResources(this.label59, "label59");
            this.label59.Name = "label59";
            // 
            // cmdNBCopyStyles
            // 
            resources.ApplyResources(this.cmdNBCopyStyles, "cmdNBCopyStyles");
            this.cmdNBCopyStyles.Name = "cmdNBCopyStyles";
            this.cmdNBCopyStyles.Click += new System.EventHandler(this.cmdNBCopyStyles_Click);
            // 
            // cmdNBInsertBefore
            // 
            resources.ApplyResources(this.cmdNBInsertBefore, "cmdNBInsertBefore");
            this.cmdNBInsertBefore.Name = "cmdNBInsertBefore";
            this.cmdNBInsertBefore.Click += new System.EventHandler(this.cmdNBInsertBefore_Click);
            // 
            // cmdNBInsertAfter
            // 
            resources.ApplyResources(this.cmdNBInsertAfter, "cmdNBInsertAfter");
            this.cmdNBInsertAfter.Name = "cmdNBInsertAfter";
            this.cmdNBInsertAfter.Click += new System.EventHandler(this.cmdNBInsertAfter_Click);
            // 
            // cmdNBReplaceLine
            // 
            resources.ApplyResources(this.cmdNBReplaceLine, "cmdNBReplaceLine");
            this.cmdNBReplaceLine.Name = "cmdNBReplaceLine";
            this.cmdNBReplaceLine.Click += new System.EventHandler(this.cmdNBReplaceLine_Click);
            // 
            // cmdNoteBox
            // 
            resources.ApplyResources(this.cmdNoteBox, "cmdNoteBox");
            this.cmdNoteBox.Name = "cmdNoteBox";
            this.cmdNoteBox.Click += new System.EventHandler(this.cmdNoteBox_Click);
            // 
            // groupBox21
            // 
            this.groupBox21.Controls.Add(this.textNBStyles);
            this.groupBox21.Controls.Add(this.textNBOut);
            resources.ApplyResources(this.groupBox21, "groupBox21");
            this.groupBox21.Name = "groupBox21";
            this.groupBox21.TabStop = false;
            // 
            // textNBStyles
            // 
            resources.ApplyResources(this.textNBStyles, "textNBStyles");
            this.textNBStyles.Name = "textNBStyles";
            // 
            // textNBOut
            // 
            resources.ApplyResources(this.textNBOut, "textNBOut");
            this.textNBOut.Name = "textNBOut";
            // 
            // groupBox20
            // 
            this.groupBox20.Controls.Add(this.cmdNBGetTimes);
            this.groupBox20.Controls.Add(this.maskedTextNBEnd);
            this.groupBox20.Controls.Add(this.label58);
            this.groupBox20.Controls.Add(this.maskedTextNBStart);
            this.groupBox20.Controls.Add(this.label57);
            this.groupBox20.Controls.Add(this.textNotebox2);
            this.groupBox20.Controls.Add(this.textNotebox1);
            this.groupBox20.Controls.Add(this.label56);
            this.groupBox20.Controls.Add(this.label48);
            resources.ApplyResources(this.groupBox20, "groupBox20");
            this.groupBox20.Name = "groupBox20";
            this.groupBox20.TabStop = false;
            // 
            // cmdNBGetTimes
            // 
            resources.ApplyResources(this.cmdNBGetTimes, "cmdNBGetTimes");
            this.cmdNBGetTimes.Name = "cmdNBGetTimes";
            this.cmdNBGetTimes.UseCompatibleTextRendering = true;
            this.cmdNBGetTimes.UseVisualStyleBackColor = true;
            this.cmdNBGetTimes.Click += new System.EventHandler(this.cmdNBGetTimes_Click);
            // 
            // maskedTextNBEnd
            // 
            resources.ApplyResources(this.maskedTextNBEnd, "maskedTextNBEnd");
            this.maskedTextNBEnd.Name = "maskedTextNBEnd";
            this.maskedTextNBEnd.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // label58
            // 
            resources.ApplyResources(this.label58, "label58");
            this.label58.Name = "label58";
            // 
            // maskedTextNBStart
            // 
            this.maskedTextNBStart.InsertKeyMode = System.Windows.Forms.InsertKeyMode.Overwrite;
            resources.ApplyResources(this.maskedTextNBStart, "maskedTextNBStart");
            this.maskedTextNBStart.Name = "maskedTextNBStart";
            this.maskedTextNBStart.TextMaskFormat = System.Windows.Forms.MaskFormat.IncludePromptAndLiterals;
            // 
            // label57
            // 
            resources.ApplyResources(this.label57, "label57");
            this.label57.Name = "label57";
            // 
            // textNotebox2
            // 
            resources.ApplyResources(this.textNotebox2, "textNotebox2");
            this.textNotebox2.Name = "textNotebox2";
            // 
            // textNotebox1
            // 
            resources.ApplyResources(this.textNotebox1, "textNotebox1");
            this.textNotebox1.Name = "textNotebox1";
            // 
            // label56
            // 
            resources.ApplyResources(this.label56, "label56");
            this.label56.Name = "label56";
            // 
            // label48
            // 
            resources.ApplyResources(this.label48, "label48");
            this.label48.Name = "label48";
            // 
            // Form1
            // 
            resources.ApplyResources(this, "$this");
            this.Controls.Add(this.splitContainer1);
            this.DoubleBuffered = true;
            this.Menu = this.mainMenu1;
            this.Name = "Form1";
            this.Resize += new System.EventHandler(this.Resize_Handler);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.tabPage6.ResumeLayout(false);
            this.tabPage6.PerformLayout();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.tabMain.ResumeLayout(false);
            this.tabPage1.ResumeLayout(false);
            this.groupBox12.ResumeLayout(false);
            this.groupBox12.PerformLayout();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.groupBox3.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.tabShiftType.ResumeLayout(false);
            this.tabPage9.ResumeLayout(false);
            this.tabPage9.PerformLayout();
            this.tabPage10.ResumeLayout(false);
            this.tabPage10.PerformLayout();
            this.tabPage11.ResumeLayout(false);
            this.tabPage11.PerformLayout();
            this.tabPage12.ResumeLayout(false);
            this.tabPage12.PerformLayout();
            this.tabPage2.ResumeLayout(false);
            this.groupBox11.ResumeLayout(false);
            this.groupBox11.PerformLayout();
            this.groupBox10.ResumeLayout(false);
            this.groupBox10.PerformLayout();
            this.groupBox6.ResumeLayout(false);
            this.groupBox6.PerformLayout();
            this.groupBox5.ResumeLayout(false);
            this.groupBox5.PerformLayout();
            this.tabPage3.ResumeLayout(false);
            this.panelEffect.ResumeLayout(false);
            this.tabControl1.ResumeLayout(false);
            this.tabPage7.ResumeLayout(false);
            this.tabPage8.ResumeLayout(false);
            this.panelLayer.ResumeLayout(false);
            this.panelLayer.PerformLayout();
            this.groupBox16.ResumeLayout(false);
            this.groupBox16.PerformLayout();
            this.tabPage4.ResumeLayout(false);
            this.tabPage4.PerformLayout();
            this.panelGradient.ResumeLayout(false);
            this.panelGradient.PerformLayout();
            this.groupBox7.ResumeLayout(false);
            this.tabPage14.ResumeLayout(false);
            this.tabPage14.PerformLayout();
            this.panelBlur.ResumeLayout(false);
            this.panelBlur.PerformLayout();
            this.groupBlurAffect.ResumeLayout(false);
            this.groupBox9.ResumeLayout(false);
            this.groupBlurSub.ResumeLayout(false);
            this.groupBox8.ResumeLayout(false);
            this.tabPage16.ResumeLayout(false);
            this.tabPage16.PerformLayout();
            this.tabPage13.ResumeLayout(false);
            this.tabPage15.ResumeLayout(false);
            this.tabPage5.ResumeLayout(false);
            this.groupBox19.ResumeLayout(false);
            this.groupBox19.PerformLayout();
            this.groupBox18.ResumeLayout(false);
            this.groupBox17.ResumeLayout(false);
            this.groupBox15.ResumeLayout(false);
            this.groupBox14.ResumeLayout(false);
            this.groupBox14.PerformLayout();
            this.groupBox13.ResumeLayout(false);
            this.groupBox13.PerformLayout();
            this.tabPage17.ResumeLayout(false);
            this.groupBox22.ResumeLayout(false);
            this.groupBox22.PerformLayout();
            this.groupBox21.ResumeLayout(false);
            this.groupBox21.PerformLayout();
            this.groupBox20.ResumeLayout(false);
            this.groupBox20.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        [STAThread]
        
        static void Main(string[] args) {
            Application.ThreadException += new System.Threading.ThreadExceptionEventHandler(Application_ThreadException);
            Application.SetUnhandledExceptionMode(UnhandledExceptionMode.CatchException);
            //Application.EnableVisualStyles();
            Application.Run(new Form1(args));
            
        }

        static void Application_ThreadException(object sender, System.Threading.ThreadExceptionEventArgs e) {
            DialogResult result = DialogResult.Ignore;
            try {
                result = MessageBox.Show(e.Exception.Message + " Abort operation?", "Windows Forms Error", MessageBoxButtons.YesNo);
            } catch {

            }

            // Exits the program when the user clicks Abort.
            if (result == DialogResult.No)
                Application.Exit();
        }

        public Form1(string[] args) {
            //
            // Required for Windows Form Designer support
            //
            FormMain = this;
            InitializeComponent();
            Evaluate.FillOpsList();
            listSSAg = Graphics.FromHwnd(listSSA.Handle);
            lineColl = new List<Line>();
            layerColl = new System.Collections.Generic.List<Layer>();
            templateFilterColl = new System.Collections.Generic.List<Filter>();
            undoStack = new Stack();
            redoStack = new Stack();
            //CurFile = String.Empty;

            // Add the "raw code" filter as a default filter
            Filter raw = new Filter("Raw code", "$Code$");
            FilterOption rawfo = new FilterOption("Code", "");
            raw.AddOption(rawfo);
            templateFilterColl.Add(raw);

            string path = "effects.exml";
            if (File.Exists(path)) LoadEffectsFile(path);
            enc = new System.Text.UnicodeEncoding();
            Notebox.readNoteBoxes();

            string arglower;
            foreach (string arg in args) {
                if (File.Exists(arg)) {
                    arglower = arg.ToLower(Util.cfi);
                    if (arglower.EndsWith(".xml"))
                        LoadProject(arg);
                    else if (arglower.EndsWith(".ass") || arglower.EndsWith(".ssa") || arglower.EndsWith(".assa"))
                        LoadFile(args[0], null);
                }
            }
            listSSA.LabelEdit = false;
        }

        #region user-defined variables
        //Note: Some function-specific variables are defined in their own section instead of here
        Regex r = new Regex(@"(\\[kK][of]?)(\d+)([^\}]*)(\}?)([^\{]*)", RegexOptions.Compiled | RegexOptions.Singleline);
        public static List<Layer> layerColl;
        public static List<Filter> templateFilterColl;
        public static List<Style> styleColl;
        public static List<Line> lineColl;
        private static Stack undoStack, redoStack;
        private Layer CurLayer;
        private string CurFile;
        private Encoding enc;
        public static int ResX, ResY;
        public static byte sver;
        public static Graphics listSSAg;

        #endregion

        private void Form1_Load(object sender, System.EventArgs e) {
            EventContext();
            ContextMenu listcm = new ContextMenu();
            MenuItem mi;
            for (int index = 0; index != menuItem2.MenuItems.Count; index+=1) {
                mi = menuItem2.MenuItems[index];
                listcm.MenuItems.Add(mi.Index, mi.CloneMenu());
            }
            listSSA.ContextMenu = listcm;
            splitContainer1.SplitterDistance = splitContainer1.Height - (tabMain.Height + 16);
        }

        void Resize_Handler(object sender, EventArgs e) {
            if (this.WindowState == FormWindowState.Normal) {
                Control control = (Control)sender;
                int newSplitterDistance = splitContainer1.Height - splitContainer1.SplitterDistance + 8;
                splitContainer1.Size = this.Size;
                splitContainer1.SplitterDistance = Math.Max(this.Height - newSplitterDistance, 64);
                listSSA.Size = new System.Drawing.Size(splitContainer1.Panel1.Width - (groupBox1.Width + 16),
                                           splitContainer1.Panel1.Height);
                groupBox1.Left = splitContainer1.Panel1.Width - (groupBox1.Width + 8);
                tabMain.Size = splitContainer1.Panel2.Size;
            }
        }
        void splitterMoved(object sender, EventArgs e) {
            tabMain.Size = splitContainer1.Panel2.Size;
        }
        void tabMain_Resize(object sender, EventArgs e) {
            int tabHeight = tabMain.Height - (tabMain.ItemSize.Height + 60);
            int tabWidth = tabMain.Width - 16;
            lstErrors.Height = tabHeight;
            lstFonts.Height = tabHeight-8;

            //Noteboxes
            groupBox21.Height = tabHeight - groupBox21.Top;
            groupBox20.Width = groupBox21.Width = tabWidth - groupBox21.Left;
            textNBStyles.Height = groupBox21.Height / 3;
            textNBOut.Top = textNBStyles.Top + textNBStyles.Height + 4;
            textNBOut.Height = groupBox21.Height - (textNBOut.Top + 4);
            textNBStyles.Width = textNBOut.Width = groupBox21.Width - (textNBOut.Left << 1);
            textNotebox1.Width = groupBox20.Width - (textNotebox1.Left + 8);
            textNotebox2.Width = textNotebox1.Width;
            
            panelGradient.Left = tabWidth - panelGradient.Width;
            txtGradOut.Width = panelGradient.Left - (txtGradOut.Left + 8);
            txtGradIn.Width = txtGradOut.Width;
            txtGradBefore.Width = txtGradOut.Width - (cmdGradMoveBefore.Width + 8);
            cmdGradMoveBefore.Left = txtGradBefore.Left + txtGradBefore.Width + 8;

            panelBlur.Left = tabWidth - panelBlur.Width;
            txtBlurOut.Width = panelBlur.Left - (txtBlurOut.Left + 8);
            txtBlurIn.Width = txtBlurOut.Width;
            txtBlurBefore.Width = txtBlurOut.Width - (cmdBlurMoveBefore.Width + 8);
            cmdBlurMoveBefore.Left = txtBlurBefore.Left + txtBlurBefore.Width + 8;

            //karaoke
            panelEffect.Width = tabWidth - (panelEffect.Left + 4);
            panelLayer.Width = panelEffect.Width;
            tabControl1.Width = panelEffect.Width - 8;
            lstEffectOptions.Width = panelEffect.Width - 32;
            lstEffectConditions.Width = lstEffectOptions.Width;
            lstLayerConditions.Width = panelEffect.Width - 8;
        }


        #region Splitter/Focus stuff
        Control focused = null;
        private Control getFocused(Control.ControlCollection controls) {
            foreach (Control c in controls) {
                if (c.Focused)
                    return c;
                else if (c.ContainsFocus)
                    return getFocused(c.Controls);
            }
            return null;
        }
        void splitContainer1_MouseDown(object sender, MouseEventArgs e) {
            focused = getFocused(this.Controls);
        }

        void splitContainer1_MouseUp(object sender, MouseEventArgs e) {
            if (focused!=null) {
                focused.Focus();
                focused = null;
            }
        }

        #endregion

        #region Menu
        private void menuOpen_Click(object sender, System.EventArgs e) {
            openFileDialog1.Filter = "ASS,SSA Files|*.ass;*.ssa";
            openFileDialog1.FileName = String.Empty;
            if (openFileDialog1.ShowDialog() == DialogResult.OK) LoadFile(openFileDialog1.FileName, null);
        }
        private void menuOpenSJIS_Click(object sender, System.EventArgs e) {
            openFileDialog1.Filter = "ASS,SSA Files|*.ass;*.ssa";
            openFileDialog1.FileName = String.Empty;
            //openFileDialog1.Filter
            if (openFileDialog1.ShowDialog() == DialogResult.OK) LoadFile(openFileDialog1.FileName, Encoding.GetEncoding("Shift_JIS"));
        }
        private void menuOpenEUCJP_Click(object sender, EventArgs e) {
            openFileDialog1.Filter = "ASS,SSA Files|*.ass;*.ssa";
            openFileDialog1.FileName = String.Empty;
            if (openFileDialog1.ShowDialog() == DialogResult.OK) LoadFile(openFileDialog1.FileName, Encoding.GetEncoding("euc-jp"));
        }
        private void menuLoadEffects_Click(object sender, System.EventArgs e) {
            openFileDialog1.Filter = "XML Files (*.xml)|*.xml";
            if (openFileDialog1.ShowDialog() == DialogResult.OK) LoadEffectsFile(openFileDialog1.FileName);
        }
        private void menuLoadProj_Click(object sender, System.EventArgs e) {
            openFileDialog1.Filter = "XML files (*.xml)|*.xml";
            if (openFileDialog1.ShowDialog() == DialogResult.OK) {
                layerColl = LoadProject(openFileDialog1.FileName);
                RedrawKaraEffectsTree();
                if (layerColl.Count != 0) treeKaraoke.SelectedNode = treeKaraoke.Nodes[0];
            }
        }
        private void menuSaveProj_Click(object sender, System.EventArgs e) {
            SaveFileDialogWithEncoding ofd = new SaveFileDialogWithEncoding();
            ofd.DefaultExt = "xml";
            ofd.EncodingType = EncodingType.UTF8;
            ofd.Filter = "XML files (*.xml)|*.xml|All files (*.*)|*.*";
            if (ofd.ShowDialog((IntPtr)this.Handle, Screen.FromControl(this), "UTF-8") == DialogResult.OK) {
                SaveProject(ofd.FileName);
            }
        }
        private void menuFilter_Click(object sender, System.EventArgs e) {
            if (CurLayer != null) {
                Filter tf = GetTemplateFilterFromName(((MenuItem)sender).Text);
                if (tf != null) {
                    CurLayer.AddFilter((Filter)tf.Clone());
                    TreeNode tn = new TreeNode(tf.Name);
                    tn.Checked = true;
                    int selected = treeKaraoke.SelectedNode.Level == 0 ? treeKaraoke.SelectedNode.Index : treeKaraoke.SelectedNode.Parent.Index;
                    treeKaraoke.Nodes[selected].Nodes.Add(tn);
                    treeKaraoke.SelectedNode = tn;
                    //  = treeKaraoke.Nodes[selected].Nodes[CurLayer.Count - 1];
                }
                else MessageBox.Show("Filter not found in template class. Try reloading the program.");
            }
            else MessageBox.Show("Add or select a layer first.");
        }
        private void menuFont_Click(object sender, System.EventArgs e) {
            fontDialog1.Font = listSSA.Font;
            DialogResult dr = fontDialog1.ShowDialog(this);
            if (dr == DialogResult.OK)
                listSSA.Font = fontDialog1.Font;
        }
        private void menuSave_Click(object sender, System.EventArgs e) {
            if (!CurFile.Equals(String.Empty)) SaveFile(CurFile);
            else menuSaveAs.PerformClick();
        }
        private void menuSaveAs_Click(object sender, System.EventArgs e) {
            SaveFileDialogWithEncoding ofd = new SaveFileDialogWithEncoding();
            ofd.DefaultExt = "ass";
            ofd.EncodingType = EncodingType.Unicode;
            ofd.Filter = "SSA/ASS files (*.ass;*.ssa)|*.ass;*.ssa|All files (*.*)|*.*";
            if (ofd.ShowDialog((IntPtr)this.Handle, Screen.FromControl(this), enc == null ? string.Empty : enc.ToString()) == DialogResult.OK) {
                enc = SaveFileDialogWithEncoding.stringToEncodingType(ofd.EncodingType.ToString());
                SaveFile(ofd.FileName);

            }
        }

        private void menuShowEffectsEditor_Click(object sender, System.EventArgs e) {
            formEffectsEditor editor = new formEffectsEditor(templateFilterColl);
            editor.Show();
        }
        private void menuGoTo_Click(object sender, System.EventArgs e) {
            InputBoxResult newvalue = (InputBox.Show("Enter line number (from 1)", "Input", GetFirstSelected().ToString(Util.cfi)));
            int index;
            if ((newvalue.OK == true) && (int.TryParse(newvalue.Text, out index))) {
                if ((index >= 0) && (index < lineColl.Count)) {
                    for (int listindex = lineColl.Count; listindex != 0; --listindex)
                        listSSA.Items[listindex].Selected = false;
                    listSSA.Items[index].Selected = true;
                }
                else
                    MessageBox.Show("Invalid line.");
            }
        }
        private void menuNewLineBeginning_Click(object sender, System.EventArgs e) {
            NewLine(0);
        }
        private void menuNewLineEnd_Click(object sender, System.EventArgs e) {
            NewLine((lineColl.Count == 0) ? 0 : (lineColl.Count - 1));
        }
        private void menuNewLineAbove_Click(object sender, System.EventArgs e) {
            int index = GetFirstSelected();
            if (index != -1)
                NewLine(index);
            else MessageBox.Show("Select an item first.");
        }
        private void menuNewLineBelow_Click(object sender, System.EventArgs e) {
            int index = GetFirstSelected();
            if (index != -1)
                NewLine(index + 1);
            else MessageBox.Show("Select an item first.");
        }
        private void menuMoveLineUp_Click(object sender, System.EventArgs e) {
            if (lineColl != null) {
                int index = GetFirstSelected();
                if (index > 0) { // can't be -1 and can't be 0 (how can you move the first line up?)
                    Line swap = lineColl[index];
                    lineColl[index] = lineColl[index - 1];
                    lineColl[index - 1] = swap;
                    DrawListOnScreen();
                }
                else MessageBox.Show("Select an item that is not on the top first.");
            }
        }
        private void menuMoveLineDown_Click(object sender, System.EventArgs e) {
            if (lineColl != null) {
                int index = GetFirstSelected();
                if ((index != -1) && (index + 1 != lineColl.Count)) {
                    Line swap = lineColl[index];
                    lineColl[index] = lineColl[index + 1];
                    lineColl[index + 1] = swap;
                    DrawListOnScreen();
                }
                else MessageBox.Show("Select an item that is not on the bottom first.");
            }
        }
        private void menuChangeLine_Click(object sender, System.EventArgs e) {
            int index = GetFirstSelected();
            if (index != -1) {
                Line oldline = lineColl[index];
                InputBoxResult newvalue = (InputBox.Show("Enter new text", "Input", oldline.ToString()));
                if (newvalue.OK == true) {
                    Line newline = Line.ParseLine(newvalue.Text);
                    newline.enabled = oldline.enabled;
                    newline.selected = oldline.selected;
                    Style newstyle=null;
                    if (newline.lineType == LineType.style) {
                        newstyle = (Style)newline.line;
                        styleColl.Add(newstyle);
                        lstStyles.Items.Add(newstyle.name);
                    }
                    if (oldline.lineType == LineType.style && newline.lineType == LineType.style) {
                        for (int i = 0; i != lineColl.Count; i += 1) {
                            Line l = lineColl[i];
                            if (l.lineType==LineType.dialogue&&((DialogueLine)l.line).style==oldline.line)
                                ((DialogueLine)l.line).style=newstyle;
                        }
                        ReparseErrors();
                    }
                    if (oldline.lineType == LineType.style) {
                        Style oldstyle = (Style)oldline.line;
                        lstStyles.Items.Remove(oldstyle.name);
                        styleColl.Remove(oldstyle);
                    }

                    LineCUndo lu = new LineCUndo();
                    lu.index = index;
                    lu.line = oldline;
                    undoStack.Push(lu);
                    menuUndo.Enabled = true;
                    lineColl[index] = newline;
                    DrawListOnScreen();
                }
            }
            else MessageBox.Show("Select a line first.");
        }
        private void menuRemoveLine_Click(object sender, System.EventArgs e) {
            if (lineColl != null && undoStack != null) {
                listSSA.BeginUpdate();
                List<LineUndo> lul = new List<LineUndo>();
                LineUndo lu;
                for (int index = lineColl.Count - 1; index != -1; index--) {
                    if (lineColl[index].selected) {
                        listSSA.VirtualListSize--;
                        lu = new LineUndo();
                        lu.index = index;
                        lu.line = lineColl[index];
                        lul.Add(lu);
                        lineColl.RemoveAt(index);
                    }
                }
                undoStack.Push(lul);
                menuUndo.Enabled = true;
                listSSA.EndUpdate();
            }
        }
        private void menuExit_Click(object sender, System.EventArgs e) {
            Application.Exit();
        }
        private void menuPrecisioncs_Click(object sender, EventArgs e) {
            Util.PrecisionF = 10.0F;
            Util.Precision = 2;
            menuPrecisioncs.Checked = !(menuPrecisionms.Checked = false);
            DrawListOnScreen();
        }
        private void menuPrecisionms_Click(object sender, EventArgs e) {
            Util.PrecisionF = 1.0F;
            Util.Precision = 3;
            menuPrecisionms.Checked = !(menuPrecisioncs.Checked = false);
            DrawListOnScreen();
        }
        private void menuSelectRange_Click(object sender, EventArgs e) {
            if (lineColl.Count != 0) {
                InputBoxResult firstline = InputBox.Show("Enter the first line number", "Input", string.Empty);
                InputBoxResult lastline = InputBox.Show("Enter the last line number", "Input", string.Empty);
                if ((firstline.OK == true) && (lastline.OK == true)) {
                    int first, last;
                    if ((int.TryParse(firstline.Text, out first)) && (int.TryParse(lastline.Text, out last))
                            && (first > 0) && (first <= last) && (last <= lineColl.Count)) {
                        first--; last--;
                        for (int index = lineColl.Count; index != 0; --index)
                            listSSA.Items[index].Selected = (index >= first && index < last) ? true : false;
                    }
                    else MessageBox.Show("Enter valid numbers.");
                }
                else MessageBox.Show("You must enter both first and last line numbers.");
            }
            else MessageBox.Show("Load a file first.");
        }
        private void menuDeselectRange_Click(object sender, EventArgs e) {
            if (lineColl.Count != 0) {
                InputBoxResult firstline = InputBox.Show("Enter the first line number", "Input", string.Empty);
                InputBoxResult lastline = InputBox.Show("Enter the last line number", "Input", string.Empty);
                if ((firstline.OK == true) && (lastline.OK == true)) {
                    //if ((Evaluate.isNum(firstline.Text)) && (Evaluate.isNum(lastline.Text)) && (int.Parse(firstline.Text, Util.cfi) > 0) && (int.Parse(firstline.Text, Util.cfi) <= int.Parse(lastline.Text, Util.cfi)) && (int.Parse(lastline.Text, Util.cfi) <= listSSA.Items.Count))
                    //   for (int index = listSSA.Items.Count;index!=0;--index)
                    //     listSSA.Items[index].Selected = (index >= first && index < last)?true:false;
                    //else MessageBox.Show("Enter valid numbers.");
                }
                else MessageBox.Show("You must enter both first and last line numbers.");
            }
            else MessageBox.Show("Load a file first.");
        }

        private void menuUndo_Click(object sender, EventArgs e) {
            UndoRedo(undoStack, redoStack);
        }
        private void menuRedo_Click(object sender, EventArgs e) {
            UndoRedo(redoStack, undoStack);
        }
        private void menuClearUndoRedo_Click(object sender, EventArgs e) {
            undoStack = new Stack();
            redoStack = new Stack();
            GC.Collect();
            menuUndo.Enabled = menuRedo.Enabled = false;
        }
        #endregion

        #region Karaoke effects
        private void textLayerRepetitions_KeyPress(object sender, KeyPressEventArgs e) {
            if (((!char.IsDigit(e.KeyChar)) || (e.KeyChar == '.')) && (e.KeyChar != 8) && (e.KeyChar != 177))
                e.Handled = true;
        }
        private void textLayerRepetitions_Leave(object sender, EventArgs e) {
            int reps;
            if (string.IsNullOrEmpty(textLayerRepetitions.Text) || !int.TryParse(textLayerRepetitions.Text, out reps) || (reps < 1))
                textLayerRepetitions.Text = CurLayer.Repetitions.ToString(Util.cfi);
            else if (CurLayer != null)
                CurLayer.Repetitions = reps;
        }
        private void chkSyllablePerLine_CheckedChanged(object sender, EventArgs e) {
            if (CurLayer != null) CurLayer.SyllablePerLine = chkSyllablePerLine.Checked;
            chkLayerPerSyllable.Enabled = !chkSyllablePerLine.Checked;
            textLayerRepetitions.Enabled = !chkSyllablePerLine.Checked;
        }
        private void chkLayerPerSyllable_CheckedChanged(object sender, System.EventArgs e) {
            if (CurLayer != null) CurLayer.PerSyllable = chkLayerPerSyllable.Checked;
            textLayerRepetitions.Enabled = !chkLayerPerSyllable.Checked;
            chkSyllablePerLine.Enabled = !chkLayerPerSyllable.Checked;
        }
        private void chkAddAnyway_CheckedChanged(object sender, System.EventArgs e) {
            if (CurLayer != null) CurLayer.AddAll = chkAddAnyway.Checked;
        }
        private void chkAddOnce_CheckedChanged(object sender, System.EventArgs e) {
            if (CurLayer != null) CurLayer.AddOnce = chkAddOnce.Checked;
        }

        private void chkKaraAddK_CheckedChanged(object sender, EventArgs e) {
            CurLayer.AddK = chkKaraAddK.Checked;
        }
        private void chkKaraAddASSA_CheckedChanged(object sender, EventArgs e) {
            CurLayer.AddASSA = chkKaraAddASSA.Checked;
        }
        private void chkKaraAddClosingBracket_CheckedChanged(object sender, EventArgs e) {
            CurLayer.AddBracket = chkKaraAddClosingBracket.Checked;
        }
        private void chkKaraAddText_CheckedChanged(object sender, EventArgs e) {
            CurLayer.AddText = chkKaraAddText.Checked;
        }

        private void cmdNewLayer_Click(object sender, System.EventArgs e) {
            Layer nl = new Layer();
            nl.Enabled = true;
            nl.Name = "Layer " + (layerColl.Count).ToString(Util.nfi);
            layerColl.Add(nl);
            TreeNode tn = new TreeNode("Layer " + (layerColl.Count-1).ToString(Util.nfi));
            tn.Checked = true;
            treeKaraoke.SelectedNode = treeKaraoke.Nodes[treeKaraoke.Nodes.Add(tn)];
        }
        private void cmdKaraDel_Click(object sender, System.EventArgs e) {
            if (treeKaraoke.SelectedNode != null) {
                if (treeKaraoke.SelectedNode != null) {
                    if (treeKaraoke.SelectedNode.Level == 0) // Removing a layer
                        layerColl.RemoveAt(treeKaraoke.SelectedNode.Index);
                    else // Removing a filter
                        CurLayer.RemoveFilter(treeKaraoke.SelectedNode.Index);
                }
                treeKaraoke.SelectedNode.Remove();
            }
        }
        private void cmdKaraDup_Click(object sender, System.EventArgs e) {
            int level = treeKaraoke.SelectedNode.Level;
            TreeNode tn;
            if (level == 0) {
                layerColl.Add((Layer)(layerColl[treeKaraoke.SelectedNode.Index]).Clone());
                RedrawKaraEffectsTree();
            }
            else if (CurLayer != null) {
                CurLayer.AddFilter((Filter)CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).Clone());
                tn = treeKaraoke.SelectedNode.Parent.Nodes.Add(treeKaraoke.SelectedNode.Text);
                tn.Checked = treeKaraoke.SelectedNode.Checked;
            }
            else MessageBox.Show("Select a layer first.");
        }
        private void cmdAddEffect_Click(object sender, System.EventArgs e) {
            //Context menu is created in function "EventContext"
            cmdAddEffect.ContextMenu.Show(cmdAddEffect, new System.Drawing.Point(cmdAddEffect.Width >> 1));
        }
        private void cmdAddEffectCondition_Click(object sender, System.EventArgs e) {
            if (treeKaraoke.SelectedNode.Level == 1) {
                ConditionDialogResult cdr = ConditionDialog.Show("Design a new effect condition.", "Input");
                if (cdr.OK == true) {
                    string[] cond = new string[3];
                    ListViewItem lvi;

                    CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).AddCondition(cdr.ConditionOne, cdr.ConditionComp, cdr.ConditionTwo, true);

                    cond[0] = cdr.ConditionOne;
                    cond[1] = cdr.ConditionComp;
                    cond[2] = cdr.ConditionTwo;
                    lvi = new ListViewItem(cond);
                    lvi.Checked = true;
                    lstEffectConditions.Items.Add(lvi);
                }
            }
            else MessageBox.Show("Add or select an existing effect first.");
        }
        private void cmdDelEffectCondition_Click(object sender, System.EventArgs e) {
            if ((lstEffectConditions.SelectedItems.Count > 0) && (treeKaraoke.SelectedNode.Level == 1)) {
                CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).RemoveCondition(lstEffectConditions.SelectedItems[0].Index);
                lstEffectConditions.SelectedItems[0].Remove();
            }
            else MessageBox.Show("Add or select an existing effect first.");
        }
        private void cmdNewLayerCondition_Click(object sender, EventArgs e) {
            if (treeKaraoke.SelectedNode != null && treeKaraoke.SelectedNode.Level == 0) {
                ConditionDialogResult cdr = ConditionDialog.Show("Design a new layer condition.", "Input");
                if (cdr.OK == true) {
                    string[] cond = new string[3];
                    ListViewItem lvi;

                    CurLayer.AddCondition(cdr.ConditionOne, cdr.ConditionComp, cdr.ConditionTwo, true);

                    cond[0] = cdr.ConditionOne;
                    cond[1] = cdr.ConditionComp;
                    cond[2] = cdr.ConditionTwo;
                    lvi = new ListViewItem(cond);
                    lvi.Checked = true;
                    lstLayerConditions.Items.Add(lvi);
                }
            }
            else MessageBox.Show("Add or select an existing layer first.");
        }
        private void cmdDelLayerCondition_Click(object sender, System.EventArgs e) {
            if ((treeKaraoke.SelectedNode.Level == 0) && (lstLayerConditions.SelectedItems.Count > 0)) {
                CurLayer.RemoveCondition(lstLayerConditions.SelectedItems[0].Index);
                lstLayerConditions.SelectedItems[0].Remove();
            }
            else MessageBox.Show("Add or select an existing layer and layer condition first.");
        }

        private void cmdDoEffects_Click(object sender, System.EventArgs e) {
            double startTime = 0, endTime = 0;
            string syllableText = string.Empty, sFiltCode;
            int[,] times;
            System.Text.StringBuilder sb, thisFiltCode, conditionOne;
            MatchCollection mc;
            Condition c;
            List<Line> newList = new List<Line>();
            Line mainline, line;
            DialogueLine dl;
            Match m;
            Layer thislayer;
            Filter thisfilter;
            FilterOption thisoption;
            int layerNumLPSTot, totKaraNum, karaIndex, thisKaraLen, thisKaraStart;
            int filterIndex, optionIndex, thisKaraEnd, layerIndex, lineindex, lastindex;
            int textstart, textlen;
            bool conditionMet;
            labelSelLine.Text = string.Empty;


            for (lineindex = 0; lineindex != lineColl.Count; lineindex+=1) {
                mainline = lineColl[lineindex];

                if ((mainline.enabled == false) || (mainline.lineType != LineType.dialogue) || ((dl = (DialogueLine)mainline.line).style.enabled == false) || (r.IsMatch(((DialogueLine)mainline.line).text) == false)) {
                    newList.Add(mainline);
                    continue;
                }
                mc = r.Matches(dl.text);

                startTime = dl.start.TotalSeconds;
                endTime = dl.end.TotalSeconds;

                totKaraNum = mc.Count;
                times = new int[totKaraNum, 5];
                for (karaIndex = 0; karaIndex != mc.Count; karaIndex+=1) {
                    m = mc[karaIndex];
                    if (karaIndex != 0) {
                        times[karaIndex, 0] = times[karaIndex - 1, 2]; // end time of last syllable is start time of this syllable
                        times[karaIndex, 3] = times[karaIndex - 1, 3] + times[karaIndex - 1, 4];
                    }
                    times[karaIndex, 1] = int.Parse(m.Groups[2].Value, Util.nfi) * 10;
                    times[karaIndex, 2] = times[karaIndex, 0] + times[karaIndex, 1]; // end time of this syllable is start time + length
                    times[karaIndex, 4] = m.Groups[5].Value.Length;
                }

                for (layerIndex = 0; layerIndex != layerColl.Count; layerIndex+=1) {
                    thislayer = layerColl[layerIndex];
                    if (thislayer.Enabled == false) continue;

                    layerNumLPSTot = (thislayer.PerSyllable) ? totKaraNum : thislayer.Repetitions;

                    // The following loop is for layer-per-syllable
                    for (int layerIndexInside = 0; layerIndexInside != layerNumLPSTot; layerIndexInside+=1) {
                        line = (Line)mainline.Clone();
                        dl = (DialogueLine)line.line;

                        conditionMet = true;

                        for (int layercondIndex = 0; layercondIndex != thislayer.ConditionCount; layercondIndex+=1) {
                            c = thislayer.GetCondition(layercondIndex);
                            if (c.ConditionEnabled == false) continue;
                            conditionOne = new StringBuilder(32);
                            conditionOne.Append(String.Format("({0}){1}({2})", c.ConditionOne, c.ConditionOp, c.ConditionTwo));

                            if (conditionOne.ToString().Contains("%")) {
                                conditionOne = conditionOne.Replace("%karanumtot%", totKaraNum.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%layernumtot%", layerColl.Count.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%layernum%", layerIndex.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%layernumlpstot%", layerNumLPSTot.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%layernumlps%", layerIndexInside.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%linenum%", layerIndex.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%linelen%", (endTime - startTime).ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%linestart%", startTime.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%lineend%", endTime.ToString(Util.nfi));
                                conditionOne = conditionOne.Replace("%style%", dl.style.name);
                                conditionOne = conditionOne.Replace("%name%", dl.actor);

                                if (conditionOne.ToString().IndexOf("%") != -1) { // see if the % is STILL there
                                    string c1 = conditionOne.ToString();
                                    string cnew = String.Empty;
                                    int lastindex1=0,krindex=0,krcindex=0;
                                    while(true) {
                                        krindex = Math.Min(c1.IndexOf("%karastart[", krcindex) + 10, Math.Min(c1.IndexOf("%karaend[", krcindex) + 8, c1.IndexOf("%karalength[", krcindex) + 11));
                                        if (krindex==-1 || krindex<krcindex) break;
                                        krcindex = c1.IndexOf("]", krindex);
                                        if (krcindex==-1) break;
                                        string r1 = c1.Substring(krindex, krcindex-krindex);
                                        cnew = cnew + c1.Substring(lastindex1, krindex - lastindex1) + Evaluate.ScriptParse(r1);
                                        lastindex1 = krcindex;
                                    }
                                    if (lastindex1 < c1.Length) cnew += c1.Substring(lastindex1, c1.Length - lastindex1);
                                    conditionOne = new StringBuilder(cnew);

                                    for (krindex = 0; krindex != totKaraNum; krindex+=1) {
                                        conditionOne = conditionOne.Replace("%karastart[" + krindex.ToString(Util.cfi) + "]%", times[krindex, 0].ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%karalength[" + krindex.ToString(Util.cfi) + "]%", times[krindex, 1].ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%karaend[" + krindex.ToString(Util.cfi) + "]%", times[krindex, 2].ToString(Util.nfi));
                                    }
                                }
                            }

                            conditionMet = Evaluate.Eval(Evaluate.ScriptParse(conditionOne.ToString())).Equals(true);

                            if (conditionMet == false) break;
                        }

                        if (conditionMet == false) {
                            if ((thislayer.AddAll == true) || ((thislayer.AddOnce == true) && (layerIndexInside == 1)))
                                newList.Add(line);
                            continue;
                        }

                        sb = new System.Text.StringBuilder(1024); // create a new stringbuilder

                        lastindex = 0;
                        for (karaIndex = 0; karaIndex != mc.Count; karaIndex+=1) {
                            m = mc[karaIndex];
                            if (m.Index > lastindex) sb.Append(dl.text.Substring(lastindex, m.Index - lastindex));
                            thisKaraStart = times[karaIndex, 0];
                            thisKaraLen = times[karaIndex, 1];
                            thisKaraEnd = times[karaIndex, 2];
                            textstart = times[karaIndex, 3];
                            textlen = times[karaIndex, 4];

                            for (filterIndex = 0; filterIndex != thislayer.Count; filterIndex+=1) {
                                thisfilter = thislayer.GetFilter(filterIndex);
                                if (thisfilter.Enabled == false) continue;

                                conditionMet = true;

                                //effect conditions
                                for (int effectcondIndex = 0; effectcondIndex != thisfilter.ConditionCount; effectcondIndex+=1) {
                                    c = thisfilter.GetCondition(effectcondIndex);
                                    if (c.ConditionEnabled == false) continue;
                                    conditionOne = new StringBuilder(32);

                                    conditionOne.Append(String.Format("({0}){1}({2})", c.ConditionOne, c.ConditionOp, c.ConditionTwo));

                                    if (conditionOne.ToString().Contains("%")) {
                                        conditionOne = conditionOne.Replace("%karalen%", thisKaraLen.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%karastart%", thisKaraStart.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%karamid%", (thisKaraStart + (thisKaraLen >> 1)).ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%karaend%", thisKaraEnd.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%karanum%", karaIndex.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%karanumtot%", totKaraNum.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%linenum%", lineindex.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%linelen%", (endTime - startTime).ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%linestart%", startTime.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%lineend%", endTime.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%layernumtot%", layerColl.Count.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%layernum%", layerIndex.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%layernumlpstot%", layerNumLPSTot.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%layernumlps%", layerIndexInside.ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%style%", dl.style.name);
                                        conditionOne = conditionOne.Replace("%name%", dl.actor);
                                        conditionOne = conditionOne.Replace("%lastkarastart%", (times[Math.Max(0, karaIndex - 2), 0]).ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%nextkaraend%", (times[Math.Min(totKaraNum - 1, karaIndex + 1), 0]).ToString(Util.nfi));
                                        conditionOne = conditionOne.Replace("%text%", String.IsNullOrEmpty(m.Groups[5].Value) ? string.Empty : m.Groups[5].Value);

                                        if (conditionOne.ToString().IndexOf("%") != -1) { // see if the % is STILL there
                                            string c1 = conditionOne.ToString();
                                            string cnew = String.Empty;
                                            int lastindex1 = 0, krindex = 0, krcindex = 0;
                                            while (true) {
                                                krindex = Math.Min(c1.IndexOf("%karastart[", krcindex) + 10, Math.Min(c1.IndexOf("%karaend[", krcindex) + 8, c1.IndexOf("%karalength[", krcindex) + 11));
                                                if (krindex == -1 || krindex < krcindex) break;
                                                krcindex = c1.IndexOf("]", krindex);
                                                if (krcindex == -1) break;
                                                string r1 = c1.Substring(krindex, krcindex - krindex);
                                                cnew = cnew + c1.Substring(lastindex1, krindex - lastindex1) + Evaluate.ScriptParse(r1);
                                                lastindex1 = krcindex;
                                            }
                                            if (lastindex1 < c1.Length) cnew += c1.Substring(lastindex1, c1.Length - lastindex1);
                                            conditionOne = new StringBuilder(cnew);

                                            for (krindex = 0; krindex != totKaraNum; krindex+=1) {
                                                conditionOne = conditionOne.Replace("%karastart[" + krindex.ToString(Util.cfi) + "]%", times[krindex, 0].ToString(Util.nfi));
                                                conditionOne = conditionOne.Replace("%karalength[" + krindex.ToString(Util.cfi) + "]%", times[krindex, 1].ToString(Util.nfi));
                                                conditionOne = conditionOne.Replace("%karaend[" + krindex.ToString(Util.cfi) + "]%", times[krindex, 2].ToString(Util.nfi));
                                            }
                                        }
                                    }

                                    conditionMet = Evaluate.Eval(Evaluate.ScriptParse(conditionOne.ToString())).Equals(true);
                                    if (conditionMet == false) break;
                                }

                                if (conditionMet == false) continue;

                                thisFiltCode = new StringBuilder(128);
                                thisFiltCode.Append(thisfilter.Template.Code);

                                if (thisFiltCode.Equals(String.Empty)) break;

                                for (optionIndex = 0; optionIndex != thisfilter.NumOptions; optionIndex+=1) {
                                    thisoption = thisfilter.GetOptionByIndex(optionIndex);
                                    thisFiltCode = thisFiltCode.Replace("$" + thisoption.Name + "$", thisoption.Value);
                                }
                                sFiltCode = thisFiltCode.ToString();
                                if (sFiltCode.Contains("%")) {
                                    //Variables
                                    if (sFiltCode.Contains("%pos%")) {
                                        thisFiltCode = thisFiltCode.Replace("%pos%", dl.GetRect(textstart, textlen).X.ToString(Util.nfi) + "," + dl.GetRect(textstart, textlen).Y.ToString(Util.nfi));
                                    }
                                    thisFiltCode = thisFiltCode.Replace("%karalen%", thisKaraLen.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%karastart%", thisKaraStart.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%karamid%", (thisKaraStart + (thisKaraLen >> 1)).ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%karaend%", thisKaraEnd.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%karanum%", karaIndex.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%karanumtot%", totKaraNum.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%linenum%", lineindex.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%linelen%", (endTime - startTime).ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%linestart%", startTime.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%lineend%", endTime.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%layernumtot%", layerColl.Count.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%layernum%", layerIndex.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%layernumlpstot%", layerNumLPSTot.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%layernumlps%", layerIndexInside.ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%style%", dl.style.name);
                                    thisFiltCode = thisFiltCode.Replace("%name%", dl.actor);
                                    thisFiltCode = thisFiltCode.Replace("%lastkarastart%", (times[Math.Max(0, karaIndex - 2), 0]).ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%nextkaraend%", (times[Math.Min(totKaraNum - 1, karaIndex + 1), 0]).ToString(Util.nfi));
                                    thisFiltCode = thisFiltCode.Replace("%text%", String.IsNullOrEmpty(m.Groups[5].Value) ? string.Empty : m.Groups[5].Value);

                                    if (sFiltCode.IndexOf("%") != -1) { // see if the % is STILL there
                                        string tfc1 = thisFiltCode.ToString();
                                        string tfcnew = String.Empty;
                                        int lastindex1 = 0, krindex = 0, krcindex = 0;
                                        while (true) {
                                            krindex = Math.Min(tfc1.IndexOf("%karastart[", krcindex) + 10, Math.Min(tfc1.IndexOf("%karaend[", krcindex) + 8, tfc1.IndexOf("%karalength[", krcindex) + 11));
                                            if (krindex == -1 || krindex < krcindex) break;
                                            krcindex = tfc1.IndexOf("]", krindex);
                                            if (krcindex == -1) break;
                                            string r1 = tfc1.Substring(krindex, krcindex - krindex);
                                            tfcnew = tfcnew + tfc1.Substring(lastindex1, krindex - lastindex1) + Evaluate.ScriptParse(r1);
                                            lastindex1 = krcindex;
                                        }
                                        if (lastindex1 < tfc1.Length) tfcnew += tfc1.Substring(lastindex1, tfc1.Length - lastindex1);
                                        thisFiltCode = new StringBuilder(tfcnew);

                                        for (krindex = 0; krindex != totKaraNum; krindex+=1) {
                                            thisFiltCode = thisFiltCode.Replace("%karastart[" + krindex.ToString(Util.nfi) + "]%", times[krindex, 0].ToString(Util.nfi));
                                            thisFiltCode = thisFiltCode.Replace("%karalength[" + krindex.ToString(Util.nfi) + "]%", times[krindex, 1].ToString(Util.nfi));
                                            thisFiltCode = thisFiltCode.Replace("%karaend[" + krindex.ToString(Util.nfi) + "]%", times[krindex, 2].ToString(Util.nfi));
                                        }
                                    }
                                }

                                sb.Append(Evaluate.ScriptParse(thisFiltCode.ToString()));
                            } // filterIndex

                            if (thislayer.AddK)
                                sb.Append(m.Groups[1].Value + m.Groups[2].Value);
                            if (thislayer.AddASSA)
                                sb.Append(m.Groups[3].Value);
                            if (thislayer.AddBracket)
                                sb.Append(m.Groups[4].Value);
                            if (thislayer.AddText && m.Groups.Count == 6)
                                sb.Append(m.Groups[5].Value);

                            if (thislayer.SyllablePerLine) {
                                //dl.start = dl.start.Add(TimeSpan.FromMilliseconds(thisKaraStart));
                                //dl.end = dl.start.Add(TimeSpan.FromMilliseconds(thisKaraLen));
                                dl.text = sb.ToString();
                                newList.Add(line);
                                line = (Line)mainline.Clone();
                                dl = (DialogueLine)line.line;
                                sb = new StringBuilder(768);
                            }

                            lastindex = m.Index + m.Length;
                        } // karaIndex
                        if (!thislayer.SyllablePerLine) {
                            if (dl.text.Length > lastindex) sb.Append(dl.text.Substring(lastindex, dl.text.Length - lastindex));
                            dl.text = sb.ToString();
                            newList.Add(line);
                        }
                    } // layerIndexInside
                } // layerIndex
            } // lineindex
            SetLineColl(newList);
            ExtractStyles();
        }

        private void lstEffectOptions_Click(object sender, System.EventArgs e) {
            if ((lstEffectOptions.SelectedItems.Count > 0) && (treeKaraoke.SelectedNode.Level == 1)) {
                InputBoxResult newvalue = (InputBox.Show("Enter a new value for " + lstEffectOptions.SelectedItems[0].Text, "Input", lstEffectOptions.SelectedItems[0].SubItems[1].Text));
                if (newvalue.OK == true) {
                    CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).SetOptionValueByIndex(lstEffectOptions.SelectedItems[0].Index, newvalue.Text);
                    RedrawOptionsList();
                }
            }
            else MessageBox.Show("Select an effect first.");
        }
        private void lstLayerConditions_ItemCheck(object sender, System.Windows.Forms.ItemCheckEventArgs e) {
            //Index of checked item is e.Index
            if (treeKaraoke.SelectedNode != null) {
                Condition c = CurLayer.GetCondition(e.Index);
                c.ConditionEnabled = (e.NewValue == CheckState.Checked);
            }
        }
        private void lstEffectConditions_ItemCheck(object sender, System.Windows.Forms.ItemCheckEventArgs e) {
            //Index of checked item is e.Index
            if (treeKaraoke.SelectedNode.Level == 1) {
                Condition c = CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).GetCondition(e.Index);
                c.ConditionEnabled = (e.NewValue == CheckState.Checked);
            }
            else MessageBox.Show("Select an effect first.");
        }
        private void lstEffectConditions_SubItemEndEditing(object sender, ListViewES.LabelSubEditEndEventArgs e) {
            if (!e.Cancel) {
                Condition c = CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).GetCondition(e.Item.Index);
                switch (e.SubIndex) {
                    case 0:
                        c.ConditionOne = e.Item.Text;
                        break;
                    case 1:
                        c.ConditionOp = e.Item.SubItems[1].Text;

                        break;
                    default:
                        c.ConditionTwo = e.Item.SubItems[2].Text;
                        break;
                }
            }
        }
        private void lstLayerConditions_SubItemEndEditing(object sender, ListViewES.LabelSubEditEndEventArgs e) {
            if (!e.Cancel) {
                Condition c = CurLayer.GetCondition(e.Item.Index);
                switch (e.SubIndex) {
                    case 0:
                        c.ConditionOne = e.Item.Text;
                        break;
                    case 1:
                        c.ConditionOp = e.Item.SubItems[1].Text;
                        break;
                    default:
                        c.ConditionTwo = e.Item.SubItems[2].Text;
                        break;
                }
            }
        }
        private void lstLayerConditions_SubItemClicked(object sender, ListViewES.LabelSubEditEventArgs e) {
            if (e.SubIndex != 1) lstLayerConditions.StartEditing(e.Item, e.SubIndex);
            else lstLayerConditions.StartEditing(e.Item, e.SubIndex, new string[] { "=", "==", "!=", "!==", ">", ">=", "<", "<=" });
        }
        private void lstEffectConditions_SubItemClicked(object sender, ListViewES.LabelSubEditEventArgs e) {
            if (e.SubIndex != 1) lstEffectConditions.StartEditing(e.Item, e.SubIndex);
            else lstEffectConditions.StartEditing(e.Item, e.SubIndex, new string[] { "=", "==", "!=", "!==", ">", ">=", "<", "<=" });
        }

        private void treeKaraoke_AfterLabelEdit(object sender, NodeLabelEditEventArgs e) {
            if (e.Label != null) {
                if (treeKaraoke.SelectedNode.Level == 0) {
                    layerColl[e.Node.Index].Name = (e.Label.Length == 0) ? "Layer " + e.Node.Index.ToString(Util.cfi) : e.Label;
                }
                else {
                    Filter f = layerColl[e.Node.Parent.Index].GetFilter(e.Node.Index);
                    f.Name = (e.Label.Length == 0) ? f.Template.Name : e.Label;
                }
            }
        }
        private void treeKaraoke_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e) {
            if (treeKaraoke.SelectedNode.Level == 0) {
                CurLayer = (Layer)layerColl[e.Node.Index];
                RedrawLayerConditions();
                panelLayer.Visible = !(panelEffect.Visible = false);
            }
            else {
                CurLayer = (Layer)layerColl[e.Node.Parent.Index];
                RedrawOptionsList();
                RedrawEffectConditions();
                panelEffect.Visible = !(panelLayer.Visible = false);
            }
        }
        private void treeKaraoke_ItemDrag(object sender, ItemDragEventArgs e) {
            treeKaraoke.DoDragDrop(e.Item, DragDropEffects.Move);
        }
        private void treeKaraoke_DragDrop(object sender, DragEventArgs e) {
            TreeNode tn_src = (TreeNode)e.Data.GetData(typeof(TreeNode));
            Point p = treeKaraoke.PointToClient(new Point(e.X, e.Y));
            TreeNode tn_dst = treeKaraoke.GetNodeAt(p.X, p.Y);
            int level_src = tn_src.Level;

            if (tn_dst != null) {
                int level_dst = tn_dst.Level;
                int level = (level_src << 1) + level_dst;

                switch (level) {
                    case 0: // layer-layer
                        Layer l5s = (Layer)layerColl[tn_src.Index];

                        if (tn_src.Index < tn_dst.Index) {
                            layerColl.RemoveAt(tn_src.Index);
                            layerColl.Insert(tn_dst.Index, l5s);
                        }
                        else {
                            layerColl.Insert(tn_dst.Index, l5s);
                            layerColl.RemoveAt(tn_src.Index + 1);
                        }
                        break;

                    case 2: // effect-layer
                        if (tn_src.Parent != tn_dst) {
                            Layer l9s = (Layer)layerColl[tn_src.Parent.Index];
                            Layer l9d = (Layer)layerColl[tn_dst.Index];
                            Filter f9 = l9s.GetFilter(tn_src.Index);
                            l9d.AddFilter(f9);
                            l9s.RemoveFilter(tn_src.Index);
                        }

                        break;

                    case 3: //effect-effect
                        Layer l10s = (Layer)layerColl[tn_src.Parent.Index];
                        Filter f10 = l10s.GetFilter(tn_src.Index);

                        if (tn_src.Parent == tn_dst.Parent) {
                            if (tn_src.Index < tn_dst.Index) {
                                l10s.RemoveFilter(tn_src.Index);
                                l10s.InsertFilter(tn_dst.Index, f10);
                            }
                            else {
                                l10s.InsertFilter(tn_dst.Index, f10);
                                l10s.RemoveFilter(tn_src.Index + 1);
                            }
                        }
                        else {
                            Layer l10d = (Layer)layerColl[tn_dst.Parent.Index];
                            l10d.InsertFilter(tn_dst.Index, f10);
                            l10s.RemoveFilter(tn_src.Index);
                        }

                        break;
                }
            }
            else { // Dragging to the end
                if (level_src == 0) {
                    Layer lbs = (Layer)layerColl[tn_src.Index];
                    layerColl.RemoveAt(tn_src.Index);
                    layerColl.Insert(layerColl.Count, lbs);
                }
                else if (tn_src.Index + 1 != layerColl.Count) {
                    Layer lbs = (Layer)layerColl[tn_src.Parent.Index];
                    Layer lbd = (Layer)layerColl[layerColl.Count - 1];
                    Filter fb = lbs.GetFilter(tn_src.Index);
                    lbd.AddFilter(fb);
                    lbs.RemoveFilter(tn_src.Index);
                }
            }
            RedrawKaraEffectsTree();
        }
        private void treeKaraoke_DragEnter(object sender, DragEventArgs e) {
            e.Effect = DragDropEffects.Move;
        }
        private void treeKaraoke_AfterCheck(object sender, TreeViewEventArgs e) {
            if (e.Node.Level == 0)
                ((Layer)layerColl[e.Node.Index]).Enabled = e.Node.Checked;
            else
                ((Layer)layerColl[e.Node.Parent.Index]).GetFilter(e.Node.Index).Enabled = e.Node.Checked;
        }
        private void EventContext() {
            ContextMenu effectcm = new ContextMenu();
            MenuItem mi;
            for (int index = 0; index != menuItem15.MenuItems.Count; index+=1) {
                mi = menuItem15.MenuItems[index];
                effectcm.MenuItems.Add(mi.Index, mi.CloneMenu());
            }
            cmdAddEffect.ContextMenu = effectcm;
        }
        private void RedrawLayerConditions() {
            lstLayerConditions.Items.Clear();
            if (treeKaraoke.SelectedNode != null) {
                string[] cond;
                ListViewItem lvi;

                for (int index = 0; index != CurLayer.ConditionCount; index+=1) {
                    cond = new string[3];
                    cond[0] = CurLayer.GetCondition(index).ConditionOne;
                    cond[1] = CurLayer.GetCondition(index).ConditionOp;
                    cond[2] = CurLayer.GetCondition(index).ConditionTwo;
                    lvi = new ListViewItem(cond);
                    lvi.Checked = CurLayer.GetCondition(index).ConditionEnabled;
                    lstLayerConditions.Items.Add(lvi);
                }

                chkLayerPerSyllable.Checked = CurLayer.PerSyllable;
                chkSyllablePerLine.Checked = CurLayer.SyllablePerLine;
                chkAddAnyway.Checked = CurLayer.AddAll;
                chkAddOnce.Checked = CurLayer.AddOnce;
                chkKaraAddK.Checked = CurLayer.AddK;
                chkKaraAddText.Checked = CurLayer.AddText;
                chkKaraAddASSA.Checked = CurLayer.AddASSA;
                chkKaraAddClosingBracket.Checked = CurLayer.AddBracket;

                textLayerRepetitions.Enabled = !CurLayer.PerSyllable;
                textLayerRepetitions.Text = CurLayer.Repetitions.ToString(Util.cfi);
            }
        }
        private void RedrawEffectConditions() {
            lstEffectConditions.Items.Clear();
            if (treeKaraoke.SelectedNode.Level == 1) {
                string[] cond;
                Condition c;
                ListViewItem lvi;

                for (int index = 0; index != CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).ConditionCount; index+=1) {
                    c = CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).GetCondition(index);
                    cond = new string[3];
                    cond[0] = c.ConditionOne;
                    cond[1] = c.ConditionOp;
                    cond[2] = c.ConditionTwo;
                    lvi = new ListViewItem(cond);
                    lvi.Checked = c.ConditionEnabled;
                    lstEffectConditions.Items.Add(lvi);
                }
            }
        }
        private void RedrawKaraEffectsTree() {
            TreeNode tnl, tnf;
            Layer l;
            Filter f;
            int lindex, findex;

            treeKaraoke.BeginUpdate();
            treeKaraoke.Nodes.Clear();
            for (lindex = 0; lindex != layerColl.Count; lindex+=1) {
                l = layerColl[lindex];
                tnl = new TreeNode(l.Name == null ? "Layer " + lindex.ToString(Util.cfi) : l.Name);
                for (findex = 0; findex != l.Count; findex+=1) {
                    f = l.GetFilter(findex);
                    tnf = new TreeNode(f.Name == null ? f.Template.Name : f.Name);
                    tnf.Checked = f.Enabled;
                    tnl.Nodes.Add(tnf);
                }
                tnl.Checked = l.Enabled;
                treeKaraoke.Nodes.Add(tnl);
            }
            treeKaraoke.ExpandAll();
            treeKaraoke.EndUpdate();
        }
        private void RedrawOptionsList() {
            if (treeKaraoke.SelectedNode.Level == 1) {
                lstEffectOptions.Items.Clear();
                string[] opt = new string[2];
                ListViewItem lvi;
                int treeindex = treeKaraoke.SelectedNode.Index;
                bool foundOption = false;

                Filter tc = CurLayer.GetFilter(treeindex);

                Filter tf = tc.Template;

                for (int index = 0; index != tf.NumOptions; index+=1) {
                    opt[0] = (tf.GetOptionByIndex(index)).Name;

                    for (int optionIndex = 0; optionIndex != CurLayer.GetFilter(treeKaraoke.SelectedNode.Index).NumOptions; optionIndex+=1) {
                        if ((tc.GetOptionByIndex(optionIndex)).Name == (tf.GetOptionByIndex(index)).Name) {
                            foundOption = true;
                            opt[1] = (tc.GetOptionByIndex(optionIndex)).Value;
                            break;
                        }
                    }

                    if (foundOption == false)
                        opt[1] = (tc.GetOptionByIndex(index)).Value;

                    lvi = new ListViewItem(opt);
                    lstEffectOptions.Items.Add(lvi);
                }
            }
        }
        private Filter GetTemplateFilterFromName(string name) {
            return templateFilterColl.Find(delegate(Filter tf) { return String.Equals(tf.Name, name, StringComparison.Ordinal); });
        }
        #endregion

        #region Gradient
        private void cmdGradMoveBefore_Click(object sender, System.EventArgs e) {
            txtGradBefore.Text = txtGradIn.SelectedText;
            txtGradIn.SelectedText = String.Empty;
        }
        private void cmdGradListToInput_Click(object sender, System.EventArgs e) {
            int index = GetFirstSelected();
            if (index == -1)
                MessageBox.Show("Select an item in the list first.");
            else
                txtGradIn.Text = lineColl[index].ToString();
        }
        private void cmdGradReplaceLine_Click(object sender, System.EventArgs e) {
            TextToList(txtGradOut, ListInsert.ReplaceLine);
        }
        private void cmdGradInsertBefore_Click(object sender, System.EventArgs e) {
            TextToList(txtGradOut, ListInsert.BeforeLine);
        }
        private void cmdGradInsertAfter_Click(object sender, System.EventArgs e) {
            TextToList(txtGradOut, ListInsert.AfterLine);
        }
        private void cmdGradDoGrad_Click(object sender, System.EventArgs e) {
            Regex rcolor = new Regex(@"H?[0-9a-fA-F]{1,8}$");
            double clines, expbase;
            Point startpos,endpos,offset;
            int mirrorint;

            if (txtGradIn.TextLength == 0)
                MessageBox.Show("Enter the text to be gradiented.");
            else if (!double.TryParse(txtGradCLines.Text, System.Globalization.NumberStyles.Integer, Util.nfi, out clines))
                MessageBox.Show("Enter the number of lines to generate.");
            else if (txtGradStartBGR.Text == txtGradEndBGR.Text)
                MessageBox.Show("Start BGR can't equal end BGR.");
            else if (!Util.TryParseCoordinate(txtGradStartPos.Text,out startpos))
                MessageBox.Show("Enter a valid start position.");
            else if (!Util.TryParseCoordinate(txtGradEndPos.Text,out endpos))
                MessageBox.Show("Enter a valid end position.");
            else if (!Util.TryParseCoordinate(txtGradOffset.Text,out offset))
                MessageBox.Show("Enter a valid end offset (0,0 for none).");
            else if (!(double.TryParse(txtGradBase.Text, System.Globalization.NumberStyles.AllowDecimalPoint, Util.nfi, out expbase)) || (expbase < 1) || (expbase == 1 && radGradLog.Checked))
                MessageBox.Show("Enter a valid base (greater than 1).");
            else if ((!int.TryParse(txtGradMirror.Text, System.Globalization.NumberStyles.Integer, Util.nfi, out mirrorint)) || (mirrorint < 1) || (mirrorint > 100))
                MessageBox.Show("Enter a valid number for mirroring (1-100). 100 disables mirroring. See the Readme for more info.");
            else if ((txtGradStartBGR.TextLength == 0) || (!rcolor.IsMatch(txtGradStartBGR.Text)))
                MessageBox.Show("Enter a valid start BGR.");
            else if ((txtGradEndBGR.TextLength == 0) || (!rcolor.IsMatch(txtGradEndBGR.Text)))
                MessageBox.Show("Enter a valid end BGR.");

            else {
                double curx, cury, cura, curr, curg, curb, groupx, groupy, ratio;
                int clinesnow;
                try {
                    double mirror = mirrorint / 100.0;
                    StringBuilder sb = new StringBuilder(4096);

                    int startx = startpos.X;
                    int starty = startpos.Y;
                    int endx = endpos.X;
                    int endy = endpos.Y;
                    int offx = offset.X;
                    int offy = offset.Y;

                    uint startbgr = 0, endbgr = 0;

                    startbgr = Util.ReadColor(txtGradStartBGR.Text);
                    endbgr = Util.ReadColor(txtGradEndBGR.Text);

                    byte starta = (byte)((startbgr >> 24) & 0xFF); // alpha comes first
                    byte startb = (byte)((startbgr >> 16) & 0xFF); //because the first 8 bits are B, they need to be shifted over 16 bits to get B alone
                    byte startg = (byte)((startbgr >> 8) & 0xFF); //because the second 8 bits are B, they need to be shifted over 8 bits to get G alone
                    byte startr = (byte)(startbgr & 0xFF); // and with 255 to get the BG out of there
                    byte enda = (byte)((endbgr >> 24) & 0xFF);
                    byte endb = (byte)((endbgr >> 16) & 0xFF); //because the first 8 bits are B, they need to be shifted over 16 bits to get B alone
                    byte endg = (byte)((endbgr >> 8) & 0xFF); //because the second 8 bits are B, they need to be shifted over 8 bits to get G alone
                    byte endr = (byte)(endbgr & 0xFF); // and with 255 to get the BG out of there
                    bool usealpha = (starta > 0) || (enda > 0);
                    bool evenlines = (clines % 2 == 0);

                    groupx = (double)(endx - startx) / (clines);
                    groupy = (double)(endy - starty) / (clines);
                    curx = startx;
                    cury = starty;
                    cura = (double)starta;
                    curr = (double)startr;
                    curg = (double)startg;
                    curb = (double)startb;
                    clinesnow = Convert.ToInt32(Math.Ceiling(clines * mirror), Util.nfi);

                    for (int mirrorindex = 0; mirrorindex != (mirror == 1 ? 1 : 2); mirrorindex+=1) {
                        for (int index = (mirrorindex == 0 || evenlines ? 0 : 1); index != clinesnow; index+=1) {
                            ratio = (double)index / (clinesnow - 1);
                            if (mirrorindex == 1) ratio = 1.0 - ratio;

                            if (radGradPoly.Checked) {
                                cura = Math.Pow(ratio, expbase) * (double)(enda - starta);
                                curr = Math.Pow(ratio, expbase) * (double)(endr - startr);
                                curg = Math.Pow(ratio, expbase) * (double)(endg - startg);
                                curb = Math.Pow(ratio, expbase) * (double)(endb - startb);
                            }
                            else if (radGradLog.Checked) {
                                cura = Math.Log(1 + ((expbase - 1) * ratio), expbase) * (double)(enda - starta);
                                curr = Math.Log(1 + ((expbase - 1) * ratio), expbase) * (double)(endr - startr);
                                curg = Math.Log(1 + ((expbase - 1) * ratio), expbase) * (double)(endg - startg);
                                curb = Math.Log(1 + ((expbase - 1) * ratio), expbase) * (double)(endb - startb);
                            }
                            else {
                                cura = Math.Pow(expbase, ratio) / expbase * (double)(enda - starta);
                                curr = Math.Pow(expbase, ratio) / expbase * (double)(endr - startr);
                                curg = Math.Pow(expbase, ratio) / expbase * (double)(endg - startg);
                                curb = Math.Pow(expbase, ratio) / expbase * (double)(endb - startb);
                            }

                            cura = (double)starta + cura;
                            curr = (double)startr + curr;
                            curg = (double)startg + curg;
                            curb = (double)startb + curb;

                            sb.Append(txtGradBefore.Text);
                            sb.Append(String.Format("{{\\clip({0},{1},{2},{3})",
                                (int)Math.Round(curx, 0), (int)Math.Round(cury, 0),
                                (int)Math.Round(curx + groupx, 0), (int)Math.Round(cury + groupy, 0)));

                            if (chkGrad1c.Checked) GradFormatter(sb, 1, usealpha, cura, curr, curg, curb);
                            if (chkGrad2c.Checked) GradFormatter(sb, 2, usealpha, cura, curr, curg, curb);
                            if (chkGrad3c.Checked) GradFormatter(sb, 3, usealpha, cura, curr, curg, curb);
                            if (chkGrad4c.Checked) GradFormatter(sb, 4, usealpha, cura, curr, curg, curb);

                            sb.AppendLine("}" + txtGradIn.Text);

                            curx += groupx;
                            cury += groupy;
                        }
                        if (mirror != 1.0) clinesnow = Convert.ToInt32(Math.Ceiling(clines * (1 - mirror)),Util.nfi);
                    }


                    txtGradOut.Text = sb.ToString().TrimEnd();
                } catch { MessageBox.Show("An error occured. Check that your input parameters are correct."); }

            }
        }
        private void GradFormatter(StringBuilder sb, int c, bool usealpha, double a, double r, double g, double b) {
            sb.Append(String.Format("\\{0}c&H{4}{1:X2}{2:X2}{3:X2}&", 
                        c, (int)Math.Round(b, 0), (int)Math.Round(g, 0), (int)Math.Round(r, 0),
                        (usealpha?Convert.ToString(Convert.ToInt32(Math.Round(a, 0), Util.nfi), 16):String.Empty)));
        }
        #endregion

        #region Blur
        private void chkBlur1a_CheckedChanged(object sender, System.EventArgs e) {
            chkBlur1aSub.Enabled = !chkBlur1a.Checked;

        }
        private void chkBlur2a_CheckedChanged(object sender, System.EventArgs e) {
            chkBlur2aSub.Enabled = !chkBlur2a.Checked;
        }
        private void chkBlur3a_CheckedChanged(object sender, System.EventArgs e) {
            chkBlur3aSub.Enabled = !chkBlur3a.Checked;
        }
        private void chkBlur4a_CheckedChanged(object sender, System.EventArgs e) {
            chkBlur4aSub.Enabled = !chkBlur4a.Checked;
        }
        private void radBlurHV_CheckedChanged(object sender, System.EventArgs e) {
            if (radBlurHV.Checked) {
                txtBlurLines.Enabled = false;
                txtBlurPos.Enabled = true;
                groupBlurSub.Enabled = true;
                groupBlurAffect.Enabled = true;
                label34.Text = "H. Blur Radius";
                txtBlurRadV.Enabled = true;
            }
        }
        private void radBlurGlow_CheckedChanged(object sender, System.EventArgs e) {
            if (radBlurGlow.Checked) {
                txtBlurLines.Enabled = true;
                txtBlurPos.Enabled = false;
                groupBlurSub.Enabled = false;
                groupBlurAffect.Enabled = false;
                label34.Text = "Blur Radius";
                txtBlurRadV.Enabled = false;
            }
        }

        private void cmdBlurMoveBefore_Click(object sender, System.EventArgs e) {
            txtBlurBefore.Text = txtBlurIn.SelectedText;
            txtBlurIn.SelectedText = String.Empty;
        }
        private void cmdBlurListToInput_Click(object sender, System.EventArgs e) {
            int index = GetFirstSelected();
            if (index == -1)
                MessageBox.Show("Select an item in the list first.");
            else
                txtBlurIn.Text = lineColl[index].ToString();
        }
        private void cmdBlurInsertBefore_Click(object sender, System.EventArgs e) {
            TextToList(txtBlurOut, ListInsert.BeforeLine);
        }
        private void cmdBlurInsertAfter_Click(object sender, System.EventArgs e) {
            TextToList(txtBlurOut, ListInsert.AfterLine);
        }
        private void cmdBlurReplaceLine_Click(object sender, System.EventArgs e) {
            TextToList(txtBlurOut, ListInsert.ReplaceLine);
        }
        private void cmdDoBlur_Click(object sender, System.EventArgs e) {
            double expbase;
            int BlurLines, BlurRadH, BlurRadV=1;

            if (txtBlurIn.Text.Contains("\\bord"))
                MessageBox.Show("Input text must not have a border.");
            else if (txtBlurIn.TextLength == 0)
                MessageBox.Show("Enter an input line first.");
            else if (!int.TryParse(txtBlurLines.Text, System.Globalization.NumberStyles.Integer, Util.nfi, out BlurLines))
                MessageBox.Show("Enter a valid number of lines to blur.");
            else if ((!int.TryParse(txtBlurRadH.Text, System.Globalization.NumberStyles.Integer, Util.nfi, out BlurRadH)) || (BlurRadH < 0)
                    || ((radBlurHV.Checked) && ((!int.TryParse(txtBlurRadV.Text, System.Globalization.NumberStyles.Integer, Util.nfi, out BlurRadV)) || (BlurRadV < 0))))
                MessageBox.Show("Enter a valid border.");
            else if (!double.TryParse(txtBlurBase.Text, System.Globalization.NumberStyles.AllowDecimalPoint, Util.nfi, out expbase) || (expbase < 1))
                MessageBox.Show("Enter a valid base (>= 1).");

            else {
                StringBuilder sb = new StringBuilder(4096);
                double curblur, ratio;
                uint BlurStart = 0, BlurEnd = 0, BlurDiff;

                try {
                    BlurStart = Util.ReadColor(txtBlurStartAlpha.Text);
                    BlurEnd = Util.ReadColor(txtBlurEndAlpha.Text);

                    expbase = Math.Min(1, double.Parse(txtBlurBase.Text));
                } catch {
                    MessageBox.Show("There was an error parsing one of your values.");
                    return;
                }

                BlurDiff = BlurEnd - BlurStart;

                if ((BlurStart > 255) || (BlurEnd > 255))
                    MessageBox.Show("Blur must be from 0 to 255 (H0 to HFF)");

                else if (expbase <= 1 && radBlurLog.Checked)
                    MessageBox.Show("Enter a valid base (greater than 1)");

                else if (radBlurGlow.Checked) {
                    double currad;
                    curblur = BlurStart;
                    for (int blurindexhv = 1; blurindexhv <= BlurLines; blurindexhv+=1) {
                        currad = (blurindexhv * ((double)BlurRadH / BlurLines));

                        sb.Append(String.Format("{0}{{\\bord{1}\\3a&H{2:X6}&}}{3}", txtBlurBefore.Text,
                            (int)Math.Round(currad, 1), (int)Math.Round(curblur, 0), txtBlurIn.Text));

                        if (blurindexhv != BlurLines) sb.AppendLine();

                        ratio = (double)blurindexhv / (double)(BlurLines - 1);

                        if (radBlurPoly.Checked) curblur = BlurStart + Math.Pow(ratio, expbase) * (double)BlurDiff;
                        else if (radBlurLog.Checked) curblur = (double)BlurStart + Math.Log(1 + ((expbase - 1) * ratio), expbase) * ((double)BlurDiff);
                        else curblur = (double)BlurStart + Math.Pow(expbase, ratio) / expbase * (double)BlurDiff;
                    }
                    txtBlurOut.Text = sb.ToString();
                }

                else {
                    Point p;

                    if (txtBlurIn.Text.Contains("\\pos"))
                        MessageBox.Show("Input must not contain \\pos");

                    else if (Util.TryParseCoordinate(txtBlurPos.Text,out p)) {
                        double distance;
                        double maxdistance = Math.Sqrt(BlurRadH * BlurRadH + BlurRadV * BlurRadV);

                        for (int blurindexhvh = -1 * BlurRadH; blurindexhvh <= BlurRadH; blurindexhvh+=1) {
                            for (int blurindexhvv = -1 * BlurRadV; blurindexhvv <= BlurRadV; blurindexhvv+=1) {
                                distance = (double)Math.Sqrt((blurindexhvh * blurindexhvh) + (blurindexhvv * blurindexhvv));

                                if (radBlurPoly.Checked) curblur = (double)BlurStart + Math.Pow(distance / maxdistance, expbase) * (double)BlurDiff;
                                else if (radBlurLog.Checked) curblur = (double)BlurStart + Math.Log(1 + ((expbase - 1) * (distance / maxdistance)), expbase) * (double)BlurDiff;
                                else curblur = (double)BlurStart + Math.Pow(expbase, distance / maxdistance) / expbase * (double)BlurDiff;
                                
                                sb.Append(String.Format("{0}{{\\pos({1},{2})\\bord1", txtBlurBefore.Text, 
                                            p.X + blurindexhvh, p.Y + blurindexhvv));

                                if (chkBlur1a.Checked) BlurFormatter(sb, 1, curblur);
                                else if ((distance != 0) && (chkBlur1aSub.Checked)) sb.Append("\\1a&HFF&");
                                if (chkBlur2a.Checked) BlurFormatter(sb, 2, curblur);
                                else if ((distance != 0) && (chkBlur2aSub.Checked)) sb.Append("\\2a&HFF&");
                                if (chkBlur3a.Checked) BlurFormatter(sb, 3, curblur);
                                else if ((distance != 0) && (chkBlur3aSub.Checked)) sb.Append("\\3a&HFF&");
                                if (chkBlur4a.Checked) BlurFormatter(sb, 4, curblur);
                                else if ((distance != 0) && (chkBlur4aSub.Checked)) sb.Append("\\4a&HFF&");

                                sb.AppendLine("}" + txtBlurIn.Text);
                            }
                        }

                        txtBlurOut.Text = sb.ToString().TrimEnd();
                    }
                    else
                        MessageBox.Show("Enter a valid position.");
                }
            }
        }
        private void BlurFormatter(StringBuilder sb, int anum, double a) {

            sb.Append(String.Format("\\{0}&H{1:X2}&", anum, (int)Math.Round(a, 0)));
        }
        #endregion

        #region Manual Transform
        private List<TimeSpan> TransformTimes;
        private void cmdTransformAddTime_Click(object sender, System.EventArgs e) {
            double fr;
            if (double.TryParse(cmbFR.Text,System.Globalization.NumberStyles.AllowDecimalPoint,Util.nfi,out fr)) {
                try {
                    if (listTransformTimes.Items.Count == 0) TransformTimes = new List<TimeSpan>(4);

                    // how often to add a frame. just divide the framerate by it. default 1.
                    int precision;
                    if (!int.TryParse(txtTransformPrecision.Text,System.Globalization.NumberStyles.Integer,Util.nfi,out precision)) {
                        txtTransformPrecision.Text = "1";
                        precision = 1;
                    }

                    TimeSpan t = TimeSpan.Parse(maskedTextTransformTime.Text);
                    TimeSpan halfframe = TimeSpan.FromSeconds(0.5 / fr);
                    fr = fr / Convert.ToDouble(precision,Util.nfi);
                    t = TimeSpan.FromSeconds(Math.Round(t.TotalSeconds * fr, 0) / fr).Subtract(halfframe);

                    //maskedTextTransformTime.Text = (Util.Precision==2?"0":String.Empty) + Util.TimeSpanSSA(t, false);

                    int index = 0;
                    while ((index < TransformTimes.Count) && ((TransformTimes[index]).CompareTo(t) != 1)) index+=1;

                    listTransformTimes.Items.Insert(index, maskedTextTransformTime.Text);

                    TransformTimes.Insert(index, t);
                    index <<= 1;
                    if (TransformTimes.Count != 1) {
                        if (index == 0)
                            listTransformVars.Columns.Insert(1, "Acceleration", 100, HorizontalAlignment.Left);
                        else
                            listTransformVars.Columns.Insert(index, "Acceleration", 100, HorizontalAlignment.Left);
                    }
                    listTransformVars.Columns.Insert(index + 1, "Val at " + maskedTextTransformTime.Text, 128, HorizontalAlignment.Left);
                    listTransformVars.RecopyListItems();

                } catch (Exception exc) {
                    MessageBox.Show(exc.Message);
                }
            }
            else MessageBox.Show("Enter a valid framerate.");
        }
        private void cmdTransformDelTime_Click(object sender, System.EventArgs e) {
            if (listTransformTimes.SelectedItems.Count == 1) {
                try {
                    int index = listTransformTimes.SelectedIndex;
                    TransformTimes.RemoveAt(index);
                    listTransformTimes.Items.RemoveAt(index);

                    if (listTransformVars.Columns.Count > index) {
                        listTransformVars.Columns.RemoveAt((index << 1) + 1);
                        if (index != 0)
                            listTransformVars.Columns.RemoveAt(index << 1);
                        else if (TransformTimes.Count != 0)
                            listTransformVars.Columns.RemoveAt((index << 1) + 1);
                    }
                    listTransformVars.RecopyListItems();
                } catch (Exception exc) {
                    MessageBox.Show(exc.Message);
                }
            }
            else MessageBox.Show("Select a time first.");

        }
        private void cmdTransformNewVar_Click(object sender, System.EventArgs e) {
            InputBoxResult newvalue = InputBox.Show("Enter the name of the variable:", "Input", String.Empty);
            if (newvalue.OK == true) {
                string[] news = new string[listTransformVars.Columns.Count];
                news[0] = newvalue.Text;
                for (int index = 1; index != news.Length; index+=1)
                    news[index] = String.Empty;
                listTransformVars.Items.Add(new ListViewItem(news));
            }
        }
        private void cmdTransformDelVar_Click(object sender, System.EventArgs e) {
            if (listTransformVars.SelectedItems.Count == 1) {
                listTransformVars.Items.RemoveAt(listTransformVars.SelectedItems[0].Index);
            }
            else MessageBox.Show("Select an item first.");
        }
        private void cmdTransformListToInput_Click(object sender, System.EventArgs e) {
            int index = GetFirstSelected();
            if (index == -1)
                MessageBox.Show("Select an item in the list first.");
            else
                txtTransformCode.Text = lineColl[index].ToString();
        }
        private void cmdTransformInsertBefore_Click(object sender, System.EventArgs e) {
            TextToList(txtTransformOut, ListInsert.BeforeLine);
        }
        private void cmdTransformInsertAfter_Click(object sender, System.EventArgs e) {
            TextToList(txtTransformOut, ListInsert.AfterLine);
        }
        private void cmdTransformReplaceLine_Click(object sender, System.EventArgs e) {
            TextToList(txtTransformOut, ListInsert.ReplaceLine);
        }
        private void cmdDoTransform_Click(object sender, System.EventArgs e) {
            double fr, precision;

            if (listTransformTimes.Items.Count < 2)
                MessageBox.Show("Enter at least two times.");
            else if (listTransformVars.Items.Count == 0)
                MessageBox.Show("Enter at least one variable.");
            else if (txtTransformCode.Text.Length == 0)
                MessageBox.Show("Enter the transform code.");
            else if (!double.TryParse(cmbFR.Text,System.Globalization.NumberStyles.AllowDecimalPoint,Util.nfi,out fr))
                MessageBox.Show("Enter a valid framerate.");
            else if (!double.TryParse(txtTransformPrecision.Text,System.Globalization.NumberStyles.Integer,Util.nfi,out precision))
                MessageBox.Show("Enter a valid transform precision.");
            else {
                txtTransformOut.Text = ManualTransform.DoTransform(TransformTimes, ManualTransform.ListViewToMTV(listTransformVars),
                    txtTransformCode.Text, fr/precision);
            }
        }
        #endregion

        #region List control
        private void cmdSelAll_Click(object sender, System.EventArgs e) {
            lineColl[1] = lineColl[1];
            int index;
            for (index = 0; index != lstStyles.Items.Count; index+=1)
                lstStyles.SetItemChecked(index, true);
            for (index = 0; index != styleColl.Count; index+=1)
                styleColl[index].enabled = true;
        }
        private void cmdDeselAll_Click(object sender, System.EventArgs e) {
            int index;
            for (index = 0; index != lstStyles.Items.Count; index+=1)
                lstStyles.SetItemChecked(index, false);
            for (index = 0; index != styleColl.Count; index+=1)
                styleColl[index].enabled = false;
        }
        private void cmdSSASelall_Click(object sender, System.EventArgs e) {
            for (int index = 0; index != lineColl.Count; index+=1)
                lineColl[index].enabled = true;
        }
        private void cmdSSADeselall_Click(object sender, System.EventArgs e) {
            for (int index = 0; index != lineColl.Count; index+=1)
                lineColl[index].enabled = false;
        }
        private void cmdSSAInvertSel_Click(object sender, System.EventArgs e) {
            for (int index = 0; index != lineColl.Count; index+=1) {
                listSSA.Items[index].Checked = !listSSA.Items[index].Checked;
                lineColl[index].enabled = !lineColl[index].enabled;
            }
        }

        private void listSSA_DoubleClick(object sender, EventArgs e) {
            menuChangeLine_Click(this, null);
        }
        private void DrawListOnScreen() {
            listSSA.Invalidate();
            listSSA.Update();
        }
        private void listSSA_SelectedIndexChanged(object sender, System.EventArgs e) {
            labelSelLine.Text = (GetFirstSelected() + 1).ToString(Util.cfi);
        }
        private void listSSA_ItemCheck(object sender, ItemCheckEventArgs e) {
            if (lineColl != null && e.Index < lineColl.Count)
                lineColl[e.Index].enabled = e.NewValue == CheckState.Checked;
        }
        private void lstStyles_ItemCheck(object sender, ItemCheckEventArgs e) {
            if (styleColl != null && e.Index < styleColl.Count)
                styleColl[e.Index].enabled = e.NewValue == CheckState.Checked;
        }

        private void listSSA_DrawItem(object sender, DrawListViewItemEventArgs e) {
            if ((e.ItemIndex >= listSSA.TopItem.Index) && (e.ItemIndex < listSSA.ItemsOnScreen + listSSA.TopItem.Index)) {
                Rectangle bound = e.Bounds;
                if (e.Item.Selected)
                    e.Graphics.FillRectangle(new SolidBrush(Color.FromKnownColor(KnownColor.Highlight)), 0, e.Item.Position.Y, bound.Width, bound.Height);
                else if (e.Item.BackColor == Color.LightGreen) // For LineType.error
                    e.Graphics.FillRectangle(new SolidBrush(Color.LightGreen), 0, e.Item.Position.Y, bound.Width, bound.Height);

                //else e.DrawBackground(); //clear it all on scroll instead
                Point il = bound.Location;
                Point p = new Point(il.X + 2, il.Y + 1);
                int offset = p.X + p.X + 14;
                Rectangle rc = new Rectangle(bound.X + offset, bound.Y + 1, bound.Width - offset, bound.Height - 1);
                ControlPaint.DrawCheckBox(e.Graphics, new Rectangle(p, new Size(14, 14)), (e.Item.Checked ? ButtonState.Checked : ButtonState.Normal) | ButtonState.Flat);
                e.Graphics.DrawString(e.Item.Text, listSSA.Font, new SolidBrush(e.Item.Selected ? Color.FromKnownColor(KnownColor.HighlightText) : listSSA.ForeColor), rc, StringFormat.GenericTypographic);
            }
        }
        /*
        private void listSSA_DrawItem(int index) {
            ListViewItem lvi;
            DrawListViewItemEventArgs dlvi;
            RetrieveVirtualItemEventArgs rlvi;
            rlvi = new RetrieveVirtualItemEventArgs(index);
            listSSA_RetrieveVirtualItem(this, rlvi);
            lvi = rlvi.Item;
            dlvi = new DrawListViewItemEventArgs(listSSAg, lvi, lvi.Bounds, //listSSA.GetItemRect(index),
                index, ListViewItemStates.Default);

            listSSA_DrawItem(this, dlvi);
        }
         */
        private void listSSA_KeyPress(object sender, KeyPressEventArgs e) {
            if (e.KeyChar == 32) {
                for (int index = 0; index != lineColl.Count; index+=1) {
                    if (lineColl[index].selected) lineColl[index].enabled = !lineColl[index].enabled;
                }
                DrawListOnScreen();
                e.Handled = true;
            }
        }
        private void listSSA_KeyUp(object sender, KeyEventArgs e) {
            if (e.KeyCode == Keys.Delete) {
                menuRemoveLine_Click(this, null);
                e.Handled = true;
            }
        }
        private void listSSA_OnScroll(object sender, ScrollEventArgs e) {
            if (e.ScrollOrientation == ScrollOrientation.HorizontalScroll) {
                Graphics.FromHwnd(listSSA.Handle).Clear(Color.FromKnownColor(KnownColor.Window));
                DrawListOnScreen();
            }
            /*
            listSSA.BeginUpdate();
        
            DrawListOnScreen();
            listSSA.EndUpdate();
        */
        }
        private void listSSA_MouseUp(object sender, MouseEventArgs e) {
            if (e.X > 1 && e.X < 16) {
                ListViewItem lvi = listSSA.GetItemAt(e.X, e.Y);
                if (lvi != null) {
                    lineColl[lvi.Index].enabled = !lineColl[lvi.Index].enabled;
                    listSSA_DrawItem(this, new DrawListViewItemEventArgs(listSSAg, lvi, lvi.Bounds, lvi.Index, ListViewItemStates.Default));
                }
            }
        }
        private void listSSA_VirtualItemsSelectionRangeChanged(object sender, ListViewVirtualItemsSelectionRangeChangedEventArgs e) {
            int end = (e.EndIndex == lineColl.Count) ? lineColl.Count : e.EndIndex + 1;
            for (int index = 0; index != lineColl.Count; index+=1)
                lineColl[index].selected = (index >= e.StartIndex && index <= end) ? e.IsSelected : !e.IsSelected;
        }
        private void listSSA_ItemSelectionChanged(object sender, ListViewItemSelectionChangedEventArgs e) {
            lineColl[e.ItemIndex].selected = e.IsSelected;
            labelSelLine.Text = e.ItemIndex.ToString(Util.cfi);
        }
        private void SetLineColl(List<Line> newList) {
            undoStack.Push(lineColl);
            menuUndo.Enabled = true;
            listSSA.VirtualListSize = 0;
            lineColl = newList;
            listSSA.VirtualListSize = lineColl.Count;
            ExtractStyles();
        }
        private int GetFirstSelected() {
            return (listSSA.SelectedIndices.Count != 0) ? listSSA.SelectedIndices[0] : -1;
        }
        private ListViewItem GetListItem(int index) {
            Line l = lineColl[index];
            ListViewItem lvi = new ListViewItem();
            string LineText = l.ToString();

            //ListViewItem crashes if the text is exactly (not >=) 260 characters long.
            if (LineText.Length == 260)
                lvi.Text = LineText + " "; // make it not 260 characters; only affects the display of the line
            else
                lvi.Text = LineText;

            lvi.Checked = l.enabled;
            lvi.Selected = l.selected;
            if (l.lineType == LineType.error) lvi.BackColor = Color.LightGreen;
            return lvi;
        }
        private void listSSA_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e) {
                e.Item = GetListItem(e.ItemIndex);
        }

        private void ReparseErrors() {
            for(int i=0;i!=lineColl.Count;i+=1)
                if (lineColl[i].lineType==LineType.error)
                    lineColl[i] = Line.ParseLine(lineColl[i].ToString());
        }
        #endregion

        #region Undo/Redo
        private void UndoRedo(Stack source, Stack dest) {
            if (source != null && source.Count != 0) {
                listSSA.BeginUpdate();
                object undo = source.Pop();
                LineUndo lu;
                if (undo is List<Line>) { // New list
                    dest.Push(lineColl);
                    lineColl = (List<Line>)undo;
                }
                else if (undo is List<int>) { // Added multiple lines
                    List<LineUndo> redolu = new List<LineUndo>();
                    List<int> undointlist = (List<int>)undo;
                    int rindex;
                    for (int index = 0; index != undointlist.Count; index+=1) {
                        lu = new LineUndo();
                        rindex = undointlist[index];
                        lu.index = rindex;
                        lu.line = lineColl[rindex];
                        redolu.Add(lu);
                        lineColl.RemoveAt(rindex);
                    }
                    dest.Push(redolu);
                }
                else if (undo is List<LineUndo>) { // Deleted multiple lines
                    List<int> redoint = new List<int>();
                    List<LineUndo> undolulist = (List<LineUndo>)undo;
                    for (int index = 0; index != undolulist.Count; index+=1) {
                        lu = (LineUndo)undolulist[index];
                        redoint.Add(lu.index);
                        lineColl.Insert(lu.index, lu.line);
                    }
                    dest.Push(redoint);
                }
                else if (undo is int) { // added one line
                    lu.index = (int)undo;
                    lu.line = lineColl[lu.index];
                    lineColl.RemoveAt(lu.index);
                    dest.Push(lu);
                }
                else if (undo is LineUndo) { // deleted one line
                    lu = (LineUndo)undo;
                    dest.Push(lu.index);
                    lineColl.Insert(lu.index, lu.line);
                }
                else if (undo is LineCUndo) { // Changed a line
                    LineCUndo luold = (LineCUndo)undo;
                    Line lold = lineColl[luold.index];
                    lineColl[luold.index] = luold.line;
                    luold.line = lold;
                    dest.Push(luold);
                }
                listSSA.VirtualListSize = lineColl.Count;
                listSSA.EndUpdate();
                menuUndo.Enabled = undoStack.Count != 0;
                menuRedo.Enabled = redoStack.Count != 0;
            }
            ExtractStyles();
        }
        #endregion



        private void cmdSearchReplace_Click(object sender, System.EventArgs e) {
            for (int index = lineColl.Count - 1; index != -1; index--) {
                Line thisline = lineColl[index];
                lineColl[index] = Line.ParseLine(thisline.ToString().Replace(txtSearchFor.Text, txtSearchReplaceWith.Text));
            }
            DrawListOnScreen();
        }
        private void cmdLenBasedK_Click(object sender, System.EventArgs e) {
            int thresh;
            if (int.TryParse(txtLenBasedKThreshold.Text,System.Globalization.NumberStyles.Integer,Util.nfi,out thresh)) {
                StringBuilder sb;
                List<Line> newList = new List<Line>();
                Line line;
                DialogueLine dl;
                Match m;
                MatchCollection mc;
                int lastIndex;

                for (int index = lineColl.Count - 1; index != 0; index--) {
                    line = (Line)lineColl[index].Clone();
                    if ((lineColl[index].enabled == false) || (lineColl[index].lineType != LineType.dialogue) || ((dl = (DialogueLine)line.line).style.enabled == false) || (r.IsMatch(dl.text) == false)) {
                        newList.Add(line);
                        continue;
                    }

                    sb = new StringBuilder(2048);

                    lastIndex = 0;

                    mc = r.Matches(dl.text);
                    for (int mindex = 0; mindex != mc.Count; mindex+=1) {
                        m = mc[mindex];
                        sb.Append(dl.text.Substring(lastIndex, m.Index - lastIndex));
                        if (int.Parse(m.Groups[2].Value) < thresh)
                            sb.Append("\\k");
                        else
                            sb.Append("\\K");

                        lastIndex = m.Index + m.Groups[1].Length;
                    }

                    sb.Append(dl.text.Substring(lastIndex, dl.text.Length - lastIndex));

                    dl.text = sb.ToString();
                    newList.Add(line);
                }
                SetLineColl(newList);
            }
            else
                MessageBox.Show("Enter a valid threshold.");
        }

        private void cmdShift_Click(object sender, System.EventArgs e) {
            List<Line> newList = new List<Line>();
            Line line;
            DialogueLine dl;
            TimeSpan shift;

            try {
                switch (tabShiftType.SelectedIndex) {
                    case 0:
                        shift = TimeSpan.Parse(maskedTextShiftTime.Text);
                        if (radTShiftBack.Checked) shift = shift.Negate();
                        break;
                    case 1:
                        shift = TimeSpan.FromSeconds(double.Parse(txtShiftFrames.Text, Util.cfi) / double.Parse(cmbFR.Text, Util.cfi));
                        if (radFShiftBackward.Checked) shift = shift.Negate();
                        break;
                    case 2:
                        shift = TimeSpan.Parse(maskedTextTDShiftNew.Text).Subtract(TimeSpan.Parse(maskedTextTDShiftOld.Text));
                        break;
                    default:
                        shift = TimeSpan.FromSeconds(((double.Parse(txtFDShiftNew.Text, Util.cfi) - double.Parse(txtFDShiftOld.Text, Util.cfi)) / double.Parse(cmbFR.Text, Util.cfi)));
                        break;
                }

                for (int index = 0; index != lineColl.Count; index+=1) {
                    line = (Line)lineColl[index].Clone(); ;

                    if (line.enabled == false || line.lineType != LineType.dialogue) {
                        newList.Add(line);
                        continue;
                    }

                    dl = (DialogueLine)line.line;

                    if (chkShiftStart.Checked == true) dl.start = dl.start.Add(shift);
                    if (chkShiftEnd.Checked == true) dl.end = dl.end.Add(shift);
                    if (chkShiftNoNeg.Checked == true) {
                        if (dl.start.CompareTo(TimeSpan.Zero) == -1) dl.start = TimeSpan.Zero;
                        if (dl.end.CompareTo(TimeSpan.Zero) == -1) dl.end = TimeSpan.Zero;
                    }
                    newList.Add(line);
                }
                SetLineColl(newList);
            } catch {
                MessageBox.Show("An error occured. No times have been changed.");
            }
        }

        private void cmdStrip_Click(object sender, System.EventArgs e) {
            Regex strip = new Regex(@"(\{+)(\\[^\{\}\\]+)*(\}+)", RegexOptions.Singleline);
            //will screw up for \t but it doesn't matter in this case

            List<Line> newList = new List<Line>();
            Line line;
            MatchCollection mc;
            Match m;
            GroupCollection gc;
            Group g;
            DialogueLine dl;
            StringBuilder sb;
            int lastindex;

            for (int index = 0; index != lineColl.Count; index+=1) {
                line = (Line)lineColl[index].Clone(); ;
                if ((lineColl[index].enabled == false) || (lineColl[index].lineType != LineType.dialogue) || ((dl = (DialogueLine)line.line).style.enabled == false)) {
                    newList.Add(line);
                    continue;
                }

                sb = new StringBuilder(2048);
                mc = strip.Matches(dl.text);
                lastindex = 0;
                for (int innerindex = 0; innerindex != mc.Count; innerindex+=1) {
                    m = mc[innerindex];
                    if (m.Index > lastindex)
                        sb.Append(dl.text.Substring(lastindex, m.Index - lastindex));
                    gc = m.Groups;
                    for (int groupindex = 2; groupindex != gc.Count - 1; groupindex+=1) {
                        g = gc[groupindex];
                        if (g.Value.Length > 2 && String.Equals(g.Value.Substring(0, 2), "\\k", StringComparison.OrdinalIgnoreCase))
                            sb.Append("{" + g.Value + "}");
                    }
                    lastindex = m.Index + m.Length;
                }
                if (lastindex != dl.text.Length) sb.Append(dl.text.Substring(lastindex, dl.text.Length - lastindex));

                dl.text = sb.ToString();
                newList.Add(line);
            }
            SetLineColl(newList);
        }
        private void cmdRemoveDups_Click(object sender, System.EventArgs e) {
            if (lineColl != null && lineColl.Count > 1) {
                List<Line> newList = new List<Line>();
                newList.Add((Line)lineColl[0].Clone());
                for (int index = 1; index != lineColl.Count; index+=1) {
                    if (!lineColl[index].Equals(lineColl[index - 1]))
                        newList.Add((Line)lineColl[index].Clone());
                }
                SetLineColl(newList);
            }
        }

        private void cmdScale_Click(object sender, System.EventArgs e) {
            double XNum, XDen, YNum, YDen;
            bool XNumValid, XDenValid, YNumValid, YDenValid;

            XNumValid = double.TryParse(txtScaleXNumerator.Text, System.Globalization.NumberStyles.AllowDecimalPoint, Util.nfi, out XNum);
            XDenValid = double.TryParse(txtScaleXDenominator.Text, System.Globalization.NumberStyles.AllowDecimalPoint, Util.nfi, out XDen);
            YNumValid = double.TryParse(txtScaleYNumerator.Text, System.Globalization.NumberStyles.AllowDecimalPoint, Util.nfi, out YNum);
            YDenValid = double.TryParse(txtScaleYDenominator.Text, System.Globalization.NumberStyles.AllowDecimalPoint, Util.nfi, out YDen);

            if ((XNumValid && XDenValid) && (!YNumValid || !YDenValid)) {
                YNum = XNum;
                YDen = XDen;
                YNumValid = YDenValid = true;
            }
            else if ((YNumValid && YDenValid) && (!XNumValid || !XDenValid)) {
                XNum = YNum;
                XDen = YDen;
                XNumValid = XDenValid = true;
            }

            if (XNumValid && XDenValid && YNumValid && YDenValid) {
                double scalefactorx = XNum/XDen;
                double scalefactory = YNum/YDen;

                List<Line> newList = new List<Line>();
                Line line;
                Resscale rs = new Resscale(scalefactorx, scalefactory);
                Style s;
                DialogueLine dl;

                if (chkScalePlayRes.Checked) {
                    ResX = Convert.ToInt32(Math.Round(ResX * scalefactorx, 0), Util.nfi);
                    ResY = Convert.ToInt32(Math.Round(ResY * scalefactory, 0), Util.nfi);
                }

                for (int index = 0; index != lineColl.Count; index+=1) {
                    line = (Line)lineColl[index].Clone();
                    if (line.enabled == false) {
                        newList.Add(line);
                        continue;
                    }
                    else if (line.lineType == LineType.style && ((Style)line.line).enabled == true) {
                        s = (Style)line.line;
                        s = rs.ScaleStyle(s);
                    }

                    else if ((line.lineType == LineType.dialogue) && ((dl = (DialogueLine)line.line).style.enabled == true)) {
                        dl = rs.ScaleDialogue(dl);
                    }
                    newList.Add(line);
                }
                SetLineColl(newList);
            }
        }

        private void cmdChangeLastKLen_Click(object sender, System.EventArgs e) {
            DialogueLine dl;
            List<Line> newList = new List<Line>();
            Line line;
            System.Text.StringBuilder sb;
            MatchCollection mc;
            int lineLen, totLen;


            for (int index = 0; index != lineColl.Count; index+=1) {
                line = (Line)lineColl[index].Clone();
                if ((lineColl[index].enabled == false) || (lineColl[index].lineType != LineType.dialogue) || ((dl = (DialogueLine)line.line).style.enabled == false) || (r.IsMatch(dl.text) == false)) {
                    newList.Add(line);
                    continue;
                }

                lineLen = (int)Math.Round((dl.end.Subtract(dl.start).TotalSeconds) * 100, 0);
                sb = new StringBuilder(2048);
                mc = r.Matches(dl.text);
                totLen = 0;
                foreach (Match m in mc) {
                    totLen += int.Parse(m.Groups[2].Value, Util.cfi);
                }

                sb.Append(dl.text.Substring(0, mc[mc.Count - 1].Groups[2].Index));
                sb.Append(lineLen - (totLen - int.Parse(mc[mc.Count-1].Groups[2].Value,Util.nfi)));
                sb.Append(dl.text.Substring(mc[mc.Count - 1].Groups[2].Index + mc[mc.Count - 1].Groups[2].Length, dl.text.Length - (mc[mc.Count - 1].Groups[2].Index + mc[mc.Count - 1].Groups[2].Length)));
                dl.text = sb.ToString();
                newList.Add(line);
            } // lineindex
            SetLineColl(newList);
        }
        private void cmdCheckErrors_Click(object sender, System.EventArgs e) {
            lstErrors.Items.Clear();
            Regex unclosedparen = new Regex(@"[\(^][^\)]*($|[\{\(\}])");
            Regex unopenedparen = new Regex(@"[\)}{^][^\(]*\)");
            Regex unclosedbracket = new Regex(@"\{[^\}$]*($|\{)");
            Regex unopenedbracket = new Regex(@"[}^][^\{]*\}");
            Regex badtrans = new Regex(@"\\t\(?[0-9,]*\)");
            Regex lowercasehex = new Regex(@"\\[1-4]?[cCaA][ahlopr]&?h");
            Regex unclosedampersand = new Regex(@"\\[1234]?[cCaA][ahlopr]*&[hH][0-9a-fA-F]*[}\\]");
            Regex decimalwithoutleading = new Regex(@"[^0-9].\d");

            string thisline;
            int lineindex = 0; // This is purely for user display, so start from 1 (it's incremented at the start of the loop)

            for (int index = 0; index != lineColl.Count; index+=1) {
                thisline = lineColl[index].ToString();
                lineindex+=1;
                if (unclosedparen.IsMatch(thisline))
                    lstErrors.Items.Add("Unclosed parenthesis at line " + lineindex);
                if (unopenedparen.IsMatch(thisline + "\n"))
                    lstErrors.Items.Add("Unopened parenthesis at line " + lineindex);
                if (unclosedbracket.IsMatch(thisline))
                    lstErrors.Items.Add("Unclosed bracket at line " + lineindex);
                if (unopenedbracket.IsMatch(thisline))
                    lstErrors.Items.Add("Unopened bracket at line " + lineindex);
                if (lowercasehex.IsMatch(thisline))
                    if (unclosedampersand.IsMatch(thisline))
                        lstErrors.Items.Add("Unclosed ampersand at line " + lineindex);
                if (badtrans.IsMatch(thisline))
                    lstErrors.Items.Add("Malformed transformation at line " + lineindex);
                if (decimalwithoutleading.IsMatch(thisline))
                    lstErrors.Items.Add("Decimal without leading integer at line " + lineindex);
            }
        }

        #region Fonts
        private void cmdFindFonts_Click(object sender, System.EventArgs e) {
            lstFonts.Items.Clear();

            List<string> fontList = new List<string>(16);
            Regex rfont = new Regex(@"{[^}]*\\fn([^\\}]+)");
            string[] lviadd;
            string font;

            foreach (Line line in lineColl) {
                if (line.lineType == LineType.style) {
                    font = ((Style)line.line).fontName.TrimStart("@".ToCharArray());
                    if (!fontList.Contains(font)) fontList.Add(font);
                }
                else if (line.lineType == LineType.dialogue) {
                    foreach (Match m in rfont.Matches(((DialogueLine)line.line).text)) {
                        font = m.Groups[1].Value.TrimStart("@".ToCharArray());
                        if (!fontList.Contains(font)) fontList.Add(font);
                    }
                }
            }

            foreach (FontFamily oneFontFamily in System.Drawing.FontFamily.Families) {
                for (int fontindex = 0; fontindex != fontList.Count; fontindex+=1) {
                    font = fontList[fontindex];
                    if (String.Equals(oneFontFamily.Name, font, StringComparison.OrdinalIgnoreCase)) {
                        lviadd = new string[3];
                        lviadd[0] = font;
                        lviadd[1] = "Found";

                        lstFonts.Items.Add(new ListViewItem(lviadd));
                        fontList.RemoveAt(fontindex--);
                    }
                }
            }

            foreach (string unfoundFont in fontList) {
                lviadd = new string[3];
                lviadd[0] = unfoundFont;
                lviadd[1] = "Not Found";
                lstFonts.Items.Add(new ListViewItem(lviadd));
            }
        }
        private void cmdFontSearchFolder_Click(object sender, System.EventArgs e) {
            if (folderBrowserDialog1.ShowDialog() == DialogResult.OK) {
                int index = 0;
                bool stopnow = false;

                System.Drawing.Text.PrivateFontCollection pfc;
                System.IO.DirectoryInfo dir = new System.IO.DirectoryInfo(folderBrowserDialog1.SelectedPath);
                System.IO.FileInfo[] FileArray = dir.GetFiles("*.ttf");
                for (int outerindex = 0; outerindex != 2; outerindex+=1) {
                    foreach (System.IO.FileInfo f in FileArray) {
                        try {
                            stopnow = true;
                            index+=1;
                            labelFontNum.Text = index.ToString(Util.cfi);
                            Application.DoEvents();
                            pfc = new System.Drawing.Text.PrivateFontCollection();
                            pfc.AddFontFile(f.FullName);

                            if (pfc.Families.Length != 0) {
                                foreach (ListViewItem lvi in lstFonts.Items) {
                                    stopnow = false;
                                    try {
                                        string familyName = lvi.Text.TrimStart("@".ToCharArray());

                                        foreach (System.Drawing.FontFamily pFamily in pfc.Families) {
                                            if (String.Equals(pFamily.Name, familyName, StringComparison.OrdinalIgnoreCase)) {
                                                lvi.SubItems[1].Text = "Found In Folder";
                                                lvi.SubItems[2].Text = f.FullName;
                                            }
                                        }
                                    } catch { }
                                }
                            }
                            pfc.Dispose();
                        } catch {
                            stopnow = false;
                            MessageBox.Show("Couldn't add font " + f.FullName);
                        }
                        if (stopnow) break;
                    }
                    FileArray = dir.GetFiles("*.ttc");
                }

                System.GC.Collect();
            }
            labelFontNum.Text = "done";
        }

        #endregion


        private void lstErrors_DoubleClick(object sender, System.EventArgs e) {
            string line = lstErrors.SelectedItem.ToString();
            int splitIndex = line.LastIndexOf(' ');
            string stringIndex = line.Substring(splitIndex);
            int index;
            if (int.TryParse(stringIndex, System.Globalization.NumberStyles.Integer, Util.nfi, out index)) {
                if (index <= lineColl.Count) {
                    index--; // We called it line 1, but it's really lineColl[0]
                    for (int listindex = 0; listindex != lineColl.Count; listindex+=1)
                        listSSA.Items[listindex].Selected = false;
                    listSSA.Items[index].Selected = true;
                }
            }
        }
        


        #region File Access
        private void LoadFile(string theFile, Encoding encoding) {
            string read;
            listSSA.VirtualListSize = 0;
            styleColl = new List<Style>();
            lineColl = new List<Line>();
            undoStack = new Stack();
            redoStack = new Stack();
            menuUndo.Enabled = false;
            menuRedo.Enabled = false;
            byte[] benc = new byte[2];
            int sjischars = 0, eucchars = 0;

            CurFile = theFile;
            StreamReader fs;
            if (encoding != null) fs = new StreamReader(theFile, encoding);
            else fs = new StreamReader(theFile, true);

            fs.BaseStream.Read(benc, 0, 2);

            if (benc[0] == 255 && benc[1] == 254)
                enc = Encoding.Unicode;
            else if (benc[0] == 255 && benc[1] == 255)
                enc = Encoding.BigEndianUnicode;
            else if (encoding == null) {
                enc = fs.CurrentEncoding;

                // Begin encoding detection
                fs.BaseStream.Position = 0;
                byte[] bread = new byte[131072];
                Stream bs = fs.BaseStream;
                int remaining = Convert.ToInt32(bs.Length,Util.nfi), readbytes;
                while (remaining != 0) {
                    readbytes = bs.Read(bread, 0, Math.Min(remaining, 131072));
                    remaining -= readbytes;
                    sjischars += DetectEncoding.DetectSJIS(bread);
                    eucchars += DetectEncoding.DetectEUCJP(bread);
                }
                if ((bs.Length < 4096 && (sjischars > 100 || eucchars > 100))
                 || (bs.Length < 8192 && (sjischars > 300 || eucchars > 300))
                 || sjischars > 600 || eucchars > 600) {
                    if (sjischars > eucchars) {
                        if (MessageBox.Show(sjischars.ToString(Util.cfi) + " characters were detected as likely being Shift-JIS.\r\nWould you like to load the file using Shift-JIS?", "Encoding", MessageBoxButtons.YesNo) == DialogResult.Yes)
                            encoding = Encoding.GetEncoding("Shift_JIS");
                    }
                    else {
                        if (MessageBox.Show(sjischars.ToString(Util.cfi) + " characters were detected as likely being EUC-JP.\r\nWould you like to load the file using EUC-JP?", "Encoding", MessageBoxButtons.YesNo) == DialogResult.Yes)
                            encoding = Encoding.GetEncoding("euc-jp");
                    }
                    if (encoding != null) fs = new StreamReader(theFile, encoding);
                }
                bread = null;
            }
            else enc = encoding;

            fs.BaseStream.Position = 0;
            while ((read = fs.ReadLine()) != null)
                lineColl.Add(Line.ParseLine(read));

            if (ResX == 0 && ResY != 0) ResX = (int)((double)ResY * (4.0 / 3));
            else if (ResY == 0 && ResX != 0) ResY = (int)((double)ResX * (3.0 / 4));

            fs.Close();
            System.GC.Collect();
            ExtractStyles();
        }
        private void cmdReloadFile_Click(object sender, System.EventArgs e) {
            if ((CurFile != null) && (File.Exists(CurFile))) LoadFile(CurFile, enc);
        }
        private void SaveFile(string FileName) {
            try {
                FileStream file = new FileStream(FileName, FileMode.Create, FileAccess.Write);

                if (file.CanWrite == true) {
                    StreamWriter sw = new StreamWriter(file, enc);
                    for (int index = 0; index != lineColl.Count; index+=1)
                        sw.WriteLine(lineColl[index].ToString());
                    sw.Close();
                }
                else MessageBox.Show("You don't have permission to write/create file " + FileName);

                file.Close();
            } catch {
                MessageBox.Show("Error while trying to save file.");
            }
        }

        private void LoadEffectsFile(string filename) {
            if (File.Exists(filename)) {
                try {
                    XmlDocument xDocM = new XmlDocument();
                    xDocM.Load(filename);
                    XmlNode xDoc = xDocM.SelectSingleNode("ESSA");

                    XmlNodeList effects = xDoc.SelectNodes("Effect");
                    Filter thisFilter;
                    int index = 0;

                    foreach (XmlNode enode in effects) {
                        if (enode.Attributes["Name"] == null) {
                            MessageBox.Show("Encountered unnamed effect. Skipping.");
                            continue;
                        }

                        thisFilter = new Filter(enode.Attributes["Name"].Value);

                        if (enode.Attributes["Code"] != null)
                            thisFilter.Code = enode.Attributes["Code"].Value;

                        //add default options
                        XmlNodeList options = enode.SelectNodes("Option");
                        foreach (XmlNode onode in options) {
                            if (onode.Attributes["Name"] == null) continue;
                            thisFilter.AddOption(onode.Attributes["Name"].Value, onode.InnerText);
                        }

                        //add default conditions
                        XmlNodeList econditions = enode.SelectNodes("Condition");
                        foreach (XmlNode ecnode in econditions) {
                            if ((ecnode.Attributes["ConditionOne"] == null) || (ecnode.Attributes["ConditionTwo"] == null) ||
                                (ecnode.Attributes["ConditionOperation"] == null)) { continue; }
                            Condition thisCondition = new Condition();
                            thisCondition.ConditionOne = ecnode.Attributes["ConditionOne"].Value;
                            thisCondition.ConditionTwo = ecnode.Attributes["ConditionTwo"].Value;
                            thisCondition.ConditionOp = ecnode.Attributes["ConditionOperation"].Value;
                            thisCondition.ConditionEnabled = (ecnode.Attributes["Enabled"] != null) ?
                                (String.Equals(ecnode.Attributes["Enabled"].Value, "true", StringComparison.OrdinalIgnoreCase)) : true;
                            thisFilter.AddCondition(thisCondition);
                        }

                        templateFilterColl.Add(thisFilter);
                        menuItem15.MenuItems.Add(thisFilter.ToString(), new EventHandler(this.menuFilter_Click));
                        index+=1;
                    }
                    //templateFilterColl.Sort();
                    EventContext();
                } catch (XmlException xmle) {
                    MessageBox.Show(xmle.Message);
                } catch (Exception e) {
                    MessageBox.Show(e.Message);
                }
            }
        }
        public static void SaveEffectsFile(string filename, System.Collections.Generic.List<Filter> templatefilterColl) {
            XmlTextWriter tw = new XmlTextWriter(filename, null);
            tw.Formatting = Formatting.Indented;
            tw.WriteStartDocument();
            tw.WriteStartElement("ESSA");

            foreach (Filter thisfilter in templatefilterColl) {

                tw.WriteAttributeString("Name", thisfilter.ToString());
                tw.WriteAttributeString("Code", thisfilter.Code);

                foreach (FilterOption fo in thisfilter) {
                    tw.WriteStartElement("Option");
                    tw.WriteAttributeString("Name", fo.Name);
                    tw.WriteString(fo.Value);
                    tw.WriteEndElement();
                }

                tw.WriteEndElement();
            }

            tw.WriteEndElement();
            tw.WriteEndDocument();
            tw.Flush();
            tw.Close();
        }

        private List<Layer> LoadProject(string filename) {
            //Returns a List of Layers which represents the layers of the project (layerColl/lColl)
            if (File.Exists(filename)) {
                try {
                    XmlDocument xDocM = new XmlDocument();
                    xDocM.Load(filename);
                    XmlNode xDoc = xDocM.SelectSingleNode("PSSA");

                    XmlNodeList filelist = xDoc.SelectNodes("Files");
                    foreach (XmlNode filenode in filelist) {
                        XmlNodeList ssalist = filenode.SelectNodes("SSA");
                        foreach (XmlNode ssanode in ssalist)
                            if (File.Exists(ssanode.InnerText)) LoadFile(ssanode.InnerText, null);

                        XmlNodeList effectfilelist = filenode.SelectNodes("EffectFile");
                        foreach (XmlNode effectfilenode in effectfilelist)
                            if (File.Exists(effectfilenode.InnerText)) LoadEffectsFile(effectfilenode.InnerText);

                    }

                    XmlNodeList layers = xDoc.SelectNodes("Layer");
                    System.Collections.Generic.List<Layer> lColl = new System.Collections.Generic.List<Layer>();
                    Layer thisLayer;
                    Filter thisFilter, templateFilter;
                    Condition thisCondition;
                    int repetitions;

                    foreach (XmlNode lnode in layers) {
                        thisLayer = new Layer();

                        thisLayer.Enabled = (lnode.Attributes["Enabled"] != null) ?
                            (String.Compare(lnode.Attributes["Enabled"].Value, "true", true) == 0) : true;
                        thisLayer.PerSyllable = (lnode.Attributes["PerSyllable"] != null) ?
                            (String.Compare(lnode.Attributes["PerSyllable"].Value, "true", true) == 0) : false;
                        thisLayer.SyllablePerLine = !(thisLayer.PerSyllable) && (lnode.Attributes["SyllablePerLine"] != null) ?
                            (String.Compare(lnode.Attributes["SyllablePerLine"].Value, "true", true) == 0) : false;
                        thisLayer.AddAll = (lnode.Attributes["AddAll"] != null) ?
                            (String.Compare(lnode.Attributes["AddAll"].Value, "true", true) == 0) : false;
                        thisLayer.AddOnce = (lnode.Attributes["AddOnce"] != null) ?
                            (String.Compare(lnode.Attributes["AddOnce"].Value, "true", true) == 0) : false;
                        thisLayer.AddText = (lnode.Attributes["DontAddText"] != null) ?
                            (String.Compare(lnode.Attributes["DontAddText"].Value, "false", true) == 0) : true;
                        thisLayer.AddK = (lnode.Attributes["RemoveK"] != null) ?
                            (String.Compare(lnode.Attributes["RemoveK"].Value, "false", true) == 0) : true;
                        thisLayer.AddASSA = (lnode.Attributes["AddASSA"] != null) ?
                            (String.Compare(lnode.Attributes["AddASSA"].Value, "true", true) == 0) : true;
                        thisLayer.AddBracket = (lnode.Attributes["AddClosingBracket"] != null) ?
                            (String.Compare(lnode.Attributes["AddClosingBracket"].Value, "true", true) == 0) : true;
                        thisLayer.Name = (lnode.Attributes["Name"] != null) ?
                            lnode.Attributes["Name"].Value : "Layer " + layerColl.Count.ToString(Util.nfi);

                        if ((lnode.Attributes["Repetitions"] != null) && 
                            (int.TryParse(lnode.Attributes["Repetitions"].Value,System.Globalization.NumberStyles.Integer,Util.nfi,out repetitions))) {
                            try {
                                if (repetitions > 0) thisLayer.Repetitions = repetitions;
                            } catch { }
                        } // Repetitions is 1 by default, so we only need to set if it's different

                        XmlNodeList effects = lnode.SelectNodes("Effect");

                        foreach (XmlNode enode in effects) {
                            if (enode.Attributes["Name"] == null) {
                                MessageBox.Show("Encountered unnamed effect. Skipping.");
                                continue;
                            }

                            templateFilter = GetTemplateFilterFromName(enode.Attributes["Name"].Value);

                            if (templateFilter == null) {
                                MessageBox.Show("Could not find effect " + enode.Attributes["Name"].Value + ", skipping effect");
                                continue;
                            }

                            thisFilter = (Filter)templateFilter.Clone();

                            thisFilter.Enabled = (enode.Attributes["Enabled"] != null) ?
                                (String.Compare(enode.Attributes["Enabled"].Value, "true", true) == 0) : true;

                            if (enode.Attributes["InstanceName"] != null)
                                thisFilter.Name = enode.Attributes["InstanceName"].Value;

                            //override default options where present
                            XmlNodeList options = enode.SelectNodes("Option");
                            foreach (XmlNode onode in options) {
                                if (onode.Attributes["Name"] == null) continue;
                                thisFilter.SetOptionValueByName(onode.Attributes["Name"].Value, onode.InnerText);
                            }

                            XmlNodeList econditions = enode.SelectNodes("Condition");
                            foreach (XmlNode ecnode in econditions) {
                                if ((ecnode.Attributes["ConditionOne"] == null) || (ecnode.Attributes["ConditionTwo"] == null) ||
                                    (ecnode.Attributes["ConditionOperation"] == null)) { continue; }

                                thisCondition = new Condition();
                                thisCondition.ConditionOne = ecnode.Attributes["ConditionOne"].Value;
                                thisCondition.ConditionTwo = ecnode.Attributes["ConditionTwo"].Value;
                                thisCondition.ConditionOp = ecnode.Attributes["ConditionOperation"].Value;
                                thisCondition.ConditionEnabled = (ecnode.Attributes["Enabled"] != null) ?
                                    (String.Compare(ecnode.Attributes["Enabled"].Value, "true", true) == 0) : true;

                                thisFilter.AddCondition(thisCondition);
                            }

                            thisLayer.AddFilter(thisFilter);
                        }

                        XmlNodeList lconditions = lnode.SelectNodes("Condition");
                        foreach (XmlNode lcnode in lconditions) {
                            if ((lcnode.Attributes["ConditionOne"] == null) || (lcnode.Attributes["ConditionTwo"] == null) ||
                                (lcnode.Attributes["ConditionOperation"] == null)) { continue; }
                            thisCondition = new Condition();
                            thisCondition.ConditionOne = lcnode.Attributes["ConditionOne"].Value;
                            thisCondition.ConditionTwo = lcnode.Attributes["ConditionTwo"].Value;
                            thisCondition.ConditionOp = lcnode.Attributes["ConditionOperation"].Value;
                            thisCondition.ConditionEnabled = (lcnode.Attributes["Enabled"] != null) ?
                                (String.Compare(lcnode.Attributes["Enabled"].Value, "true", true) == 0) : true;
                            thisLayer.AddCondition(thisCondition);
                        }
                        lColl.Add(thisLayer);
                    }

                    EventContext();
                    return lColl;
                } catch (Exception e) {
                    MessageBox.Show(e.Message);
                    return new System.Collections.Generic.List<Layer>();
                }
            }

            return new System.Collections.Generic.List<Layer>();
        }
        private void SaveProject(string filename) {
            XmlTextWriter tw = new XmlTextWriter(filename, null);
            tw.Formatting = Formatting.Indented;
            tw.WriteStartDocument();
            tw.WriteStartElement("PSSA");
            tw.WriteAttributeString("Version", "4");
            tw.WriteStartElement("Files");
            if (File.Exists(CurFile)) tw.WriteElementString("SSA", CurFile);
            tw.WriteEndElement();

            foreach (Layer thislayer in layerColl) {
                tw.WriteStartElement("Layer");
                tw.WriteAttributeString("Name", thislayer.Name);
                tw.WriteAttributeString("Enabled", thislayer.Enabled.ToString(Util.cfi));
                tw.WriteAttributeString("PerSyllable", thislayer.PerSyllable.ToString(Util.cfi));
                tw.WriteAttributeString("SyllablePerLine", thislayer.SyllablePerLine.ToString(Util.cfi));
                tw.WriteAttributeString("RemoveK", (!thislayer.AddK).ToString(Util.cfi));
                tw.WriteAttributeString("AddAll", thislayer.AddAll.ToString(Util.cfi));
                tw.WriteAttributeString("AddOnce", thislayer.AddOnce.ToString(Util.cfi));
                tw.WriteAttributeString("DontAddText", (!thislayer.AddText).ToString(Util.cfi));
                tw.WriteAttributeString("AddText", thislayer.AddText.ToString(Util.cfi));
                tw.WriteAttributeString("AddK", thislayer.AddK.ToString(Util.cfi));
                tw.WriteAttributeString("AddASSA", thislayer.AddASSA.ToString(Util.cfi));
                tw.WriteAttributeString("AddClosingBracket", thislayer.AddBracket.ToString(Util.cfi));

                tw.WriteAttributeString("Repetitions", thislayer.Repetitions.ToString(Util.cfi));

                foreach (Filter thisfilter in thislayer) {
                    tw.WriteStartElement("Effect");
                    tw.WriteAttributeString("Name", thisfilter.Template.Name);
                    tw.WriteAttributeString("InstanceName", thisfilter.Name);
                    tw.WriteAttributeString("Enabled", thisfilter.Enabled.ToString(Util.cfi));

                    foreach (FilterOption fo in thisfilter) {
                        tw.WriteStartElement("Option");
                        tw.WriteAttributeString("Name", fo.Name);
                        tw.WriteString(fo.Value);
                        tw.WriteEndElement();
                    }

                    for (int fcindex = 0; fcindex != thisfilter.ConditionCount; fcindex+=1) {
                        tw.WriteStartElement("Condition");
                        tw.WriteAttributeString("ConditionOne", thisfilter.GetCondition(fcindex).ConditionOne);
                        tw.WriteAttributeString("ConditionTwo", thisfilter.GetCondition(fcindex).ConditionTwo);
                        tw.WriteAttributeString("ConditionOperation", thisfilter.GetCondition(fcindex).ConditionOp);
                        tw.WriteAttributeString("Enabled", thisfilter.GetCondition(fcindex).ConditionEnabled.ToString(Util.cfi));
                        tw.WriteEndElement();
                    }

                    tw.WriteEndElement();

                }
                for (int flindex = 0; flindex != thislayer.ConditionCount; flindex+=1) {
                    tw.WriteStartElement("Condition");
                    tw.WriteAttributeString("ConditionOne", thislayer.GetCondition(flindex).ConditionOne);
                    tw.WriteAttributeString("ConditionTwo", thislayer.GetCondition(flindex).ConditionTwo);
                    tw.WriteAttributeString("ConditionOperation", thislayer.GetCondition(flindex).ConditionOp);
                    tw.WriteAttributeString("Enabled", thislayer.GetCondition(flindex).ConditionEnabled.ToString(Util.cfi));
                    tw.WriteEndElement();
                }

                tw.WriteEndElement();
            }

            tw.WriteEndElement();
            tw.WriteEndDocument();
            tw.Flush();
            tw.Close();
        }
        #endregion

        #region Generic form functions
        public void ExtractStyles() {
            Line line;
            Style s;
            labelSelLine.Text = string.Empty;
            styleColl.Clear();

            lstStyles.BeginUpdate();
            listSSA.BeginUpdate();
            cmbRegroupDest.BeginUpdate();
            cmbRegroupSource.BeginUpdate();

            lstStyles.Items.Clear();
            cmbRegroupDest.Items.Clear();
            cmbRegroupSource.Items.Clear();

            for (int index = 0; index != lineColl.Count; index+=1) {
                line = lineColl[index];
                if (line.lineType == LineType.style) {
                    s = (Style)line.line;
                    cmbRegroupDest.Items.Add(s.name);
                    cmbRegroupSource.Items.Add(s.name);
                    lstStyles.Items.Add(s.name, s.enabled);
                    styleColl.Add(s);
                }
            }
            listSSA.VirtualListSize = lineColl.Count;
            listSSA.Columns[0].Width = -2;
            lstStyles.EndUpdate();
            cmbRegroupDest.EndUpdate();
            cmbRegroupSource.EndUpdate();
            listSSA.EndUpdate();
        }
        private void NewLine(int index) {
            if (lineColl != null) {
                InputBoxResult newvalue = (InputBox.Show("Enter new text", "Input", String.Empty));
                if (newvalue.OK == true) {
                    Line newline = Line.ParseLine(newvalue.Text);
                    lineColl.Insert(index, newline);
                    undoStack.Push(index);
                    if (newline.lineType == LineType.style) {
                        Style s = (Style)newline.line;
                        styleColl.Add(s);
                        lstStyles.Items.Add(s.name);
                    }
                    menuUndo.Enabled = true;
                    DrawListOnScreen(); // refresh
                }
            }
            else MessageBox.Show("Select an item that is not on the top first.");
        }
        private void TextToList(TextBox tb, ListInsert type) {
            int index = GetFirstSelected();
            if (index == -1)
                MessageBox.Show("Select an item in the list first.");
            else {
                int indexIns;

                switch (type) {
                    case ListInsert.AfterLine:
                        indexIns = index + 1;
                        break;

                    case ListInsert.BeforeLine:
                        indexIns = index;
                        break;

                    default:
                        lineColl.RemoveAt(index);
                        indexIns = index;
                        break;
                }
                for (int lIndex = tb.Lines.Length - 1; lIndex != -1; lIndex--) {
                    string thisline = (string)tb.Lines.GetValue(lIndex);
                    if (!String.IsNullOrEmpty(thisline))
                        lineColl.Insert(indexIns, Line.ParseLine(thisline));
                }
                listSSA.VirtualListSize = lineColl.Count;
                DrawListOnScreen(); //update list on screen
            }
        }
        enum ListInsert {
            BeforeLine,
            AfterLine,
            ReplaceLine
        }
        #endregion



        private void cmdLead_Click(object sender, EventArgs e) {
            TimeSpan leadin, leadout;
            leadin = (chkLeadIn.Checked) ? TimeSpan.Parse(txtLeadIn.Text) : TimeSpan.Zero;
            leadout = (chkLeadOut.Checked) ? TimeSpan.Parse(txtLeadIn.Text) : TimeSpan.Zero;
            Regex rlead = new Regex(@"(\\[(t)(move)]\()([\d\-,]+)", RegexOptions.IgnoreCase | RegexOptions.Singleline | RegexOptions.Compiled);
            GroupCollection gc;
            List<Line> newList = new List<Line>();
            Line line;
            DialogueLine dl;
            Match m;
            MatchCollection mc;
            string leadink = "{\\k" + ((int)Math.Round(leadin.TotalMilliseconds / 10.0, 0)).ToString(Util.cfi) + "}";
            string[] split;
            StringBuilder sb;
            int lastindex;
            bool containsK;

            for (int index = 0; index != lineColl.Count; index+=1) {
                line = (Line)lineColl[index].Clone();
                if ((lineColl[index].enabled == false) || (lineColl[index].lineType != LineType.dialogue) || ((dl = (DialogueLine)line.line).style.enabled == false) || (r.IsMatch(dl.text) == false)) {
                    newList.Add(line);
                    continue;
                }

                containsK = dl.text.ToLowerInvariant().Contains("\\k");

                sb = new StringBuilder(1024);

                if (chkLeadIn.Checked)
                    dl.start = dl.start.Subtract(leadin);
                if (chkLeadOut.Checked)
                    dl.end = dl.end.Add(leadout);

                if (chkLeadIn.Checked && containsK && chkLeadKaraEmptyK.Checked)
                    sb.Append(leadink);

                lastindex = 0;
                mc = rlead.Matches(dl.text);
                for (int mindex = 0; mindex != mc.Count; mindex+=1) {
                    m = mc[mindex];
                    if (m.Index > lastindex)
                        sb.Append(dl.text.Substring(lastindex, m.Index - lastindex));

                    gc = m.Groups;
                    sb.Append(gc[1]);

                    switch (gc[1].Value) {
                        case "\\move(":
                            split = gc[2].Value.Split(",".ToCharArray());
                            for (int tokindex = 0; tokindex != split.Length; tokindex+=1) {
                                if (tokindex != 1) sb.Append(",");
                                if (tokindex < 5)
                                    sb.Append(split[tokindex]);
                                else //5 and 6 are times
                                    sb.Append((int.Parse(split[tokindex]) + leadin.TotalMilliseconds).ToString(Util.cfi));
                            }
                            break;

                        case "\\t(":
                            split = gc[2].Value.Split(",".ToCharArray());
                            sb.Append(String.Format("{0},{1},",
                              (int.Parse(split[0]) + leadin.TotalMilliseconds),
                              (int.Parse(split[1]) + leadin.TotalMilliseconds)));
                            for (int tokindex = 3; tokindex != split.Length; tokindex+=1)
                                sb.Append(split[tokindex] + ",");

                            break;

                        default:
                            sb.Append(gc[2]);
                            break;

                    }
                    lastindex = m.Index + m.Length;

                }
                if (dl.text.Length > lastindex)
                    sb.Append(dl.text.Substring(lastindex, dl.text.Length - lastindex));

                dl.text = sb.ToString();
                newList.Add(line);
            }
            SetLineColl(newList);
        }

        #region Kanji timing
        private ushort[] RegroupSourceLengths;
        private int RegroupSourceSelected, RegroupSourceOffset;
        private void cmdRegroupLink_Click(object sender, EventArgs e) {
            if (txtRegroupSource.SelectionLength == 0)
                MessageBox.Show("Select text from the source textbox.");
            else if (txtRegroupDest.SelectionStart != 0 || txtRegroupSource.SelectionStart != 0)
                MessageBox.Show("Both selections must start from the left.");
            else {
                string[] lvs = new string[2];
                ListViewItem lvi;
                lvs[0] = txtRegroupSource.SelectedText;
                lvs[1] = txtRegroupDest.SelectedText;
                lvi = new ListViewItem(lvs);
                lvi.Tag = RegroupSourceSelected;
                listRegroupPairs.Items.Add(lvi);
                txtRegroupSource.Text = txtRegroupSource.Text.Substring(txtRegroupSource.SelectionLength, txtRegroupSource.TextLength - txtRegroupSource.SelectionLength);
                txtRegroupDest.Text = txtRegroupDest.Text.Substring(txtRegroupDest.SelectionLength, txtRegroupDest.TextLength - txtRegroupDest.SelectionLength);
                RegroupSourceOffset += RegroupSourceSelected;
                if (txtRegroupSource.TextLength != 0) {
                    RegroupSourceSelected = 1;
                    RegroupSetSourceSelected();
                }
            }
            if (txtRegroupDest.SelectionStart != 0 || txtRegroupDest.SelectionLength == 0)
                RegroupSetDestSelected();
        }
        private void txtRegroupDest_KeyDown(object sender, KeyEventArgs e) {
            if (e.KeyData.Equals(Keys.Left)) {
                if (txtRegroupDest.SelectionLength != 0) txtRegroupDest.SelectionLength -= 1;
                e.Handled = true;
            }
            else if (e.KeyData.Equals(Keys.Right)) {
                if (txtRegroupDest.SelectionStart != 0) txtRegroupDest.SelectionStart = 0;
                if (txtRegroupDest.TextLength > txtRegroupDest.SelectionLength) txtRegroupDest.SelectionLength += 1;
                e.Handled = true;
            }
            else if (e.KeyData.Equals(Keys.Down)) {
                if (txtRegroupSource.SelectionLength != 0) {
                    RegroupSourceSelected -= 1;
                    RegroupSetSourceSelected();
                }
                e.Handled = true;
            }
            else if (e.KeyData.Equals(Keys.Up)) {
                if (txtRegroupSource.SelectionStart != 0) txtRegroupSource.SelectionStart = 0;
                if (txtRegroupSource.TextLength > txtRegroupSource.SelectionLength) {
                    RegroupSourceSelected += 1;
                    RegroupSetSourceSelected();
                }
                e.Handled = true;
            }
            else if (e.KeyData.Equals(Keys.Enter)) {
                if ((txtRegroupDest.TextLength == 0) && (txtRegroupSource.TextLength == 0) && (listRegroupPairs.Items.Count != 0))
                    cmdRegroupAcceptLine_Click(null, null);
                else cmdRegroupLink_Click(null, null);

                e.Handled = true;
            }
            else if (e.KeyData.Equals(Keys.Back)) {
                cmdRegroupUnlinkLast_Click(null, null);
                e.Handled = true;
            }
        }
        private void txtRegroupDest_KeyPress(object sender, KeyPressEventArgs e) {
            if (e.KeyChar == 8) e.Handled = true;
        }
        private void cmdRegroupUnlinkLast_Click(object sender, EventArgs e) {
            if (listRegroupPairs.Items.Count != 0) {
                ListViewItem lvi = listRegroupPairs.Items[listRegroupPairs.Items.Count - 1];
                txtRegroupSource.Text = lvi.SubItems[0].Text + txtRegroupSource.Text;
                txtRegroupDest.Text = lvi.SubItems[1].Text + txtRegroupDest.Text;
                RegroupSourceOffset -= (int)lvi.Tag;
                lvi.Remove();
                RegroupSourceSelected = 1;
                RegroupSetSourceSelected();
                RegroupSetDestSelected();
            }
        }
        private void cmdRegroupSkipSourceLine_Click(object sender, EventArgs e) {
            int lineindex;
            int index = int.Parse(lblRegroupSourceIndex.Text);
            index+=1;
            lblRegroupSourceIndex.Text = index.ToString(Util.cfi);
            lineindex = ListIndexFromStyleandIndex(cmbRegroupSource.Text, index);
            if (lineindex != -1) {
                RegroupSourceSelected = 0;
                txtRegroupSource.Text = ((DialogueLine)lineColl[lineindex].line).JustText;
                RegroupSetSyllableLengths(lineindex);
            }
            if (txtRegroupSource.TextLength != 0) {
                RegroupSourceOffset = 0;
                RegroupSourceSelected = 1;
                RegroupSetSourceSelected();
                txtRegroupDest.Focus();
            }
        }
        private void cmdRegroupSkipDestLine_Click(object sender, EventArgs e) {
            int lineindex;
            int index = int.Parse(lblRegroupDestIndex.Text);
            index+=1;
            lblRegroupDestIndex.Text = index.ToString(Util.cfi);
            lineindex = ListIndexFromStyleandIndex(cmbRegroupDest.Text, index);
            if (lineindex != -1) txtRegroupDest.Text = ((DialogueLine)lineColl[lineindex].line).JustText;
            RegroupSetDestSelected();
        }
        private void cmdRegroupAcceptLine_Click(object sender, EventArgs e) {
            if ((txtRegroupDest.TextLength != 0) || (txtRegroupSource.TextLength != 0) || (listRegroupPairs.Items.Count == 0))
                MessageBox.Show("Group the text completely first.");
            else try {
                    StringBuilder sb = new StringBuilder(1024);
                    Line line;
                    DialogueLine dl;

                    GroupCollection gc;
                    int srcindex, destindex, karalen, lastindex, listindex = 0, stextlen = 0;
                    bool setstextlen = true;

                    srcindex = ListIndexFromStyleandIndex(cmbRegroupSource.Text, int.Parse(lblRegroupSourceIndex.Text));
                    destindex = ListIndexFromStyleandIndex(cmbRegroupDest.Text, int.Parse(lblRegroupDestIndex.Text));

                    if (srcindex != -1 && destindex != -1) {
                        lastindex = 0;
                        karalen = 0;
                        line = (Line)lineColl[srcindex].Clone();
                        dl = (DialogueLine)line.line;
                        foreach (Match m in r.Matches(dl.text)) {
                            gc = m.Groups;
                            karalen += int.Parse(gc[2].Value);
                            if (m.Index > lastindex)
                                sb.Append(dl.text.Substring(lastindex, m.Index - lastindex));
                            lastindex = m.Index + m.Length;
                            if (setstextlen) stextlen = (gc[5].Value.Length == 0) ? 0 : listRegroupPairs.Items[listindex].Text.Length;
                            stextlen -= gc[5].Value.Length;


                            if (stextlen == 0) {
                                sb.Append(gc[1].Value + karalen.ToString(Util.cfi) + "}");
                                karalen = 0;
                                if (gc[5].Value.Length != 0) {
                                    sb.Append(listRegroupPairs.Items[listindex].SubItems[1].Text);
                                    listindex+=1;
                                }
                                setstextlen = true;
                            }

                            else {
                                lastindex+=1;
                                setstextlen = false;
                            }
                        }
                        if (dl.text.Length > lastindex)
                            sb.Append(dl.text.Substring(lastindex, dl.text.Length - lastindex));

                        dl.style = ((DialogueLine)lineColl[destindex].line).style;
                        dl.text = sb.ToString();
                        lineColl[destindex] = line;
                        ExtractStyles();

                        listRegroupPairs.Items.Clear();
                        cmdRegroupSkipDestLine_Click(null, null);
                        cmdRegroupSkipSourceLine_Click(null, null);
                    }
                } catch { MessageBox.Show("An error occured. Make sure no lines changed since you started."); }
        }

        private void cmdRegroupStart_Click(object sender, EventArgs e) {
            lblRegroupDestIndex.Text = "0";
            lblRegroupSourceIndex.Text = "0";
            txtRegroupDest.SelectionStart = 0;
            txtRegroupDest.SelectionLength = 0;
            txtRegroupSource.SelectionStart = 0;
            txtRegroupSource.SelectionLength = 0;
            cmdRegroupSkipSourceLine_Click(null, null);
            cmdRegroupSkipDestLine_Click(null, null);
        }

        private void cmdRegroupGoBack_Click(object sender, EventArgs e) {
            int index = int.Parse(lblRegroupSourceIndex.Text);
            index -= 2;
            if (index >= 0) lblRegroupSourceIndex.Text = index.ToString(Util.cfi);
            index = int.Parse(lblRegroupDestIndex.Text);
            index -= 2;
            if (index >= 0) lblRegroupDestIndex.Text = index.ToString(Util.cfi);
            cmdRegroupSkipSourceLine_Click(null, null);
            cmdRegroupSkipDestLine_Click(null, null);
        }
        private int ListIndexFromStyleandIndex(string StyleName, int Occurance) {
            Line line;
            int occindex = 0;
            for (int index = 0; index != lineColl.Count; index+=1) {
                line = lineColl[index];
                if (line.lineType == LineType.dialogue && ((DialogueLine)line.line).style.Equals(StyleName)) {
                    if (++occindex == Occurance) return index;
                }
            }
            return -1;
        }
        private void RegroupSetSyllableLengths(int index) {
            if (index <= lineColl.Count) {
                MatchCollection mc;
                Match m;
                Line thisline = lineColl[index];

                if (thisline.lineType == LineType.dialogue) {
                    string linetext = ((DialogueLine)thisline.line).text;

                    mc = r.Matches(linetext);
                    if (mc.Count == 0) {
                        MessageBox.Show("Kanji timing: Current source line has no \\k times");
                        return;
                    }
                    RegroupSourceLengths = new ushort[mc.Count];
                    for (int mindex = 0; mindex != mc.Count; mindex+=1) {
                        m = mc[mindex];
                        RegroupSourceLengths[mindex] = (ushort)m.Groups[5].Value.Length;
                    }

                }
            }
        }
        private void RegroupSetSourceSelected() {
            if (RegroupSourceLengths != null && RegroupSourceLengths.Length != 0) {
                int sellen = 0;
                if (RegroupSourceSelected < RegroupSourceLengths.Length && RegroupSourceLengths[RegroupSourceSelected + RegroupSourceOffset - 1] == 0)
                    RegroupSourceSelected+=1;

                for (int index = 0; index != RegroupSourceSelected; index+=1)
                    sellen += RegroupSourceLengths[index + RegroupSourceOffset];
                txtRegroupSource.SelectionStart = 0;
                txtRegroupSource.SelectionLength = sellen;
            }
        }
        private void RegroupSetDestSelected() {
            if (txtRegroupDest.TextLength != 0) {
                txtRegroupDest.SelectionStart = 0;
                if (txtRegroupSource.SelectionLength != 0 && txtRegroupDest.Text.StartsWith(txtRegroupSource.SelectedText))
                    txtRegroupDest.SelectionLength = txtRegroupSource.SelectionLength;
                else if (txtRegroupSource.SelectionLength != 0 && txtRegroupSource.SelectedText.EndsWith(" ") &&
                    txtRegroupDest.TextLength > 1 && String.CompareOrdinal(txtRegroupDest.Text.Substring(1, 1), " ") == 0)
                    txtRegroupDest.SelectionLength = 2;
                else
                    txtRegroupDest.SelectionLength = 1;
                txtRegroupDest.Focus();
            }
        }
        #endregion

        #region Noteboxes
        private void cmbNBStyle_SelectedIndexChanged(object sender, EventArgs e) {
            if (cmbNBStyle.SelectedIndex < Notebox.noteBoxColl.Count)
                textNBDesc.Text = (Notebox.noteBoxColl[cmbNBStyle.SelectedIndex]).description;
        }
        private void cmdNoteBox_Click(object sender, EventArgs e) {
            Notebox.DoNotebox(textNotebox1.Text, textNotebox2.Text, cmbNBStyle.SelectedIndex);
        }
        private void cmdNBInsertBefore_Click(object sender, EventArgs e) {
            TextToList(textNBOut, ListInsert.BeforeLine);
            ExtractStyles();
        }
        private void cmdNBInsertAfter_Click(object sender, EventArgs e) {
            TextToList(textNBOut, ListInsert.AfterLine);
            ExtractStyles();
        }
        private void cmdNBReplaceLine_Click(object sender, EventArgs e) {
            TextToList(textNBOut, ListInsert.ReplaceLine);
            ExtractStyles();
        }
        private void cmdNBReparse_Click(object sender, EventArgs e) {
            Notebox.readNoteBoxes();
        }
        private void cmdNBCopyStyles_Click(object sender, EventArgs e) {
            Notebox.CopyStyles();
        }
        private void cmdNBGetTimes_Click(object sender, EventArgs e) {
            int index = GetFirstSelected();
            if (index != -1) {
                Line l = lineColl[index];
                if (l.lineType == LineType.dialogue) {
                    DialogueLine dl = (DialogueLine)l.line;
                    maskedTextNBStart.Text = Util.TimeSpanSSA(dl.start, false, 2);
                    maskedTextNBEnd.Text = Util.TimeSpanSSA(dl.end, true, 2);
                }
                else MessageBox.Show("Please select a dialogue line.");
            }
            else MessageBox.Show("Please select a line.");
        }
        #endregion

        #region Timecodes
        private void cmdTimecodesBrowse_Click(object sender, EventArgs e) {
            openFileDialog1.Filter = "Text Files (*.txt)|*.txt|All Files (*)|*";
            openFileDialog1.FileName = String.Empty;
            if (openFileDialog1.ShowDialog() == DialogResult.OK)
                textTimecodeFile.Text = openFileDialog1.FileName;

        }

        private void cmdTimecodesApply_Click(object sender, EventArgs e) {
            if (File.Exists(textTimecodeFile.Text)) {
                TimeCodes tc = new TimeCodes(textTimecodeFile.Text, double.Parse(cmbFR.Text));
                Line l;
                DialogueLine dl;
                List<Line> newList = new List<Line>();

                for (int index = 0; index != lineColl.Count; index+=1) {
                    l = lineColl[index];
                    if (l.lineType == LineType.dialogue) {
                        l = (Line)l.Clone();
                        dl = (DialogueLine)l.line;
                        dl.start = tc.GetTime(dl.start);
                        dl.end = tc.GetTime(dl.end);
                    }
                    newList.Add(l);
                }

                undoStack.Push(lineColl);
                SetLineColl(newList);
            }
            else
                MessageBox.Show("Timecode file does not exist.");
        }

        private void cmdTimecodesDeapply_Click(object sender, EventArgs e) {
            if (File.Exists(textTimecodeFile.Text)) {
                TimeCodes tc = new TimeCodes(textTimecodeFile.Text, double.Parse(cmbFR.Text));
                Line l;
                DialogueLine dl;
                List<Line> newList = new List<Line>();

                for (int index = 0; index != lineColl.Count; index+=1) {
                    l = lineColl[index];
                    if (l.lineType == LineType.dialogue) {
                        l = (Line)l.Clone();
                        dl = (DialogueLine)l.line;
                        dl.start = tc.GetTimeInverse(dl.start);
                        dl.end = tc.GetTimeInverse(dl.end);
                    }
                    newList.Add(l);
                }

                undoStack.Push(lineColl);
                SetLineColl(newList);
            }
            else
                MessageBox.Show("Timecode file does not exist.");
        }
        #endregion

    }
}