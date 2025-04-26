# masm - A C++ MIPS Interpreter

An interpreter for the *MIPS* assembly language.  `masm` takes in an assembly source code file as input and outputs the standard output stream of the program. Programs are processed in three steps:

* Tokenization - where the source code is split into groups of valid program tokens
* Parsing - where tokens are transformed into a memory template that will be loaded in at runtime
* Interpreting - where the raw, parsed memory is executed in the same manner as a CPU would interact with its registers and main memory

## Usage

`masm` takes in a valid *MIPS* program and executes the instructions within on a virtual CPU that interacts with virtual registers and memory.

```bash
masm prog.asm
```

## Implementation

Similar to other *MIPS* simulators like [MARS](https://dpetersanderson.github.io/) and [SPIM](https://spimsimulator.sourceforge.net/), `masm` implements a subset of the full MIPS instruction set architecture and executes instructions within an emulated environment. Here, instructions and data are stored in memory in a *big endian* format, similar to the original *MIPS* specification.

This program uses a 32 element array composed of 32-bit integers to represent its register file and an unordered map that can accommodate up to 4GiB of memory. The CPU is implemented within the interpreter, which keeps the current state of the register file and memory to load and operate on instructions.