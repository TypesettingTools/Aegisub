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
using System.Text.RegularExpressions;
using System.Collections;
using System.Windows.Forms;
using System.Collections.Generic;

namespace SSATool {
	public static class Evaluate {

        public static string ScriptParse(string input) {
            char[] charain = input.ToCharArray(); // It is over twice as fast to parse this as a character array!
            StringBuilder sb = new StringBuilder();

            int dIndex = 0, eIndex = 0, pIndex = 0, lastIndex = -1, parenLevel = 0, index;
            bool foundds = false;

            for (index=0;index<input.Length;index+=1) {
                if (charain[index].Equals('$')) {
                    if (parenLevel == 0) {
                        if (index != (lastIndex+1))
                            sb.Append(input, lastIndex+1, index-(lastIndex+1));
                        dIndex = index;
                        foundds = true;
                    }
                }

                else if (charain[index].Equals('(')) {
                    if ((parenLevel == 0)) pIndex = index;
                    parenLevel+=1;
                }

                else if (charain[index].Equals(')')) {
                    parenLevel--;

                    if ((parenLevel == 0) && (foundds) && (pIndex != 0)) {
                        eIndex = index;
                        sb.Append(ScriptParse_Helper(input.Substring(dIndex+1, pIndex-(dIndex+1)), ScriptParse(input.Substring(pIndex+1, index-(pIndex+1)))));
                        lastIndex = eIndex;
                        foundds = false;
                    }
                    else if (parenLevel == 0) {
                        sb.Append(input.Substring(lastIndex+1, pIndex-lastIndex) + ScriptParse(input.Substring(pIndex+1, index-(pIndex+1))) + ")");
                        lastIndex = eIndex = index;
                    }
                }
            }

            if (input.Length > lastIndex) sb.Append(input, lastIndex+1, input.Length-(lastIndex+1));

            return sb.ToString();
        }

        public static string ScriptParse_Helper(string function, string args) {
            unsafe {
                switch (function) {
                    case "eval":
                        return Evaluate.Eval(args).ToString();
                    case "logic":
                        return Evaluate.Eval(args).ToString();
                    case "iif":
                        string[] tok = args.Split(",".ToCharArray());
                        if (tok.Length != 3) return "error";
                        else if (Evaluate.Eval(tok[0]).Equals(true)) return tok[1];
                        else return tok[2];
                    case "len":
                        return args.Length.ToString(Util.nfi);
                    case "left":
                        string[] tokl = args.Split(",".ToCharArray());
                        if ((tokl.Length != 2) || (!Evaluate.isNum(tokl[1]))) return "error";
                        else try {
                                return tokl[0].Substring(0, int.Parse(tokl[1], Util.cfi));
                            } catch {
                                return "error";
                            }
                    case "mid":
                        string[] tokm = args.Split(",".ToCharArray());
                        if ((tokm.Length != 3) || (!Evaluate.isNum(tokm[1])) || (!Evaluate.isNum(tokm[2]))) return "error";
                        else try {
                                return tokm[0].Substring(int.Parse(tokm[1]), int.Parse(tokm[2], Util.cfi));
                            } catch {
                                return "error";
                            }
                    case "right":
                        string[] tokr = args.Split(",".ToCharArray());
                        if ((tokr.Length != 2) || (!Evaluate.isNum(tokr[1]))) return "error";
                        else try {
                                return tokr[0].Substring(tokr[0].Length - int.Parse(tokr[1], Util.cfi), int.Parse(tokr[1], Util.cfi));
                            } catch {
                                return "error";
                            }
                    case "str":
                        string[] str = args.Split(",".ToCharArray());
                        if ((str.Length != 2) || (!Evaluate.isNum(str[1]))) return "error";
                        else try {
                                string retstr = String.Empty;
                                for (int strindex=0;strindex<int.Parse(str[1], Util.cfi);strindex--)
                                    retstr = retstr + str[0];
                                return retstr;
                            } catch {
                                return "error";
                            }
                    case "randlist":
                        string[] splitlist = args.Split(",".ToCharArray());
                        return splitlist[Evaluate.rand.Next(0, splitlist.Length)];

                    case "listindex":
                        string[] splitlist2 = args.Split(",".ToCharArray());
                        if ((splitlist2.Length < 2) || (Evaluate.isNum(splitlist2[0]) == false)) return "error";
                        int index = int.Parse(splitlist2[0], Util.cfi);
                        if ((index < 1) || (index <= splitlist2.Length)) return "error";
                        return splitlist2[index];

                    case "listindexwrap":
                        string[] splitlist3 = args.Split(",".ToCharArray());
                        if ((splitlist3.Length < 2) || (Evaluate.isNum(splitlist3[0]) == false)) return "error";
                        int index2 = int.Parse(splitlist3[0], Util.cfi);
                        if (index2 < 1) return "error";
                        return splitlist3[index2%splitlist3.Length];

                    case "listindexlast":
                        string[] splitlist4 = args.Split(",".ToCharArray());
                        if ((splitlist4.Length < 2) || (Evaluate.isNum(splitlist4[0]) == false)) return "error";
                        int index3 = int.Parse(splitlist4[0], Util.cfi);
                        if (index3 < 1) return "error";
                        else if (index3 >= splitlist4.Length) return splitlist4[splitlist4.Length-1];
                        else return splitlist4[index3];

                    default: // Unrecognized function... Let's treat it as a string.
                        return "$" + function + "(" + args + ")";
                }
            }
        }

