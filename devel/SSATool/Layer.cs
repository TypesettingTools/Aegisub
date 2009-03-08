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

namespace SSATool
{
	public class Layer : ConditionColl, IEnumerable, ICloneable {
		private List<Filter> filterColl;
        public string Name;
		public int Repetitions;
		public bool Enabled;
		public bool PerSyllable, SyllablePerLine;
        public bool AddAll, AddOnce;
        public bool AddK, AddASSA, AddBracket, AddText;

		public Layer() {
			filterColl = new List<Filter>();
			Enabled = true;
            PerSyllable = SyllablePerLine = AddAll = AddOnce = false;
            AddK = AddASSA = AddBracket = AddText = true;
			Repetitions = 1;
		}

		public int Count {
			get { return filterColl.Count; }
		}

		public Filter GetFilter(int index) {
			return filterColl[index];
		}

		public void AddFilter(Filter tf) {
			filterColl.Add(tf);
		}

		public void InsertFilter(int index, Filter tf) {
			filterColl.Insert(index,tf);
		}

		public void RemoveFilter(int index) {
			if (filterColl.Count > index) filterColl.RemoveAt(index);
		}

		public void SwapFilterPositions(int indexone, int indextwo) {
			if ((filterColl.Count > Math.Max(indexone,indextwo)) && (indexone != indextwo)) {
				Filter swap;
				swap = filterColl[indexone];
				filterColl[indexone] = filterColl[indextwo];
				filterColl[indextwo] = swap;
			}
		}


        public List<Filter> CloneFilters() {
            List<Filter> nl = new List<Filter>();
			for (int index=0;index!=filterColl.Count;index+=1)
				nl.Add((Filter)filterColl[index].Clone());
			return nl;
		}

		public object Clone() {
			Layer nl = (Layer)this.MemberwiseClone();
			nl.filterColl = CloneFilters();
            nl.conditionColl = CloneConditions();
			return nl;
		}


		public IEnumerator GetEnumerator() {
			return new LayerEnumerator(this);
		}


	}

	public class LayerEnumerator : IEnumerator {
		Layer l;
		int index;

		#region IEnumerator Members

		public void Reset() {
			index = -1;
		}

		public object Current {
			get {
				return l.GetFilter(index);
			}
		}

		public bool MoveNext() {
			if (++index >= l.Count) return false;
			else return true;
		}

		#endregion

		internal LayerEnumerator(Layer l) {
			this.l = l;
			Reset();
		}

	}

}
