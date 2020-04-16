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
        git_url="https://github.com/google/googletest.git"
        self.git_commits = ["71d5df6c6b6769f13885a7a05dd6721a21e20c96", "01e4fbf5ca60d178007eb7900d728d73a61f5888"]

        self.git_manager=GitManager(git_url, self.tmp)

        # compilation
        self.compiler=CompileManager(self.tmp)

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

        cmd=([str(self.hydrogen_binary)]+[str(arg) for arg in args])

        subprocess.run(cmd)

def demo_override():
    # setup
    hg=HydroGit()
    hg.setup()

    # clone
    hg.clone()

    # override clone
    hg.tmp=hg.wd/"yolotests"
    hg.setup()

    # fake compilation
    hg.compiler.run_all()
    for version in hg.compiler.versions:
        for p in version.path.iterdir():
            if p.suffix==".c":
                version.c_paths.append(p)
            else:
                assert p.suffix==".bc"
                version.bc_path=p

    # hydrogen
    hg.hydrogen()

if __name__ == '__main__':
    demo_override()