from sys import argv
from pefile import PE
from os.path import join, isdir
from re import match
from os import walk
from queue import Queue
from shutil import copy

# constant definitions
KNOWN_SYS_DLLS = ['WINMM.DLL', 'MSVCRT.DLL', 'VERSION.DLL', 'MPR.DLL',
                  'DWMAPI.DLL', 'UXTHEME.DLL', 'DNSAPI.DLL', 'IPHLPAPI.DLL']
# below is for Ubuntu 18.04 with specified PPA enabled, if you are using
# other distro or different repositories, change the following accordingly
DLL_PATH = [
    '/usr/x86_64-w64-mingw32/bin/',
    '/usr/x86_64-w64-mingw32/lib/',
    '/usr/lib/gcc/x86_64-w64-mingw32/7.3-posix/'
]

missing = []


def parse_imports(file_name):
    results = []
    pe = PE(file_name, fast_load=True)
    pe.parse_data_directories()

    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        current = entry.dll.decode()
        current_u = current.upper()  # b/c Windows is often case insensitive
        # here we filter out system dlls
        # dll w/ names like *32.dll are likely to be system dlls
        if current_u.upper() not in KNOWN_SYS_DLLS and not match(string=current_u, pattern=r'.*32\.DLL'):
            results.append(current)

    return results


def parse_imports_recursive(file_name, path_list=[]):
    q = Queue()  # create a FIFO queue
    # file_name can be a string or a list for the convience
    if isinstance(file_name, str):
        q.put(file_name)
    elif isinstance(file_name, list):
        for i in file_name:
            q.put(i)
    full_list = []
    while q.qsize():
        current = q.get_nowait()
        print('> {}'.format(current))
        deps = parse_imports(current)
        # if this dll does not have any import, ignore it
        if not deps:
            continue
        for dep in deps:
            # the dependency already included in the list, skip
            if dep in full_list:
                continue
            # find the requested dll in the provided paths
            full_path = find_dll(dep)
            if not full_path:
                missing.append(dep)
                continue
            full_list.append(dep)
            q.put(full_path)
            path_list.append(full_path)
    return full_list


def find_dll(name):
    for path in DLL_PATH:
        for root, _, files in os.walk(path):
            for f in files:
                if name.lower() == f.lower():
                    return join(root, f)


def deploy(name, dst, dry_run=False):
    dlls_path = []
    parse_imports_recursive(name, dlls_path)
    for dll_entry in dlls_path:
        if not dry_run:
            copy(dll_entry, dst)
        else:
            print('[Dry-Run] Copy {} to {}'.format(dll_entry, dst))
    print('Deploy completed.')
    return dlls_path


def main():
    if len(argv) < 3:
        print('Usage: scan_dll.py [files to examine ...] [target deploy directory]')
        return 1
    to_deploy = argv[1:-1]
    tgt_dir = argv[-1]
    if not isdir(tgt_dir):
        print('{} is not a directory.'.format(tgt_dir))
        
    print('Scanning dependencies...')
    deploy(to_deploy, tgt_dir)
    if missing:
        print('Following DLLs are not found: {}'.format('\n'.join(missing)))


if __name__ == '__main__':
    main()
