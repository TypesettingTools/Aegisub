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
using System.Collections.Generic;
using System.Text;
using System.Drawing;

namespace SSATool {
    public enum LineType {
        dialogue,
        comment,
        style,
        other,
        error
    }
    public class DialogueLine : ICloneable {
        public TimeSpan start, end;
        public Style style;
        public string actor, effect;
        public string text;
        public int marginl, marginr, margint, marginb;
        public int layer;
        public bool marked;

        public override string ToString() {
            string retStr = String.Format("Dialogue: {0},{1},{2},{3},{4},{5:D4},{6:D4},{7:D4},{8}{9},{10}",
                (Form1.sver==4)?("Marked="+(this.marked?"1":"0")):this.layer.ToString(Util.nfi),
                Util.TimeSpanSSA(this.start,false,1),
                Util.TimeSpanSSA(this.end,true,1),
                this.style.name, this.actor,
                this.marginl, this.marginr, this.margint,
                ((Form1.sver == 6) ? this.marginb.ToString(Util.nfi).PadLeft(4, '0') + "," : String.Empty),
                this.effect,this.text);
            return retStr;
        }

        public Object Clone() {
            return this.MemberwiseClone(); // A shallow memberwise-clone is fine
        }

        /// <summary>
        /// Return just the text of the line without any SSA
        /// </summary>
        /// <returns></returns>
        public string JustText {
            get {
                string justtext = string.Empty;
                char[] linechar = this.text.ToCharArray();
                bool inCode = false;
                for (int charindex = 0; charindex != this.text.Length; charindex+=1) {
                    if (linechar[charindex] == '{') inCode = true;
                    else if (linechar[charindex] == '}') inCode = false;
                    else if (!inCode) justtext = justtext + linechar[charindex];
                }
                return justtext;
            }
        }

        /// <summary>
        /// Return the margin of this line, taking overrides into account
        /// </summary>
        /// <returns>The margin as a rectangle</returns>
        public RectangleF GetMargin() {
            int Marginl = (this.marginl==0)?this.style.marginl:this.marginl;
            int Margint = (this.margint==0)?this.style.margint:this.margint;
            int Marginb = (this.marginb==0)?this.style.marginb:this.marginb;
            int Marginr = (this.marginr==0)?this.style.marginr:this.marginr;
            int Height = Form1.ResY - (Margint+Marginb);
            int Width = Form1.ResX - (Marginl+Marginr);
            return new RectangleF(Marginl, Margint, Width, Height);
        }

        /// <summary>
        /// Find the bounding rectangle of an area of the line
        /// </summary>
        /// <param name="startIndex"></param>
        /// <param name="length"></param>
        /// <returns>The bounding rectangle</returns>
        public RectangleF GetRect(int startIndex, int length) {
            Graphics graphics = Form1.listSSAg;
            StringFormat format  = new System.Drawing.StringFormat();
            CharacterRange[] ranges  = { new CharacterRange(startIndex, length) };
            Region[] regions = new Region[1];
            RectangleF margin, rect;
            margin = this.GetMargin();
            format.SetMeasurableCharacterRanges(ranges);
            int align = this.style.scrAlignment;
            switch (align&3) {
                case 1:
                    format.LineAlignment = StringAlignment.Near;
                    break;
                case 2:
                    format.LineAlignment = StringAlignment.Center;
                    break;
                default:
                    format.LineAlignment = StringAlignment.Far;
                    break;
            }
            switch (align&12) {
                case 0:
                    format.Alignment = StringAlignment.Far;
                    break;
                case 4:
                    format.Alignment = StringAlignment.Near;
                    break;
                default:
                    format.Alignment = StringAlignment.Center;
                    break;
            }
            

            regions = graphics.MeasureCharacterRanges(this.JustText, this.style.GetFont(), margin, format);
            rect = regions[0].GetBounds(graphics);
            rect = new RectangleF(margin.X+rect.X, margin.Y+rect.Y, rect.Width, rect.Height);
            return rect;
        }

