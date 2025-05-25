from . import pymasm as _pymasm


# Make submodules available at package level
tokenizer = _pymasm.tokenizer
parser = _pymasm.parser
interpreter = _pymasm.interpreter
