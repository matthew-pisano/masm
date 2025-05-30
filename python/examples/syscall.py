import os
from io import BytesIO

from pymasm import *


def syscall_asm():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    with open(f"{script_dir}/syscall.asm", "r") as f:
        return f.read()


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
    istream = BytesIO(b'5\n4\n3\n2\n1\n0\n')
    ostream = BytesIO()

    # Execute the program
    masm_interpreter = interpreter.Interpreter(io_mode, istream, ostream)
    masm_interpreter.init_program(mem_layout)

    pos = 0
    while True:
        try:
            masm_interpreter.step()
            ostream.seek(pos)
            output = ostream.read().decode()
            pos = len(ostream.getvalue())
            ostream.seek(pos)
            if output:
                print(output, end='')
        except exceptions.ExecExit as e:
            break


if __name__ == "__main__":
    main()
