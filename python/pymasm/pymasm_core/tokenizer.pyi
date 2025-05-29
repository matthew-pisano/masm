"""Masm Tokenizer"""

from enum import Enum
from typing import List


class TokenType(Enum):
    """Token types for MIPS assembly programs"""

    UNKNOWN = ...
    SEC_DIRECTIVE = ...
    ALLOC_DIRECTIVE = ...
    META_DIRECTIVE = ...
    LABEL_DEF = ...
    LABEL_REF = ...
    INSTRUCTION = ...
    REGISTER = ...
    IMMEDIATE = ...
    SEPERATOR = ...
    OPEN_PAREN = ...
    CLOSE_PAREN = ...
    STRING = ...
    MACRO_PARAM = ...


class RawFile:
    """Represents a raw source code file with a name and lines of text"""

    name: str
    """The name of the file, typically a path or filename"""

    lines: List[str]
    """The lines of text in the file, each line as a string"""

    def __init__(self) -> None: ...

    def __init__(self, name: str, lines: List[str]) -> None: ...

    def __repr__(self) -> str: ...


class Token:
    """Represents a token in the MIPS assembly language with a type and value"""

    type: TokenType
    """The type of the token, indicating its category (e.g., instruction, label, directive)"""

    value: str
    """The string value of the token, which is the actual text representation of the token"""

    def __init__(self) -> None: ...

    def __init__(self, type: TokenType, value: str) -> None: ...

    def __repr__(self) -> str: ...

    def __str__(self) -> str: ...

    def __eq__(self, other: Token) -> bool: ...

    def __ne__(self, other: Token) -> bool: ...


class SourceLine:
    """Represents a line of tokenized source code with its line number and tokens"""

    filename: str
    """The name of the source file this line belongs to"""

    lineno: int
    """The line number of the source code line, starting from 1"""

    tokens: List[Token]
    """A list of tokens that represent the content of the source code line"""

    def __init__(self) -> None: ...

    def __init__(self, filename: str, lineno: int, tokens: List[Token]) -> None: ...

    def __repr__(self) -> str: ...


class Tokenizer:
    """Tokenizer for MIPS assembly programs that converts raw source files into tokenized source lines"""

    def __init__(self) -> None: ...

    @staticmethod
    def tokenize_file(raw_file: RawFile) -> List[SourceLine]:
        """Tokenizes the given file and returns a vector of SourceLine objects

        Args:
            raw_file (RawFile): The raw file to tokenize
        Returns:
            List[SourceLine]: A list of SourceLine objects representing the tokenized lines of the file
        Raises:
            MasmSyntaxError: If a syntax error occurs during tokenization"""
        ...

    @staticmethod
    def tokenize(raw_files: List[RawFile]) -> List[SourceLine]:
        """Tokenizes and post-processes the given raw files and returns a vector of SourceLine objects

        Args:
            raw_files (List[RawFile]): The list of raw files to tokenize
        Returns:
            List[SourceLine]: A list of SourceLine objects representing the tokenized lines of the files
        Raises:
            MasmSyntaxError: If a syntax error occurs during tokenization"""
        ...
