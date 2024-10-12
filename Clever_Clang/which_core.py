import subprocess

def check_core(i : int):
    run_res = subprocess.run(["taskset", "-c", str(i), "echo","1"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return run_res.returncode

if __name__ == '__main__':
    res_lst = []
    for i in range(128):
        if check_core(i) == 0:
            res_lst.append(i)
    print(f"get cores: {res_lst}")