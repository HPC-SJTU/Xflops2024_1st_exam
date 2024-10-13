import json
import subprocess
import os, shutil
import re

py_dir = os.path.dirname(__file__)

critical_file = {"make_env.json","Make.xflops", "hpl.slurm","HPL.dat"}


def check_all_file():
    file_lst = os.listdir(f"{py_dir}/source_code/submit")
    file_lst = set(file_lst)
    assert len(critical_file - file_lst) == 0, f"Missing file: {critical_file - file_lst}"

def gen_compile_sh():
    with open(f"{py_dir}/source_code/submit/make_env.json", "r") as f:
        make_dic = json.load(f)
    check_re = re.compile(r"^[a-zA-Z0-9_\./\-]+$")
    for i in make_dic["modules"]:
        assert check_re.match(i), f"Module {i} is invalid"
    with open(f"{py_dir}/source_code/hpl-2.3/make.sh", "w") as f:
        for i in make_dic["modules"]:
            f.write(f"module load '{i}'\n")
        # Evil feature: The Makefile seems to not handle parallel dependencies well. Let's restart to fix it
        f.write("make arch=xflops -j\nmake arch=xflops -j")

def handle_make():
    with open(f"{py_dir}/source_code/submit/Make.xflops", "r") as f:
        chars = f.read()
    chars, count = re.subn(r'TOPdir([\s]*)=[\s]*(.*)\n',r'TOPdir\1= '+f"{py_dir}/source_code/hpl-2.3\n", chars)
    assert count == 1, "replace error"
    with open(f"{py_dir}/source_code/hpl-2.3/Make.xflops", "w") as f:
        f.write(chars)

def remove_history_make():
    os.system(f"cd {py_dir}/source_code/hpl-2.3 && rm -r bin lib src/*/xflops testing/*/xflops")

def clear_files():
    subprocess.run(["rm", "Make.xflops", "make.sh"], cwd=f"{py_dir}/source_code/hpl-2.3")

def compile():
    check_all_file()
    gen_compile_sh()
    handle_make()
    remove_history_make()
    compile_run = subprocess.run(["bash", "make.sh"], cwd=f"{py_dir}/source_code/hpl-2.3", stdout= log_f, stderr=log_f)
    # fall back to serial
    if compile_run.returncode != 0:
        log_f.write("\n\n" + "="*20 + "Fall back to serial" + "="*20 +"\n\n")
        log_f.flush()
        print(f"Compile parallel error, fall back to serial")
        with open(f"{py_dir}/source_code/hpl-2.3/make.sh","r") as f:
            all_lines = f.readlines()
        all_lines = all_lines[:-2]
        all_lines.append("make arch=xflops")
        with open(f"{py_dir}/source_code/hpl-2.3/make.sh","w") as f:
            f.writelines(all_lines)
        
        compile_run = subprocess.run(["bash", "make.sh"], cwd=f"{py_dir}/source_code/hpl-2.3", stdout= log_f, stderr=log_f)
        assert compile_run.returncode == 0, "Compile error"
    clear_files()
    print(f"Compile success!")

def run_hpl():
    # change HPL.dat
    shutil.copy(f"{py_dir}/source_code/submit/HPL.dat", f"{py_dir}/source_code/hpl-2.3/bin/xflops/HPL.dat")
    shutil.copy(f"{py_dir}/source_code/submit/hpl.slurm", f"{py_dir}/source_code/hpl-2.3/hpl.slurm")
    subprocess.run(["sbatch", "--time=1:00:00" , "hpl.slurm"], cwd=f"{py_dir}/source_code/hpl-2.3")
    os.remove(f"{py_dir}/source_code/hpl-2.3/hpl.slurm")


if __name__ == "__main__":
    log_f = open(f"{py_dir}/evaluate.log","w")
    compile()
    run_hpl()
    log_f.close()