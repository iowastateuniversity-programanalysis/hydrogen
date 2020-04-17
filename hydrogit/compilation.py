from pathlib import Path
import subprocess
import os
import fileinput
import re
import shutil

class CompileManager:
    def __init__(self, tmp, language='CXX'):
        self.tmp=tmp
        self.versions_built=[]
        self.language=language

    def gather_version_files(self):
        '''
        Gather sources 
        '''
        for version in self.versions_built:
            # gather C sources
            if self.language == 'C':
                for p in (version.path).glob('*.c'):
                    version.c_paths.append(p)
                for p in (version.path / 'src').glob('**/*.c'):
                    version.c_paths.append(p)
            
            # gather C++ sources
            elif self.language == 'CXX':
                for p in (version.path).glob('*.cpp'):
                    version.c_paths.append(p)
                for p in (version.path / 'src').glob('**/*.cpp'):
                    version.c_paths.append(p)

            # gather compiled bytecode
            for p in (version.path / 'build' / 'llvm-ir').glob('**/*_llvmlink.bc'):
                version.bc_path=p

    def build_all(self, force):
        '''
        Run compilation step for all versions.
        '''

        for version_path in Path(self.tmp).iterdir():
            if version_path.is_dir() and version_path.name!="cloned":
                self.build_one(version_path, force)

    def build_one(self, version_path, force):
        ver=Version(version_path)
        ver.build(self.language, force)
        self.versions_built.append(ver)

class Version:
    def __init__(self, path):
        self.path=path
        self.c_paths=[]
        self.bc_path=None
    
    def build(self, language, force):
        # Set up build directory
        build_path = self.path / 'build'

        # Skip if built already unless we wanna HULK SMASH
        if build_path.exists():
            if force:
                shutil.rmtree(build_path)
            else:
                print(f'Version {self.path} is already built, skipping')
                return
        
        build_path.mkdir()

        # Transform CMakeLists.txt
        cmake_utils_dir = (Path(__file__).parent / 'llvm-ir-cmake-utils' / 'cmake').resolve()

        root_cmakelist = self.path / 'CMakeLists.txt'
        filecontents = ''
        with root_cmakelist.open('r') as file:
            for line in file:
                # Find target name
                target_name_regex = r'add_executable\s*\(\s*([a-zA-Z0-9_]+)[^)]*\)'
                target_match = re.search(target_name_regex, line)
                if target_match:
                    target_name = target_match.group(1)

                # Replace project name
                project_name_regex = r'project\s*\(\s*([a-zA-Z0-9_]+)[^)]*\)'
                project_match = re.search(project_name_regex, line)
                if project_match:
                    project_name = project_match.group(1)
                    line = re.sub(
                        project_name_regex,
                        f'project({project_name} C CXX)',
                        line)

                filecontents += line
            
            llvmlink_target_name = f'{target_name}_llvmlink'
            to_add = f'''
# LLVM-IR Generation
list(APPEND CMAKE_MODULE_PATH "{cmake_utils_dir}")
include(LLVMIRUtil)
set_target_properties({target_name} PROPERTIES LINKER_LANGUAGE {language})
add_compile_options(-c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S)
llvmir_attach_bc_target({target_name}_bc {target_name})
add_dependencies({target_name}_bc {target_name})
llvmir_attach_link_target({llvmlink_target_name} {target_name}_bc -S)
            '''

            filecontents += to_add

        with root_cmakelist.open('w') as file:
            file.write(filecontents)

        # Run CMake
        compile_env = os.environ.copy()
        if language == 'C':
            compile_env['CC'] = 'clang'
        elif language == 'CXX':
            compile_env['CXX'] = 'clang++'
        subprocess.run(args=['cmake', '-B', str(build_path), str(self.path)], env=compile_env)
        subprocess.run(args=['cmake', '--build', str(build_path), '--target', llvmlink_target_name, '--verbose'], env=compile_env)

def main():
    cm=CompileManager(Path("./tmp").absolute())
    cm.build_all()

if __name__ == '__main__':
    main()