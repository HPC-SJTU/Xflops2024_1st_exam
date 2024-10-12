import subprocess
import os
import math
import yaml
import re
import argparse
import hashlib

parser = argparse.ArgumentParser()
parser.add_argument('-t', type=int, default=5, help='Number of times to run each case')
parser.add_argument('--no-warmup', action='store_true', help='Do not run warmup cases')
parser.add_argument('-c', type=int, default=0, help='question to evaluate')
args = parser.parse_args()

# Define case-specific information
tries = args.t  # Number of times to run each case
assert tries > 0, f"Number of tries must be greater than 0, but now it's {tries}"
baseline_times = [30, 360, 360]  # Baseline times for each case
full_score_times = [3. , 6., 20.]  # Times to achieve full score for each case
full_scores = [15, 20, 25]  # Full scores for each case
run_time_limit = [30, 360, 360]
omp_case_name = ['omp_1024', 'omp_4096', 'omp_8192']

with open("reference_answers.yaml","r",encoding='utf-8') as f:
    reference_answers = yaml.safe_load(f)
reference_answers = {k:set(v) for k,v in reference_answers.items()}


# Get the path to the source_code directory
cwd = os.path.dirname(__file__)  # Get current directory (where evaluate.py is located)
src_path = os.path.abspath(os.path.join(cwd, "source_code"))  # Full path to source_code directory

