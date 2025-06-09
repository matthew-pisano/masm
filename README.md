# masm - A C++ MIPS Assembler and Interpreter

`masm` takes in one or more assembly source code files as input and outputs the standard output stream of the program. Programs are processed in three steps:

* Tokenization - where the source code is split into groups of valid program tokens
* Parsing - where tokens are transformed into a memory template that will be loaded in at runtime
* Interpreting - where the raw, parsed memory is first assembled into MIPS machine code and then executed in the same manner as a CPU would interact with its registers and main memory.

## Usage

`masm` takes in a valid *MIPS* program and executes the instructions within on a virtual CPU that interacts with virtual registers and memory.

```bash
masm [options...] module1.asm module2.asm ...
```

By default, `masm` will use *syscall I/O*. This means that the console input and output are only accessible through syscalls. Alternately, the `--mmio` option enables *memory-mapped I/O*. With this option, console I/O is routed through the MMIO registers (located at *0xffff0000* - *0xffff000f*). Reading from or writing to these registers passes that information to the program.

## Python Bindings

In addition to the main executable, this project also builds a set of *Python* bindings accessible through the `pymasm` package. This allows for *Python* code to directly interact with `masm` to assemble and execute strings of assembly programs.

### Python Usage

After installing the package through the wheel file built by this project, it can be imported as `pymasm`. This then exposes the following submodules, which contain the `masm` bindings.

```python
from pymasm import exceptions
from pymasm import tokenizer
from pymasm import parser
from pymasm import interpreter
```

For more detailed usage examples, see the [python/examples](python/examples) directory.

## Implementation

Similar to other *MIPS* simulators like [MARS](https://dpetersanderson.github.io/) and [SPIM](https://spimsimulator.sourceforge.net/), `masm` implements a subset of the full MIPS instruction set architecture and executes instructions within an emulated environment. Here, instructions and data are stored in memory in a *big endian* format, similar to the original *MIPS* specification.

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
