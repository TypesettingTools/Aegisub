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
using System.Drawing;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;

namespace SSATool
{
	/// <summary>
	/// Summary description for formEffectsEditor.
	/// </summary>
	public class formEffectsEditor : System.Windows.Forms.Form
	{
		#region form stuff

		private System.Windows.Forms.MainMenu mainMenu1;
		private System.Windows.Forms.MenuItem menuItem1;
		private System.Windows.Forms.MenuItem menuItem4;
		private System.Windows.Forms.Button cmdNewEffect;
		private System.Windows.Forms.Button cmdDelEffect;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.TextBox txtCode;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.TextBox txtDefaultVal;
		private System.Windows.Forms.TextBox txtOptName;
		private System.ComponentModel.Container components = null;
		private System.Windows.Forms.ListBox lstEffects;
		private System.Windows.Forms.Button cmdChangeOpt;
		private System.Windows.Forms.Button cmdRemoveOpt;
		private System.Windows.Forms.Button cmdNewOpt;
		private System.Windows.Forms.ListBox lstParams;
		private System.Windows.Forms.MenuItem menuLoadEffectsFile;
		private System.Windows.Forms.MenuItem menuSaveEffectsFile;
		private System.Windows.Forms.MenuItem menuClose;
		private System.Windows.Forms.Button cmdChangeCode;
		#endregion

        private System.Collections.Generic.List<Filter> tColl;

        public formEffectsEditor(System.Collections.Generic.List<Filter> templateFilterColl) {
			InitializeComponent();
			tColl = templateFilterColl;
		}
		public formEffectsEditor() {
			InitializeComponent();
            tColl = new System.Collections.Generic.List<Filter>();
		}

