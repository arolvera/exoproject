import os, pathlib, subprocess

#todo get device params from the yaml file

class ManifestManifestor():
    def __init__(self) -> None:
        self.header = "#Path:Device Id:Magic Number:Git SHA:VersionMajor.VersionMinor.Revision\n"
        self.manifest = None 

    def create_manifest(self, manifest_file):
        self.manifest = open(manifest_file, "w")
        self.manifest.write(self.header)

    def add_file(self, file):
        fpath = os.path.abspath(file)
        self.manifest.write(f"{fpath}:{self.get_device_id()}:{self.get_magic_num()}:{self.get_git_sha()}:{self.get_ver_maj()}.{self.get_ver_min()}.{self.get_ver_rev()}")

    def get_device_id(self):
        return "0" #system_control
    def get_magic_num(self):
        return "61646373"
    def get_git_sha(self):
        gitsha = subprocess.check_output(["git", "log", "-n", "1", "--pretty=format:%h"], stderr=subprocess.STDOUT, text=True)
# subprocess.run("git log -n 1 --pretty=format: '%h'")
        return gitsha #str(os.system("git log -n 1 --pretty=format:'%h'"))
    def get_ver_maj(self):
        return "0"
    def get_ver_min(self):
        return "0"
    def get_ver_rev(self):
        return "0"
    

if __name__ == "__main__":
    m = ManifestManifestor()
    m.create_manifest("manifest.txt")
    m.add_file("system-control-halo12-va41630-xenon-silver-boeing.bin")