from re import L
import subprocess,os
import yaml
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-t', type=int, default=5, help='Number of times to run each case')
parser.add_argument('--no-warmup', action='store_true', help='Do not run warmup cases')
args = parser.parse_args()

n_case=5
tries = args.t
cwd = os.path.dirname(__file__)+"/source_code"
src_path = os.path.abspath(f"{cwd}/gd")

case_ratio = [0,0.25,0.25,0.25,0.25]
full_time = [0,75,75,2500,1000]
base_time = [0,3000,3000,20000,6000]

def check_clang():
    try:
        # 尝试执行clang --version命令来检查clang是否存在
        subprocess.run(["clang++", "--version"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return True
    except subprocess.CalledProcessError:
        # 如果clang命令执行失败，返回False
        return False
    except FileNotFoundError:
        # 如果找不到clang命令，返回False
        return False

def check_core(i : int):
    run_res = subprocess.run(["taskset", "-c", str(i), "echo","1"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    return run_res.returncode
    
def run_compile():
    cmp = subprocess.run(["clang++", "-O3", "-fopenmp", "gd.cpp","main.cpp", "-o", "gd"], cwd=cwd, text=True, capture_output=True)
    assert cmp.returncode == 0, "Compile Error"
    
def run_case(i):
    try:
        run_1 = subprocess.run(["taskset","-c",core_chars ,"./gd", f"conf{i}.data", f"out{i}.data"], cwd=cwd, text=True, capture_output=True, timeout=30)
    except subprocess.TimeoutExpired:
        raise AssertionError("Time Limit Exceeded")
    assert run_1.returncode == 0, "Runtime Error"
    time = float(run_1.stdout.split('\n')[-2].replace('Time: ','').replace('ms',''))
    print(f"cost: {time}ms")
    return time

def run_ans(i):
    if not args.no_warmup:
        run_case(i)
    time_list = [run_case(i) for _ in range(tries)]
    mean = sum(time_list) / len(time_list)
    return mean
    

def check_correct(i):
    f1 = open(cwd + f"/out{i}.data", 'r')
    f2 = open(cwd + f"/ref{i}.data", 'r')
    mae = 0
    n = 0
    for line1, line2 in zip(f1, f2):
        if line1 == line2:
            continue
        mae += abs(float(line1) - float(line2))
        n += 1
    f1.close()
    f2.close()
    if n == 0:
        return True
    mae/=n
    # print(f"Case {i} MAE: {mae}")
    return mae < 1e-6



if __name__ == "__main__":
    if not check_clang():
        print(f"make sure you have run `module load bisheng/2.5.0`")
        exit(1)
        
    # find cores
    res_lst = []
    for i in range(128):
        if check_core(i) == 0:
            res_lst.append(i)
    if not len(res_lst) >= 4:
        print(f"No enough cores: need 4 but now is {len(res_lst)}")
        exit(1)
    
    core_chars = ','.join([str(i) for i in res_lst[:4]])
    print(f"Using cores: {core_chars}")

    res_dict = {
        i:{
            "info": "Accepted",
            "performance": 0,
            "zero_flag": 0,
        } for i in range(n_case)
    }
    try:
        run_compile()
    except AssertionError as e:
        print(e)
        for i in range(n_case):
            res_dict[i]["info"] = e.args[0]
            res_dict[i]["zero_flag"] = 1
        exit(1)
        
    for i in range(n_case):
        print(f"Running case {i}/{n_case}")
        try:
            time = run_ans(i)
        except AssertionError as e:
            print(f"Running case {i} failed: {e}")
            res_dict[i]["info"] = e.args[0]
            res_dict[i]["zero_flag"] = 1
            continue
        
        res_dict[i]["performance"] = time
        if not check_correct(i):
            res_dict[i]["zero_flag"] = 1
            res_dict[i]["info"] = "Wrong Answer"
        
    score = 0
    for i in range(n_case):
        if res_dict[i]["zero_flag"] or case_ratio[i] == 0:
            continue
        # print("DEBUG: ",full_time[i]*(base_time[i] - res_dict[i]["performance"])/(res_dict[i]["performance"]*(base_time[i]-full_time[i])))
        # breakpoint()
        avg_time = res_dict[i]["performance"]
        case_score =  min(max(0, 
                        full_time[i]*(base_time[i] - avg_time)/(avg_time*(base_time[i]-full_time[i]))
                        ), 1)
        score += case_ratio[i] *case_score
        print(f"Case {i} score: {case_score*100:.2f}/100\t avg_time: {avg_time}")
    print(f"Your score: {score*100:.2f}/100")
    # print(res_dict)
    yaml.dump(res_dict, open("result.yaml", "w"))
