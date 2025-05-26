"""MASM Exceptions"""


class MasmException(RuntimeError):
    """A base class for all MASM exceptions"""

    def __init__(self, message: str, use_addr: bool, loc: int = 0) -> None: ...


class MasmSyntaxError(MasmException):
    """A class for syntax errors in MASM"""

    def __init__(self, message: str, lineno: int = 0) -> None: ...


class MasmRuntimeError(MasmException):
    """A class for runtime errors in MASM"""

    def __init__(self, message: str, addr: int = -1) -> None: ...


class ExecExit(RuntimeError):
    """Exception to indicate that the program has terminated successfully with the given code"""

    def __init__(self, message: str, code: int) -> None: ...

    def code(self) -> int:
        """Get the error code of the exception"""
        ...