		public static readonly Random rand = new Random();
        private static SortedList<string, char> ops;
        private static SortedList<string, object> constants;

        private static void evaluate_stack_tops_function(Stack numStack, Stack<string> funcStack) {
            short numParams = (short)numStack.Pop();
            string func = funcStack.Pop();
			switch (func.ToLowerInvariant()) {
				case "rand" :
					if (numParams > 2) throw new ArgumentException("Eval.rand");
					switch (numParams) {
						case 0 :
							numStack.Push(rand.Next());
							break;
						case 1 :
							int randOneR = Convert.ToInt32(numStack.Pop());
							numStack.Push(rand.Next(randOneR));
							break;
						case 2 :
							int randOne, randTwo;
                            randTwo = Convert.ToInt32(numStack.Pop(), Util.nfi);
                            randOne = Convert.ToInt32(numStack.Pop(), Util.nfi);
							numStack.Push(rand.Next(randOne,randTwo));
							break;
					}
					break;
				case "randf" :
					if (numParams > 2) throw new ArgumentException("Eval.randf");

					switch (numParams) {
						case 0 :
							numStack.Push(rand.NextDouble());
							break;
						case 1 :
							double randOneRF = (double)numStack.Pop();
							numStack.Push(rand.NextDouble()*randOneRF);
							break;
						case 2 :
							double randOneF, randTwoF;
							randTwoF = (double)numStack.Pop();
							randOneF = (double)numStack.Pop();
							numStack.Push(randOneF + rand.NextDouble()*(randTwoF-randOneF));
							break;
					}
					break;
                case "randlist" :
                    int listindex = rand.Next(0, numParams - 1);
                    string randlist = string.Empty;
                    for (int i = 0; i != numParams; i += 1) {
                        if (i == listindex) randlist = (string)numStack.Pop();
                        else numStack.Pop();
                    }
                    numStack.Push(randlist);
                    break;
				case "round" :
					if (numParams != 2) throw new ArgumentException("Eval.round");
					double roundme;
					int dplaces;
					dplaces = Convert.ToInt32(numStack.Pop(), Util.nfi);
					roundme = (double) numStack.Pop();
					numStack.Push(Math.Round(roundme,dplaces));
					break;
				case "bound" :
					if (numParams != 3) throw new ArgumentException("Eval.bound");
					double num, min, max;
					max = (double) numStack.Pop();
					min = (double) numStack.Pop();
					num = (double) numStack.Pop();
					num = ((num < min) ? min : num);
					num = ((num > max) ? max : num);
					numStack.Push(num);
					break;
				case "max" :
					if (numParams < 2) throw new ArgumentException("Eval.max");
					double ansMax;
					ansMax = Math.Max((double)numStack.Pop(),(double)numStack.Pop());
					for(int index=2;index!=numParams;index+=1) {
						ansMax = Math.Max(ansMax,(double)numStack.Pop());
					}
					numStack.Push(ansMax);
					break;
				case "min" :
					if (numParams < 2) throw new ArgumentException("Eval.min");
					double ansMin;
					ansMin = Math.Min((double)numStack.Pop(),(double)numStack.Pop());
					for(int index=2;index!=numParams;index+=1) {
						ansMin = Math.Min(ansMin,(double)numStack.Pop());
					}
					numStack.Push(ansMin);
					break;
				case "ceil" :
					if (numParams != 1) throw new ArgumentException("Eval.ceil");
					double ceil;
					ceil = (double) numStack.Pop();
					numStack.Push(Math.Ceiling(ceil));
					break;
				case "floor" :
					if (numParams != 1) throw new ArgumentException("Eval.floor");
					double floor;
					floor = (double) numStack.Pop();
					numStack.Push(Math.Floor(floor));
					break;
				case "int" :
					if (numParams != 1) throw new ArgumentException("Eval.int");
					double thisnum;
					thisnum = (double) numStack.Pop();
					numStack.Push(Convert.ToInt32(thisnum,Util.nfi));
					break;
				case "abs" :
					if (numParams != 1) throw new ArgumentException("Eval.abs");
					double abs;
					abs = (double) numStack.Pop();
					numStack.Push(Math.Abs(abs));
					break;
				case "sin" :
					if (numParams != 1) throw new ArgumentException("Eval.sin");
					double sin;
					sin = (double) numStack.Pop();
					numStack.Push(Math.Sin(sin));
					break;
				case "asin" :
					if (numParams != 1) throw new ArgumentException("Eval.asin");
					double asin;
					asin = (double) numStack.Pop();
					numStack.Push(Math.Asin(asin));
					break;
				case "cos" :
					if (numParams != 1) throw new ArgumentException("Eval.cos");
					double cos;
					cos = (double) numStack.Pop();
					numStack.Push(Math.Cos(cos));
					break;
				case "acos" :
					if (numParams != 1) throw new ArgumentException("Eval.acos");
					double acos;
					acos = (double) numStack.Pop();
					numStack.Push(Math.Acos(acos));
					break;
				case "tan" :
					if (numParams != 1) throw new ArgumentException("Eval.tan");
					double tan;
					tan = (double) numStack.Pop();
					numStack.Push(Math.Tan(tan));
					break;
				case "atan" :
					if (numParams != 1) throw new ArgumentException("Eval.atan");
					double atan;
					atan = (double) numStack.Pop();
					numStack.Push(Math.Atan(atan));
					break;
				case "atan2" :
					if (numParams != 1) throw new ArgumentException("Eval.atan2");
					double atanx,atany;
					atanx = (double) numStack.Pop();
					atany = (double) numStack.Pop();
					numStack.Push(Math.Atan2(atanx,atany));
					break;
				case "log" :
					if (numParams != 1) throw new ArgumentException("Eval.log");
					double log;
					log = (double) numStack.Pop();
					numStack.Push(Math.Log(log));
					break;
				case "sqrt" :
					if (numParams != 1) throw new ArgumentException("Eval.sqrt");
					numStack.Push(Math.Sqrt((double)numStack.Pop()));
					break;
                case "len" :
                    if (numParams != 1) throw new ArgumentException("Eval.len");
                    numStack.Push(((string)numStack.Pop()).Length);
                    break;
                case "fact" :
                case "factorial" :
                    if (numParams != 1) throw new ArgumentException("Eval.fact");
                    numStack.Push(Convert.ToDouble(Factorial(Convert.ToInt32(numStack.Pop(), Util.nfi)), Util.nfi));
                    break;
                case "gcd" :
                    if (numParams != 2) throw new ArgumentException("Eval.gcd");
                    numStack.Push(EuclidGCD(Convert.ToInt32(numStack.Pop(), Util.nfi), Convert.ToInt32(numStack.Pop(), Util.nfi)));
                    break;
                case "lcm" :
                    if (numParams != 2) throw new ArgumentException("Eval.lcm");
                    int lcm2 = Convert.ToInt32(numStack.Pop(),Util.nfi);
                    int lcm1 = Convert.ToInt32(numStack.Pop(),Util.nfi);
                    numStack.Push((lcm1+lcm2)/EuclidGCD(lcm1,lcm2));
                    break;
                default :
                    if (numParams==1 && numStack.Peek() is string) // might be a string w/o quotes, so push it back on as one
                        numStack.Push(func+"("+numStack.Pop()+")");
                    break;
			}
		}
        private static void evaluate_stack_tops(Stack numStack, System.Collections.Generic.Stack<char> opStack, System.Collections.Generic.Stack<string> funcStack, bool safetyKill) {
            if (((numStack.Count != 0) || (funcStack.Count != 0)) && (opStack.Count != 0) && (!opStack.Peek().Equals(","))) {
                char Operation = opStack.Pop();
                if (Operation == '@') { // @ is the placeholder for a function. The function itself is in funcStack.
                    evaluate_stack_tops_function(numStack, funcStack);
                }
                else if (Operation == '~') {
                    object obj = numStack.Pop();
                    if (obj is bool) numStack.Push(!(bool)obj);
                    else if (obj is int) numStack.Push(~(int)obj);
                    else numStack.Push(obj);
                }
                else if (Operation == '!') {
                    int fact = Convert.ToInt32(numStack.Pop(), Util.cfi);
                    numStack.Push(Convert.ToDouble(Factorial(fact), Util.nfi));
                }
                else if (numStack.Count >= 2) {
                    object objOne, objTwo;
                    objTwo = numStack.Pop();
                    objOne = numStack.Pop();
                    try {
                        switch (Operation) {
                            case 'c': // concatenation
                                numStack.Push((string)objOne + objTwo.ToString());
                                break;
                            case '^': // power
                                numStack.Push(Math.Pow((double)objOne, (double)objTwo));
                                break;
                            case '*': // multiplication
                                numStack.Push((double)objOne * (double)objTwo);
                                break;
                            case '/': // division
                                numStack.Push((double)objOne / (double)objTwo);
                                break;
                            case '%': // modulus
                                //numStack.Push((int)(double)objOne%(int)(double)objTwo);
                                numStack.Push(Math.IEEERemainder((double)objOne, (double)objTwo));
                                break;
                            case '+': // addition
                                numStack.Push((double)objOne + (double)objTwo);
                                break;
                            case '-': // subtraction
                                numStack.Push((double)objOne - (double)objTwo);
                                break;
                            case '>': // greater than
                                numStack.Push(doCompare(objOne, objTwo, false) >= 0);
                                break;
                            case 'G': // greater than or equal to
                                numStack.Push(doCompare(objOne, objTwo, false) == 1);
                                break;
                            case '<': // less than
                                numStack.Push(doCompare(objOne, objTwo, false) == -1);
                                break;
                            case 'L': // less than or equal to
                                numStack.Push(doCompare(objOne, objTwo, false) <= 0);
                                break;
                            case '=': // case-sensitive equal
                                numStack.Push(doCompare(objOne, objTwo, false) == 0);
                                break;
                            case 'i': // case-insensitive equal
                                numStack.Push(doCompare(objOne, objTwo, true) == 0);
                                break;
                            case 'N': // case-sensitive not equal
                                numStack.Push(doCompare(objOne, objTwo, false) != 0);
                                break;
                            case 'I': // case-insensitive not equal
                                numStack.Push(doCompare(objOne, objTwo, true) != 0);
                                break;
                            case '&': // bitwise-and
                                numStack.Push((int)objOne & (int)objTwo);
                                break;
                            case 'X': // bitwise-xor
                                numStack.Push((int)objOne ^ (int)objTwo);
                                break;
                            case '|': // bitwise-or
                                numStack.Push((int)objOne | (int)objTwo);
                                break;
                            case 'A': // boolean and
                                numStack.Push((bool)objOne && (bool)objTwo);
                                break;
                            case 'O': // boolean or
                                numStack.Push((bool)objOne || (bool)objTwo);
                                break;
                            case 'r': // shift right
                                numStack.Push((int)objOne >> (int)objTwo);
                                break;
                            case 'l': // shift left
                                numStack.Push((int)objOne << (int)objTwo);
                                break;
                        }
                    } catch { throw new FormatException(); }
                }
                // Can't make sense of the stacks, so just clear them...
                else if (safetyKill) {
                    funcStack.Clear();
                    opStack.Clear();
                    numStack.Clear();
                }
            }

        }
        private static int doCompare(object objOne, object objTwo, bool caseIns) {
            if (objOne is string || objTwo is string)
                return String.Compare(objOne.ToString(), objTwo.ToString(), caseIns);
            else if (objOne is bool && objTwo is bool)
                return ((bool)objOne).CompareTo((bool)objTwo);
            else if (objOne is double && objTwo is double)
                return ((double)objOne).CompareTo((double)objTwo);
            else if (objOne is bool && objTwo is double)
                return ((bool)objOne).CompareTo((double)objTwo!=0.0);
            else if (objOne is double && objTwo is bool)
                return ((double)objOne!=0.0).CompareTo((bool)objTwo);
            /*
            else if (objOne is int && objTwo is int)
                return ((int)objOne).CompareTo((int)objTwo);
            else if (objOne is bool && objTwo is int)
                return ((bool)objOne).CompareTo((int)objTwo!=0);
            else if (objOne is int && objTwo is bool)
                return ((int)objOne!=0).CompareTo((bool)objTwo);
            else if (objOne is double && objTwo is int)
                return ((double)objOne).CompareTo((double)(int)objTwo);
            else if (objOne is int && objTwo is double)
                return ((double)(int)objOne).CompareTo((double)objTwo);
            */
            throw new ArgumentException("Eval.doCompare");
        }
		public static bool isNum(string input) {
			if (input.Length == 0) return false;
			char[] charain = input.ToCharArray();
			for (int index=(input.StartsWith("-")?1:0);index<input.Length;index+=1)
				if ((!char.IsDigit(charain[index])) && (charain[index] != '.')) return false;
			return true;
		}
        public static int EuclidGCD(int u, int v) {
            int k = 0;
            if ((u|v)==0) return 0;
            while (((u|v)&1)==0) { /* while both u and v are even */
                u >>= 1;   /* shift u right, dividing it by 2 */
                v >>= 1;   /* shift v right, dividing it by 2 */
                k+=1;       /* add a power of 2 to the final result */
            }
            /* At this point either u or v (or both) is odd */
            do {
                if ((u & 1) == 0)      /* if u is even */
                    u >>= 1;           /* divide u by 2 */
                else if ((v & 1) == 0) /* else if v is even */
                    v >>= 1;           /* divide v by 2 */
                else if (u >= v)       /* u and v are both odd */
                    u = (u-v) >> 1;
                else                   /* u and v both odd, v > u */
                    v = (v-u) >> 1;
            } while (u > 0);
            return v << k;  /* returns v * 2^k */
        }
        public static int Factorial(int x) {
            int result=1;
            for (int index = x; index != 0; index--) {
                result *= index;
            }
            return result;
        }

