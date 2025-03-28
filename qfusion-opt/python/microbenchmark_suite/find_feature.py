import argparse
import math
import os
import subprocess

import pandas as pd
from sklearn.feature_selection import SelectKBest, f_regression
from sklearn.preprocessing import StandardScaler


def set_ini_file(total_qbit, FILE_QUBIT, size, RUNNER_TYPE):
    f = open("./sub_cpu.ini", "w")
    if RUNNER_TYPE == "MEM":
        f.write(
            "[system]\ntotal_qbit="
            + str(total_qbit)
            + "\nfile_qbit="
            + str(FILE_QUBIT)
            + "\nchunk_qbit="
            + str(size)
            + "\nrunner_type=MEM\nis_subcircuit=1"
        )
    elif RUNNER_TYPE == "IO":
        f.write(
            "[system]\ntotal_qbit="
            + str(total_qbit)
            + "\nfile_qbit="
            + str(FILE_QUBIT)
            + "\nchunk_qbit="
            + str(size)
            + "\nrunner_type=IO\nis_subcircuit=1\nstate_paths=/mnt/nvme/card0/0/path0,/mnt/nvme/card0/0/path1,/mnt/nvme/card0/0/path2,/mnt/nvme/card0/0/path3,/mnt/nvme/card0/0/path4,/mnt/nvme/card0/0/path5,/mnt/nvme/card0/0/path6,/mnt/nvme/card0/0/path7,/mnt/nvme/card0/1/path0,/mnt/nvme/card0/1/path1,/mnt/nvme/card0/1/path2,/mnt/nvme/card0/1/path3,/mnt/nvme/card0/1/path4,/mnt/nvme/card0/1/path5,/mnt/nvme/card0/1/path6,/mnt/nvme/card0/1/path7,/mnt/nvme/card0/2/path0,/mnt/nvme/card0/2/path1,/mnt/nvme/card0/2/path2,/mnt/nvme/card0/2/path3,/mnt/nvme/card0/2/path4,/mnt/nvme/card0/2/path5,/mnt/nvme/card0/2/path6,/mnt/nvme/card0/2/path7,/mnt/nvme/card0/3/path0,/mnt/nvme/card0/3/path1,/mnt/nvme/card0/3/path2,/mnt/nvme/card0/3/path3,/mnt/nvme/card0/3/path4,/mnt/nvme/card0/3/path5,/mnt/nvme/card0/3/path6,/mnt/nvme/card0/3/path7,/mnt/nvme/card1/0/path0,/mnt/nvme/card1/0/path1,/mnt/nvme/card1/0/path2,/mnt/nvme/card1/0/path3,/mnt/nvme/card1/0/path4,/mnt/nvme/card1/0/path5,/mnt/nvme/card1/0/path6,/mnt/nvme/card1/0/path7,/mnt/nvme/card1/1/path0,/mnt/nvme/card1/1/path1,/mnt/nvme/card1/1/path2,/mnt/nvme/card1/1/path3,/mnt/nvme/card1/1/path4,/mnt/nvme/card1/1/path5,/mnt/nvme/card1/1/path6,/mnt/nvme/card1/1/path7,/mnt/nvme/card1/2/path0,/mnt/nvme/card1/2/path1,/mnt/nvme/card1/2/path2,/mnt/nvme/card1/2/path3,/mnt/nvme/card1/2/path4,/mnt/nvme/card1/2/path5,/mnt/nvme/card1/2/path6,/mnt/nvme/card1/2/path7,/mnt/nvme/card1/3/path0,/mnt/nvme/card1/3/path1,/mnt/nvme/card1/3/path2,/mnt/nvme/card1/3/path3,/mnt/nvme/card1/3/path4,/mnt/nvme/card1/3/path5,/mnt/nvme/card1/3/path6,/mnt/nvme/card1/3/path7\nis_directIO=0"
        )
    elif RUNNER_TYPE == "DirectIO":
        f.write(
            "[system]\ntotal_qbit="
            + str(total_qbit)
            + "\nfile_qbit="
            + str(FILE_QUBIT)
            + "\nchunk_qbit="
            + str(size)
            + "\nrunner_type=DirectIO\nis_subcircuit=1\nstate_paths=/mnt/nvme/card0/0/path0,/mnt/nvme/card0/0/path1,/mnt/nvme/card0/0/path2,/mnt/nvme/card0/0/path3,/mnt/nvme/card0/0/path4,/mnt/nvme/card0/0/path5,/mnt/nvme/card0/0/path6,/mnt/nvme/card0/0/path7,/mnt/nvme/card0/1/path0,/mnt/nvme/card0/1/path1,/mnt/nvme/card0/1/path2,/mnt/nvme/card0/1/path3,/mnt/nvme/card0/1/path4,/mnt/nvme/card0/1/path5,/mnt/nvme/card0/1/path6,/mnt/nvme/card0/1/path7,/mnt/nvme/card0/2/path0,/mnt/nvme/card0/2/path1,/mnt/nvme/card0/2/path2,/mnt/nvme/card0/2/path3,/mnt/nvme/card0/2/path4,/mnt/nvme/card0/2/path5,/mnt/nvme/card0/2/path6,/mnt/nvme/card0/2/path7,/mnt/nvme/card0/3/path0,/mnt/nvme/card0/3/path1,/mnt/nvme/card0/3/path2,/mnt/nvme/card0/3/path3,/mnt/nvme/card0/3/path4,/mnt/nvme/card0/3/path5,/mnt/nvme/card0/3/path6,/mnt/nvme/card0/3/path7,/mnt/nvme/card1/0/path0,/mnt/nvme/card1/0/path1,/mnt/nvme/card1/0/path2,/mnt/nvme/card1/0/path3,/mnt/nvme/card1/0/path4,/mnt/nvme/card1/0/path5,/mnt/nvme/card1/0/path6,/mnt/nvme/card1/0/path7,/mnt/nvme/card1/1/path0,/mnt/nvme/card1/1/path1,/mnt/nvme/card1/1/path2,/mnt/nvme/card1/1/path3,/mnt/nvme/card1/1/path4,/mnt/nvme/card1/1/path5,/mnt/nvme/card1/1/path6,/mnt/nvme/card1/1/path7,/mnt/nvme/card1/2/path0,/mnt/nvme/card1/2/path1,/mnt/nvme/card1/2/path2,/mnt/nvme/card1/2/path3,/mnt/nvme/card1/2/path4,/mnt/nvme/card1/2/path5,/mnt/nvme/card1/2/path6,/mnt/nvme/card1/2/path7,/mnt/nvme/card1/3/path0,/mnt/nvme/card1/3/path1,/mnt/nvme/card1/3/path2,/mnt/nvme/card1/3/path3,/mnt/nvme/card1/3/path4,/mnt/nvme/card1/3/path5,/mnt/nvme/card1/3/path6,/mnt/nvme/card1/3/path7\nis_directIO=1"
        )
    f.close()


