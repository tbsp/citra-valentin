from re import search
from subprocess import run
from os.path import join, abspath

with open(abspath(join('src', 'common', 'version.h'))) as h:
    text = h.read()
    VERSION_RE = r'constexpr u16 TYPE = \b(.*)\b;'

    major = None
    minor = None
    patch = None

    for t in ['major', 'minor', 'patch']:
        exec('{} = {}'.format(t, search(VERSION_RE.replace("TYPE", t), text)[1]))

    print(f'{major}.{minor}.{patch}')
