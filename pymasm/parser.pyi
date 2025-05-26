"""Masm Parser"""

from enum import Enum
from typing import Dict, Any, List

from .tokenizer import SourceLine


class MemSection(Enum):
    """Memory sections for MIPS assembly programs"""

    TEXT = ...
    DATA = ...
    HEAP = ...
    KTEXT = ...
    KDATA = ...
    MMIO = ...


# MemLayout is a dictionary-like mapping
class MemLayout(Dict[MemSection, Any]):
    """Memory layout mapping"""

    def __init__(self) -> None: ...

    def __getitem__(self, key: MemSection) -> Any: ...

    def __setitem__(self, key: MemSection, value: Any) -> None: ...

    def __delitem__(self, key: MemSection) -> None: ...

    def __iter__(self) -> Any: ...

    def __len__(self) -> int: ...


class Parser:
    """Parser for MIPS assembly programs"""

    def __init__(self) -> None: ...

    def parse(self, program: List[SourceLine]) -> MemLayout:
        """Parses the given program and returns a MemLayout object

        Args:
            program (List[SourceLine]): The tokenized program to parse
        Returns:
            MemLayout: The memory layout of the program
        Raises:
            MasmSyntaxError: If a syntax error occurs during parsing"""
        ...
