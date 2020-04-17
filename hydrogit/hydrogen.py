from pathlib import Path
import subprocess

class HydrogenAdapter:
    def __init__(self, hydrogen_binary):
        self.hy=hydrogen_binary
        assert self.hy.exists()

    def run(self, versions):
        # ask the compiler for the results
        args=[]
        for version in versions:
            args.append(version.bc_path)
        for version in versions:
            args.append("::")
            for c_path in version.c_paths:
                args.append(c_path)

        cmd = [str(self.hy)] + [str(arg) for arg in args]
        print(' '.join(cmd))

        subprocess.run(cmd)
