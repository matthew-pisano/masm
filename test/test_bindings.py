from io import BytesIO

import pytest

import pymasm


@pytest.fixture(scope="module")
def syscall_asm():
    with open("test/fixtures/syscall/syscall.asm") as f:
        return f.read()


class TestBindings:

    def test_syscall(self, syscall_asm: str):
        raw_lines = syscall_asm.split("\n")
        raw_file = pymasm.tokenizer.RawFile("syscall.asm", raw_lines)
        program = pymasm.tokenizer.Tokenizer.tokenize([raw_file])

        parser = pymasm.parser.Parser()
        layout = parser.parse(program)

        istream = BytesIO()
        ostream = BytesIO()
        interpreter = pymasm.interpreter.Interpreter(istream, ostream)
        exitCode = interpreter.interpret(layout)

        print("Out:", ostream.getvalue())
        print("Exit:", exitCode)
