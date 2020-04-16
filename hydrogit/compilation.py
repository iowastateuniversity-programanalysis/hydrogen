from pathlib import Path

class CompileManager:
    def __init__(self, tmp):
        self.tmp=tmp
        self.versions=[]


    def run_all(self):
        for version_path in Path(self.tmp).iterdir():
            if version_path.is_dir() and version_path.name!="cloned":
                self.run(version_path)

    def run(self, version_path):
        ver=Version(version_path)
        self.versions.append(ver)

        print("Some compilation magic " + version_path.name)


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