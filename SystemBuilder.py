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
pathToDir = os.path.abspath('./TestPrograms/')

files = {
    'Buggy': {
        'Source': ['Prog.c'],
        'Bytecode': 'Prog.bc',
    },
    'Buggy2': {
        'Source': ['Prog.c'],
        'Bytecode': 'Prog.bc',
    },
    'Correct': {
        'Source': ['Prog.c'],
        'Bytecode': 'Prog.bc',
    },
}

input_files_relative = [[os.path.join(k, f) for f in v['Source']] for k, v in files.items()]
input_files = [[os.path.join(pathToDir, f) for f in fl] for fl in input_files_relative]

output_files_relative = [os.path.join(k, v['Bytecode']) for k, v in files.items()]
output_files = [os.path.join(pathToDir, f) for f in output_files_relative]

path_argument = demarcator.join([
    ' '.join(output_files),
    *(' '.join(fl) for fl in input_files)
])
print('Argument:\n' + path_argument)
########################################
#         Running Hydrogen             #
########################################
print('Running Hydrogen')
os.chdir('./build')
os.system(f'./Hydrogen.out {path_argument}')
os.chdir('../')
########################################
#        Collecting Results            #
########################################
os.system(f'mv ./build/Result.txt {pathToDir}/Result.txt')
os.system(f'mv ./build/MVICFG.dot {pathToDir}/MVICFG.dot')
