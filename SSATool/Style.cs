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
using System.Windows.Forms;

namespace SSATool {
	/// <summary>
	/// (A)SSA style (copied/adapted from VSFilter)
	/// </summary>
	public class Style : ICloneable {
		public double       outlineWidth;
        public double       shadowDepth;
        public double       fontScaleX, fontScaleY; // percent
        public double       fontSpacing;            // +/- pixels
        public double       fontAngleZ, fontAngleX, fontAngleY;
        public float        fontSize;
        public string       name, fontName;
        public Color[]      colors;                 //pri,sec,outline/bg,shadow
        public int          marginl, marginr, margint, marginb;
        public int          charset;
        public int          scrAlignment;           // 1 - 9: as on the numpad, 0: default
        public int          borderStyle;            // 0: outline, 1: opaque box
        public bool         fBlur, fBold, fItalic, fUnderline, fStrikeOut;
        public bool         enabled;
        public byte         relativeTo;

		public Style() {
			// Set default values
			name = "Default";
			fontName = "Arial";
			fontSize = 20;
			colors = new System.Drawing.Color[4];
			colors[0] = System.Drawing.Color.White;
			colors[1] = System.Drawing.Color.FromArgb(0,0xFF,0xFF,0);
			colors[2] = System.Drawing.Color.Black;
			colors[3] = System.Drawing.Color.Black;

            marginl = marginr = margint = marginb = 30;
			scrAlignment = 2;
			borderStyle = 0;
			outlineWidth = 2;
			shadowDepth = 3;
			fontScaleX = fontScaleY = 100.0;
			fontSpacing = 0;
			fBlur = fBold = fItalic = fUnderline = false;
			fontAngleZ = fontAngleX = fontAngleY = 0;
            enabled = true;
		}

        public Style(string Name) : this() {
            this.name=Name;
        }

        public Object Clone() {
            return this.MemberwiseClone();
        }

        public Font GetFont() {
            float size = (float)fontSize;
            Font prelim = new Font(fontName, size);
            FontFamily ff = prelim.FontFamily;
            FontStyle fs = FontStyle.Regular;
            if (fBold) fs |= FontStyle.Bold;
            if (fItalic) fs |= FontStyle.Italic;
            if (fUnderline) fs |= FontStyle.Underline;
            if (fStrikeOut) fs |= FontStyle.Strikeout;
            
            bool vsfilter=true;
            bool scaledpi=false;
            if (vsfilter)
                size *= ff.GetEmHeight(fs) / (ff.GetCellAscent(fs) + ff.GetCellDescent(fs));
            if (scaledpi)
                size*=(72.0f/Form1.listSSAg.DpiY);
            
            return new System.Drawing.Font(fontName, size, fs);
        }

        public Rectangle GetMarginRect() {
            if (Form1.sver <= 5) marginb = margint;
            return new Rectangle(marginl,marginr,Form1.ResX-(marginl+marginr),Form1.ResY-(margint+marginb));
        }


		public override bool Equals(object obj) {
            if (obj is Style) {
                Style s = (Style)obj;
                return ((String.Equals(this.fontName, s.fontName, StringComparison.OrdinalIgnoreCase)) &&
                                        (this.fontSize.Equals(s.fontSize)) &&
                                        (this.fItalic.Equals(s.fItalic)) &&
                                        (this.fUnderline.Equals(s.fUnderline)) &&
                                        (this.fStrikeOut.Equals(s.fStrikeOut)) &&
                                        (this.colors[0].Equals(s.colors[0])) &&
                                        (this.colors[1].Equals(s.colors[1])) &&
                                        (this.colors[2].Equals(s.colors[2])) &&
                                        (this.colors[3].Equals(s.colors[3])) &&
                                        (this.scrAlignment == s.scrAlignment) &&
                                        (this.borderStyle == s.borderStyle) &&
                                        (this.outlineWidth == s.outlineWidth) &&
                                        (this.shadowDepth == s.shadowDepth) &&
                                        (this.fontScaleX == s.fontScaleX) &&
                                        (this.fontScaleY == s.fontScaleY) &&
                                        (this.fontSpacing == s.fontSpacing) &&
                                        (this.fBold == s.fBold) &&
                                        (this.fBlur == s.fBlur) &&
                                        (this.fontAngleZ == s.fontAngleZ) &&
                                        (this.fontAngleX == s.fontAngleX) &&
                                        (this.fontAngleY == s.fontAngleY));
            }
            else if (obj is string) return (String.Equals(this.name, (string)obj, StringComparison.OrdinalIgnoreCase));
            else return false;
		}

		public override int GetHashCode() {
			return base.GetHashCode ();
		}

