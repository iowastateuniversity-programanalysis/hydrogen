from pathlib import Path
import subprocess
import os
import fileinput
import re
import shutil

cmake_utils_dir = (Path(__file__).parent / 'llvm-ir-cmake-utils' / 'cmake').resolve()
assert cmake_utils_dir.exists()

class CompileManager:
    def __init__(self, language, tmp):
        self.language=language
        self.tmp=tmp
        self.versions_built=[]

    def build_all(self, force):
        '''
        Run compilation step for all versions.
        '''

        for version_path in Path(self.tmp).iterdir():
            if version_path.is_dir() and version_path.name!="cloned":
                self.build_one(version_path, force)

    def build_one(self, version_path, force):
        ver=Version(version_path, self.language)
        ver.build(force)
        self.versions_built.append(ver)

class Version:
    def __init__(self, path, language):
        self.path=path
        self.build_path = self.path / 'build'
        self.c_paths=[]
        self.bc_path=None
        self.language = language
    
    def build(self, force):
        # Set up build path
        self.setup_build_path(force)

        # Transform CMakeLists.txt
        root_cmakelist = self.path / 'CMakeLists.txt'
        self.transform_cmakelists(root_cmakelist)

        # Run CMake and collect the output
        self.cmake()
        self.glob_files()

    def setup_build_path(self, force):
        # Skip if built already unless we wanna HULK SMASH
        if self.build_path.exists():
            if force:
                shutil.rmtree(self.build_path)
            else:
                print(f'Version {self.path} is already built, skipping')
                return

        self.build_path.mkdir(exist_ok=True)
    
    def glob_files(self):
        '''
        Gather sources and compiled bytecode for this version
        '''
        # gather C sources
        if self.language == 'C':
            for p in (self.path).glob('*.c'):
                self.c_paths.append(p)
            for p in (self.path / 'src').glob('**/*.c'):
                self.c_paths.append(p)
        
        # gather C++ sources
        elif self.language == 'CXX':
            for p in (self.path).glob('*.cpp'):
                self.c_paths.append(p)
            for p in (self.path / 'src').glob('**/*.cpp'):
                self.c_paths.append(p)
                
        assert len(self.c_paths) > 0

        # gather compiled bytecode
        self.bc_path = next((self.build_path / 'llvm-ir').glob('**/*_hydrogit_llvm_link.bc'))
        assert self.bc_path

    def transform_cmakelists(self, path):
        '''
        Transform given CMakeLists.txt and return the llvmlink target name
        '''

        assert path.exists()

        with path.open('a') as file:        
            ir_gen = f'''
#{'='*10}LLVM IR generation
list(APPEND CMAKE_MODULE_PATH "{cmake_utils_dir}")
include(LLVMIRUtil)
enable_language(C)
get_directory_property(_allTargets BUILDSYSTEM_TARGETS)
foreach(_target ${{_allTargets}})
    set_target_properties(${{_target}} PROPERTIES LINKER_LANGUAGE C)
    add_compile_options(-c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S)
    llvmir_attach_bc_target(${{_target}}_bc ${{_target}})
    add_dependencies(${{_target}}_bc ${{_target}})
    llvmir_attach_link_target(${{_target}}_hydrogit_llvm_link ${{_target}}_bc -S)
endforeach(_target ${{_allTargets}})
# end LLVM IR generation
#{'='*10}'''

            file.write(ir_gen)

    def cmake(self):
        '''
        Run CMake with the given target
        '''

        compile_env = os.environ.copy()
        if self.language == 'C':
            compile_env['CC'] = 'clang'
        elif self.language == 'CXX':
            compile_env['CXX'] = 'clang++'

        subprocess.run(args=[
            'cmake',
            '-B', str(self.build_path),
            str(self.path)
        ], env=compile_env)

        llvm_ir_path = self.build_path / 'llvm-ir'
        assert(llvm_ir_path.exists())

        target_bcs = list(llvm_ir_path.glob('*_hydrogit_llvm_link'))
        assert(bc.exists() for bc in target_bcs)
        assert(len(target_bcs) > 0)

        target_names = [p.stem for p in target_bcs]
        print('building targets', target_names)
        subprocess.run(args=[
            'cmake',
            '--build',
            str(self.build_path),
            '--target', ' '.join(target_names),
            # '--verbose' # Uncomment to show Make output
        ], env=compile_env)

def main():
    cm=CompileManager('C', Path("./tmp").absolute())
    cm.build_all(True)

if __name__ == '__main__':
    main()