import sys
from io import BytesIO

from pymasm import *


def syscall_asm():
    return """
.data
prompt: .asciiz "Enter a number: "
resp: .asciiz "Your number is: "

.text

main:
    la $a0, prompt      # Load in address of prompt
    li $v0, 4           # Set syscall flag to print string
    syscall             # Print the prompt

    li $v0, 5           # Set syscall flag to read integer
    syscall             # Print the message
    
    move $t0, $v0
    
    la $a0, resp        # Load in address of response
    li $v0, 4           # Set syscall flag to print string
    syscall             # Print the response
    
    li $v0, 1           # Set syscall flag to print integer
    move $a0, $t0
    syscall             # Print the integer

    li $v0, 10
    syscall             # Exit gracefully
"""


def main():
    # Load in an assembly file
    mips_source = syscall_asm()
    mips_file = tokenizer.SourceFile("syscall.asm", mips_source)

    # Tokenize the program
    program = tokenizer.Tokenizer.tokenize([mips_file])

    # Parse the tokenized program into a memory layout
    masm_parser = parser.Parser()
    mem_layout = masm_parser.parse(program)

    # Set the interpreter to use syscalls for input and output
    io_mode = interpreter.IOMode.SYSCALL
    # Set up input and output streams
    istream = BytesIO(b"5\n")
    ostream = BytesIO()

    # Execute the program
    masm_interpreter = interpreter.Interpreter(io_mode, istream, ostream)
    exitCode = masm_interpreter.interpret(mem_layout)

    # Print out the result and exit
    print(ostream.getvalue().decode())
    sys.exit(exitCode)


if __name__ == "__main__":
    main()
