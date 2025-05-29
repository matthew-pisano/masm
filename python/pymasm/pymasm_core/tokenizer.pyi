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


class SourceFile:
    """Represents a raw source code file with a name and source text"""

    name: str
    """The name of the file, typically a path or filename"""

    source: str
    """The source text of the file"""

    def __init__(self, name: str, source: str) -> None: ...

    def __repr__(self) -> str: ...


class Token:
    """Represents a token in the MIPS assembly language with a type and value"""

    type: TokenType
    """The type of the token, indicating its category (e.g., instruction, label, directive)"""

    value: str
    """The string value of the token, which is the actual text representation of the token"""

    def __init__(self, type: TokenType, value: str) -> None: ...

    def __repr__(self) -> str: ...

    def __str__(self) -> str: ...

    def __eq__(self, other: Token) -> bool: ...

    def __ne__(self, other: Token) -> bool: ...


class LineTokens:
    """Represents a line of tokenized source code with its line number and tokens"""

    filename: str
    """The name of the source file this line belongs to"""

    lineno: int
    """The line number of the source code line, starting from 1"""

    tokens: List[Token]
    """A list of tokens that represent the content of the source code line"""

    def __init__(self, filename: str, lineno: int, tokens: List[Token]) -> None: ...

    def __repr__(self) -> str: ...


class Tokenizer:
    """Tokenizer for MIPS assembly programs that converts raw source files into tokenized source lines"""

    def __init__(self) -> None: ...

    @staticmethod
    def tokenize_file(source_file: SourceFile) -> List[LineTokens]:
        """Tokenizes the given file and returns a vector of LineTokens objects

        Args:
            source_file (SourceFile): The source file to tokenize
        Returns:
            List[LineTokens]: A list of LineTokens objects representing the tokenized lines of the file
        Raises:
            MasmSyntaxError: If a syntax error occurs during tokenization"""
        ...

    @staticmethod
    def tokenize(source_file: List[SourceFile]) -> List[LineTokens]:
        """Tokenizes and post-processes the given source files and returns a vector of LineTokens objects

        Args:
            source_file (List[SourceFile]): The list of source files to tokenize
        Returns:
            List[SourceLine]: A list of LineTokens objects representing the tokenized lines of the files
        Raises:
            MasmSyntaxError: If a syntax error occurs during tokenization"""
        ...
