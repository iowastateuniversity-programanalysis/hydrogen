from pathlib import Path
import subprocess
import os
import fileinput
import re
import shutil

class CompileManager:
    def __init__(self, tmp):
        self.tmp=tmp
        self.versions=[]


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
        # for cmakelist in version_path.glob('**/CMakeLists.txt'):
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
                    # print(f'target_name: {target_name}')

                # Replace project name
                project_name_regex = r'project\s*\(\s*([a-zA-Z0-9_]+)[^)]*\)'
                project_match = re.search(project_name_regex, line)
                if project_match:
                    project_name = project_match.group(1)
                    line = re.sub(
                        project_name_regex,
                        f'project({project_name} C CXX)',
                        line)
                    # print(f'replaced project line: {line}', end='')

                filecontents += line
            
            llvmlink_target_name = f'{target_name}_llvmlink'
            to_add = f'''
# LLVM-IR Generation
list(APPEND CMAKE_MODULE_PATH "{cmake_utils_dir}")
include(LLVMIRUtil)
if("${{LINKER_LANGUAGE}}" STREQUAL "")
  set_target_properties(include_gardener PROPERTIES LINKER_LANGUAGE CXX)
endif()
llvmir_attach_bc_target({target_name}_bc {target_name})
set(-c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S)
set(llvmirBytecodeCompileOptions "-c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S")
add_compile_options({target_name}_bc "${{llvmirBytecodeCompileOptions}}")
add_dependencies({target_name}_bc {target_name})
llvmir_attach_link_target({llvmlink_target_name} {target_name}_bc -S)
            '''
            # print(f'adding {to_add}')

            filecontents += to_add

        with root_cmakelist.open('w') as file:
            file.write(filecontents)

        # Run CMake
        compile_env = os.environ.copy()
        compile_env['CXX'] = 'clang++'
        compile_env['C'] = 'clang'
        subprocess.run(['cmake', '-B', build_path, version_path], env=compile_env)
        subprocess.run(['cmake', '--build', build_path, '--target', llvmlink_target_name])


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