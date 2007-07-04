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
using System.IO;

namespace SSATool {
    public class TimeCodes {
        private float[] times;
        private double frameRate;

        public TimeCodes(string filename, double frameRate) {
            this.frameRate = frameRate;
            double AssumeFPS = 29.97;
            string thisline;
            int frames = 0, index;
            bool v1;
            StreamReader fs = new StreamReader(filename);

            thisline = fs.ReadLine(); // Save the first line now
            if (string.Compare(thisline, "# timecode format v1", true, Util.cfi) == 0) v1 = true;
            else if (string.Compare(thisline, "# timecode format v2", true, Util.cfi) == 0) v1 = false;
            else throw new InvalidDataException("File was not detected as an MKV TimeCode v1 or v2 file.");

            //count frames
            while (!fs.EndOfStream) {
                if ((fs.Peek() != '#') && ((thisline = fs.ReadLine()).Length > 3)) {
                    if (v1 == false) frames+=1;
                    else frames = Math.Max(frames, int.Parse(thisline.Split(",".ToCharArray())[1]));
                }
                else if ((fs.Peek() == 'A' || fs.Peek() == 'a') && (string.Compare(thisline.Split(" ".ToCharArray())[0], "assumefps", true, Util.cfi) == 0))
                    AssumeFPS = double.Parse(thisline.Split(" ".ToCharArray())[1]);
            }
            fs.Close();

            //restart, actually saving info
            fs = new StreamReader(filename);
            while ((fs.Peek() == '#') || (fs.Peek() == 'a') || (fs.Peek() == 'A'))
                fs.ReadLine();

            times = new float[frames];
            for (index = 0;index < frames;index+=1) {
                if (v1) {
                    string[] splitv1 = fs.ReadLine().Split(",".ToCharArray());
                    for (;index <= int.Parse(splitv1[1]);index+=1) // frames is the number of frames with v1, so index SHOULD go up for every frame
                        times[index] = ((index != 0) ? times[index - 1] : 0) + (float)(1.0 / ((splitv1.Length == 3) ? float.Parse(splitv1[2]) : AssumeFPS));
                }
                else
                    times[index] = float.Parse(fs.ReadLine());
            }
            fs.Close();

        }

        /// <summary>
        /// Finds the frame number of a given VFR time, so that frame number can be used to convert to a CFR time 
        /// </summary>
        /// <param name="time">The time in MILLISECONDS</param>
        /// <returns></returns>
        public int FindFrame(float time) {
            int k, low = 0, high = times.Length-1;
            
            while (low!=high) {
                k=(int)Math.Ceiling((high+low)/2.0f);
                if (times[k].CompareTo(time) == -1)
                    low=k;
                else if (k!=0 && times[k-1].CompareTo(time) == -1)
                    return k;
                else high=k;
            }
            return 0;
        }

        public TimeSpan GetTimeInverse(TimeSpan time) {
            int frame = System.Convert.ToInt32(time.TotalSeconds * frameRate, Util.cfi);
            return TimeSpan.FromMilliseconds((frame<times.Length)?times[frame]:0);
        }

        public TimeSpan GetTime(TimeSpan time) {
            return TimeSpan.FromSeconds(FindFrame((float)time.TotalMilliseconds) / frameRate);
        }
    }
}