        public static DialogueLine ParseLine(string theLine, LineType lt) {
            try {
                DialogueLine retLine = new DialogueLine();
                char[] linechar = theLine.ToCharArray();
                string buff = String.Empty;
                int pos = 0;
                bool neg = false;
                for (int index = (Form1.sver == 4) ? 17 : 9; index != linechar.Length; index+=1) {
                    if ((pos<9||pos==9&&Form1.sver==6) && linechar[index].Equals(',')) {
                        switch (pos) {
                            case 0:
                                if (Form1.sver == 4) retLine.marked = linechar[index - 1].Equals('1');
                                else retLine.layer = int.Parse(buff, Util.cfi);
                                break;
                            case 1:
                                if (neg) retLine.start = TimeSpan.Parse(buff.Replace("-", String.Empty)).Negate();
                                else retLine.start = TimeSpan.Parse(buff);
                                break;
                            case 2:
                                if (neg) retLine.end = TimeSpan.Parse(buff.Replace("-", String.Empty)).Negate();
                                else retLine.end = TimeSpan.Parse(buff);
                                break;
                            case 3:
                                retLine.style = Form1.styleColl.Find(delegate(Style s) { return s.Equals(buff); });
                                if (retLine.style == null) retLine.style = new Style(buff);
                                break;
                            case 4:
                                retLine.actor = buff;
                                break;
                            case 5:
                                retLine.marginl = int.Parse(buff, Util.cfi);
                                break;
                            case 6:
                                retLine.marginr = int.Parse(buff, Util.cfi);
                                break;
                            case 7:
                                retLine.margint = int.Parse(buff, Util.cfi);
                                break;
                            case 8:
                                if (Form1.sver < 6) retLine.effect = buff;
                                else retLine.marginb = int.Parse(buff, Util.cfi);
                                break;
                            case 9:
                                retLine.effect = buff;
                                break;
                        }
                        pos += 1;
                        buff = string.Empty;
                        neg = false;
                    }
                    else if ((pos != 0 || char.IsDigit(linechar[index])) && (pos == 3 || pos == 4 || pos >= 8 || !char.IsWhiteSpace(linechar[index]))) {
                        buff += linechar[index];
                        neg = neg || (linechar[index] == '-');
                    }
                }
                retLine.text = buff;
                return retLine;
            } catch {
                throw new FormatException();
            }
        }
        public DialogueLine() { }
        public DialogueLine(TimeSpan StartTime, TimeSpan EndTime, Style Style) {
            start = StartTime;
            end = EndTime;
            style = Style;
        }
        public DialogueLine(string Text) {
            text = Text;
        }
        public DialogueLine(Style Style, string Text) : this(Text) {
            style = Style;
        }
    }
    public class Line : IEquatable<Line>, ICloneable  {
        public LineType lineType;
        public object line;
        public bool enabled, selected;

        public Line() {
            this.enabled = true;
        }

        public static LineType getLineType(string theLine) {
            if (theLine.Length > 20 && String.Compare(theLine.Substring(0, 9), "dialogue:", true, Util.cfi) == 0)
                return LineType.dialogue;
            else if (theLine.Length > 8 && String.Compare(theLine.Substring(0, 8), "comment:", true, Util.cfi) == 0)
                return LineType.comment;
            else if (theLine.Length > 20 && String.Compare(theLine.Substring(0, 6), "style:", true, Util.cfi) == 0)
                return LineType.style;
            else
                return LineType.other;
        }
        public static Line ParseLine(string inStr) {
            Line l = new Line();
            l.lineType = getLineType(inStr);
            switch (l.lineType) {
                case LineType.dialogue:
                    try {
                        l.line = DialogueLine.ParseLine(inStr, LineType.dialogue);
                    } catch {
                        l.lineType = LineType.error;
                        l.line = inStr;
                    }
                    break;
                case LineType.style:
                    try {
                        Style s = Style.ParseStyle(inStr);
                        l.line = s;
                        Form1.styleColl.Add(s);
                    } catch {
                        l.lineType = LineType.error;
                        l.line = inStr;
                    }
                    break;
                default:
                    if (inStr.Length > 8 && String.Compare(inStr.Substring(0, 7), "playres", true, Util.cfi) == 0) {
                        switch (inStr.ToCharArray()[7]) {
                            case 'x':
                            case 'X':
                                Form1.ResX = int.Parse(inStr.Substring(9, inStr.Length - 9), Util.cfi);

                                break;
                            case 'y':
                            case 'Y':
                                Form1.ResY = int.Parse(inStr.Substring(9, inStr.Length - 9), Util.cfi);
                                break;
                        }
                    }
                    else if (inStr.Length > 10 && String.Compare(inStr.Substring(0, 10), "scripttype", true, Util.cfi) == 0) {
                        //TODO: Improve this detection
                        inStr = inStr.TrimEnd();
                        if (inStr.EndsWith("+=1")) Form1.sver = 6;
                        else if (inStr.EndsWith("+")) Form1.sver = 5;
                        else Form1.sver = 4;
                    }
                    l.line = inStr;
                    break;
            }
            return l;
        }

        bool System.IEquatable<Line>.Equals(Line line) {
            return this.line==line.line;
        }

        public override int GetHashCode() {
            return base.GetHashCode();
        }

        public Object Clone() {
            Line retLine = new Line();
            retLine.enabled = this.enabled;
            retLine.lineType = this.lineType;
            if (this.lineType == LineType.dialogue)
                retLine.line = ((DialogueLine)line).Clone();
            else if (this.lineType == LineType.style)
                retLine.line = ((Style)line).Clone();
            else
                retLine.line = this.line;

            return retLine;
        }

        public override string ToString() {
            return line.ToString();
        }

    }
}
