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

namespace SSATool {
    public static class DetectEncoding {
        // This detection could use some serious improving.
        // All detection should return the occurances as an integer.
        public static int DetectSJIS(byte[] input) {
            int occ=0;
            byte lastMatched=0;
            byte thisbyte;
            for (int index=0;index!=input.Length;index++) {
                thisbyte = input[index];
                if ((lastMatched==130&&thisbyte>=159&&thisbyte<=241) ||
                    (lastMatched==131&&thisbyte>=64&&thisbyte<=151)) occ++;
                else if (thisbyte==130||thisbyte==131||thisbyte>=136) lastMatched=thisbyte;
                else lastMatched=0;
            }
            return occ;
        }

        public static int DetectEUCJP(byte[] input) {
            int occ=0;
            byte lastMatched=0;
            byte thisbyte;
            for (int index=0;index!=input.Length;index++) {
                thisbyte = input[index];
                if (lastMatched>=163&&lastMatched<=250&&thisbyte>=160) occ++;
                else if (thisbyte>=163&&thisbyte<=250) lastMatched=thisbyte;
                else lastMatched=0;
            }
            return occ;


        }
    }
}
