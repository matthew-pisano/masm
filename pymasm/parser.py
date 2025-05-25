from .pymasm import parser


__all__ = dir(parser)
globals().update({name: getattr(parser, name) for name in dir(parser) if not name.startswith('_')})