def microbenchmark_one_qubit_gate(
    FILE_QUBIT,
    CHUNK_MAX,
    CHUNK_MIN,
    TIMES,
    TOTAL_QUBIT_MAX,
    TOTAL_QUBIT_MIN,
    RUNNER_TYPE,
):
    gate_list = ["H", "X", "RX", "RY", "RZ", "U1"]

    f_log = open("./log/feature_one_qubit.csv", "a")
    for gate in gate_list:
        for target in range(CHUNK_MAX):
            circuit_name = "./txt/" + gate + str(target) + ".txt"
            f = open(circuit_name, "w")
            f.write(str(TIMES) + "\n")
            for i in range(TIMES):
                if gate == "H" or gate == "X":
                    f.write(gate + " " + str(target) + "\n")
                elif gate == "U1":
                    f.write(
                        gate
                        + " "
                        + str(target)
                        + " 3.141592653589793 3.141592653589793 3.141592653589793\n"
                    )
                else:
                    f.write(gate + " " + str(target) + " 3.141592653589793\n")
            f.close()
        for total_qbit in range(TOTAL_QUBIT_MIN, TOTAL_QUBIT_MAX + 1):
            for size in range(CHUNK_MIN, CHUNK_MAX + 1):
                set_ini_file(total_qbit, FILE_QUBIT, size, RUNNER_TYPE)
                for target in range(size):
                    circuit_name = "./txt/" + gate + str(target) + ".txt"
                    output = subprocess.run(
                        ["./cpu/Quokka", "-i", "sub_cpu.ini", "-c", circuit_name],
                        capture_output=True,
                        text=True,
                    )
                    # ms
                    data = (
                        float(output.stdout.split("\n")[-2].split("s")[0])
                        / TIMES
                        * 1000
                    )
                    f_log.write(
                        gate
                        + ", "
                        + str(target)
                        + ", "
                        + str(total_qbit)
                        + ", "
                        + str(size)
                        + ", "
                        + str(data)
                        + "\n"
                    )
    f_log.close()


