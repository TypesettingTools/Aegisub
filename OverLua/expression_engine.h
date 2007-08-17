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

#ifndef EXPRESSION_ENGINE_H
#define EXPRESSION_ENGINE_H

#include <vector>
#include <string>


namespace ExpressionEngine {

	// The stack passed around
	typedef std::vector<double> Stack;

	// Type of callable functions
	typedef bool (*FunctionPtr)(Stack &stack, void *data);

	// A callable function
	struct Function {
		// The name of the function
		const char *name;
		// Function pointer; null for builtins
		FunctionPtr function;
		// Function data
		void *data;
	};

	// A machine specification that can compile programs
	struct Specification {
		// The input and output register names
		// The index of a register name in this vector translates to the register's number in the machine
		std::vector<std::string> registers;

		// The callable functions
		std::vector<Function> functions;

		// Just adds standard functions to functions vector
		Specification();
	};

	// Operation type
	enum Opcode {
		INST_PUSH_CONST = 0,
		INST_PUSH_REG,
		INST_ADD,
		INST_SUB,
		INST_MUL,
		INST_DIV,
		INST_POW,
		INST_UNM, // unary minus
		INST_CALL,
		INST_STORE
	};

	// A single program instruction
	struct Instruction {
		Opcode op;
		union {
			double vd; // double value, for const push
			size_t vu; // uint value, for register indices
			struct {
				FunctionPtr vf; // function to call
				void *vfd; // data for function
			};
		};
	};

	// A program is a sequence of instructions
	typedef std::vector<Instruction> Program;

	// A machine running a program
	struct Machine {
		// Values in the registers
		std::vector<double> registers;
		// The program
		Program program;

		// Run the machine.
		// Return true is no errors, false if error
		bool Run();

		// Create a machine from a specification and a program source
		Machine(const Specification &spec, const char *source);

		// Create a blank machine
		Machine() { }
	};

};


#endif
