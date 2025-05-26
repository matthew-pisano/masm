from io import BytesIO

import pymasm
import pytest


@pytest.fixture(scope="module")
def syscall_asm():
    with open("test/fixtures/syscall/syscall.asm") as f:
        return f.read()


class TestBindings:

    def test_syscall_io(self, syscall_asm: str):
        raw_lines = syscall_asm.split("\n")
        raw_file = pymasm.tokenizer.RawFile("syscall.asm", raw_lines)
        program = pymasm.tokenizer.Tokenizer.tokenize([raw_file])

        parser = pymasm.parser.Parser()
        layout = parser.parse(program)

        io_mode = pymasm.interpreter.IOMode.SYSCALL
        istream = BytesIO()
        ostream = BytesIO()
        interpreter = pymasm.interpreter.Interpreter(io_mode, istream, ostream)
        exitCode = interpreter.interpret(layout)

        assert exitCode == 0
        assert ostream.getvalue() == b"hello"
