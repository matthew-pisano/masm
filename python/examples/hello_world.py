import os
import sys
from io import BytesIO

from pymasm import *


def hello_world_asm():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    with open(f"{script_dir}/hello_world.asm", "r") as f:
        return f.read()


def main():
    # Load in an assembly file
    mips_source = hello_world_asm()
    mips_file = tokenizer.SourceFile("hello_world.asm", mips_source)

    # Tokenize the program
    program = tokenizer.Tokenizer.tokenize([mips_file])

    # Parse the tokenized program into a memory layout
    masm_parser = parser.Parser()
    mem_layout = masm_parser.parse(program)

    # Set the interpreter to use syscalls for input and output
    io_mode = interpreter.IOMode.SYSCALL
    # Set up input and output streams
    istream = BytesIO()
    ostream = BytesIO()

    # Execute the program
    masm_interpreter = interpreter.Interpreter(io_mode, istream, ostream)
    exit_code = masm_interpreter.interpret(mem_layout)

    # Print out the result and exit
    print(ostream.getvalue().decode())
    sys.exit(exit_code)


if __name__ == "__main__":
    main()