file_not_found_ele = {
    "info": "File not found",
    "performance": 0,
    "zero_flag": 1,
}
complier_err_ele =  {
    "info": "Compile Error",
    "performance": 0,
    "zero_flag": 1,
}
complier_err_dic = {i:complier_err_ele for i in omp_case_name}

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
        subprocess.run(["clang++", "--version"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return True
    except subprocess.CalledProcessError:
        # 如果clang命令执行失败，返回False
        return False
    except FileNotFoundError:
        # 如果找不到clang命令，返回False
        return False

# Function to compile the code using 'make omp'
def compile_code():
    with open(f"{src_path}/Makeflag","r") as f:
        flag_line = f.read().split('\n')[0]

    with open(f"{src_path}/Makefile","r") as f:
        makefile_content = f.read()

    pattern = re.compile(r'^(CFLAGS\s*=\s*)(.*)$', re.MULTILINE)
    new_makefile_content = pattern.sub(flag_line, makefile_content)

    with open(f"{src_path}/Makefile","w") as f:
        f.write(new_makefile_content)

    try:
        # Run `make omp` in the source_code directory
        cmp = subprocess.run(["make", "omp"], cwd=src_path, text=True, capture_output=True)
        assert cmp.returncode == 0, "Compilation Error"
        print("Compilation successful.")
    except subprocess.CalledProcessError as e:
        raise AssertionError(f"Compilation failed: {e}")

# Function to run the simulation and return execution time
def run_simulation(case, input_file : str):
    try:
        # Navigate to the source_code directory and run the simulation
        run_1 = subprocess.run([f"./nbody_simulator_omp", input_file], cwd=src_path, text=True, capture_output=True, timeout=run_time_limit[case-1])
    except subprocess.TimeoutExpired:
        raise AssertionError("Time Limit Exceeded")
    assert run_1.returncode == 0, "Runtime Error"
    
    # Extract time from the output (assuming it prints "total simulation ends in %f sec")
    for line in run_1.stdout.split('\n'):
        if "total simulation ends in" in line:
            time = float(line.split("total simulation ends in ")[-1].replace(' sec', ''))
            break
    print(f"Case {omp_case_name[case-1]} use {input_file} cost: {time:.3f}s")
    return time

# Function to compute the score based on the logarithmic scale
def compute_score(case, avg_time):
    baseline = baseline_times[case-1]
    full_time = full_score_times[case-1]
    full_score = full_scores[case-1]

    if avg_time <= full_time:
        return full_score
    if avg_time >= baseline:
        return 0
    
    # Logarithmic interpolation
    score = full_score * (1 - (math.log(avg_time) - math.log(full_time)) / (math.log(baseline) - math.log(full_time)))
    return max(0, min(full_score, score))

def get_ref_path(case, tries):
    return f"ref_data/{omp_case_name[case-1].split('_')[-1]}_{tries+1}.ref"

# function to evaluate 1
def eval_1() -> tuple[float, dict[str, dict]]:
    print("="*20 + "begin to evaluate question 1" + "="*20)
    if not check_clang():
        print(f"make sure you have run `module load bisheng/2.5.0`")
        exit(1)
    check_cores()
    # Compile the code before running simulations
    try:
        compile_code()
    except AssertionError as e:
        print(e)
        return 0, complier_err_dic

    # Create a result dictionary for each case
    res_dict = {
        i: {
            "info": "Accepted",
            "performance": 0,
            "zero_flag": 0,
        } for i in omp_case_name
    }
    
    # Loop over all cases (1: 1024 particles, 2: 4096 particles, 3: 8192 particles)
    for case in range(1, 4):
        print(f"Running case {case}/3")
        try:
            # Run the simulation multiple times (tries) and calculate the average time
            if not args.no_warmup:
                run_simulation(case, get_ref_path(case, 0) )
            time_list = [run_simulation(case, get_ref_path(case, i) ) for i in range(tries)]
            avg_time = sum(time_list) / len(time_list)
            print(f"Case {case} avg_time = {avg_time:.3f} s")
            res_dict[omp_case_name[case -1]]["performance"] = avg_time
        except AssertionError as e:
            print(f"Running case {case} failed: {e}")
            res_dict[omp_case_name[case -1]]["info"] = e.args[0]
            res_dict[omp_case_name[case -1]]["zero_flag"] = 1
            continue
    
    # Calculate the total score
    total_score = 0
    for case in range(1, 4):
        if res_dict[omp_case_name[case -1]]["zero_flag"]:
            continue
        
        avg_time = res_dict[omp_case_name[case -1]]["performance"]
        case_score = compute_score(case, avg_time)
        total_score += case_score
        print(f"Case {case} score: {case_score:.2f}/{full_scores[case-1]}\t avg_time: {avg_time:.2f}s")
    print("="*20 + "finish to evaluate question 1" + "="*20)
    
    return total_score, res_dict
    

# calculate sha3_512 hash of a string
def sha3_512(char: str):
    hash_object = hashlib.sha3_512()
    hash_object.update(char.encode('utf-8'))
    return hash_object.hexdigest()

# Function to normalize code, specifically handling known multiplication patterns and removing the & symbol
def normalize_code(code_line):
    # Remove all whitespace
    code_line = re.sub(r"\s+", "", code_line)

    # Remove & from particles or local_particles
    code_line = re.sub(r"&(?=\bparticles\b|\blocal_particles\b)", "", code_line)

    # Handle specific cases like local_n * sizeof(Particle) vs sizeof(Particle) * local_n
    # and N * sizeof(Particle) vs sizeof(Particle) * N
    patterns_to_normalize = [
        r"local_n\*sizeof\(Particle\)",
        r"sizeof\(Particle\)\*local_n",
        r"N\*sizeof\(Particle\)",
        r"sizeof\(Particle\)\*N"
    ]

    for pattern in patterns_to_normalize:
        if re.search(pattern, code_line):
            # Normalize both patterns to a consistent form
            code_line = re.sub(pattern, "local_n*sizeof(Particle)" if "local_n" in pattern else "N*sizeof(Particle)", code_line)
    
    # Handle general multiplication, reorder terms lexicographically
    code_line = reorder_multiplications(code_line)
    
    return code_line

# Helper function to reorder multiplication terms lexicographically
def reorder_multiplications(code_line):
    # Find all multiplication patterns
    pattern = r"(\w+\s*\*\s*\w+)"
    matches = re.findall(pattern, code_line)

    for match in matches:
        # Split the multiplication terms and reorder them lexicographically
        terms = sorted(re.split(r"\*", match))
        reordered = "*".join(terms)
        code_line = code_line.replace(match, reordered)
    
    return code_line

def compare_lines(expected: set[str], actual: list[str]):
    actual_set = set()
    for linei in actual:
        linei = normalize_code(linei)
        actual_set.add(sha3_512(linei))

    return expected == actual_set

# Function to check the correctness of the my_mpi_comm.txt content
def evaluate_mpi_code(file_path):
    with open(file_path, 'r') as f:
        user_code_lines = f.readlines()

    # Initializing score and matching criteria
    score = 0

    # Assign weights to each section
    scoring_weights = {
        "TODO: 广播粒子数据到所有进程": 10,
        "TODO: 同步所有进程模拟的信息": 15,
        "TODO: 归约所有进程计算的局部动量分量和能量（4次归约操作，到0号进程）": 15
    }

    # Reading the user's answers from my_mpi_comm.txt
    user_answers = {}
    current_todo = None
    for line in user_code_lines:
        line = line.strip()
        if line.startswith("TODO"):
            current_todo = line
            user_answers[current_todo] = []
        elif current_todo and line:
            user_answers[current_todo].append(line)
    
    # Compare each TODO section
    for todo, ref_answer in reference_answers.items():
        if todo in user_answers:
            user_lines = user_answers[todo]
            
            if compare_lines(ref_answer, user_lines):
                score += scoring_weights[todo]  # Full points for this TODO
            else:
                print(f"Wrong answer in {todo}")
        else:
            print(f"Missing answer for {todo}")

    return score

def get_score_dic(score):
    return {'mpi' :{
        "info": "Accepted",
        "performance": score,
        "zero_flag": 0,
    }}

def eval_2() -> tuple[float, dict[str, dict]]:

    file_path = os.path.join(cwd, "source_code", "my_mpi_comm.txt")
    if os.path.exists(file_path):
        score = evaluate_mpi_code(file_path)
        return score, get_score_dic(score) 
    else:
        print(f"Error: {file_path} not found.")
        return 0, {'mpi' : file_not_found_ele}
    
if __name__ == "__main__":
    score_1 = None
    score_2 = None
    dic_1 = None
    dic_2 = None
    if args.c == 0:
        score_1, dic_1 = eval_1()
        score_2, dic_2 = eval_2()
        print(f"question 1 score : {score_1:.2f}/60")
        print(f"question 2 score : {score_2:.2f}/40")
        print(f"total score : {(score_1 + score_2):.2f}/100")
        dic_1.update(dic_2)
        with open(os.path.join(cwd, "result.yaml"), "w") as result_file:
            yaml.dump(dic_1, result_file)
        
    elif args.c == 1:
        score_1, dic_1 = eval_1()
        print(f"question 1 score : {score_1:.2f}/60")
        with open(os.path.join(cwd, "result.yaml"), "w") as result_file:
            yaml.dump(dic_1, result_file)
    elif args.c == 2:
        score_2, dic_2 = eval_2()
        print(f"question 2 score : {score_2:.2f}/40")
        with open(os.path.join(cwd, "result.yaml"), "w") as result_file:
            yaml.dump(dic_2, result_file)
    

    
