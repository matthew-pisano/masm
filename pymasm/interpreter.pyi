"""Masm Interpreter"""

from typing import Any, IO

from .parser import MemLayout


class Interpreter:
    """Interpreter for MIPS assembly programs"""

    def __init__(self, istream: IO[Any], ostream: IO[Any]) -> None: ...

    def init_program(self, layout: MemLayout) -> None:
        """Initializes the program with the given memory layout

        Args:
            layout (MemLayout): The memory layout to initialize the program with"""
        ...

    def step(self) -> None:
        """Executes a single instruction

        Raises:
            ExecExit: If the program has terminated successfully with an exit code
            MasmRuntimeError: If a runtime error occurs during execution"""
        ...

    def interpret(self, layout: MemLayout) -> int:
        """Interprets the given memory layout and returns an exit code

        Args:
            layout (MemLayout): The memory layout to interpret
        Returns:
            int: The exit code of the program"""
        ...
