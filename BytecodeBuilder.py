import os
from sys import argv

def build(file, tag=''):
    if not os.path.isfile(file):
        print(f'{file} is not a valid input file.')
        return

    pwd = os.getcwd()

    path, infile = os.path.split(file)
    outfile = infile.replace('.c', f'{tag}.bc', 1)

    os.chdir(path)
    command = f"clang -c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S {infile} -o {outfile}"
    print(f'In directory {path} execute {command}')
    os.system(command)
    os.chdir(pwd)

files = [
    './TestPrograms/Buggy/Prog.c',
    './TestPrograms/Buggy2/Prog.c',
    './TestPrograms/Correct/Prog.c',
    ]
for v in files:
    build(v)

# if len(argv) > 1:
#     build(argv[1])
# else:
#     print('Usage: python3 BytecodeBuilder.py <filename>')