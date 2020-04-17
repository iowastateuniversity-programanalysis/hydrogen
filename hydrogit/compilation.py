from pathlib import Path
import subprocess
import os
import fileinput
import re
import shutil

class CompileManager:
    def __init__(self, tmp, language='CXX'):
        self.tmp=tmp
        self.versions=[]
        self.language=language


    def run_all(self, force=False):
        for version_path in Path(self.tmp).iterdir():
            if version_path.is_dir() and version_path.name!="cloned":
                self.run(version_path, force)

    def run(self, version_path, force=False):
        ver=Version(version_path)
        self.versions.append(ver)
        
        # Set up build directory
        build_path = version_path / 'build'

        # Skip if built already unless we wanna HULK SMASH
        if build_path.exists():
            if force:
                shutil.rmtree(build_path)
            else:
                print(f'Version {version_path} is already built, skipping')
                return
        
        build_path.mkdir()

        # Transform CMakeLists.txt
        cmake_utils_dir = (Path(__file__).parent / 'llvm-ir-cmake-utils' / 'cmake').resolve()

        root_cmakelist = version_path / 'CMakeLists.txt'
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
set_target_properties({target_name} PROPERTIES LINKER_LANGUAGE {self.language})
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
        if self.language == 'C':
            compile_env['CC'] = 'clang'
        elif self.language == 'CXX':
            compile_env['CXX'] = 'clang++'
        subprocess.run(args=['cmake', '-B', str(build_path), str(version_path)], env=compile_env)
        subprocess.run(args=['cmake', '--build', str(build_path), '--target', llvmlink_target_name, '--verbose'], env=compile_env)


class Version:
    def __init__(self, path):
        self.path=path
        self.c_paths=[]
        self.bc_path=None

def main():
    cm=CompileManager(Path("./tmp").absolute())
    cm.run_all()

if __name__ == '__main__':
    main()