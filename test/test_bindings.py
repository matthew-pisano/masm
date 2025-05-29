from io import BytesIO

import pymasm
import pytest


@pytest.fixture(scope="module")
def syscall_io_asm():
    with open("test/fixtures/input_output/input_output.asm") as f:
        return f.read()


@pytest.fixture(scope="module")
def mem_mapped_io_asm():
    with open("test/fixtures/mmio/mmio.asm") as f:
        return f.read()


class TestBindings:

    def test_syscall_io(self, syscall_io_asm: str):
        raw_file = pymasm.tokenizer.SourceFile("input_output.asm", syscall_io_asm)
        program = pymasm.tokenizer.Tokenizer.tokenize([raw_file])

        parser = pymasm.parser.Parser()
        layout = parser.parse(program)

        io_mode = pymasm.interpreter.IOMode.SYSCALL
        input_string = "5\n"
        istream = BytesIO(input_string.encode())
        ostream = BytesIO()
        interpreter = pymasm.interpreter.Interpreter(io_mode, istream, ostream)
        exitCode = interpreter.interpret(layout)

        with open("test/fixtures/input_output/input_output.txt", "rb") as f:
            expected_output = f.read()

        assert exitCode == 0
        assert ostream.getvalue() == expected_output

    def test_mem_mapped_io(self, mem_mapped_io_asm: str):
        raw_file = pymasm.tokenizer.SourceFile("mmio.asm", mem_mapped_io_asm)
        program = pymasm.tokenizer.Tokenizer.tokenize([raw_file])

        parser = pymasm.parser.Parser()
        layout = parser.parse(program)

        io_mode = pymasm.interpreter.IOMode.MMIO
        input_string = "1234"
        istream = BytesIO(input_string.encode())
        ostream = BytesIO()
        interpreter = pymasm.interpreter.Interpreter(io_mode, istream, ostream)
        exitCode = interpreter.interpret(layout)

        with open("test/fixtures/mmio/mmio.txt", "rb") as f:
            expected_output = f.read()

        assert exitCode == 0
        assert ostream.getvalue() == expected_output
