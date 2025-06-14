"""Version information for the pymasm"""

import json
from pathlib import Path


def _get_version():
    """Retrieve the version of the pymasm package

    Returns:
        str: The version of the pymasm package, or '0.0.0' if not found"""

    # Look for version.json in the project root
    current_dir = Path(__file__).parent.parent

    version_file = current_dir / "version.json"
    if version_file.exists():
        try:
            with open(version_file, 'r') as f:
                data = json.load(f)
                return data.get('version', '0.0.0')
        except (json.JSONDecodeError, FileNotFoundError, KeyError):
            ...

    # Fallback version if version.json is not found
    return "0.0.0"


__version__ = _get_version()
