import os, shutil
import subprocess
import hashlib
import yaml

def calculate_sha256(file_path):
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()

work_dir = os.path.dirname(__file__)
submit_path = os.path.join(work_dir, "xflops2024_submit")
short_submit_path = "xflops2024_submit"

with open(f"{work_dir}/submit.yaml", "r") as f:
    submit_files = yaml.safe_load(f)

question_dir_lst = list(submit_files.keys())

input_char_lst = [f"{ind}: {questioni}" for ind, questioni in enumerate(question_dir_lst)]
input_char_lst.append("-1: ALL")
input_char = "\n".join(input_char_lst)
choose = int(input(f"{input_char}\nchoose index to submit\n>>>"))

choose_ind_dic = {-1 : "xflops2024_submit"}
for ind, questioni in enumerate(question_dir_lst):
    choose_ind_dic[ind] = questioni

tar_dir_name = choose_ind_dic[choose]
if choose!= -1:
    question_dir_lst = [question_dir_lst[choose]]



shutil.rmtree(submit_path, ignore_errors=True)
if os.path.exists(f"{work_dir}/{tar_dir_name}.tar"):
    os.remove(f"{work_dir}/{tar_dir_name}.tar")
os.mkdir(submit_path)


for questioni in question_dir_lst:
    if questioni not in submit_files:
        continue
    os.mkdir(os.path.join(submit_path, questioni))

    file = "writeup.md"
    if not os.path.exists(os.path.join(work_dir, questioni, file)):
        print(f"WARNING: {questioni}'s file : {file} not found")
    else:
        shutil.copyfile(os.path.join(work_dir, questioni, file), os.path.join(submit_path, questioni, file))

    for file in submit_files[questioni]:
        file_name = file.split("/")[-1]
        if not os.path.exists(os.path.join(work_dir, questioni, file)):
            print(f"WARNING: {questioni}'s file : {file} not found")
            continue
        shutil.copyfile(os.path.join(work_dir, questioni, file), os.path.join(submit_path, questioni, file_name))

if choose!= -1:
    subprocess.run(["tar", "-cvf", f"{tar_dir_name}.tar", f"{short_submit_path}/{tar_dir_name}"], cwd=work_dir , stdout=subprocess.PIPE, stderr=subprocess.PIPE)
else:
    subprocess.run(["tar", "-cvf", f"{tar_dir_name}.tar", f"{short_submit_path}"], cwd=work_dir , stdout=subprocess.PIPE, stderr=subprocess.PIPE)

sha256_zip = calculate_sha256(f"{work_dir}/{tar_dir_name}.tar")

print(f"sha256({tar_dir_name}.tar) : {sha256_zip}")