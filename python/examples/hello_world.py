import sys
from io import BytesIO

from pymasm import *


def hello_world_asm():
    return """
.data
msg: .asciiz "Hello there"

.text

main:
    la $a0, msg         # Load in address of message
    li $v0, 4           # Set syscall flag to print string
    syscall             # Print the message

    li $v0, 10
    syscall             # Exit gracefully
"""


def main():
    # Load in an assembly file
    mips_lines = hello_world_asm().split("\n")
    mips_file = tokenizer.RawFile("hello_world.asm", mips_lines)

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
    exitCode = masm_interpreter.interpret(mem_layout)

    # Print out the result and exit
    print(ostream.getvalue().decode())
    sys.exit(exitCode)


if __name__ == "__main__":
    main()
