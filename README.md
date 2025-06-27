# masm - A C++ MIPS Assembler and Interpreter

*masm* takes in one or more assembly source code files as input and outputs the standard output stream of the program. Programs are processed in three steps:

* Tokenization - where the source code is split into groups of valid program tokens
* Parsing - where tokens are transformed into a memory template that will be loaded in at runtime
* Interpreting - where the raw, parsed memory is first assembled into MIPS machine code and then executed in the same manner as a CPU would interact with its registers and main memory.

## Installation

While this project is designed to be primarily be built using Linux, it can generate both Linux and Windows versions of the main executable. The accompanying *Python* package is Linux-only for the time being.

For the main executable, simply navigate to the [latest release](https://github.com/matthew-pisano/masm/releases/latest) and download the executable for your operating system.  *masm* requires no external dependencies or libraries, so it can be installed and run as a standalone executable.

For the accompanying *Python* package, download the wheel file included in the release and run the following to install it to your current environment:

```bash
pip install pymasm-x.x.x-py3-none-any.whl 
```

Remember to replace the `x.x.x` portion with the release version that you have selected.

## Usage

*masm* takes in a valid *MIPS* program and executes the instructions within on a virtual CPU that interacts with virtual registers and memory.

```bash
masm [options...] module1.asm module2.asm ...
```

Note that program execution will begin at the first instruction in the text section of the first file argument (`module1.asm` in this case).

By default, *masm* will use *syscall I/O*. This means that the console input and output are only accessible through syscalls. Alternately, the `--mmio` option enables *memory-mapped I/O*. With this option, console I/O is routed through the MMIO registers (located at *0xffff0000* - *0xffff000f*). Reading from or writing to these registers passes that information to the program.

### Interrupts

*masm* handles interrupts differently from other *MIPS* simulators. Interrupts, by default, are disabled. Keyboard interrupts can be enabled by setting the interrupt enable bit (bit zero) of the coprocessor zero *status* ($12) register to 1. Once this is set, both keyboard and display interrupts will be enabled. Each interrupt can be selectively turned off by setting bit 8 (keyboard) or bit 9 (display) to 0. When an interrupt event is detected, control of the program will be transferred at the
interrupt
handler at `0x80000000`. If no such handler exists, an exception will be thrown and the program will halt.

```asm
# Modify interrupt status

# Move interrupt status in $12 to $t0
mfc0    $t0, $12
# Set interrupt enable flag (keep all bits as they are, except last which gets set)
ori     $t0, $t0, 0x0001
# Set keyboard interrupt enable flag to zero to disable it
andi    $t0, $t0, 0xfeff
# Set display interrupt enable flag to zero to disable it
andi    $t0, $t0, 0xfdff
# Move new status with set enable flag back into $12
mtc0    $t0, $12
```

### Exceptions

Exceptions are handled similarly from interrupts. When a runtime exception is triggered, control is transferred to the interrupt handler at `0x80000000`. If no such handler exists, the exception is not handled and is thrown, halting the program.

### Little Endian Compatibility

By default, *masm* stores words in a *big endian* format to keep in line with the original *MIPS* standard. However, *little endian* compatibility can be enabled with the `--little-endian` option. This changes how words are stored, so certain programs, such as those working with MMIO, may not work without modification.

## Python Bindings

In addition to the main executable, this project also builds a set of *Python* bindings accessible through the `pymasm` package. This allows for *Python* code to directly interact with *masm* to assemble and execute strings of assembly programs.

### Python Usage

After installing the package through the wheel file built by this project, it can be imported as `pymasm`. This then exposes the following submodules, which contain the *masm* bindings.

```python
from pymasm import exceptions
from pymasm import tokenizer
from pymasm import parser
from pymasm import interpreter
```

For more detailed usage examples, see the [python/examples](python/examples) directory.

## Implementation

Similar to other *MIPS* simulators like [MARS](https://dpetersanderson.github.io/) and [SPIM](https://spimsimulator.sourceforge.net/), *masm* implements a subset of the full MIPS instruction set architecture and executes instructions within an emulated environment. Here, instructions and data are stored in memory in a *big endian* format, similar to the original *MIPS* specification. Additionally, *masm* also supports assembling code in *little endian* format for compatibility with other
simulators.

This program uses a 32 element array composed of 32-bit integers to represent its register file and an unordered map that can accommodate up to 4GiB of memory. The CPU is implemented within the interpreter, which keeps the current state of the register file and memory to load and operate on instructions.

## Implemented Features

- [x] MIPS ISA Instructions
    - [x] MIPS Pseudo Instructions
- [X] Syscalls
    - [X] Keyboard/Display Syscalls
    - [X] MARS Extended Syscalls
    - [X] Heap Allocation Syscalls
- [X] Basic Memory Directives (`.data`, `.text`)
- [X] Allocation directives (`.word`, `.space`, etc.)
- [X] Kernel Memory Directives (`.ktext`, `.kdata`)
    - [ ] Custom Addresses for Memory Sections
- [X] Macro Directives
    - [X] Include Directive
    - [X] Macros/Eqv Directives
- [X] Syntax and Runtime Errors
- [X] Keyboard/Display MMIO
- [X] Interrupts
    - [X] MMIO Interrupts
    - [X] Exceptions
- [X] Floating Point Coprocessor
    - [X] Floating Point Syscalls
- [X] Interactive Debugger
