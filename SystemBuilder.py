#!/bin/python3
########################################
#           Compile Hydrogen           #

from sys import argv

########################################
import os
print('Compiling Hydrogen')
os.chdir('./build')
os.system('ninja')
if (os.path.isfile('./Result.txt')):
    os.system('rm ./Result.txt')
if (os.path.isfile('./MVICFG.dot')):
    os.system('rm ./MVICFG.dot')
os.chdir('../')
########################################
#           Path Settings              #
########################################
demarcator = ' :: '

# test_root = './TestPrograms/Basic'
# files = {
#     'Buggy': {
#         'Source': ['Prog.c'],
#         'Bytecode': 'Prog.bc',
#     },
#     'Buggy2': {
#         'Source': ['Prog.c'],
#         'Bytecode': 'Prog.bc',
#     },
#     'Correct': {
#         'Source': ['Prog.c'],
#         'Bytecode': 'Prog.bc',
#     },
# }

# test_root = './TestPrograms/Mine'
# files = {
#     'Ladybug1': {
#         'Source': ['main.c'],
#         'Bytecode': 'main.bc',
#     },
#     'Ladybug2': {
#         'Source': ['main.c'],
#         'Bytecode': 'main.bc',
#     },
#     'Ladybug3': {
#         'Source': ['main.c'],
#         'Bytecode': 'main.bc',
#     },
# }

test_root = './TestPrograms/OSS'
files = {
    'fiche_1': {
        'Source': ['fiche.c', 'main.c'],
        'Bytecode': 'prog.bc',
    },
    'fiche_2': {
        'Source': ['fiche.c', 'main.c'],
        'Bytecode': 'prog.bc',
    },
    'fiche_3': {
        'Source': ['fiche.c', 'main.c'],
        'Bytecode': 'prog.bc',
    },
}

if len(argv) == 3:
    items = list(files.items())
    i1 = items[int(argv[1])]
    i2 = items[int(argv[2])]
    files = {i1[0]: i1[1], i2[0]: i2[1]}

input_files = [[os.path.join(test_root, k, f) for f in v['Source']] for k, v in files.items()]

output_files = [os.path.join(test_root, k, v['Bytecode']) for k, v in files.items()]

# ./Hydrogen.out ../TestPrograms/Buggy/ProgV1.bc ../TestPrograms/Correct/ProgV2.bc :: ../TestPrograms/Buggy/Prog.c :: ../TestPrograms/Correct/Prog.c

path_argument = demarcator.join([
    ' '.join(output_files),
    *(' '.join(fl) for fl in input_files)
])
print('Argument:\n' + path_argument)
########################################
#         Running Hydrogen             #
########################################
print('Running Hydrogen')
os.system(f'./build/Hydrogen.out {path_argument}')
########################################
#        Collecting Results            #
########################################
tag = ''
if len(argv) == 3:
    tag = f'_{argv[1]}_{argv[2]}'

os.system(f'mv Result.txt {test_root}/Result{tag}.txt')
os.system(f'mv MVICFG.dot {test_root}/MVICFG{tag}.dot')
