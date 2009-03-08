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
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
//using System.ComponentModel.Design;
//using System.Windows.Forms.Design;
//using System.Drawing.Design;

namespace SSATool {
	/// <summary>
	/// Summary description for ListViewES.
	/// </summary>
	public class ListViewES : System.Windows.Forms.ListView {
        
		private System.Windows.Forms.TextBox textBox1;
        private ComboBox comboBox1;

        

		private System.ComponentModel.Container components = null;
        #region Component Designer generated code
        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.comboBox1 = new System.Windows.Forms.ComboBox();
            this.SuspendLayout();
            // 
            // textBox1
            // 
            this.textBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBox1.Enabled = false;
            this.textBox1.Location = new System.Drawing.Point(17, 17);
            this.textBox1.Margin = new System.Windows.Forms.Padding(3, 1, 3, 1);
            this.textBox1.Name = "textBox1";
            this.textBox1.Size = new System.Drawing.Size(20, 13);
            this.textBox1.TabIndex = 1;
            this.textBox1.Visible = false;
            this.textBox1.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textBox1_KeyPress);
            // 
            // comboBox1
            // 
            this.comboBox1.Cursor = System.Windows.Forms.Cursors.Default;
            this.comboBox1.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox1.Enabled = false;
            this.comboBox1.ImeMode = System.Windows.Forms.ImeMode.Disable;
            this.comboBox1.IntegralHeight = false;
            this.comboBox1.Location = new System.Drawing.Point(32, 32);
            this.comboBox1.Margin = new System.Windows.Forms.Padding(1);
            this.comboBox1.Name = "comboBox1";
            this.comboBox1.Size = new System.Drawing.Size(121, 21);
            this.comboBox1.TabIndex = 0;
            this.comboBox1.Visible = false;
            this.comboBox1.Leave += new System.EventHandler(this.ListViewES_Leave);
            this.comboBox1.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.textBox1_KeyPress);
            // 
            // ListViewES
            // 
            this.Controls.Add(this.textBox1);
            this.Controls.Add(this.comboBox1);
            this.LabelEdit = true;
            this.Size = new System.Drawing.Size(992, 616);
            this.View = System.Windows.Forms.View.Details;
            this.MouseUp += new System.Windows.Forms.MouseEventHandler(this.ListViewES_MouseUp);
            
            this.Leave += new System.EventHandler(this.ListViewES_Leave);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

		public event LabelSubEditEventHandler SubItemClicked;
		public event LabelSubEditEventHandler SubItemBeginEditing;
		public event LabelSubEditEndEventHandler SubItemEndEditing;
        public event ScrollEventHandler OnScroll;

		public ListViewES()	{
			InitializeComponent();
            //this.SetStyle(ControlStyles.DoubleBuffer | ControlStyles.OptimizedDoubleBuffer | ControlStyles.AllPaintingInWmPaint, true);

            this.DoubleBuffered = true;
		}


        protected override void Dispose(bool disposing) {
            if (disposing) {
                if (components != null) {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }

        public new void SetStyle(System.Windows.Forms.ControlStyles flag, bool value) {
            base.SetStyle(flag, value);
        }

		private ListViewItem item;
		private int subItemIndex;
        private Control c;

        public int ItemsOnScreen {
            get {
                Rectangle tib = this.TopItem.Bounds;
                int offH = this.Height - tib.Top;
                int height = tib.Height;
                int num = Convert.ToInt32(Math.Floor((float)offH / height),Util.nfi);
                return num;
            }
        }


        public void StartEditing(ListViewItem item, int subitem) {
            if (c != null) EndEditing(true);
            this.item = item;
            this.subItemIndex = subitem;
            c = (Control) this.textBox1;
            StartEditingHelper();
        }

        public void StartEditing(ListViewItem item, int subitem, string[] ComboChoices) {
            if (c != null) EndEditing(true);
            this.item = item;
            this.subItemIndex = subitem;
            c = (Control)this.comboBox1;
            comboBox1.Items.Clear();
            comboBox1.Items.AddRange(ComboChoices);
            StartEditingHelper();
        }

        private void StartEditingHelper() {
            this.SuspendLayout();
            c.Enabled = true;
            PositionControl();
            c.Text = item.SubItems[subItemIndex].Text;
            c.Show();
            c.Focus();
            this.ResumeLayout(true);
        }

        private void PositionControl() {
            if (c != null) {
                c.Bounds = this.GetSubItemBounds(item, subItemIndex);
                //if (GridLines == true) c.Height -= 1;

                if (this.CheckBoxes==true && subItemIndex==0) {
                    c.Left += 20;
                    c.Width -= 22;
                }
                else {
                    c.Left += 4;
                    c.Width -= 6;
                }
            }
        }

		private void EndEditing(bool saveValue) {
			if ((item != null) && (subItemIndex != -1)) {
				this.SuspendLayout();
				if (saveValue) item.SubItems[subItemIndex].Text = c.Text;
                if (SubItemEndEditing != null) 
                    this.SubItemEndEditing(this,
                         new LabelSubEditEndEventArgs(item, subItemIndex, 
                             item.SubItems[subItemIndex].Text, !saveValue));
				this.item = null;
				this.subItemIndex = -1;

				c.Text = String.Empty;
				c.Hide();
				c.Enabled = false;
				this.ResumeLayout(true);
				saveValue = true;
                c = null;
			}
		}

		private void textBox1_KeyPress(object sender, KeyPressEventArgs e) {
			if (e.KeyChar == (char)(int)Keys.Enter) {
				EndEditing(true);
				e.Handled = true;
			}
			else if (e.KeyChar == (char)(int)Keys.Escape) {
				EndEditing(false);
				e.Handled = true;
			}
		}

		private void ListViewES_MouseUp(object sender, MouseEventArgs e) {
            if (c != null) EndEditing(true);
			if (this.View==System.Windows.Forms.View.Details) {
				int index;
				ListViewItem lvi;

				index = this.GetSubItemAt(e.X,e.Y,out lvi);

				if ((index != -1) && ((this.CheckBoxes == false) || (e.X > 15))) {
                    LabelSubEditEventArgs lsa = new LabelSubEditEventArgs(lvi, index);
                    if (SubItemBeginEditing != null) this.SubItemBeginEditing(this, lsa);
                    if (SubItemClicked != null) this.SubItemClicked(this, lsa);
                    else if (this.LabelEdit == true) this.StartEditing(lvi, index);
				}
			}
		}

		public int GetSubItemAt(int x, int y, out ListViewItem item) {
			if (this.Items.Count != 0) {
                Rectangle tib = this.TopItem.Bounds;
                int left = tib.Left;
                if (left < 0) {
                    item = null;
                    return -1;
                }
                int offH = this.Height - tib.Top;
				int top = this.TopItem.Index;
                int height = tib.Height;
				int bottom = top + Convert.ToInt32(Math.Floor((float)offH / (float)height),Util.nfi);
				int row = top - 1 + Convert.ToInt32((((float)bottom - top) * ((float)y / offH)),Util.nfi);

				if ((row < this.Items.Count) && (row >= 0)) {

					for(int column=0;column<this.Columns.Count;column+=1) {
						left += this.Columns[column].Width;
						if (x <= left) { item = this.Items[row]; return column;}
					}
				}
			}
			item = null;
			return -1;
		}

		public Rectangle GetSubItemBounds(ListViewItem Item, int SubItem) {
			if (SubItem >= this.Columns.Count)
				throw new IndexOutOfRangeException("SubItem " + SubItem + " out of range");
			else if (Item == null)
                return Rectangle.Empty; // throw new ArgumentNullException("Item");

			int top, left, width, height;
			//Item.EnsureVisible();
            Rectangle bounds = Item.GetBounds(ItemBoundsPortion.Entire);
			top = bounds.Top;
			width = this.Columns[SubItem].Width;
            height = bounds.Height;

            left = bounds.Left;
			for(int index=0;index<SubItem;index+=1)
				left += this.Columns[index].Width;

			return new Rectangle(new Point(left,top),new Size(width,height));
		}
        
        /// <summary>
        /// Used to recreate the list items when columns are modified
        /// </summary>
		public void RecopyListItems() {
			this.BeginUpdate();
			string[] news;
            ListViewItem lvi;
			for(int outerindex=0; outerindex!=this.Items.Count; outerindex+=1) {
                lvi = this.Items[outerindex];
				news = new string[this.Columns.Count];

				// Can't copy more parameters than in the old one, 
				//  and can't copy more than the new one can store
				for(int index=0;index!=lvi.SubItems.Count&&index!=news.Length;index+=1) {
					news[index] = lvi.SubItems[index].Text;
				}
				this.Items[outerindex] = new ListViewItem(news);
			}
			this.EndUpdate();
		}

		private void ListViewES_Leave(object sender, EventArgs e) {
			EndEditing(true);
		}
        //[Category("Action")]
        //public event ScrollEventHandler Scrolled = null;

		[Serializable]
			public delegate void LabelSubEditEventHandler(
			object sender,
			LabelSubEditEventArgs e
			);

		[Serializable]
			public delegate void LabelSubEditEndEventHandler(
			object sender,
			LabelSubEditEndEventArgs e
			);

		public class LabelSubEditEventArgs : EventArgs {
			private int _subIndex;
			private ListViewItem _item;

			public LabelSubEditEventArgs(ListViewItem lvi, int subItem) {
				_subIndex = subItem;
				_item = lvi;
			}

			public ListViewItem Item {
				get { return _item; }
			}

			public int SubIndex {
				get { return _subIndex; }
			}
		}

		public class LabelSubEditEndEventArgs : LabelSubEditEventArgs {
			string _text;
			bool _cancel;

			public LabelSubEditEndEventArgs(ListViewItem lvi, int subItem, string text, bool cancel) :
			base(lvi,subItem) {
				_text = text;
				_cancel = cancel;
			}

			public bool Cancel {
				get { return _cancel; }
			}

			public string Text {
				get { return _text; }
			}
		} 

        private const int WM_HSCROLL = 0x114;
        private const int WM_VSCROLL = 0x115;
        
        protected override void WndProc(ref System.Windows.Forms.Message msg) {
            
            if ((msg.Msg == WM_HSCROLL) || (msg.Msg == WM_VSCROLL)) {
                if (c!=null) PositionControl();
                if (OnScroll != null) OnScroll(this,new ScrollEventArgs(ScrollEventType.EndScroll,0,0,(msg.Msg == WM_HSCROLL)?ScrollOrientation.HorizontalScroll:ScrollOrientation.VerticalScroll));
                base.WndProc(ref msg);
            }
            
            else base.WndProc(ref msg);
        }
    }
}