        private static byte opOrder(char tOp) {
            byte retVal;
            switch (tOp) {
                case '!': // factorial
                    retVal = 20;
                    break;
                case '@': // function
                    retVal=16;
                    break;
                case ',': // function argument separator
                    retVal=15;
                    break;
                case 'c': // concatenation
                    retVal=14;
                    break;
                case '~': // inversion
                    retVal=13;
                    break;
                case '^': // power
                    retVal=12;
                    break;
                case '*': // multiplication
                case '/': // division
                case '%': // modulus
                case '\\': // integer division
                    retVal=11;
                    break;
                case '+': // addition
                case '-': // subtraction
                    retVal=10;
                    break;
                case '>': // greater than
                case 'G': // greater than or equal to
                case '<': // less than
                case 'L': // less than or equal to
                    retVal=8;
                    break;
                case '=': // case-sensitive equal
                case 'i': // case-insensitive equal
                case 'N': // case-sensitive not equal
                case 'I': // case-insensitive not equal
                    retVal=7;
                    break;
                case '&': // bitwise-and
                    retVal=6;
                    break;
                case 'X': // bitwise-xor
                    retVal=5;
                    break;
                case '|': // bitwise-or
                    retVal=4;
                    break;
                case 'A': // boolean and
                    retVal=3;
                    break;
                case 'O': // boolean or
                    retVal=2;
                    break;
                case 'r': // shift right
                case 'l': // shift left
                    retVal=9;
                    break;
                default: // not an operation
                    retVal=0;
                    break;
            }
            return retVal;
        }
        public static bool isOp(char i, short pos) {
            if (pos==1) return ("!+-*/\\%^&|=<>~".IndexOf(i) != -1);
            else if (pos==2) return ("&|^=)".IndexOf(i) != -1);
            else if (pos==3) return i=='=';
            return false;
        }
        public static void FillOpsList() {
            ops = new SortedList<string, char>(23);
            ops.Add(">>", 'r');
            ops.Add(">=", 'G');
            ops.Add(">", '>');
            ops.Add("===", '=');
            ops.Add("==", '=');
            ops.Add("=", 'i');
            ops.Add("<=", 'L');
            ops.Add("<<", 'l');
            ops.Add("<", '<');
            ops.Add("+", '+');
            ops.Add("||", 'O');
            ops.Add("|", '|');
            ops.Add("^^", 'X');
            ops.Add("^", '^');
            ops.Add("/", '/');
            ops.Add("*", '*');
            ops.Add("&&", 'A');
            ops.Add("&", '&');
            ops.Add("%", '%');
            ops.Add("-", '-');
            ops.Add("!==", 'I');
            ops.Add("!=", 'N');
            ops.Add("!", '!');
            ops.Add("~", '~');

            constants = new SortedList<string, object>(6);
            constants.Add("e", Math.E);
            constants.Add("false",false);
            constants.Add("g", 1.6180339887498948482045868);
            constants.Add("pi", Math.PI);
            constants.Add("true", true);
            constants.Add("y", 0.5772156649015328606065120);
        }

