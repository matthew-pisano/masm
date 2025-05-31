"""Masm Parser"""

from enum import Enum
from typing import Dict, List

from .tokenizer import LineTokens


class MemSection(Enum):
    """Memory sections for MIPS assembly programs"""

    DATA = ...
    HEAP = ...
    GLOBAL = ...
    STACK = ...
    TEXT = ...
    KTEXT = ...
    KDATA = ...
    MMIO = ...


class MemLayout:
    """Memory layout for MIPS assembly programs"""

    data: Dict[MemSection, bytes]
    """A dictionary mapping memory sections to bytes"""

    def __init__(self, data: Dict[MemSection, bytes]) -> None: ...

    def __repr__(self) -> str: ...


class Parser:
    """Parser for MIPS assembly programs"""

    def __init__(self) -> None: ...

    def parse(self, program: List[LineTokens]) -> MemLayout:
        """Parses the given program and returns a MemLayout object

        Args:
            program (List[LineTokens]): The tokenized program to parse
        Returns:
            MemLayout: The memory layout of the program
        Raises:
            MasmSyntaxError: If a syntax error occurs during parsing"""
        ...
