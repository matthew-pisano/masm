import pytest

import pymasm


@pytest.fixture(scope="module")
def syscall_asm():
    with open("test/fixtures/syscall/syscall.asm") as f:
        return f.read()


class TestBindings:

    def test_syscall(self, syscall_asm: str):
        raw_lines = syscall_asm.split()
        raw_file = pymasm.tokenizer.RawFile("syscall.asm", raw_lines)
        program = pymasm.tokenizer.Tokenizer.tokenize(raw_file)

        parser = pymasm.parser.Parser()
        layout = parser.parse(program)

        interpreter = pymasm.interpreter.Interpreter()
        exitCode = interpreter.interpret(layout)

        print(exitCode)
