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
using System.ComponentModel;
using System.Windows.Forms;

namespace SSATool
{
	/// <summary>
	/// Summary description for ConditionDialog.
	/// </summary>
	public class ConditionDialog : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.ComboBox comboBox1;
		private System.Windows.Forms.Button cmdCancel;
		private System.Windows.Forms.Button cmdOK;
		private System.Windows.Forms.TextBox txtC1;
		private System.Windows.Forms.TextBox txtC2;
		private System.Windows.Forms.Label lblPrompt;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ConditionDialog() {
			InitializeComponent();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.txtC1 = new System.Windows.Forms.TextBox();
			this.txtC2 = new System.Windows.Forms.TextBox();
			this.lblPrompt = new System.Windows.Forms.Label();
			this.comboBox1 = new System.Windows.Forms.ComboBox();
			this.cmdCancel = new System.Windows.Forms.Button();
			this.cmdOK = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// label1
			// 
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.Location = new System.Drawing.Point(8, 56);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(128, 20);
			this.label1.TabIndex = 6;
			this.label1.Text = "Condition One:";
			this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// label2
			// 
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label2.Location = new System.Drawing.Point(8, 80);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(128, 20);
			this.label2.TabIndex = 7;
			this.label2.Text = "Condition Two:";
			this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// label3
			// 
			this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label3.Location = new System.Drawing.Point(8, 104);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(128, 20);
			this.label3.TabIndex = 8;
			this.label3.Text = "Comparison:";
			this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// txtC1
			// 
			this.txtC1.Location = new System.Drawing.Point(144, 56);
			this.txtC1.Name = "txtC1";
			this.txtC1.Size = new System.Drawing.Size(336, 20);
			this.txtC1.TabIndex = 0;
			this.txtC1.Text = "";
			this.txtC1.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtC_KeyPress);
			// 
			// txtC2
			// 
			this.txtC2.Location = new System.Drawing.Point(144, 80);
			this.txtC2.Name = "txtC2";
			this.txtC2.Size = new System.Drawing.Size(336, 20);
			this.txtC2.TabIndex = 1;
			this.txtC2.Text = "";
			this.txtC2.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtC_KeyPress);
			// 
			// lblPrompt
			// 
			this.lblPrompt.Font = new System.Drawing.Font("Microsoft Sans Serif", 11.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblPrompt.Location = new System.Drawing.Point(0, 8);
			this.lblPrompt.Name = "lblPrompt";
			this.lblPrompt.Size = new System.Drawing.Size(480, 40);
			this.lblPrompt.TabIndex = 5;
			// 
			// comboBox1
			// 
			this.comboBox1.ImeMode = System.Windows.Forms.ImeMode.Off;
			this.comboBox1.Items.AddRange(new object[] {
														   "=",
														   "==",
														   "!=",
                                                           "!==",
														   ">",
														   ">=",
														   "<",
														   "<="});
			this.comboBox1.Location = new System.Drawing.Point(144, 104);
			this.comboBox1.Name = "comboBox1";
			this.comboBox1.Size = new System.Drawing.Size(120, 21);
			this.comboBox1.TabIndex = 2;
			this.comboBox1.Text = "==";
			this.comboBox1.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.comboBox1_KeyPress);
			// 
			// cmdCancel
			// 
			this.cmdCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.cmdCancel.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
			this.cmdCancel.Location = new System.Drawing.Point(384, 104);
			this.cmdCancel.Name = "cmdCancel";
			this.cmdCancel.Size = new System.Drawing.Size(96, 24);
			this.cmdCancel.TabIndex = 4;
			this.cmdCancel.Text = "Cancel";
			// 
			// cmdOK
			// 
			this.cmdOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.cmdOK.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
			this.cmdOK.Location = new System.Drawing.Point(272, 104);
			this.cmdOK.Name = "cmdOK";
			this.cmdOK.Size = new System.Drawing.Size(96, 24);
			this.cmdOK.TabIndex = 3;
			this.cmdOK.Text = "OK";
			// 
			// ConditionDialog
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(488, 135);
			this.Controls.Add(this.cmdOK);
			this.Controls.Add(this.cmdCancel);
			this.Controls.Add(this.comboBox1);
			this.Controls.Add(this.lblPrompt);
			this.Controls.Add(this.txtC2);
			this.Controls.Add(this.txtC1);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "ConditionDialog";
			this.Text = "ConditionDialog";
			this.Load += new System.EventHandler(this.ConditionDialog_Load);
			this.ResumeLayout(false);

		}
		#endregion

		private void ConditionDialog_Load(object sender, System.EventArgs e) {
		
		}
		public static ConditionDialogResult Show(string prompt, string title, int xpos, int ypos) { 
			using (ConditionDialog form = new ConditionDialog()) {
				form.Text = title;
				form.lblPrompt.Text = prompt;

				if (xpos >= 0 && ypos >= 0) { 
					form.StartPosition = FormStartPosition.Manual; 
					form.Left = xpos; 
					form.Top = ypos; 
				} 

				DialogResult result = form.ShowDialog(); 

				ConditionDialogResult retval = new ConditionDialogResult(); 
				if (result == DialogResult.OK) { 
					retval.ConditionOne = form.txtC1.Text;
					retval.ConditionTwo = form.txtC2.Text;
					retval.ConditionComp = form.comboBox1.Text;
					retval.OK = true; 
				} 
				return retval; 
			} 
		}

		private void txtC_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e) {
			if ((e.KeyChar == (char)10) || (e.KeyChar == (char)13)) {
				e.Handled = true;
				cmdOK.PerformClick();
			}
		}

		private void comboBox1_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e) {
			e.Handled = true; // We don't want anyone typing here anyway!
			if (e.KeyChar == (char)Keys.Enter)
				cmdOK.PerformClick();
		}

		public static ConditionDialogResult Show(string prompt, string title) { 
			return Show(prompt, title, -1, -1); 
		}
	}

	public class ConditionDialogResult {
		public string ConditionOne;
		public string ConditionTwo;
		public string ConditionComp;
		public bool OK;
	}
}
