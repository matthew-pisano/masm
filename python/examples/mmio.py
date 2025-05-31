import os
from io import BytesIO

from pymasm import *


def mmio_asm():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    with open(f"{script_dir}/mmio.asm", "r") as f:
        return f.read()


def input_timings(step: int):
    """Function to simulate getting inputs over time

    Args:
        step: An integer representing the current step in the program execution.
    Returns:
        The input character for the given step, or None if no input is needed at the current step."""

    return {
        1000: "1",
        2000: "\n",
        3000: "0",
        4000: "\n"
    }.get(step)


def main():
    # Load in an assembly file
    mips_source = mmio_asm()
    mips_file = tokenizer.SourceFile("mmio.asm", mips_source)

    # Tokenize the program
    program = tokenizer.Tokenizer.tokenize([mips_file])

    # Parse the tokenized program into a memory layout
    masm_parser = parser.Parser()
    mem_layout = masm_parser.parse(program)

    # Set the interpreter to use syscalls for input and output
    io_mode = interpreter.IOMode.MMIO
    # Set up input and output streams
    istream = BytesIO()
    ostream = BytesIO()

    # Execute the program
    masm_interpreter = interpreter.Interpreter(io_mode, istream, ostream)
    masm_interpreter.init_program(mem_layout)

    # The current position to read the output buffer at
    opos = 0
    step = 0
    while True:
        try:
            curr_char = input_timings(step)
            if curr_char:
                # Save the current position so interpreter can read from where it left off
                ipos = istream.tell()
                # Seek to the end to append a character
                istream.seek(0, os.SEEK_END)
                istream.write(curr_char.encode())
                # Restore the saved position for reading
                istream.seek(ipos)

            # Step the interpreter
            masm_interpreter.step()

            ostream.seek(opos)
            if output := ostream.read().decode():
                print(output, end='')
            opos = ostream.tell()

            step += 1
        except exceptions.ExecExit as e:
            break


if __name__ == "__main__":
    main()
