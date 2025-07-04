# PyMasm Examples

This directory contains several example usages of the `pymasm` package build by this repository.

* [hello_world.py](hello_world.py) contains a minimum working example of the package's functionality:
    * Tokenizing a file.
    * Parsing those tokens into a memory layout.
    * Interpreting the given memory layout as executable code.
    * Reading output from the `ostream` provided to the interpreter.

* [syscall.py](syscall.py) contains additionally functionality related to *syscall I/O*:
    * Sending input to the `istream` provided to the interpreter.
    * Manually initializing the program.
    * Manually stepping through the program and taking output.

* [mmio.py](mmio.py) contains an example for using *MMIO*:
    * Inputs sent to the program over time at sparse timesteps
    * Outputs gotten from the program over time at sparse timesteps