		protected override void Dispose(bool disposing) {
			if(disposing)	{
				if(components != null)
					components.Dispose();
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
			this.lstEffects = new System.Windows.Forms.ListBox();
			this.mainMenu1 = new System.Windows.Forms.MainMenu();
			this.menuItem1 = new System.Windows.Forms.MenuItem();
			this.menuLoadEffectsFile = new System.Windows.Forms.MenuItem();
			this.menuSaveEffectsFile = new System.Windows.Forms.MenuItem();
			this.menuItem4 = new System.Windows.Forms.MenuItem();
			this.menuClose = new System.Windows.Forms.MenuItem();
			this.cmdNewEffect = new System.Windows.Forms.Button();
			this.cmdDelEffect = new System.Windows.Forms.Button();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.cmdChangeCode = new System.Windows.Forms.Button();
			this.txtOptName = new System.Windows.Forms.TextBox();
			this.txtDefaultVal = new System.Windows.Forms.TextBox();
			this.label4 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.txtCode = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.cmdChangeOpt = new System.Windows.Forms.Button();
			this.cmdRemoveOpt = new System.Windows.Forms.Button();
			this.cmdNewOpt = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.lstParams = new System.Windows.Forms.ListBox();
			this.groupBox1.SuspendLayout();
			this.SuspendLayout();
			// 
			// lstEffects
			// 
			this.lstEffects.Location = new System.Drawing.Point(0, 8);
			this.lstEffects.Name = "lstEffects";
			this.lstEffects.Size = new System.Drawing.Size(160, 251);
			this.lstEffects.TabIndex = 0;
			this.lstEffects.SelectedIndexChanged += new System.EventHandler(this.lstEffects_SelectedIndexChanged);
			// 
			// mainMenu1
			// 
			this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuItem1});
			// 
			// menuItem1
			// 
			this.menuItem1.Index = 0;
			this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																					  this.menuLoadEffectsFile,
																					  this.menuSaveEffectsFile,
																					  this.menuItem4,
																					  this.menuClose});
			this.menuItem1.Text = "File";
			// 
			// menuLoadEffectsFile
			// 
			this.menuLoadEffectsFile.Index = 0;
			this.menuLoadEffectsFile.Text = "Load Effects File";
			// 
			// menuSaveEffectsFile
			// 
			this.menuSaveEffectsFile.Index = 1;
			this.menuSaveEffectsFile.Text = "Save Effects File";
			this.menuSaveEffectsFile.Click += new System.EventHandler(this.menuSaveEffectsFile_Click);
			// 
			// menuItem4
			// 
			this.menuItem4.Index = 2;
			this.menuItem4.Text = "-";
			// 
			// menuClose
			// 
			this.menuClose.Index = 3;
			this.menuClose.Text = "Exit";
			this.menuClose.Click += new System.EventHandler(this.menuClose_Click);
			// 
			// cmdNewEffect
			// 
			this.cmdNewEffect.Location = new System.Drawing.Point(0, 264);
			this.cmdNewEffect.Name = "cmdNewEffect";
			this.cmdNewEffect.Size = new System.Drawing.Size(80, 24);
			this.cmdNewEffect.TabIndex = 1;
			this.cmdNewEffect.Text = "New";
			this.cmdNewEffect.Click += new System.EventHandler(this.cmdNewEffect_Click);
			// 
			// cmdDelEffect
			// 
			this.cmdDelEffect.Location = new System.Drawing.Point(80, 264);
			this.cmdDelEffect.Name = "cmdDelEffect";
			this.cmdDelEffect.Size = new System.Drawing.Size(80, 24);
			this.cmdDelEffect.TabIndex = 2;
			this.cmdDelEffect.Text = "Remove";
			this.cmdDelEffect.Click += new System.EventHandler(this.cmdDelEffect_Click);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.cmdChangeCode);
			this.groupBox1.Controls.Add(this.txtOptName);
			this.groupBox1.Controls.Add(this.txtDefaultVal);
			this.groupBox1.Controls.Add(this.label4);
			this.groupBox1.Controls.Add(this.label3);
			this.groupBox1.Controls.Add(this.txtCode);
			this.groupBox1.Controls.Add(this.label2);
			this.groupBox1.Controls.Add(this.cmdChangeOpt);
			this.groupBox1.Controls.Add(this.cmdRemoveOpt);
			this.groupBox1.Controls.Add(this.cmdNewOpt);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.lstParams);
			this.groupBox1.Location = new System.Drawing.Point(168, 8);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(416, 280);
			this.groupBox1.TabIndex = 3;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Selected Effect";
			// 
			// cmdChangeCode
			// 
			this.cmdChangeCode.Location = new System.Drawing.Point(328, 256);
			this.cmdChangeCode.Name = "cmdChangeCode";
			this.cmdChangeCode.Size = new System.Drawing.Size(80, 18);
			this.cmdChangeCode.TabIndex = 11;
			this.cmdChangeCode.Text = "Change";
			this.cmdChangeCode.Click += new System.EventHandler(this.cmdChangeCode_Click);
			// 
			// txtOptName
			// 
			this.txtOptName.Location = new System.Drawing.Point(104, 144);
			this.txtOptName.Name = "txtOptName";
			this.txtOptName.Size = new System.Drawing.Size(304, 20);
			this.txtOptName.TabIndex = 10;
			this.txtOptName.Text = "";
			// 
			// txtDefaultVal
			// 
			this.txtDefaultVal.Location = new System.Drawing.Point(104, 168);
			this.txtDefaultVal.Name = "txtDefaultVal";
			this.txtDefaultVal.Size = new System.Drawing.Size(304, 20);
			this.txtDefaultVal.TabIndex = 9;
			this.txtDefaultVal.Text = "";
			// 
			// label4
			// 
			this.label4.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label4.Location = new System.Drawing.Point(8, 168);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(96, 24);
			this.label4.TabIndex = 8;
			this.label4.Text = "Default Value:";
			this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// label3
			// 
			this.label3.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label3.Location = new System.Drawing.Point(8, 144);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(96, 24);
			this.label3.TabIndex = 7;
			this.label3.Text = "Option Name:";
			this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
			// 
			// txtCode
			// 
			this.txtCode.Location = new System.Drawing.Point(8, 256);
			this.txtCode.Name = "txtCode";
			this.txtCode.Size = new System.Drawing.Size(312, 20);
			this.txtCode.TabIndex = 6;
			this.txtCode.Text = "";
			this.txtCode.TextChanged += new System.EventHandler(this.txtCode_TextChanged);
			// 
			// label2
			// 
			this.label2.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label2.Location = new System.Drawing.Point(8, 192);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(400, 64);
			this.label2.TabIndex = 5;
			this.label2.Text = "Code: Put together the SSA code using the variables above (surrounded by dollar s" +
				"igns, IE length would be $length$) here. You may use variables as well.";
			this.label2.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			// 
			// cmdChangeOpt
			// 
			this.cmdChangeOpt.Location = new System.Drawing.Point(296, 104);
			this.cmdChangeOpt.Name = "cmdChangeOpt";
			this.cmdChangeOpt.Size = new System.Drawing.Size(112, 24);
			this.cmdChangeOpt.TabIndex = 4;
			this.cmdChangeOpt.Text = "Change";
			this.cmdChangeOpt.Click += new System.EventHandler(this.cmdChangeOpt_Click);
			// 
			// cmdRemoveOpt
			// 
			this.cmdRemoveOpt.Location = new System.Drawing.Point(296, 64);
			this.cmdRemoveOpt.Name = "cmdRemoveOpt";
			this.cmdRemoveOpt.Size = new System.Drawing.Size(112, 24);
			this.cmdRemoveOpt.TabIndex = 3;
			this.cmdRemoveOpt.Text = "Remove";
			this.cmdRemoveOpt.Click += new System.EventHandler(this.cmdRemoveOpt_Click);
			// 
			// cmdNewOpt
			// 
			this.cmdNewOpt.Location = new System.Drawing.Point(296, 40);
			this.cmdNewOpt.Name = "cmdNewOpt";
			this.cmdNewOpt.Size = new System.Drawing.Size(112, 24);
			this.cmdNewOpt.TabIndex = 2;
			this.cmdNewOpt.Text = "New";
			this.cmdNewOpt.Click += new System.EventHandler(this.cmdNewOpt_Click);
			// 
			// label1
			// 
			this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
			this.label1.Location = new System.Drawing.Point(8, 16);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(280, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "Parameters";
			this.label1.TextAlign = System.Drawing.ContentAlignment.BottomCenter;
			// 
			// lstParams
			// 
			this.lstParams.Location = new System.Drawing.Point(8, 40);
			this.lstParams.Name = "lstParams";
			this.lstParams.Size = new System.Drawing.Size(280, 95);
			this.lstParams.TabIndex = 0;
			this.lstParams.SelectedIndexChanged += new System.EventHandler(this.lstParams_SelectedIndexChanged);
			// 
			// formEffectsEditor
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(584, 291);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.cmdDelEffect);
			this.Controls.Add(this.cmdNewEffect);
			this.Controls.Add(this.lstEffects);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.ImeMode = System.Windows.Forms.ImeMode.On;
			this.MaximizeBox = false;
			this.Menu = this.mainMenu1;
			this.Name = "formEffectsEditor";
			this.Text = "SSATool Effects Editor";
			this.Load += new System.EventHandler(this.formEffectsEditor_Load);
			this.groupBox1.ResumeLayout(false);
			this.ResumeLayout(false);

		}
		#endregion

		private void formEffectsEditor_Load(object sender, System.EventArgs e) {
			foreach(Filter tf in tColl)
				lstEffects.Items.Add(tf.ToString());
		}


		private void cmdNewEffect_Click(object sender, System.EventArgs e) {
			InputBoxResult newvalue = InputBox.Show("Enter the name of the new effect:","Input",String.Empty);
			if (newvalue.OK == true) {
				int index, cmp;
				for(index=0;index<tColl.Count;index++) {
					cmp = newvalue.Text.CompareTo(tColl[index].ToString());
					if (cmp == 0) {
						MessageBox.Show("An effect of that name already exists.");
						return;
					}
					if (cmp == -1) break;
				}
				Filter nt = new Filter(newvalue.Text);
				tColl.Insert(index, nt);
			}
		}

		private void cmdDelEffect_Click(object sender, System.EventArgs e) {
			if (lstEffects.SelectedItems.Count != 0) {
				tColl.RemoveAt(lstEffects.SelectedIndex);
				lstEffects.Items.RemoveAt(lstEffects.SelectedIndex);

			}
		}

		private void cmdNewOpt_Click(object sender, System.EventArgs e) {
			if (lstEffects.SelectedItems.Count != 0) {
				(tColl[lstEffects.SelectedIndex]).AddOption("New Option",String.Empty);
				lstParams.Items.Add("New Option");

			}
			else MessageBox.Show("Select an effect first.");
		}

		private void cmdRemoveOpt_Click(object sender, System.EventArgs e) {
			if ((lstEffects.SelectedItems.Count != 0) && (lstParams.SelectedItems.Count != 0)) {
				(tColl[lstEffects.SelectedIndex]).RemoveOptionByIndex(lstParams.SelectedIndex);
				lstParams.Items.RemoveAt(lstParams.SelectedIndex);
			}
			else MessageBox.Show("Select an effect first.");
		}
		private void cmdChangeOpt_Click(object sender, System.EventArgs e) {
			if ((lstEffects.SelectedItems.Count != 0) && (lstParams.SelectedItems.Count != 0)) {
				(tColl[lstEffects.SelectedIndex]).SetOptionByIndex(lstParams.SelectedIndex,txtOptName.Text,txtDefaultVal.Text);
				lstParams.Items.Clear();
				foreach(FilterOption fo in (Filter)tColl[lstEffects.SelectedIndex]) {
					lstParams.Items.Add(fo.Name);
				}
			}
			else MessageBox.Show("Select an effect and a parameter first.");
		}
		private void cmdChangeCode_Click(object sender, System.EventArgs e) {
			if (lstEffects.SelectedItems.Count != 0)
				(tColl[lstEffects.SelectedIndex]).Code = txtCode.Text;

			else MessageBox.Show("Select an effect first.");
		}


		private void txtCode_TextChanged(object sender, System.EventArgs e) {
			if (lstEffects.SelectedItems.Count != 0) 
				(tColl[lstEffects.SelectedIndex]).Code = txtCode.Text;
		}


		private void lstEffects_SelectedIndexChanged(object sender, System.EventArgs e) {
			lstParams.Items.Clear();
			if (lstEffects.SelectedItems.Count != 0) {
				foreach(FilterOption fo in (tColl[lstEffects.SelectedIndex])) {
					lstParams.Items.Add(fo.Name);
					txtCode.Text = (tColl[lstEffects.SelectedIndex]).Code;
				}
			}
		}

		private void lstParams_SelectedIndexChanged(object sender, System.EventArgs e) {
			if ((lstEffects.SelectedItems.Count != 0) && (lstParams.SelectedItems.Count != 0)) {
				txtOptName.Text = lstParams.Items[lstParams.SelectedIndex].ToString();
				txtDefaultVal.Text = (tColl[lstEffects.SelectedIndex]).GetOptionValueByName(lstParams.Items[lstParams.SelectedIndex].ToString());
			}
			else
				txtOptName.Text = txtDefaultVal.Text = String.Empty;
		}


		private void menuClose_Click(object sender, System.EventArgs e) {
			this.Close();
		}

		private void menuSaveEffectsFile_Click(object sender, System.EventArgs e) {
			SaveFileDialogWithEncoding ofd=new SaveFileDialogWithEncoding();
			ofd.DefaultExt="exml";
			ofd.EncodingType=EncodingType.ANSI;
			ofd.Filter="XML effect files (*.exml)|*.exml|All files (*.*)|*.*";
			if (ofd.ShowDialog((IntPtr)this.Handle,Screen.FromControl(this))==DialogResult.OK) {
				Form1.SaveEffectsFile(ofd.FileName,tColl);
			}
		}

	}
}
