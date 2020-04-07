import os
from sys import argv

if len(argv) > 1:
    infile = argv[1]
    output = argv[1].replace('.c', '.bc', 1)
    if not (infile.endswith('.c') and os.path.isfile(infile)):
        print(f'{infile} is not a valid input file.')
        exit()

    os.system(f"clang -c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S {infile} -o {output}")
    print(f'Spit out {output}')
else:
    print('Usage: python3 BytecodeBuilder.py <filename>')