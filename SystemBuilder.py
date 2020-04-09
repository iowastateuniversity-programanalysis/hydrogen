#!/bin/python3
########################################
#           Compile Hydrogen           #
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
test_root = './TestPrograms'

files = {
    'Buggy': {
        'Source': ['Prog.c'],
        'Bytecode': 'Prog.bc',
    },
    # 'Buggy2': {
    #     'Source': ['Prog.c'],
    #     'Bytecode': 'Prog1.bc',
    # },
    'Correct': {
        'Source': ['Prog.c'],
        'Bytecode': 'Prog.bc',
    },
}

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
os.system(f'mv Result.txt {test_root}/Result.txt')
os.system(f'mv MVICFG.dot {test_root}/MVICFG.dot')
