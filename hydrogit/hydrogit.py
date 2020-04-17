from git_stuff import *
from compilation import *
from hydrogen import *
import os

class HydroGit:
    def __init__(self):
        self.wd=(Path(__file__).parent.absolute())
        self.tmp=self.wd/"tmp"
        self.git_manager=None
        self.git_commits=None
        self.compiler = None
        self.hydrogen_binary=None

    def interactive_setup(self):
        pass

    def setup(self):
        # git
        git_url="https://github.com/feddischson/include_gardener.git"
        self.git_commits = ["093ab9c1126c6f946e4183dcf02d8cdff837337b", "90539a60dd83a6f0a30ecbb2ddfa3eeac529e975"]
        # git_url="https://github.com/gydrogen/progolone.git"
        # self.git_commits = ["5e8651df381079d0347ddfa254f554972611d1a0", "70d03532975252bd9982beba60a8720e11ec8f02", "9cde7197d0a3fe0caf7ee0ec7fd291e19ccc18ed"]

        self.git_manager=GitManager(git_url, self.tmp)

        # compilation
        self.compiler=CompileManager(self.tmp, language='CXX')

        # hydrogen
        self.hydrogen_binary=self.wd/"../buildninja/Hydrogen.out"

    def clone(self, force=False):
        # git
        self.git_manager.clone(force)
        self.git_manager.checkout_copy_versions(self.git_commits)

    def compile(self):
        # compilation
        self.compiler.run_all()

    def hydrogen(self):
        # ask the compiler for the results
        args=[]
        for version in self.compiler.versions:
            args.append(version.bc_path)
        for version in self.compiler.versions:
            args.append("::")
            for c_path in version.c_paths:
                args.append(c_path)

        cmd = [str(self.hydrogen_binary)] + [str(arg) for arg in args]
        print(' '.join(cmd))

        subprocess.run(cmd)

def demo_override(force_pull, force_build):
    # setup
    hg=HydroGit()
    hg.setup()

    # clone
    hg.clone(force_pull)

    # override clone
    # hg.tmp=hg.wd/"yolotests"
    # hg.setup()

    # fake compilation
    hg.compiler.run_all(force_build)
    for version in hg.compiler.versions:
        for p in (version.path).glob('*.cpp'):
            version.c_paths.append(p)
        for p in (version.path).glob('*.c'):
            version.c_paths.append(p)
        for p in (version.path / 'src').glob('**/*.cpp'):
            version.c_paths.append(p)
        for p in (version.path / 'src').glob('**/*.c'):
            version.c_paths.append(p)
        for p in (version.path / 'build' / 'llvm-ir').glob('**/*_llvmlink.bc'):
            version.bc_path=p

    # hydrogen
    hg.hydrogen()

if __name__ == '__main__':
    from argparse import ArgumentParser
    parser = ArgumentParser(prog='python hydrogit.py')
    parser.add_argument(
        '-p',
        '--force-pull',
        dest='force_pull',
        help='even if an existing destination version has been pulled, remove it and pull again',
        default=False)
    parser.add_argument(
        '-b',
        '--force-build',
        dest='force_build',
        help='even if an existing destination version has been built, remove it and build again',
        default=False)

    args = parser.parse_args()

    demo_override(args.force_pull, args.force_build)
