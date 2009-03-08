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
using System.Text;
using System.Drawing;
using System.Globalization;
using System.Runtime.InteropServices;

namespace SSATool {
	/// <summary>
	/// Summary description for util.
	/// </summary>
	public static class Util {
		public enum ScaleRatioType {
			Exponential,
			Logarithmic,
			Polynomial //can be linear, native to ASSA's \t
		}
		public struct ScaleRatioS {
			public double expbase;
			public ScaleRatioType srt;
		}

        //[DllImport("User32.dll")]
        //public static extern int SendMessage(IntPtr hWnd,
        //  uint msg, int wParam, int lParam);
        //public const uint LB_INITSTORAGE = 0x01A8; 

		public static double ScaleRatio(double ratio, double expbase, ScaleRatioType srt) {
			unsafe {
				switch(srt) {
					case ScaleRatioType.Polynomial :
						return Math.Pow(ratio,expbase);
					case ScaleRatioType.Logarithmic :
						return Math.Log(1.0 + (expbase-1.0)*ratio,expbase);
					default: // exponential
						return Math.Pow(expbase,ratio)/expbase;
				}
			}
		}

		public static double ScaleRatio(double ratio, ScaleRatioS srs) {
			return ScaleRatio(ratio,srs.expbase,srs.srt);
		}

		public static readonly System.Globalization.CultureInfo cfi = System.Globalization.CultureInfo.InvariantCulture;
        public static readonly System.Globalization.NumberFormatInfo nfi = cfi.NumberFormat;
        public static int Precision = 2;
        public static float PrecisionF = 10.0F;

        public static string TimeSpanSSA(TimeSpan time, bool ceil, int PadHours) {
            if (ceil) 
                time = TimeSpan.FromMilliseconds(Math.Ceiling(time.TotalMilliseconds / PrecisionF) * PrecisionF);

            float msFloat = time.Milliseconds/PrecisionF;
            int ms = Convert.ToInt32(ceil?msFloat:Math.Floor(msFloat),Util.nfi);
            string msString = Convert.ToString(ms, Util.nfi).PadLeft(Precision, '0');

            return String.Format("{0}:{1:D2}:{2:D2}.{3}",
                    time.Hours.ToString(nfi).PadLeft(PadHours, '0'),
                    time.Minutes, time.Seconds,msString,Precision);
        }
        
        /// <summary>
        /// Returns a uint from a 32-bit RGBA string. Microsoft uses an int for this, not uint, so we need this custom function. 
        /// </summary>
        /// <param name="input"></param>
        /// <returns></returns>
        public static uint ReadColor(string input) {
            char[] charain = input.ToCharArray();
            string jColor = string.Empty;
            char c;
            bool ishex = false;
            for(int index=0;index!=charain.Length;index+=1) {
                c = charain[index];
                if ((c >= 48 && c <= 57) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102)) {
                    jColor += c;
                    if (c>=65) ishex = true;
                }
                else if (c == 'H' || c == 'h') ishex = true;
            }
            return Math.Max(uint.Parse(jColor,ishex?NumberStyles.HexNumber:NumberStyles.None, cfi), 0);
        }

        public static Color uintToColor(uint input) {
            return Color.FromArgb(Convert.ToInt32(((input>>24)&0xFF),Util.nfi)
                                 ,Convert.ToInt32(((input>>16)&0xFF),Util.nfi)
                                 ,Convert.ToInt32(((input>>8)&0xFF),Util.nfi)
                                 ,Convert.ToInt32((input&0xFF),Util.nfi));
        }

        public static Point ParseCoordinate(string input) {
            //Note: This function just ignores characters it doesn't like
            char[] charain = input.ToCharArray();
            string buff = string.Empty;
            Point p = new Point();

            for (int index=0;index!=charain.Length;index+=1) {
                if (char.IsDigit(charain[index])) buff += charain[index];
                else if (charain[index].Equals(',')) {
                    p.X = int.Parse(buff, nfi);
                    buff = string.Empty;
                }
            }
            p.Y = int.Parse(buff, nfi);
            return p;
        }

        public static bool TryParseCoordinate(string input, out Point result) {
            char[] charain = input.ToCharArray();
            string buff = string.Empty;
            Point p = new Point();
            char c;
            bool FoundComma = false;

            for (int index = 0; index != charain.Length; index+=1) {
                c = charain[index];
                if (char.IsDigit(c)) buff += c;
                else if (c.Equals(',')) {
                    p.X = int.Parse(buff, nfi);
                    buff = string.Empty;
                    FoundComma = true;
                }
                else {
                    //invalid point
                    result = p; // who knows what's in here, we shouldn't be using it when false is returned anyway
                    return false;
                }
            }
            if (!FoundComma || String.IsNullOrEmpty(buff)) {
                result = p; // who knows what's in here, we shouldn't be using it when false is returned anyway
                return false; //Invalid/empty string = invalid point
            }

            p.Y = int.Parse(buff, nfi);
            result = p;
            return true;
        }
    }
}
