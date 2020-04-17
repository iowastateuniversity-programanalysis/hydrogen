from pathlib import Path
import subprocess
import os
import fileinput
import re
import shutil

cmake_utils_dir = (Path(__file__).parent / 'llvm-ir-cmake-utils' / 'cmake').resolve()
assert cmake_utils_dir.exists()

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
        ver=Version(version_path, self.language)
        ver.build(force)
        self.versions_built.append(ver)

class Version:
    def __init__(self, path, language):
        self.path=path
        self.c_paths=[]
        self.bc_path=None
        self.language = language

    def transform_cmakelists(self, path):
        '''
        Transform given CMakeLists.txt and return the llvmlink target name
        '''

        assert path.exists()

        filecontents = ''
        with path.open('r') as file:
            # Find & replace target and project declarations
            for line in file:
                # Find target name
                target_regex = r'add_executable\s*\(\s*([a-zA-Z0-9_]+)[^)]*\)'
                target_match = re.search(target_regex, line)
                if target_match:
                    target = target_match.group(1)

                # Replace project name
                project_regex = r'project\s*\(\s*([a-zA-Z0-9_]+)[^)]*\)'
                project_match = re.search(project_regex, line)
                if project_match:
                    project_name = project_match.group(1)
                    line = re.sub(
                        project_regex,
                        f'project({project_name} C CXX)',
                        line)

                filecontents += line
            
            # Add llvmlink target if a target was found in this file
            if target:
                llvmlink_target = f'{target}_llvmlink'
                to_add = f'''
    # LLVM-IR Generation
    list(APPEND CMAKE_MODULE_PATH "{cmake_utils_dir}")
    include(LLVMIRUtil)
    set_target_properties({target} PROPERTIES LINKER_LANGUAGE {self.language})
    add_compile_options(-c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S)
    llvmir_attach_bc_target({target}_bc {target})
    add_dependencies({target}_bc {target})
    llvmir_attach_link_target({llvmlink_target} {target}_bc -S)
                '''

                filecontents += to_add

        with path.open('w') as file:
            file.write(filecontents)
        
        return llvmlink_target
    
    def build(self, force):
        # Set up build directory
        build_path = self.path / 'build'

        # Skip if built already unless we wanna HULK SMASH
        if build_path.exists():
            if force:
                shutil.rmtree(build_path)
            else:
                print(f'Version {self.path} is already built, skipping')
                return

        build_path.mkdir(exist_ok=True)

        # Transform CMakeLists.txt
        root_cmakelist = self.path / 'CMakeLists.txt'
        llvmlink_target = self.transform_cmakelists(root_cmakelist)

        # Run CMake
        compile_env = os.environ.copy()
        if self.language == 'C':
            compile_env['CC'] = 'clang'
        elif self.language == 'CXX':
            compile_env['CXX'] = 'clang++'
        subprocess.run(args=['cmake', '-B', str(build_path), str(self.path)], env=compile_env)
        subprocess.run(args=['cmake', '--build', str(build_path), '--target', llvmlink_target, '--verbose'], env=compile_env)

def main():
    cm=CompileManager(Path("./tmp").absolute())
    cm.build_all()

if __name__ == '__main__':
    main()