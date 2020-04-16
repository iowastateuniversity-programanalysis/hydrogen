from pathlib import Path
import shutil
import subprocess
import os

class GitManager:
    def __init__(self, git_url, tmp):
        self.tmp=tmp
        self.git_url=None
        self.cloned=self.tmp/"cloned"
        self.commits=[]
        self.current_cwd=os.getcwd()
        self.git_url = git_url

    def clone(self, force=False):
        if self.tmp.exists() and not force:
            print("The tmp project exists, will not clone")
        else:
            if self.tmp.exists():
                shutil.rmtree(self.tmp)
            self.tmp.mkdir(exist_ok=True)
            subprocess.run(["git", "clone", self.git_url, str(self.cloned)])


    def checkout_copy_versions(self, versions, force=False):
        wd=os.getcwd()
        os.chdir(self.cloned)
        for version in versions:
            subprocess.run(["git", "checkout", version])
            if (self.tmp/version) .exists() and not force:
                print(str(self.tmp/version) + "exists, will not copy")
            else:
                shutil.copytree(self.cloned, self.tmp/version, ignore=shutil.ignore_patterns(".git"))
        os.chdir(wd)


def main():
    gc=GitManager("https://github.com/google/googletest.git")
    gc.clone(force=False)
    gc.checkout_copy_versions(["71d5df6c6b6769f13885a7a05dd6721a21e20c96", "01e4fbf5ca60d178007eb7900d728d73a61f5888"])

if __name__ == '__main__':
    main()