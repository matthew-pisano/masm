"""Masm Interpreter"""

from typing import Any, IO

from .parser import MemLayout


class Interpreter:
    def __init__(self, istream: IO[Any], ostream: IO[Any]) -> None: ...

    def init_program(self, layout: MemLayout) -> None:
        """Initializes the program with the given memory layout"""
        ...

    def step(self) -> None:
        """Executes a single instruction"""
        ...

    def interpret(self, layout: MemLayout) -> int:
        """Interprets the given memory layout and returns an exit code"""
        ...
