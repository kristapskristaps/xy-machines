"""Publish the collection's SDK pin as GitHub Actions checkout outputs."""
import json
import os
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]


def checkout_pin(root=ROOT):
    repository = (root / 'SDK_REPOSITORY').read_text().strip()
    match = re.fullmatch(r'https://github\.com/([A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+)', repository)
    if not match:
        raise ValueError('SDK_REPOSITORY must be an HTTPS GitHub repository URL')
    revision = (root / 'SDK_REVISION').read_text().strip()
    if not re.fullmatch(r'[0-9a-f]{40}', revision):
        raise ValueError('SDK_REVISION must be a full commit SHA')
    return {'repository': match[1].removesuffix('.git'), 'revision': revision}


def main():
    pin = checkout_pin()
    if output := os.environ.get('GITHUB_OUTPUT'):
        with open(output, 'a', encoding='utf-8') as stream:
            for key, value in pin.items():
                stream.write(f'{key}={value}\n')
    print(json.dumps(pin))


if __name__ == '__main__':
    main()