        public static object Eval(string input) {
            char[] charain = input.ToCharArray();
            Stack<char> opStack = new Stack<char>(16);
            Stack numStack = new Stack(16);
            Stack<string> funcStack = new Stack<string>(8);

            string buff;
            int i = 0, pLevel;
            char thisOp;
            short loop, numParams;
            bool lastwasop = true;
            bool isnumber, isfloat, infunc, inquotes, unknownString = false;

            while (i < charain.Length) {
                if (charain[i].Equals('(')) {
                    pLevel = 1;
                    buff = string.Empty;
                    while (pLevel!=0 && ++i<charain.Length) {
                        if (charain[i].Equals('(')) pLevel+=1;
                        if (charain[i].Equals(')')) pLevel--;
                        if (pLevel!=0 || !charain[i].Equals(')')) buff += charain[i];
                    }
                    numStack.Push(Eval(buff));
                    lastwasop = false;
                }
                else if ((!lastwasop||charain[i]!='-'||(i!=0&&charain[i-1]=='!')) && isOp(charain[i], 1)) {
                    buff = charain[i].ToString(Util.cfi);
                    loop=1;
                    while (++i<charain.Length && loop++<=3 && isOp(charain[i], loop))
                        buff += charain[i];
                    i--;
                    thisOp = ops[buff];
                    if (!unknownString && thisOp.Equals('+') && numStack.Count!=0 && numStack.Peek() is string) thisOp='c';
                    while (opStack.Count!=0 && numStack.Count!=0 && opOrder(thisOp)<=opOrder(opStack.Peek()))
                        evaluate_stack_tops(numStack, opStack, funcStack, false);
                    opStack.Push(thisOp);
                    lastwasop=true;
                    unknownString = false;
                }

                else if (charain[i] != ' ') {
                    buff = string.Empty;
                    if (charain[i].Equals('"')) {
                        while (i++!=charain.Length && charain[i]!='"')
                            buff += charain[i];
                        numStack.Push(buff);
                    }
                    else {
                        isnumber = true;
                        isfloat = infunc = inquotes = false;
                        pLevel = numParams = 0;

                        if (charain[i].Equals('-')) {
                            buff = "-";
                            i+=1;
                        }
                        while (i!=charain.Length && (!isOp(charain[i], 1) || infunc)) {
                            if (charain[i].Equals('(')) {
                                if (pLevel==0) {
                                    funcStack.Push(buff);
                                    buff = string.Empty;
                                    opStack.Push('@');
                                    infunc = true;
                                }
                                else buff += '(';
                                pLevel+=1;
                            }
                            else if (infunc && charain[i]==')') {
                                pLevel--;
                                if (pLevel==0) {
                                    numStack.Push(Eval(buff));
                                    numParams+=1;
                                }
                                else buff += ')';
                            }
                            else if (infunc && pLevel==1 && charain[i]==',') {
                                numStack.Push(Eval(buff));
                                numParams+=1;
                                buff = string.Empty;
                            }
                            else {
                                if (inquotes || charain[i]!=' ') {
                                    if (charain[i] == '"') inquotes = !inquotes;
                                    if (!(char.IsDigit(charain[i])||charain[i]=='.')) isnumber = false;
                                    if (charain[i] == '.') isfloat=true;
                                    buff += charain[i];
                                }
                            }
                            i+=1;
                        }

                        if (!infunc) {
                            if (isnumber) {
                                if (isfloat = true) numStack.Push(double.Parse(buff, Util.cfi)); // effectively disable ints (for now, at least)
                                else numStack.Push(int.Parse(buff, Util.cfi));
                            }

                            else if (constants.ContainsKey(buff.ToLowerInvariant())) numStack.Push(constants[buff.ToLowerInvariant()]);

                            else {
                                numStack.Push(buff);
                                //unknownString = true;
                            }
                        }
                        else numStack.Push(numParams);
                        i--;
                    }

                    lastwasop = false;
                }

                i+=1;
            }

            while (opStack.Count!=0 && numStack.Count!=0)
                evaluate_stack_tops(numStack, opStack, funcStack, false);

            if (numStack.Count != 0) return numStack.Pop();
            else return false;
        }
	}
}