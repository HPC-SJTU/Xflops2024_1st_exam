import yaml
import subprocess, os
import re
import copy
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-t', type=int, default=5, help='Number of times to run each case')
parser.add_argument('--no-warmup', action='store_true', help='Do not run warmup cases')
args = parser.parse_args()


omp_proc_bind = os.environ.get('OMP_PROC_BIND')
omp_thread_num = os.environ.get('OMP_NUM_THREADS')
if not (omp_proc_bind == 'true' and omp_thread_num == '128'):
    print("make sure you have run `source env.sh`")
    exit(1)


case_list = [
   "random.10.in",
    "random.100000.in",
    "random.500000.in",
    "random.500000000.in",
    "random.10000000000.in" 
]

case_full_score = {
    "random.10.in": 10,
    "random.100000.in": 10,
    "random.500000.in": 10,
    "random.500000000.in": 35,
    "random.10000000000.in": 35
}

std_ans = {
    "random.10.in": 329014,
    "random.100000.in": 4991683583,
    "random.500000.in": 24980718446,
    "random.500000000.in": 25000642464863,
    "random.10000000000.in": 70504880369618
}

time_limit = {
    "random.10.in": 1.001,
    "random.100000.in": 1.001,
    "random.500000.in": 1.001,
    "random.500000000.in": 24,
    "random.10000000000.in": 85
}

full_score_performace = {
    "random.10.in": 1,
    "random.100000.in": 1,
    "random.500000.in": 1,
    "random.500000000.in": 16,
    "random.10000000000.in": 45 
}
# Get the path to the source_code directory
cwd = os.path.dirname(__file__)  # Get current directory (where evaluate.py is located)
src_path = os.path.abspath(os.path.join(cwd, "source_code"))  # Full path to source_code directory

def check_core(i : int):
    run_res = subprocess.run(["taskset", "-c", str(i), "echo","1"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return run_res.returncode

def check_cores():
    res_lst = []
    for i in range(128):
        if check_core(i) == 0:
            res_lst.append(i)
    if len(res_lst) != 128:
        print(f"No enough cores: need 128 but now is {len(res_lst)}")
        exit(1)

def check_clang():
    try:
        # 尝试执行clang --version命令来检查clang是否存在
        subprocess.run(["clang", "--version"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return True
    except subprocess.CalledProcessError:
        # 如果clang命令执行失败，返回False
        return False
    except FileNotFoundError:
        # 如果找不到clang命令，返回False
        return False
    
# Function to compile the code using 'make omp'
def compile_code():

    try:
        # Run `make omp` in the source_code directory
        cmp = subprocess.run(["make"], cwd=src_path, text=True, capture_output=True)
        assert cmp.returncode == 0, "Compilation Error"
        print("Compilation successful.")
    except subprocess.CalledProcessError as e:
        raise AssertionError(f"Compilation failed: {e}")

def run_a_case(case_name, i) -> float:
    result = subprocess.run(["numactl", "--physcpubind=all", "--interleave=0,1,2,3", "./cluster", case_name], cwd=src_path, text=True, capture_output=True, timeout=90)
    assert result.returncode == 0, "Runtime Error"
    result_strs = result.stdout.split('\n')
    runtime = float(re.search(r'^solve Time\s*([0-9\.e\-]+)$', result_strs[0]).group(1))
    ans = int(result_strs[1])
    print(f"{i}/{args.t} runtime: {runtime:.3f} s, ans: {ans}")
    if ans != std_ans[case_name]:
        print(f"{i}/{args.t} Wrong Answer in {case_name}")
        return -1 # -1 means Wrong Answer
    return runtime

def run_case(case_name):
    try:
        if not args.no_warmup:
            runtime = run_a_case(case_name, "warm up")
            if runtime == -1:
                return "Wrong Answer"
        time_lst = []
        for i in range(args.t):
            runtime = run_a_case(case_name, i+1)
            if runtime == -1:
                return "Wrong Answer"
            time_lst.append(runtime)

        runtime = sum(time_lst) / len(time_lst)
        if runtime > time_limit[case_name]:
            return "Time Limit Exceeded"
        print(f"Accepted in {case_name} with avg time = {runtime:.3f} s")
        return runtime
    except subprocess.CalledProcessError as e:
        print(f"Runtime Error in {case_name}")
        return "Runtime Error"

def get_score(runtime, case_name):
    if runtime < full_score_performace[case_name]:
        return case_full_score[case_name]
    else:
        return case_full_score[case_name] * (1 - (runtime - full_score_performace[case_name]) / (time_limit[case_name] - full_score_performace[case_name]))

def main():
    # Compile the code before running simulations
    try:
        compile_code()
    except AssertionError as e:
        print(e)
        return
    
    score = 0
    res_info = {}
    for case in case_list:
        print("="*5 + f" running {case} " + "="*5)
        res_info[case] = run_case(case)
        
    res_dict = {}

    normal_state = {'performance':0, 'zero_flag': 0, 'info': 'Accepted'}
    abnormal_state = {'performance': -1, 'zero_flag': 1, 'info': ''}
    for case in case_list:
        if isinstance(res_info[case], float):
            res_dict[case] = copy.deepcopy(normal_state)
            res_dict[case]['performance'] = res_info[case]
            score += get_score(res_info[case], case)
        else:
            res_dict[case] = copy.deepcopy(abnormal_state)
            res_dict[case]['info'] = res_info[case]
    with open(f'{cwd}/result.yaml',"w") as f:
        yaml.dump(res_dict, f)
    for case in case_list:
        if isinstance(res_info[case], float):
            print(f"{case} score: {get_score(res_info[case], case):.2f} / {case_full_score[case]:.2f}")
        else:
            print(f"{case} score: 0.00 / {case_full_score[case]:.2f}")
    print(f"total score: {score:.2f}")
if __name__ == "__main__":
    check_cores()
    if not check_clang():
        print(f"make sure you have run `source env.sh`")
        exit(1)
    main()

        