def microbenchmark_two_qubit_gate(
    FILE_QUBIT,
    CHUNK_MAX,
    CHUNK_MIN,
    TIMES,
    TOTAL_QUBIT_MAX,
    TOTAL_QUBIT_MIN,
    RUNNER_TYPE,
):
    gate_list = ["CX", "CZ", "CP", "RZZ", "U2"]

    f_log = open("./log/feature_two_qubit.csv", "a")
    for gate in gate_list:
        for control in range(CHUNK_MAX):
            for target in range(CHUNK_MAX):
                if control != target:
                    circuit_name = (
                        "./txt/"
                        + gate
                        + "_"
                        + str(control)
                        + "_"
                        + str(target)
                        + ".txt"
                    )
                    f = open(circuit_name, "w")
                    f.write(str(TIMES) + "\n")
                    if gate == "CX" or gate == "CZ":
                        for i in range(TIMES):
                            f.write(
                                gate + " " + str(control) + " " + str(target) + "\n"
                            )
                    elif gate == "U2":
                        for i in range(TIMES):
                            f.write(gate + " " + str(control) + " " + str(target))
                            for j in range(16):
                                f.write(" 3.141592653589793")
                            f.write("\n")
                    else:
                        for i in range(TIMES):
                            f.write(
                                gate
                                + " "
                                + str(control)
                                + " "
                                + str(target)
                                + " 3.141592653589793\n"
                            )
                    f.close()

        for total_qbit in range(TOTAL_QUBIT_MIN, TOTAL_QUBIT_MAX + 1):
            for size in range(CHUNK_MIN, CHUNK_MAX + 1):
                set_ini_file(total_qbit, FILE_QUBIT, size, RUNNER_TYPE)
                for control in range(size):
                    for target in range(size):
                        if target != control:
                            circuit_name = (
                                "./txt/"
                                + gate
                                + "_"
                                + str(control)
                                + "_"
                                + str(target)
                                + ".txt"
                            )
                            output = subprocess.run(
                                [
                                    "./cpu/Quokka",
                                    "-i",
                                    "sub_cpu.ini",
                                    "-c",
                                    circuit_name,
                                ],
                                capture_output=True,
                                text=True,
                            )
                            # ms
                            data = (
                                float(output.stdout.split("\n")[-2].split("s")[0])
                                / TIMES
                                * 1000
                            )
                            f_log.write(
                                gate
                                + ", "
                                + str(control)
                                + ", "
                                + str(target)
                                + ", "
                                + str(total_qbit)
                                + ", "
                                + str(size)
                                + ", "
                                + str(data)
                                + "\n"
                            )
    f_log.close()


def get_args():
    parser = argparse.ArgumentParser(
        description="Run microbenchmark Quokka",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "--simulator_binary",
        type=str,
        help="Simulator binary",
        metavar="PATH",
        default="./cpu/Quokka",
    )
    parser.add_argument(
        "--num_repeat_runs",
        type=int,
        help="Number of times to run the benchmark",
        metavar="NUM",
        default=100,
    )
    parser.add_argument(
        "--total_qubit_min",
        type=int,
        help="Total number of qubits minimum",
        metavar="NUM",
        default=24,
    )
    parser.add_argument(
        "--total_qubit_max",
        type=int,
        help="Total number of qubits maximum",
        metavar="NUM",
        default=32,
    )
    parser.add_argument(
        "--chunk_min",
        type=int,
        help="Chunk size minimum",
        metavar="NUM",
        default=10,
    )
    parser.add_argument(
        "--chunk_max",
        type=int,
        help="Chunk size maximum",
        metavar="NUM",
        default=18,
    )
    parser.add_argument(
        "--simulation_type",
        type=str,
        choices=["MEM", "IO", "DirectIO"],
        help="Runner type",
        default="MEM",
    )
    parser.add_argument(
        "--microbenchmark_result_file",
        type=str,
        help="Output microbenchmark result file",
        metavar="CSV_FILENAME",
        default="./log/microbenchmark_result_quokka.csv",
    )
    return parser.parse_args()


