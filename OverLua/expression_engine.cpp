/*
 * OverLua expression engine
 *

    Copyright 2007  Niels Martin Hansen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    Contact:
    E-mail: <jiifurusu@gmail.com>
    IRC: jfs in #aegisub on irc.rizon.net

 */


#include "expression_engine.h"
#include <math.h>


namespace ExpressionEngine {

	// Builtin functions

	static bool f_abs(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(fabs(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_abs = {"abs", f_abs};

	static bool f_floor(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(floor(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_floor = {"floor", f_floor};

	static bool f_ceil(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(ceil(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_ceil = {"ceil", f_ceil};

	static bool f_log(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(log(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_log = {"log", f_log};

	static bool f_exp(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(exp(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_exp = {"exp", f_exp};

	static bool f_sqrt(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(sqrt(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_sqrt = {"sqrt", f_sqrt};

	static bool f_e(Stack &stack, void *data)
	{
		stack.push_back(2.71828182845904523536);
		return true;
	}
	static const Function fs_e = {"e", f_e};

	static bool f_min(Stack &stack, void *data)
	{
		if (stack.size() >= 2) {
			double v1 = stack.back();
			stack.pop_back();
			double v2 = stack.back();
			stack.pop_back();
			if (v1 < v2)
				stack.push_back(v1);
			else
				stack.push_back(v2);
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_min = {"min", f_min};

	static bool f_max(Stack &stack, void *data)
	{
		if (stack.size() >= 2) {
			double v1 = stack.back();
			stack.pop_back();
			double v2 = stack.back();
			stack.pop_back();
			if (v1 > v2)
				stack.push_back(v1);
			else
				stack.push_back(v2);
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_max = {"max", f_max};

	static bool f_pi(Stack &stack, void *data)
	{
		stack.push_back(3.14159265358979323846);
		return true;
	}
	static const Function fs_pi = {"pi", f_pi};

	static bool f_sin(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(sin(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_sin = {"sin", f_sin};

	static bool f_cos(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(cos(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_cos = {"cos", f_cos};

	static bool f_tan(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(tan(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_tan = {"tan", f_tan};

	static bool f_asin(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(asin(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_asin = {"asin", f_asin};

	static bool f_acos(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(acos(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_acos = {"acos", f_acos};

	static bool f_atan(Stack &stack, void *data)
	{
		if (stack.size() >= 1) {
			double v = stack.back();
			stack.pop_back();
			stack.push_back(atan(v));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_atan = {"atan", f_atan};

	static bool f_mod(Stack &stack, void *data)
	{
		if (stack.size() >= 2) {
			double v1 = stack.back();
			stack.pop_back();
			double v2 = stack.back();
			stack.pop_back();
			stack.push_back(fmod(v2, v1));
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_mod = {"mod", f_mod};

	static bool f_rand(Stack &stack, void *data)
	{
		stack.push_back((double)rand()/RAND_MAX);
		return true;
	}
	static const Function fs_rand = {"rand", f_rand};

	static bool f_ifgtz(Stack &stack, void *data)
	{
		if (stack.size() >= 2) {
			double v1 = stack.back();
			stack.pop_back();
			double v2 = stack.back();
			stack.pop_back();
			double v3 = stack.back();
			stack.pop_back();
			if (v3 > 0)
				stack.push_back(v2);
			else
				stack.push_back(v1);
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_ifgtz = {"ifgtz", f_ifgtz};

	static bool f_ifeqz(Stack &stack, void *data)
	{
		if (stack.size() >= 2) {
			double v1 = stack.back();
			stack.pop_back();
			double v2 = stack.back();
			stack.pop_back();
			double v3 = stack.back();
			stack.pop_back();
			if (v3 == 0)
				stack.push_back(v2);
			else
				stack.push_back(v1);
			return true;
		} else {
			return false;
		}
	}
	static const Function fs_ifeqz = {"ifeqz", f_ifeqz};


	// Machine specification

	Specification::Specification()
	{
		// Add standard functions
		functions.push_back(fs_abs);
		functions.push_back(fs_floor);
		functions.push_back(fs_ceil);
		functions.push_back(fs_log);
		functions.push_back(fs_exp);
		functions.push_back(fs_sqrt);
		functions.push_back(fs_e);
		functions.push_back(fs_min);
		functions.push_back(fs_max);
		functions.push_back(fs_pi);
		functions.push_back(fs_sin);
		functions.push_back(fs_cos);
		functions.push_back(fs_tan);
		functions.push_back(fs_asin);
		functions.push_back(fs_acos);
		functions.push_back(fs_atan);
		functions.push_back(fs_mod);
		functions.push_back(fs_rand);
		functions.push_back(fs_ifgtz);
		functions.push_back(fs_ifeqz);
	}


	// Machine runner

	bool Machine::Run()
	{
		// Prepare the stack
		std::vector<double> stack;
		stack.reserve(16);

		// Assume the registers are already initialised like the manager wants

		// Execute the program
		for (size_t pc = 0; pc < program.size(); pc++) {
			Instruction &i = program[pc];
			switch (i.op) {
				double v1, v2; // values for operators;

				case INST_PUSH_CONST:
					stack.push_back(i.vd);
					break;

				case INST_PUSH_REG:
					stack.push_back(registers[i.vu]);
					break;

				case INST_ADD:
					if (stack.size() < 2) return false;
					v1 = stack.back(); stack.pop_back();
					v2 = stack.back(); stack.pop_back();
					stack.push_back(v2+v1);
					break;

				case INST_SUB:
					if (stack.size() < 2) return false;
					v1 = stack.back(); stack.pop_back();
					v2 = stack.back(); stack.pop_back();
					stack.push_back(v2-v1);
					break;

				case INST_MUL:
					if (stack.size() < 2) return false;
					v1 = stack.back(); stack.pop_back();
					v2 = stack.back(); stack.pop_back();
					stack.push_back(v2*v1);
					break;

				case INST_DIV:
					if (stack.size() < 2) return false;
					v1 = stack.back(); stack.pop_back();
					v2 = stack.back(); stack.pop_back();
					stack.push_back(v2/v1);
					break;

				case INST_POW:
					if (stack.size() < 2) return false;
					v1 = stack.back(); stack.pop_back();
					v2 = stack.back(); stack.pop_back();
					stack.push_back(pow(v2, v1));
					break;

				case INST_UNM:
					if (stack.size() < 1) return false;
					v1 = stack.back(); stack.pop_back();
					stack.push_back(-v1);
					break;

				case INST_CALL:
					if (!i.vf(stack, i.vfd))
						return false;
					break;

				case INST_STORE:
					if (stack.size() < 1) return false;
					v1 = stack.back(); stack.pop_back();
					registers[i.vu] = v1;
					break;

				default:
					return false;
			}
		}

		// The registers should now be in the final state

		return true;
	}

	static const char *parse_register_name(const char *source, const std::vector<std::string> &registers, size_t &index)
	{
		// Find end of the potential register name
		// That is, end of string or whitespace
		const char *end = source;
		while (*end && *end != ' ' && *end != '\t' && *end != '\n' && *end != '\r') end++;
		// Now end points to one past last character in name

		std::string regname(source, end-source);

		if (regname.size() == 0) return 0;

		// Check for supplied register name
		for (size_t i = 0; i < registers.size(); i++) {
			if (regname == registers[i]) {
				index = i;
				return end;
			}
		}

		// Check for temp register name
		if (regname[0] == 't' && regname.size() == 2) {
			if (regname[1] >= '0' && regname[1] <= '9') {
				index = registers.size() + regname[1] - '0';
				return end;
			}
		}

		// Nothing matches
		return 0;
	}

	static const char *parse_function_name(const char *source, const std::vector<Function> &functions, FunctionPtr &funcptr, void *&funcdata)
	{
		// Find end of the potential function name
		// That is, end of string or whitespace
		const char *end = source;
		while (*end && *end != ' ' && *end != '\t' && *end != '\n' && *end != '\r') end++;
		// Now end points to one past last character in name

		std::string funcname(source, end-source);

		if (funcname.size() == 0) return 0;

		// Check for supplied register name
		for (size_t i = 0; i < functions.size(); i++) {
			if (funcname == functions[i].name) {
				funcptr = functions[i].function;
				funcdata = functions[i].data;
				return end;
			}
		}

		return 0;
	}

	Machine::Machine(const ExpressionEngine::Specification &spec, const char *source)
	{
		// Set up the registers
		const size_t temp_register_count = 10;
		registers.resize(spec.registers.size() + temp_register_count);

		program.reserve(64);

		// Parse the program
		while (*source) {
			Instruction i;

			// Skip whitespace
			while (*source && (*source == ' ' || *source == '\t' || *source == '\n' || *source == '\r')) source++;
			if (!*source) break;

			// First see if it can be read as a number constant
			{
				char *tmp = 0;
				double v = strtod(source, &tmp);
				if (tmp && source != tmp) {
					// Could be read, so we have a constant here
					source = tmp;
					i.op = INST_PUSH_CONST;
					i.vd = v;
					program.push_back(i);
					continue;
				}
			}

			// Not a number constant
			// Check for arithmetic operator
			if (*source == '+') {
				source++;
				i.op = INST_ADD;
				program.push_back(i);
			}
			else if (*source == '-') {
				source++;
				i.op = INST_SUB;
				program.push_back(i);
			}
			else if (*source == '*') {
				source++;
				i.op = INST_MUL;
				program.push_back(i);
			}
			else if (*source == '/') {
				source++;
				i.op = INST_DIV;
				program.push_back(i);
			}
			else if (*source == '^') {
				source++;
				i.op = INST_POW;
				program.push_back(i);
			}
			else if (*source == '~') {
				source++;
				i.op = INST_UNM;
				program.push_back(i);
			}
			// Check for assignment
			else if (*source == '=') {
				i.op = INST_STORE;
				const char *tmp = parse_register_name(source+1, spec.registers, i.vu);
				if (!tmp) throw source; // No register name found, error
				source = tmp;
				program.push_back(i);
			}
			// Register push or function call
			else {
				const char *tmp = parse_register_name(source, spec.registers, i.vu);
				if (tmp) {
					// Register push
					i.op = INST_PUSH_REG;
					source = tmp;
					program.push_back(i);
				}
				else {
					tmp = parse_function_name(source, spec.functions, i.vf, i.vfd);
					if (tmp) {
						// Function call
						i.op = INST_CALL;
						source = tmp;
						program.push_back(i);
					}
					else {
						// Nothing, error
						throw source;
					}
				}
			} 

		} /* end while */

	} /* end Machine::Machine() */

};
