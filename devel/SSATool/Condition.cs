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
//using System.Collections;
using System.Collections.Generic;

namespace SSATool
{
	public class Condition : ICloneable {
		public string ConditionOne;
		public string ConditionOp;
		public string ConditionTwo;
		public bool ConditionEnabled;

		public Condition() { }

		public Condition(string ConditionOne, string ConditionOp, string ConditionTwo, bool ConditionEnabled) {
			this.ConditionOne = ConditionOne;
			this.ConditionOp = ConditionOp;
			this.ConditionTwo = ConditionTwo;
			this.ConditionEnabled = ConditionEnabled;
		}

		public object Clone() {
            return this.MemberwiseClone();
		}
	}

	public class ConditionColl {
        protected List<Condition> conditionColl;

		public ConditionColl() {
			conditionColl = new List<Condition>();
		}

		public int ConditionCount {
			get { return conditionColl.Count; }
		}
		public void AddCondition(string CondOne, string CondOp, string CondTwo) {
			Condition newCond = new Condition(CondOne,CondOp,CondTwo,true);
			conditionColl.Add(newCond);
		}

		public void AddCondition(string CondOne, string CondOp, string CondTwo, bool CondEnabled) {
			Condition newCond = new Condition();
			newCond.ConditionOne = CondOne;
			newCond.ConditionOp = CondOp;
			newCond.ConditionTwo = CondTwo;
			newCond.ConditionEnabled = CondEnabled;
			conditionColl.Add(newCond);
		}

		public void AddCondition(Condition tc) {
			conditionColl.Add(tc);
		}

		public Condition GetCondition(int index) {
			return conditionColl[index];
		}

		public void RemoveCondition(int index) {
			if (conditionColl.Count > index) conditionColl.RemoveAt(index);
		}

		public List<Condition> CloneConditions() {
            List<Condition> nl = new List<Condition>();
			for(int index=0;index!=conditionColl.Count;index+=1)
				nl.Add((Condition)conditionColl[index].Clone());
            return nl;
		}
	}
}
