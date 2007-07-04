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
using System.Text;
using System.Windows.Forms;

namespace SSATool {
	public class ManualTransform {

		public static MTVars ListViewToMTV(ListView lv) {
			double expbase, opt;
			MTVars mtv = new MTVars();
			MTVars.MTVariable mtvar;
            ListViewItem lvi;
			string text;

			for(int lvindex=0;lvindex!=lv.Items.Count;lvindex+=1){
                lvi = lv.Items[lvindex];
				mtvar = new MTVars.MTVariable(lvi.Text);
				for(int index=1;index<lvi.SubItems.Count;index+=2) {
					text = lvi.SubItems[index].Text;
					if (double.TryParse(text,out opt)) mtvar.AddOption(opt);
					else mtvar.AddOption(text);
				}
				for(int index=2;index<lvi.SubItems.Count;index+=2) {
					text = lvi.SubItems[index].Text.ToLower(Util.cfi);
					Util.ScaleRatioS srs = new Util.ScaleRatioS();
					expbase = 1;

                    if (double.TryParse(text, out opt)) {
						srs.srt = Util.ScaleRatioType.Polynomial;
						expbase = opt;
					}
					if ((text.StartsWith("log")) && (double.TryParse(text.Substring(3,text.Length-3), out opt))) {
						srs.srt = Util.ScaleRatioType.Logarithmic;
						expbase = opt;
					}
                    else if ((text.StartsWith("exp")) && (double.TryParse(text.Substring(3, text.Length-3), out opt))) {
						srs.srt = Util.ScaleRatioType.Exponential;
						expbase = opt;
					}

					srs.expbase = expbase;
					mtvar.AddAccel(srs);
				}
				mtv.AddVariable(mtvar);
			}
			return mtv;
		}

		public static string DoTransform(List<TimeSpan> TransformTimes, MTVars Vars, string Code, double FrameRate) {
			double fp = 1.0/FrameRate;
			double ratio;
			TimeSpan frame = TimeSpan.FromSeconds(fp);
			TimeSpan curtime;
			StringBuilder sb = new StringBuilder(6144);
            MTVars.MTVariable mtv;
			string rep, repwith, output;
			int index, windex, comp;

			/* The times are already sorted because the listbox is sorted.
				 * We'll loop from the first time to the last time
				 * and figure out which section we're in then.
				 * I'm doing this because rounding problems could otherwise
				 * cause us to miss a frame between sections or give us an
				 * invalid offset (for example, each frame was .02 seconds off in time)
				 */

			index = windex = 0;
            curtime = TransformTimes[0];
            TransformTimes[TransformTimes.Count-1].Add(frame);
			while(curtime.CompareTo(TransformTimes[TransformTimes.Count-1]) == -1) {
				// Find out what zone we're in (0-based)
				while((comp = curtime.CompareTo(TransformTimes[windex])) == -1)
					windex+=1;

				sb.Append(
                    Code.Replace("%starttime%",
                    Util.TimeSpanSSA(
                        (index==0&&curtime.CompareTo(TimeSpan.Zero)==-1)?TimeSpan.Zero:curtime,false,1))
					.Replace("%endtime%",Util.TimeSpanSSA(curtime=curtime.Add(frame),false,1))
					); // curtime is incremented in the line above this, notice the single equals


                for(int mtindex=0;mtindex!=Vars.Count;mtindex+=1){
                    mtv = Vars.GetVariable(mtindex);
					ratio = Convert.ToDouble(curtime.Subtract(TransformTimes[0]).Ticks,Util.nfi)
                        / Convert.ToDouble(((TransformTimes[windex+1]).Subtract(frame.Add(TransformTimes[windex]))).Ticks,Util.nfi);

					rep = "%" + mtv.Name + "%";
					repwith = mtv.Value(windex,ratio).ToString();


					sb.Replace(rep,repwith);
				}
                sb.AppendLine();

				index+=1;
			}
			output = sb.ToString().TrimEnd();
			if (Code.Contains("$")) return Evaluate.ScriptParse(output);
			return output;
		}
		public class MTVars {
			private List<MTVariable> varList;

			public MTVars() {
				varList = new List<MTVariable>(4);
			}

			public int Count {
				get { return varList.Count; }
			}

			public void AddVariable(MTVariable mtvar) {
				varList.Add(mtvar);
			}

			public MTVariable GetVariable(int index) {
				return varList[index];
			}
	
			public class MTVariable {
				private ArrayList optList;
				private List<Util.ScaleRatioS> accelList;
				private string _name;

				public object Value(int index) {
					return optList[index];
				}

				public object Value(int index, double ratio) {
					if ((optList[index] is string) || (optList[index+1] is string))
						return optList[index];
					else return (double)optList[index] + Util.ScaleRatio(ratio,accelList[index])
						 *((double)optList[index+1]-(double)optList[index]);
				}

				public MTVariable(string name) {
					optList = new ArrayList();
					accelList = new List<Util.ScaleRatioS>();
					_name = name;
				}

				public void AddOption(object opt) {
					optList.Add(opt);
				}

				public void AddAccel(Util.ScaleRatioS srs) {
					accelList.Add(srs);
				}

				public string Name {
					get { return _name; }
				}
			}

		}
	}
}
