import yaml
import subprocess, os
import re

case_lst = ["-s", "-m", "-l"]
case2ind = {'-s':0 , '-m':1 , '-l':2}
fullmark_performance = [34, 38, 43]
zeromark_performance = [14, 16, 18]
score_ratio = [0.2, 0.3, 0.5]

class ExplicitDumper(yaml.SafeDumper):
    """
    A dumper that will never emit aliases.
    """
    def ignore_aliases(self, data):
        return True

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

if not check_clang():
    print(f"make sure you have run `module load bisheng/2.5.0`")
    exit(1)

py_path = os.path.dirname(__file__)
src_path = os.path.abspath(f"{py_path}/source_code/everybit")
print(f"test in {src_path}")
test = subprocess.run(["make","testquiet"], cwd=src_path, text=True, capture_output=True)
test_str_lst = test.stdout.split('\n')
print(f"check result: {test_str_lst[-2]}")

if "PASSED" in test_str_lst[-2]:
    res_dici = {'performance':0, 'zero_flag': 0, 'info': 'Accepted'}
    res_dic = {casei: dict(res_dici) for casei in case_lst}
    for casei in case_lst:
        result = test = subprocess.run(["./everybit",casei], cwd=src_path, text=True, capture_output=True)
        result_str_lst = result.stdout.split('\n')

        re_er = re.compile(r'Succesfully completed tier: (\d+)')
        matches = int(re_er.findall(result_str_lst[-3])[0])
        res_dic[casei]['performance'] = matches
        print(f"performance of {casei}: {matches}")
else:
    print(f"Wrong Answer!")
    res_dici = {'performance': -1, 'zero_flag': 1, 'info': 'Wrong Answer'}
    res_dic = {i:res_dici for i in case_lst}


with open(f'{py_path}/result.yaml',"w") as f:
    yaml.dump(res_dic, f, Dumper=ExplicitDumper)


case_score_lst = [0,0,0]
for k, v in res_dic.items():
    if not v['zero_flag']:
        ind = case2ind[k]
        case_score_lst[ind]= 100*(v['performance'] - zeromark_performance[ind])/(fullmark_performance[ind] - zeromark_performance[ind])
        case_score_lst[ind]=min(max(case_score_lst[ind],0),100)
print(f"------score--------")
total_score = 0
for casei in case_lst:
    scorei = case_score_lst[case2ind[casei]]
    print(f"{casei} : {scorei:.2f} /100")
    total_score += scorei* score_ratio[case2ind[casei]]
print(f"total score: {total_score:.2f} /100")

