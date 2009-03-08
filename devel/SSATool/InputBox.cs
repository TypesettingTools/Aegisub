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
	/// Summary description for InputBox.
	/// 
	public class InputBox : System.Windows.Forms.Form {

		#region Windows Contols and Constructor

		private System.Windows.Forms.Label lblPrompt;
		private System.Windows.Forms.Button btnOK;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.TextBox txtInput;
		/// <summary>
		/// Required designer variable.
		/// 
		private System.ComponentModel.Container components = null;

		public InputBox() {
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		#endregion

		#region Dispose

		/// <summary>
		/// Clean up any resources being used.
		/// 
		protected override void Dispose( bool disposing ) {
			if( disposing ) {
				if(components != null) {
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#endregion

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// 
		private void InitializeComponent() {
			this.lblPrompt = new System.Windows.Forms.Label();
			this.btnOK = new System.Windows.Forms.Button();
			this.button1 = new System.Windows.Forms.Button();
			this.txtInput = new System.Windows.Forms.TextBox();
			this.SuspendLayout();
			// 
			// lblPrompt
			// 
			this.lblPrompt.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
				| System.Windows.Forms.AnchorStyles.Left) 
				| System.Windows.Forms.AnchorStyles.Right)));
			this.lblPrompt.BackColor = System.Drawing.SystemColors.Control;
			this.lblPrompt.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.lblPrompt.Location = new System.Drawing.Point(12, 9);
			this.lblPrompt.Name = "lblPrompt";
			this.lblPrompt.Size = new System.Drawing.Size(302, 82);
			this.lblPrompt.TabIndex = 3;
			// 
			// btnOK
			// 
			this.btnOK.DialogResult = System.Windows.Forms.DialogResult.OK;
			this.btnOK.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
			this.btnOK.Location = new System.Drawing.Point(326, 24);
			this.btnOK.Name = "btnOK";
			this.btnOK.Size = new System.Drawing.Size(64, 24);
			this.btnOK.TabIndex = 1;
			this.btnOK.Text = "&OK";
			// 
			// button1
			// 
			this.button1.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.button1.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
			this.button1.Location = new System.Drawing.Point(326, 56);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(64, 24);
			this.button1.TabIndex = 2;
			this.button1.Text = "&Cancel";
			// 
			// txtInput
			// 
			this.txtInput.ImeMode = System.Windows.Forms.ImeMode.Off;
			this.txtInput.Location = new System.Drawing.Point(8, 100);
			this.txtInput.Name = "txtInput";
			this.txtInput.Size = new System.Drawing.Size(379, 20);
			this.txtInput.TabIndex = 0;
			this.txtInput.Text = "";
			this.txtInput.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.txtInput_KeyPress);
			// 
			// InputBox
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(398, 128);
			this.Controls.Add(this.txtInput);
			this.Controls.Add(this.button1);
			this.Controls.Add(this.btnOK);
			this.Controls.Add(this.lblPrompt);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.ImeMode = System.Windows.Forms.ImeMode.Off;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "InputBox";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "InputBox";
			this.ResumeLayout(false);

		}
		#endregion

		#region Private Variables
		string formCaption = string.Empty;
		string formPrompt = string.Empty;
		string inputResponse = string.Empty;
		string defaultValue = string.Empty;
		#endregion

		#region Public Properties
		public string FormCaption {
			get{return formCaption;}
			set{formCaption = value;}
		} // property FormCaption
		public string FormPrompt {
			get{return formPrompt;}
			set{formPrompt = value;}
		} // property FormPrompt
		public string InputResponse {
			get{return inputResponse;}
			set{inputResponse = value;}
		} // property InputResponse
		public string DefaultValue {
			get{return defaultValue;}
			set{defaultValue = value;}
		} // property DefaultValue

		#endregion

		#region Form and Control Events
		public static InputBoxResult Show(string prompt, string title, string defaultResponse, 
			int xpos, int ypos) { 
			using (InputBox form = new InputBox()) { 
				form.lblPrompt.Text = prompt; 
				form.Text = title; 
				form.txtInput.Text = defaultResponse; 
				if (xpos >= 0 && ypos >= 0) { 
					form.StartPosition = FormStartPosition.Manual; 
					form.Left = xpos; 
					form.Top = ypos; 
				} 

				DialogResult result = form.ShowDialog(); 

				InputBoxResult retval = new InputBoxResult(); 
				if (result == DialogResult.OK) { 
					retval.Text = form.txtInput.Text; 
					retval.OK = true; 
				} 
				return retval; 
			} 
		}



		private void txtInput_KeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e) {
			if ((e.KeyChar == '\r') || (e.KeyChar == '\n')) {
				btnOK.PerformClick();
				e.Handled = true;
			}
			else if (e.KeyChar == 27) { //escape key
				button1.PerformClick();
				e.Handled = true;
			}
		}

		public static InputBoxResult Show(string prompt, string title, string defaultText) { 
			return Show(prompt, title, defaultText, -1, -1); 
		}



	}
	#endregion

	public class InputBoxResult {
		public bool OK;
		public string Text;
	}
}
