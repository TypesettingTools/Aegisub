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
using System.Text.RegularExpressions;

namespace SSATool {
    public class Resscale {
        public static Regex rscale = new Regex(@"(\\[a-zA-Z]+\(?)([0-9,\-]+)", RegexOptions.IgnoreCase | RegexOptions.Singleline);
        public static Regex rscaledrawing = new Regex(@"{.*\\p(?<scale>\d+)(.*?((\s*(c|([mnlbsp](\s+(?<num>\d+\.?\d*))+)))+))+", RegexOptions.IgnoreCase | RegexOptions.Singleline);
        
        public double scalefactorx, scalefactory;

        StringBuilder sb;
        GroupCollection gc;
        Match m;
        MatchCollection mc;
        string[] split;
        string linetok;
        int lastindex, tokindex;
        int pscale;
        bool x; // x or y coordinate, used for drawing

        public Resscale(double ScaleFactorX, double ScaleFactorY) {
            scalefactorx = ScaleFactorX;
            scalefactory = ScaleFactorY;
        }

        public Style ScaleStyle(Style s) {
            //No need to scale 1:1
            if (scalefactorx==1.0 && scalefactory==1.0) return s;

            s.fontSize *= (float)scalefactory;
            s.shadowDepth *= scalefactory;
            s.outlineWidth *= scalefactory;
            s.margint = Convert.ToInt32(Math.Round(s.margint * scalefactory, 0), Util.nfi);
            s.marginb = Convert.ToInt32(Math.Round(s.marginb * scalefactory, 0), Util.nfi);
            s.marginl = Convert.ToInt32(Math.Round(s.marginl * scalefactorx, 0), Util.nfi);
            s.marginr = Convert.ToInt32(Math.Round(s.marginr * scalefactorx, 0), Util.nfi);
            return s;
        }

        public DialogueLine ScaleDialogue(DialogueLine dl) {
            //No need to scale 1:1
            if (scalefactorx==1.0 && scalefactory==1.0) return dl;

            sb = new StringBuilder(1024);
            linetok = dl.text;

            lastindex = 0;
            mc = rscale.Matches(linetok);
            for (int mindex = 0; mindex != mc.Count; mindex+=1) {
                m = mc[mindex];
                if (m.Index > lastindex)
                    sb.Append(linetok.Substring(lastindex, m.Index - lastindex));

                gc = m.Groups; //gc[1] is the command with parenthesis, gc[2] is all the params
                sb.Append(gc[1]);

                switch (gc[1].Value) {
                    case "\\pos(":
                        split = gc[2].Value.Split(",".ToCharArray());
                        sb.Append(String.Format("{0},{1}",
                            Math.Round((double.Parse(split[0], Util.nfi) * scalefactorx), 0),
                            Math.Round((double.Parse(split[1], Util.nfi) * scalefactory), 0)));
                        break;

                    case "\\org(":
                        split = gc[2].Value.Split(",".ToCharArray());
                        sb.Append(String.Format("{0},{1}",
                            Math.Round((double.Parse(split[0], Util.nfi) * scalefactorx), 0),
                            Math.Round((double.Parse(split[1], Util.nfi) * scalefactory), 0)));
                        break;

                    case "\\clip(":
                        split = gc[2].Value.Split(",".ToCharArray());

                        // if it doesn't have 4 tokens, it's either malformed or a drawing type clip
                        //  and if it's a drawing type clip, the drawing itself will be caught later
                        if (split.Length == 4) {
                            try {
                                String.Format("{0},{1},{2},{3}",
                                    Math.Round((double.Parse(split[0], Util.cfi) * scalefactorx), 0),
                                    Math.Round((double.Parse(split[1], Util.cfi) * scalefactory), 0),
                                    Math.Round((double.Parse(split[2], Util.cfi) * scalefactorx), 0),
                                    Math.Round((double.Parse(split[3], Util.cfi) * scalefactory), 0));
                            } catch { }
                        }
                        break;

                    case "\\move(":
                        split = gc[2].Value.Split(",".ToCharArray());

                        tokindex = 0;
                        for (tokindex = 0; tokindex != split.Length; tokindex+=1) {
                            if (tokindex != 1) sb.Append(",");
                            if (tokindex < 5)
                                //Tokens will alternate x and y, so tokindex&1==0 should mean x, otherwise y
                                sb.Append(Math.Round((double.Parse(split[tokindex], Util.cfi) * (((tokindex&1) == 0) ? scalefactorx : scalefactory)), 0));
                            else //5 and 6 are times, don't scale them
                                sb.Append(split[tokindex]);
                        }
                        break;

                    case "\\bord":
                        sb.Append(Math.Round((double.Parse(gc[2].Value, Util.cfi) * scalefactory), 0));
                        break;

                    case "\\shad":
                        sb.Append(Math.Round((double.Parse(gc[2].Value, Util.cfi) * scalefactory), 0));
                        break;

                    case "\\fs":
                        sb.Append(Math.Round((double.Parse(gc[2].Value, Util.cfi) * scalefactory), 0));
                        break;

                    case "\\fsp":
                        sb.Append(Math.Round((double.Parse(gc[2].Value, Util.cfi) * scalefactorx), 0));
                        break;

                    case "\\pbo":
                        sb.Append(Math.Round((double.Parse(gc[2].Value, Util.cfi) * scalefactory), 0));
                        break;

                    default:
                        sb.Append(gc[2]); // don't know what it is, don't touch the params
                        break;
                }

                lastindex = m.Index + m.Length;
            }

            if (linetok.Length > lastindex)
                sb.Append(linetok.Substring(lastindex, linetok.Length - lastindex));

            if (rscaledrawing.IsMatch(linetok)) { // linetok isn't current yet but it's enough to just find a match
                linetok = sb.ToString();
                x = true; // x first
                sb = new StringBuilder(1024);
                lastindex = 0;
                mc = rscaledrawing.Matches(linetok);
                for (int mindex = 0; mindex != mc.Count; mindex+=1) {
                    m = mc[mindex];
                    pscale = int.Parse(m.Groups["scale"].Value);
                    foreach (Capture c in m.Groups["num"].Captures) {
                        if (c.Index > lastindex)
                            sb.Append(linetok.Substring(lastindex, c.Index - lastindex));
                        sb.Append((pscale > 0) ? Math.Round(double.Parse(c.Value) * (x ? scalefactorx : scalefactory), 0).ToString(Util.cfi) : c.Value);
                        x = !x; // Alternate X and Y
                        lastindex = c.Index + c.Length;
                    }
                }

                if (linetok.Length > lastindex)
                    sb.Append(linetok.Substring(lastindex, linetok.Length - lastindex));
            }

            dl.text = sb.ToString();
            return dl;
        }
    }
}
