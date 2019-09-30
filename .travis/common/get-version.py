from re import search
from os.path import join, abspath

with open(abspath(join('src', 'common', 'version.cpp'))) as h:
    text = h.read()
    VERSION_RE = r'const u16 TYPE = \b(.*)\b;'

    major = None
    minor = None
    patch = None

    for t in ['major', 'minor', 'patch']:
        exec('{} = {}'.format(t, search(VERSION_RE.replace("TYPE", t), text).group(1)))

    print('{}.{}.{}'.format(major, minor, patch))
