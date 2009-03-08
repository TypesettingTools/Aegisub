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
using System.Collections.Generic; //.Net 2.0


namespace SSATool {

    public class Filter : ConditionColl, System.Collections.Generic.IEnumerable<FilterOption>, IEquatable<Filter> {
        private Filter template;
		protected List<FilterOption> fOptions;
		public string Name, Code;
		public bool Enabled;

		public Filter() : base() {
			fOptions = new List<FilterOption>();
            Enabled = true;
		}
		public Filter(List<FilterOption> optionsList) : base() {
			fOptions = optionsList;
            Code = String.Empty;
		}

        public Filter(string name) : this() {
            Name = name;
            Code = String.Empty;
        }

        public Filter(string name, string code) : this() {
            Name = name;
            Code = code;
        }

        public Filter(string name, List<FilterOption> optionsList)
            : this(optionsList) {
            Name = name;
            Enabled = true;
        }

        bool System.IEquatable<Filter>.Equals(Filter f) {
            return (String.CompareOrdinal(this.Name, f.Name) == 0);
        }

		public override string ToString() {
			return Name;
		}

		public int NumOptions {
			get { return fOptions.Count; }
		}

        public Filter Template {
            get { return template; }
        }

		public void AddOption(string Name,string Val) {
			fOptions.Add(new FilterOption(Name,Val));
		}

		public void AddOption(FilterOption fo) {
			fOptions.Add(fo);
		}
		public void SetOptionValueByIndex(int index, string val) {
			if (fOptions.Count > index) fOptions[index].Value = val;
		}
		public void SetOptionValueByName(string name, string val) {
			for (int index=0;index<fOptions.Count;index+=1)
				if (string.CompareOrdinal(fOptions[index].Name,name) == 0)
					fOptions[index].Value = val;
		}

		public void SetOptionNameByIndex(int index, string name) {
			if (fOptions.Count > index) fOptions[index].Name = name;
		}
		public void SetOptionByIndex(int index, string name, string val) {
			if (fOptions.Count > index) {
				fOptions[index].Name = name;
				fOptions[index].Value = val;
			}
		}

		public FilterOption GetOptionByIndex(int index) {
			return fOptions[index];
		}
        public string GetOptionValueByIndex(int index) {
			return fOptions[index].Value;
		}
		public FilterOption GetOptionByName(string name) {
			for(int index=0;index<fOptions.Count;index+=1) 
				if (fOptions[index].Name == name)
					return fOptions[index];

			return (FilterOption) null;
		}
		public string GetOptionValueByName(string name) {
			for(int index=0;index<fOptions.Count;index+=1)
				if (fOptions[index].Name == name)
					return fOptions[index].Value;

			return null;
		}
		public void RemoveOptionByIndex(int index) {
			fOptions.RemoveAt(index);
		}
		public void RemoveOptionByName(string name) {
			for(int index=0;index<fOptions.Count;index+=1)
				if (fOptions[index].Name == name) fOptions.RemoveAt(index);
		}

        public List<FilterOption> CloneOptions() {
            List<FilterOption> nl = new List<FilterOption>();
			for(int index=0;index!=fOptions.Count;index+=1)
				nl.Add((FilterOption)fOptions[index].Clone());
			return nl;
		}

        public object Clone() {
            Filter nf = new Filter(Name, CloneOptions());
            nf.conditionColl = CloneConditions();
            nf.template = (this.template==null)?this:this.template;
            return nf;
        }

		#region IEnumerable Members
        IEnumerator<FilterOption> IEnumerable<FilterOption>.GetEnumerator() {
            return new FilterEnumerator(this);
		}
        public System.Collections.IEnumerator GetEnumerator() {
            return new FilterEnumerator(this);
        }
		#endregion
	}

	public class FilterEnumerator : IEnumerator<FilterOption>, IDisposable {
		Filter f;
		int index;

        public void Dispose() {
            
        }
		public FilterEnumerator(Filter f) {
			this.f = f;
			Reset();
		}
		public void Reset() {
			index=-1;
		}
		object IEnumerator.Current {
			get { return f.GetOptionByIndex(index); }
		}
		public FilterOption Current {
			get { return f.GetOptionByIndex(index);	}
		}
		public bool MoveNext() {
			if (++index >= f.NumOptions) return false;
			else return true;
		}
	}

	public class FilterOption : ICloneable {
		public string Name;
		public string Value;

		public FilterOption() { }

		public FilterOption(string name, string val) {
			Name = name;
			Value = val;
		}

		public object Clone() {
			return new FilterOption(Name,Value);
		}
	}

}
