import yaml, subprocess, os, argparse, threading
"""
    bobthebuilder.py
    uses the gitlab ci file to build the multiple projects and output them to a specific build dir
"""

class Bob():
    def __init__(self) -> None:
        self.output_dir = "/tmp"
        self.current_dir = "/".join(os.path.dirname(os.path.abspath(__file__)).split("/")[:-2])+"/"
        self.threads = []

        p = argparse.ArgumentParser(description="")
        p.add_argument('--ci_file', help="The config yaml file that defines the build components.", default=".gitlab-ci.yml")
        p.add_argument('--build_dir', help="change the global build dir.", default="/tmp/")
        p.add_argument('--single_thread', help="run in single threaded mode for debugging")
        args = p.parse_args()
        yaml_file = args.ci_file
        self.output_dir = args.build_dir

        #validate yaml file here
        try:
            with open(f"{yaml_file}", "r") as f:
                yams = yaml.safe_load(f)
                for vars in yams.values():
                    compile_file = vars.get("EXOPROJECT_COMPILER_FILE")
                    config_file = vars.get("EXOPROJECT_CONFIG_FILE")
                    build_dir = vars.get("BUILD_DIR")
                    extra_flags = vars.get("EXTRA_FLAGS")
                    if compile_file is not None or config_file is not None or build_dir is not None:
                        th = threading.Thread(target=self.build, args=(compile_file, config_file, build_dir, extra_flags))
                        if args.single_thread == True:
                            th.start()
                        else:
                            self.threads.append(th)

                for t in self.threads:
                    t.start()
                for t in self.threads:
                    t.join()

        except (FileNotFoundError, IsADirectoryError) as fnotfound:
            print(f"File Not Found: {fnotfound} use the ci_file to specify a build file.")

    def build(self, comp_file, config_file, build_dir, extra_flags=""):
        build_dir = f"{self.output_dir}/cmake-build-" + build_dir

        cmake_config_cmd = "/usr/bin/cmake  -DCMAKE_BUILD_TYPE=Debug {} -DEXOPROJECT_COMPILER_FILE:STRING={} \
        -DEXOPROJECT_CONFIG_FILE:STRING={} -S . -B {}".format(extra_flags, self.current_dir + comp_file, self.current_dir + config_file, build_dir)
        cmake_build_cmd = "/usr/bin/cmake --build {}".format(build_dir)
        cmake_output = ""

        try:
            cmake_output = subprocess.check_output(cmake_config_cmd.split(" "), stderr=subprocess.STDOUT, universal_newlines=True, encoding='UTF-8')
            cmake_output += subprocess.check_output(cmake_build_cmd.split(" "), stderr=subprocess.STDOUT, universal_newlines=True, encoding='UTF-8')
            print("SUCCESS ------ {}".format(build_dir))
        except subprocess.CalledProcessError as e:
            print("FAILED ------ {}".format(build_dir))
            print(cmake_config_cmd)
            print(cmake_build_cmd)
        except Exception as e:
            print(e)
        finally:
            with open(build_dir + "/res.txt", "w") as f:
                f.writelines(cmake_output)

if __name__ == "__main__":
    b = Bob()