        public override string ToString() {
            uint andMask = (Form1.sver==4)?0xFFFFFF:0xFFFFFFFF; // cut out alpha if v4 with this and

            string retStr = String.Format("Style: {0},{1},{2},&H{3:X6}&,&H{4:X6}&,&H{5:X6}&,&H{6:X6}&,{7},{8},",
                              this.name, this.fontName, this.fontSize, this.colors[0].ToArgb()&andMask,
                              this.colors[1].ToArgb()&andMask, this.colors[2].ToArgb()&andMask,
                              this.colors[3].ToArgb()&andMask, (this.fBold ? "-1" : "0"), (this.fItalic ? "-1" : "0"));

            if (Form1.sver >= 5) {
                 retStr = String.Format("{0}{1},{2},{3},{4},{5},{6},",
                             retStr,
                             (this.fUnderline ? "-1" : "0"),
                             (this.fStrikeOut ? "-1" : "0"),
                             this.fontScaleX, this.fontScaleY,
                             this.fontSpacing, this.fontAngleZ);
            }

            retStr += String.Format("{0},{1},{2},{3},{4:G4},{5:G4},{6:G4},",
                        this.borderStyle, this.outlineWidth, this.shadowDepth,
                        this.scrAlignment, this.marginl, this.marginr, this.margint);

            if (Form1.sver >= 6) retStr += Convert.ToString(this.marginb,Util.cfi) + ",";
            else if (Form1.sver == 4) retStr += Convert.ToString(this.colors[0].A,Util.cfi) + ",";
            retStr += Convert.ToString(this.charset,Util.cfi);
            if (Form1.sver >= 6) retStr += "," + this.relativeTo.ToString(Util.cfi);

            return retStr;
        }

        public static Style ParseStyle(string inStr) {
            return ParseStyle(inStr, Form1.sver);
        }

        public static Style ParseStyle(string inStr, int scriptVer) {
            int startpos = inStr.IndexOf(':')+1;
            string[] xInfo = inStr.Substring(startpos,inStr.Length-startpos).TrimStart().Split(",".ToCharArray());
            Style style = new Style();
            int index;
            uint alpha = (scriptVer != 4 ? 0 : (Util.ReadColor(xInfo[16]) << 24));
            style.name = xInfo[0];
            style.fontName = xInfo[1];
            style.fontSize = float.Parse(xInfo[2]);
            style.colors[0] = Util.uintToColor(Util.ReadColor(xInfo[3])+alpha);
            style.colors[1] = Util.uintToColor(Util.ReadColor(xInfo[4])+alpha);
            style.colors[2] = Util.uintToColor(Util.ReadColor(xInfo[5])+alpha);
            style.colors[3] = Util.uintToColor(Util.ReadColor(xInfo[6])+alpha);
            style.fBold = !String.Equals(xInfo[7], "0", StringComparison.Ordinal);
            style.fItalic = !String.Equals(xInfo[index=8], "0", StringComparison.Ordinal);
            if (scriptVer >= 5) {
                style.fUnderline = !String.Equals(xInfo[++index], "0", StringComparison.Ordinal);
                style.fStrikeOut = !String.Equals(xInfo[++index], "0", StringComparison.Ordinal);
                style.fontScaleX = double.Parse(xInfo[++index]);
                style.fontScaleY = double.Parse(xInfo[++index]);
                style.fontSpacing = double.Parse(xInfo[++index]);
                style.fontAngleZ = double.Parse(xInfo[++index]);
            }
            style.borderStyle = int.Parse(xInfo[++index]);
            style.outlineWidth = double.Parse(xInfo[++index]);
            style.shadowDepth = double.Parse(xInfo[++index]);
            style.scrAlignment = int.Parse(xInfo[++index]);
            style.marginl = int.Parse(xInfo[++index]);
            style.marginr = int.Parse(xInfo[++index]);
            style.margint = int.Parse(xInfo[++index]);
            if (scriptVer >= 6) style.marginb = int.Parse(xInfo[++index]);
            if (scriptVer == 4) index++; //alphaLevel is in v4 only and is taken care of at the top (uint alpha), so just advance the index
            style.charset = int.Parse(xInfo[++index]);
            if (scriptVer >= 6) style.relativeTo = byte.Parse(xInfo[++index]);


            //Following is code from VSFilter, but since we're not interpreting this, merely spitting it back out the way it came in, we don't currently need it
            //if (sver <= 4) style.scrAlignment = ((style.scrAlignment & 4) == 1) ? ((style.scrAlignment & 3) + 6) // top
            //                   : ((style.scrAlignment & 8) == 1) ? ((style.scrAlignment & 3) + 3) // mid
            //                   : (style.scrAlignment & 3); // bottom


            return style;
        }
	}
}
