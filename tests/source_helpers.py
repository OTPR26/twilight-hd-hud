"""Read the hook implementation, including its screen implementation fragments."""

from pathlib import Path
import re


def read_hook_source():
    source_dir = Path(__file__).resolve().parents[1] / 'src'

    def expand(path):
        return re.sub(
            r'^#include "([^"\n]+\.inc)"$',
            lambda match: expand(path.parent / match.group(1)),
            path.read_text(),
            flags=re.MULTILINE,
        )

    return expand(source_dir / 'item_slot_hooks.cpp')
