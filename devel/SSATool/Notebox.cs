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
using System.IO;
using System.Windows.Forms;
using System.Xml;

namespace SSATool {
    public static class Notebox {
        public struct NoteBox {
            public string name;
            public string author;
            public string description;
            public int resx, resy;
            public List<NoteBoxLine> nblines;
            public List<Style> stylelist;
        }
        public struct NoteBoxLine {
            public int maxLines;
            public List<DialogueLine> lines;
        }
        public static List<Notebox.NoteBox> noteBoxColl;
        public static List<Style> nbStyleColl;

        public static void readNoteBoxes() {
            if (File.Exists("notebox.xml")) {
                noteBoxColl = new List<NoteBox>();
                NoteBox nb;
                NoteBoxLine thisnbl;
                List<NoteBoxLine> nblList;
                List<DialogueLine> thisNoteList;
                List<Style> slist;
                int stylever, lsetindex;
                Form1.FormMain.cmbNBStyle.Items.Clear();

                XmlDocument xDocM = new XmlDocument();
                try {
                    xDocM.Load("notebox.xml");

                    XmlNode xDoc = xDocM.SelectSingleNode("NSSA");
                    XmlNodeList notes = xDoc.SelectNodes("Note");

                    foreach (XmlNode nnode in notes) {
                        nb = new NoteBox();
                        slist = new List<Style>(3);
                        nblList = new List<NoteBoxLine>(2);

                        nb.name = (nnode.Attributes["name"] != null) ? nnode.Attributes["name"].Value : "Untitled";
                        nb.author = (nnode.Attributes["author"] != null) ? nnode.Attributes["author"].Value : "Unknown";
                        nb.description = (nnode.Attributes["desc"] != null) ? nnode.Attributes["desc"].Value : "";
                        nb.resx = (nnode.Attributes["resx"] != null) ? int.Parse(nnode.Attributes["resx"].Value) : 640;
                        nb.resy = (nnode.Attributes["resy"] != null) ? int.Parse(nnode.Attributes["resy"].Value) : 480;

                        XmlNode styles = nnode.SelectSingleNode("Styles");
                        stylever = (styles.Attributes["version"] != null) ? int.Parse(styles.Attributes["version"].Value) : 0;

                        if (styles != null) {
                            XmlNodeList stylelist = styles.SelectNodes("Style");
                            foreach (XmlNode style in stylelist) {
                                slist.Add(Style.ParseStyle(style.InnerText, stylever));
                            }
                        }
                        nb.stylelist = slist;

                        XmlNodeList lsets = nnode.SelectNodes("LSet");
                        for (lsetindex = 0; lsetindex != lsets.Count; lsetindex+=1) {
                            XmlNode lsnode = lsets[lsetindex];

                            thisNoteList = new List<DialogueLine>(16);
                            XmlNodeList lines = lsnode.SelectNodes("line");
                            foreach (XmlNode lnode in lines) {
                                if (lnode.Attributes["style"]==null)
                                    thisNoteList.Add(new DialogueLine(lnode.InnerText));
                                else
                                    thisNoteList.Add(new DialogueLine(new Style(lnode.Attributes["style"].Value),lnode.InnerText));
                            }
                            thisnbl = new NoteBoxLine();
                            thisnbl.lines = thisNoteList;
                            thisnbl.maxLines = (lsnode.Attributes["lines"] != null) ? int.Parse(lsnode.Attributes["lines"].Value) : 1;
                            nblList.Add(thisnbl);
                        }
                        nb.nblines = nblList;
                        noteBoxColl.Add(nb);
                        Form1.FormMain.cmbNBStyle.Items.Add(nb.name + " by " + nb.author);
                        Form1.FormMain.cmbNBStyle.SelectedIndex = 0;
                    }
                } catch {
                    MessageBox.Show("Error parsing noteboxes.");
                }
            }
        }

        public static void DoNotebox(string Line1, string Line2, int NoteboxIndex) {
            double scalefactorx=0, scalefactory=0;
            int lines = String.IsNullOrEmpty(Line2) ? 1 : 2;
            NoteBox nb = noteBoxColl[NoteboxIndex];
            NoteBoxLine nbl = nb.nblines.Find(delegate(NoteBoxLine n) { return n.maxLines.Equals(lines); });
            StringBuilder sb = new StringBuilder(1024);
            TimeSpan startTime, endTime;
            Line line;
            DialogueLine dl;
            nbStyleColl = new List<Style>(4);

            if (Form1.ResX > 0 && nb.resx > 0) scalefactorx = ((double)Form1.ResX / nb.resx);
            if (Form1.ResY > 0 && nb.resy > 0) scalefactory = ((double)Form1.ResY / nb.resy);
            if (scalefactorx == 0 && scalefactory != 0) scalefactorx = scalefactory;
            else if (scalefactory == 0 && scalefactorx != 0) scalefactory = scalefactorx;
            else if ((scalefactorx+scalefactory)==0) scalefactorx = scalefactory = 1.0;

            startTime = TimeSpan.Parse(Form1.FormMain.maskedTextNBStart.Text);
            endTime = TimeSpan.Parse(Form1.FormMain.maskedTextNBEnd.Text);

            Resscale rs = new Resscale(scalefactorx,scalefactory);


            for (int index = 0; index != nbl.lines.Count; index+=1) {
                line = new Line();
                line.lineType = LineType.dialogue;
                dl = (DialogueLine)(nbl.lines[index].Clone());
                dl.start = startTime;
                dl.end = endTime;
                line.line = dl;
                dl.text = dl.text.Replace("%line1%",Line1).Replace("%line2%",Line2);
                dl = rs.ScaleDialogue(dl);

                dl.style = nb.stylelist.Find(delegate(Style s) { return s.name.Equals(dl.style.name); });
                dl.style = (Style)(dl.style.Clone());
                if (nbStyleColl.Contains(dl.style) == false) nbStyleColl.Add(rs.ScaleStyle(dl.style));
                sb.AppendLine(dl.ToString());
            }
            Form1.FormMain.textNBOut.Text = sb.ToString().TrimEnd();
            
            sb = new StringBuilder(1024);
            for (int index = 0; index != nb.stylelist.Count; index+=1)
                sb.AppendLine(nb.stylelist[index].ToString());
            Form1.FormMain.textNBStyles.Text = sb.ToString().TrimEnd();
        }

        public static void CopyStyles() {
            int styleLine = -1;
            Line newLine;

            for (int index = 0; index != Form1.lineColl.Count; index+=1) {
                if (Form1.lineColl[index].lineType == LineType.style) {
                    styleLine = index;
                    break;
                }
            }
            if (styleLine != -1) {
                for (int index = 0; index != nbStyleColl.Count; index+=1) {
                    if (Form1.styleColl.Contains(nbStyleColl[index]) == false) {
                        newLine = new Line();
                        newLine.lineType = LineType.style;
                        newLine.line = nbStyleColl[index];
                        Form1.lineColl.Insert(styleLine, newLine);
                        Form1.styleColl.Add(nbStyleColl[index]);
                    }
                }
                Form1.FormMain.ExtractStyles();
            }

        }
    }
}
