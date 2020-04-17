from git_stuff import GitManager
from compilation import CompileManager
from hydrogen import HydrogenAdapter
import os
from pathlib import Path

class HydroGit:
    def __init__(self, git_url, commit_ids, language):
        self.wd=(Path(__file__).parent.absolute())
        self.tmp=self.wd/"tmp"
        self.git_commits = commit_ids
        self.git_manager=GitManager(git_url, self.tmp)
        self.compiler=CompileManager(self.tmp, language)
        self.hydrogen_manager=HydrogenAdapter(self.wd/"../buildninja/Hydrogen.out")

    def clone(self, force):
        # git
        self.git_manager.clone(force)
        self.git_manager.checkout_copy_versions(self.git_commits)

    def compile(self, force_build):
        # compilation
        self.compiler.build_all(force_build)
        self.compiler.gather_version_files()

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
    from argparse import ArgumentParser
    parser = ArgumentParser(prog='python hydrogit.py')
    parser.add_argument(
        '-p',
        '--force-pull',
        dest='force_pull',
        help='remove existing versions and pull again',
        default=False,
        )
    parser.add_argument(
        '-b',
        '--force-build',
        dest='force_build',
        help='remove existing versions and build again',
        default=False,
        )
    parser.add_argument(
        '-l',
        '--language',
        dest='language',
        help='compile with this language - should be C or CXX',
        default='CXX',
        )
        
    parser.add_argument('url')
    parser.add_argument('first_version')
    parser.add_argument('latter_versions', nargs='+')

    args = parser.parse_args()

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