def main():
    args = get_args()
    print(args)

    TIMES = args.num_repeat_runs
    TOTAL_QUBIT_MIN = args.total_qubit_min
    TOTAL_QUBIT_MAX = args.total_qubit_max
    CHUNK_MAX = args.chunk_max
    CHUNK_MIN = args.chunk_min
    RUNNER_TYPE = args.simulation_type
    RUN_MICROBENCHMARK = 0  # FIXME: not checked
    """
    If `RUN_MICROBENCHMARK=1` in `.env` file, generate new test file and obtain the 
        execution result for feature selection.
    If `RUN_MICROBENCHMARK=0`, analyze according to previous execution result
        (`./log/feature_one_qubit.csv`, `./log/feature_two_qubit.csv`).
    """

    cpu_info = subprocess.run(
        ["lscpu | grep -E '^Core|^Socket'"], shell=True, capture_output=True, text=True
    ).stdout.split("\n")
    core_number = int(cpu_info[0].split(" ")[-1])
    socket_number = int(cpu_info[1].split(" ")[-1])
    FILE_QUBIT = int(math.log(core_number * socket_number, 2))

    if RUN_MICROBENCHMARK == "1":
        os.system("rm ./log/feature_one_qubit.csv")
        os.system("rm ./log/feature_two_qubit.csv")
        os.system("rm ./txt/*")
        microbenchmark_one_qubit_gate(
            FILE_QUBIT,
            CHUNK_MAX,
            CHUNK_MIN,
            TIMES,
            TOTAL_QUBIT_MAX,
            TOTAL_QUBIT_MIN,
            RUNNER_TYPE,
        )
        microbenchmark_two_qubit_gate(
            FILE_QUBIT,
            CHUNK_MAX,
            CHUNK_MIN,
            TIMES,
            TOTAL_QUBIT_MAX,
            TOTAL_QUBIT_MIN,
            RUNNER_TYPE,
        )

    # analyse one qubit gate
    print("one qubit gate:")
    column_names = [
        "gate_type",
        "target_qubit_one",
        "total_qubit",
        "CHUNK_MAX",
        "execution_time",
    ]
    df_microbenchmark = pd.read_csv("./log/feature_one_qubit.csv", names=column_names)

    # data preprocessing
    df_microbenchmark = pd.get_dummies(df_microbenchmark, columns=["gate_type"])
    x_data = df_microbenchmark.drop(labels=["execution_time"], axis=1)
    feature_name = list(x_data.columns)
    y_data = df_microbenchmark["execution_time"]
    sc = StandardScaler()
    x_data = sc.fit_transform(x_data)

    fs = SelectKBest(score_func=f_regression, k="all")
    fs.fit_transform(x_data, y_data)
    for i in range(len(feature_name)):
        print(feature_name[i], fs.scores_[i])

    # analyse two qubit gate
    print("\ntwo qubit gate:")
    column_names = [
        "gate_type",
        "target_qubit_one",
        "target_qubit_two",
        "total_qubit",
        "chunk_size",
        "execution_time",
    ]
    df_microbenchmark = pd.read_csv("./log/feature_two_qubit.csv", names=column_names)

    # data preprocessing
    df_microbenchmark = pd.get_dummies(df_microbenchmark, columns=["gate_type"])
    x_data = df_microbenchmark.drop(labels=["execution_time"], axis=1)
    feature_name = list(x_data.columns)
    y_data = df_microbenchmark["execution_time"]
    sc = StandardScaler()
    x_data = sc.fit_transform(x_data)

    fs = SelectKBest(score_func=f_regression, k="all")
    fs.fit_transform(x_data, y_data)
    for i in range(len(feature_name)):
        print(feature_name[i], fs.scores_[i])


if __name__ == "__main__":
    main()
