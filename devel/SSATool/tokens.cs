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


namespace SSATool
{
	// tokens.cs
	using System;
	// The System.Collections namespace is made available:
	using System.Collections;

	// Declare the Tokens class:
	public class Tokens : IEnumerable {
		private string[] elements;

		public Tokens(string source, char[] delimiters) {
			// Parse the string into tokens:
			elements = source.Split(delimiters);
		}

        public Tokens(string source, char[] delimiters, int max) {
            // Parse the string into tokens:
            elements = source.Split(delimiters,max);
        }

		// IEnumerable Interface Implementation:
		//   Declaration of the GetEnumerator() method 
		//   required by IEnumerable
		public IEnumerator GetEnumerator() {
			return new TokenEnumerator(this);
		}

		public int Count {
			get { return elements.Length; }
		}

		// Inner class implements IEnumerator interface:
		private class TokenEnumerator : IEnumerator {
			private int position = -1;
			private Tokens t;

			public TokenEnumerator(Tokens t) {
				this.t = t;
			}

			// Declare the Reset method required by IEnumerator:
			public void Reset() {
				position = -1;
			}

			// Declare the MoveNext method required by IEnumerator:
			public bool MoveNext()
			{
				if (position < t.elements.Length - 1) {
					position++;
					return true;
				}
				else
					return false;
			}

			// Declare the Current property required by IEnumerator:
			public object Current {
				get	{ return t.elements[position]; }
			}


		}

	}
}
