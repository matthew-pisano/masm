# masm - A C++ MIPS Assembler and Simulator

*masm* enables the development of software for MIPS hardware through providing a featureful assembler and hardware simulator. This project builds an assembler, a simulator, and a debugger. For development, a C++ library and Python bindings are provided.

## Quick Start

The *masm* collection of executables can be downloaded and installed from the [latest release](https://github.com/matthew-pisano/masm/releases/latest). Both Linux and Windows installations are provided. For Linux, RPM and DEB packages are built, along with plain archives. For Windows, there is an installer along with a plain archive. This project can also be built for macOS, but binaries are not distributed.

Upon installing, three programs will be made available: *masm*, *msim*, and *mdb*. The assembler takes in plain-text MIPS assembly files and outputs a compiled binary for the simulator's virtual machine. The simulator intakes a given binary, executing it on simulated registers and memory that match the 32-bit MIPS-I architecture specification. Finally, the debugger uses the same engine as the simulator, but with interactive controls which detail the simulated hardware states.

## MIPS Assembler

The assembler program, *masm*, takes in one or more MIPS assembly source code files as input and outputs an assembled binary. These binaries are built to run only through the simulator and not on native hardware.

If multiple files are given, execution will begin at the global `main` label, if defined. If not defined, the binary will begin executing the fist instruction in the text segment of the first file.

```asm
.globl main
main:
  ... # Main program body
```

### Usage

```bash
masm [--little-endian] [-g] [--save-temps] [-o a.out] module1.asm module2.asm ...
```

By default, binaries are laid out in *big endian* format to keep in line with the original MIPS standard. However, *little endian* compatibility can be enabled with the `--little-endian` option. This changes how words are stored, so certain programs, such as those working with MMIO, may not work without modification.

Also by default, debug symbols are stripped from binaries. To retain for usage with the debugger, use `-g`.

To keep preprocessing and temporary files, `--save-temps` can be set.

## MIPS Simulator

The simulator program, *msim*, takes in a single binary program and executes it on simulated hardware. This simulation includes 32 32-bit CPU registers, a floating-point coprocessor (CP1) with 32 floating point (single and double precision) registers, and an exception coprocessor (CP0).

### Usage

```bash
msim [--mmio] [--little-endian] a.out
```

By default, *masm* will use *syscall I/O*. This means that the console input and output are only accessible through syscalls. Alternately, the `--mmio` option enables *memory-mapped I/O*. With this option, console I/O is routed through the MMIO registers (located at *0xffff0000* - *0xffff000f*). Reading from or writing to these registers passes that information to the program. Attempting to call I/O syscalls in this mode will fail.

Similarly to the assembler, `--little-endian` determines the endianness of how the given binary is loaded into memory. Loading a binary with the wrong endianness will result in undefined behavior.

### Interrupts

*masm* handles interrupts differently from other MIPS simulators. Interrupts, by default, are disabled. Keyboard interrupts can be enabled by setting the interrupt enable bit (bit zero) of the coprocessor zero (CP0) *status* ($12) register to 1. Once this is set, both keyboard and display interrupts will be enabled. Each interrupt can be selectively turned off by setting bit 8 (keyboard) or bit 9 (display) to 0. When an interrupt event is detected, control of the program will be transferred at
the
interrupt
handler at `0x80000000`. If no such handler exists, an exception will be thrown and the program will halt.

For example, the following assembly code can be used:

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

Exceptions are handled similarly from interrupts. When a runtime exception is triggered, control is transferred to the interrupt handler at `0x80000000`. If no such handler exists, the exception is not handled and is thrown, halting the program. MIPS uses just a single interrupt handler for both hardware interrupts and exceptions. It is the responsibility of the program to make sure this single handler correctly processes all possible interrupts.

## MIPS Debugger

In addition to the main simulator executable, this project also contains a GDB-like debugger, *mdb*. This program allows the user to step through a running assembly program interactively. At any interactive step, the user can view the state of the program and continue when desired. The commands used for the debugger are very similar to those used with GDB. These include:

* `help` - for more detailed information on the commands
* `step` - to advance the program by one instruction
* `break` - to set a breakpoint
* `continue` - to allow the program t run freely until the next breakpoint
* `print` - to display information about the state of the program

There are many other comments in addition to these, designed to make debugging complex assembly programs more manageable.

### Usage

```bash
mdb [--mmio] [--little-endian] a.out
```

*mdb* is called similarly to *msim*, it takes in a binary program and options. However, instead of immediately running the program, it is assembled, loaded into memory, and the user is dropped into an interactive shell.

### Examples

This repository contains a variety of example files in [examples](examples) that demonstrate how to utilize the majority of *masm*'s capabilities.

## Python Bindings

In addition to the main executable, this project also builds a set of Python bindings accessible through the `pymasm` package. This allows for Python code to directly interact with *masm* to assemble and execute strings of assembly programs.

### Python Usage

After installing the package through the wheel file built by this project, it can be imported as `pymasm`. This then exposes the following submodules, which contain the *libmasm* bindings.

```python
from pymasm import exceptions
from pymasm import tokenizer
from pymasm import parser
from pymasm import simulator
```

For more detailed usage examples, see the [python/examples](python/examples) directory.

## Building from Source

### Prerequisites

- CMake 3.22 or higher
- C++23 compatible compiler
    - C++20 should also work with minimal modifications
- Git (for fetching dependencies)

**For Python bindings (Linux/macOS only):**

> NOTE: Python bindings for macOS have limited support and may not work out-of-the-box

- Python 3.7 or higher
- pybind11

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/matthew-pisano/masm
cd masm

# Configure and build
cmake -B build
cmake --build build

# Build Python library
python3 -m build python
```

## Compatability

Similar to other MIPS simulators like [MARS](https://dpetersanderson.github.io/) and [SPIM](https://spimsimulator.sourceforge.net/), *masm* implements a subset of the full MIPS instruction set architecture and executes instructions within an emulated environment. Here, instructions and data are stored in memory in a *big endian* format, similar to the original MIPS specification. Additionally, *masm* also supports assembling code in *little endian* format for compatibility with other simulators.
