from git_stuff import GitManager
from compilation import CompileManager
from hydrogen import HydrogenAdapter
from arguments import get_args
import os
from pathlib import Path

class HydroGit:
    def __init__(self, git_url, commit_ids, language):
        self.git_commits = commit_ids

        wd = (Path(__file__).parent.absolute())
        tmp = wd / "tmp"
        self.git_manager=GitManager(git_url, tmp)
        self.compiler=CompileManager(tmp, language)
        self.hydrogen_manager=HydrogenAdapter(wd / "../buildninja/Hydrogen.out")

    def clone(self, force):
        # git
        self.git_manager.clone(force)
        self.git_manager.checkout_copy_versions(self.git_commits)

    def compile(self, force_build):
        # compilation
        self.compiler.build_all(force_build)

    def hydrogen(self):
        self.hydrogen_manager.run(self.compiler.versions_built)

def run(url, commit_ids, force_pull, force_build, language):
    # setup
    hg=HydroGit(url, commit_ids, language)

    # clone
    hg.clone(force_pull)

    # fake compilation
    hg.compile(force_build)

    # hydrogen
    hg.hydrogen()

if __name__ == '__main__':
    args = get_args()

    commit_ids = [args.first_version, *args.latter_versions]

    git_url = args.url
    force_pull = args.force_pull
    force_build = args.force_build
    language = args.language

    # git_url="https://github.com/feddischson/include_gardener.git"
    # commit_ids = ["093ab9c1126c6f946e4183dcf02d8cdff837337b", "90539a60dd83a6f0a30ecbb2ddfa3eeac529e975"]
    # langauge='CXX'
    git_url="https://github.com/gydrogen/progolone.git"
    commit_ids = ["5e8651df381079d0347ddfa254f554972611d1a0", "70d03532975252bd9982beba60a8720e11ec8f02", "9cde7197d0a3fe0caf7ee0ec7fd291e19ccc18ed"]
    language='C'

    run(git_url, commit_ids, force_pull, force_build, language)
