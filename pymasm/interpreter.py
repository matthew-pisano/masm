from .pymasm import interpreter


__all__ = dir(interpreter)
globals().update({name: getattr(interpreter, name) for name in dir(interpreter) if not name.startswith('_')})
