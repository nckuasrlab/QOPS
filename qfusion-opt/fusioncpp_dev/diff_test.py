import multiprocessing
import os
import subprocess
import sys
import time
from pathlib import Path

# Resume args
resume_file = sys.argv[1] if len(sys.argv) > 1 else None
resume_mode = int(sys.argv[2]) if len(sys.argv) > 2 else None
resume_fmq = int(sys.argv[3]) if len(sys.argv) > 3 else None

resuming = all([resume_file, resume_mode, resume_fmq])
found_resume_point = multiprocessing.Value("b", False)

mode = [3, 4, 5, 6, 7, 8]
fmq = [3, 5]
skip_patterns = ["28", "29", "30", "31", "qknn", "qnn", "qaoa", "vqc", "32"]
WORKER = 64
if "32" not in skip_patterns:
    WORKER = 2
# Compile once at the beginning
# subprocess.run(
#     ["make", "clean"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT
# )
subprocess.run(
    ["make"], check=True, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT
)

# Output directories
Path("./tmp/fusion").mkdir(parents=True, exist_ok=True)
Path("./tmp/xxx").mkdir(parents=True, exist_ok=True)


def should_skip(filename):
    return any(pattern in filename for pattern in skip_patterns)


def check_line_count_equal(file1, file2):
    with open(file1, "r") as f1, open(file2, "r") as f2:
        lines1 = sum(1 for _ in f1)
        lines2 = sum(1 for _ in f2)
    return lines1 == lines2


def run_eq_check(q, xxx_out_path, answer_path):
    eq_check_cmd = [
        "python",
        "python/circuits_equivalent.py",
        str(q),
        xxx_out_path,
        answer_path,
    ]
    eq_check = subprocess.run(
        eq_check_cmd, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT
    )
    if eq_check.returncode == 0:
        # Although the circuits differ, they are functionally equivalent.
        # This indicates the fusion is correct, with only a minor loss of precision.
        # Copy the file from xxxoutpath to answer_path to cache the result.
        # subprocess.run(["cp", xxx_out_path, answer_path])
        return True
    return False


def process_task(task):
    filename, o, i = task
    if should_skip(filename):
        # print(f"Skipping {filename} (matched skip pattern)")
        return None

    # Resume logic (shared memory safe check)
    global found_resume_point
    if resuming and not found_resume_point.value:
        if filename == resume_file and o == resume_mode and i == resume_fmq:
            found_resume_point.value = True
        else:
            return None

    answer_file = f"{o}_{i}_{filename}"
    answer_path = f"./tmp/fusion/{answer_file}"
    circuit_path = f"./circuit/{filename}"
    xxx_out_path = f"./tmp/xxx/{answer_file}.xxx"

    if "24" in filename:
        q = 24
    elif "32" in filename:
        q = 32

    cmd = ["./fusion", circuit_path, xxx_out_path, str(i), str(q), str(o)]
    # print("Running:", " ".join(cmd), end="", flush=True)

    env = os.environ.copy()
    env["DYNAMIC_COST_FILENAME"] = "./log/gate_exe_time_aer.csv"

    try:
        subprocess.run(
            cmd,
            check=True,
            env=env,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
    except subprocess.CalledProcessError as e:
        return (task, f"{answer_file} FAIL: ERROR (fusion crash)\n")

    try:
        result = subprocess.run(
            ["diff", "-wq", answer_path, xxx_out_path],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT,
        )
        if result.returncode == 0:
            return (task, f"PASS")
        else:
            msg = "diff failed"
            if check_line_count_equal(answer_path, xxx_out_path):
                msg += " (line count match)"
            else:
                msg += " (line count mismatch)"
            is_sim_equal = run_eq_check(q, xxx_out_path, answer_path)
            if is_sim_equal:
                msg += " (equivalent)"
                return (task, f"PASS ({msg})")
                # return (task, f"PASS (line count mismatch; please check equivalent)")
            # return (task, f"PASS (line count match; please check equivalent)")
            return (task, f"FAIL ({msg})")
    except Exception as e:
        return (task, f"{answer_file} FAIL: ERROR (diff failed): {e}")


def main():
    t_start = time.perf_counter()
    # Collect all tasks
    tasks = []
    for circ_file in Path("./circuit").glob("*.txt"):
        filename = circ_file.name
        for o in mode:
            for i in fmq:
                tasks.append((filename, o, i))

    # Run in parallel
    with multiprocessing.Pool(processes=WORKER) as pool:
        results = pool.map(process_task, tasks)

    # Print output in order
    ignored_tasks = []
    fail_tasks = []
    pass_tasks = []
    processed_tasks = []
    for result in results:
        if result is None:
            ignored_tasks.append(result)
            continue
        (filename, o, i), output = result
        # print(f"[{filename}, mode={o}, fmq={i}] => {output}")
        if "FAIL" in output:
            fail_tasks.append(result)
        if "PASS" in output:
            pass_tasks.append(result)
        processed_tasks.append(result)

    print(f"Pass count: {len(pass_tasks)}/{len(results)-len(ignored_tasks)}")
    print(f"Fail count: {len(fail_tasks)}")
    if len(pass_tasks) + len(fail_tasks) != len(results) - len(ignored_tasks):
        print(
            f"ERROR:Unexpected task count: {len(pass_tasks) + len(fail_tasks)} != {len(results)-len(ignored_tasks)}"
        )

    for result in fail_tasks:
        (filename, o, i), output = result
        print(f"[{filename}, mode={o}, fmq={i}] => {output}")

    for result in processed_tasks:
        (filename, o, i), output = result
        print(f"[{filename}, mode={o}, fmq={i}] => {output}")

    t_end = time.perf_counter()
    print(f"Total time: {t_end - t_start:.2f} seconds")


if __name__ == "__main__":
    main()
