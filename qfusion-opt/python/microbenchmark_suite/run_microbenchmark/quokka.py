import argparse
import math
import os
import random
import subprocess

from python.common import gate_list_quokka as gate_list

# Random Seed for Reproducibility
random.seed(0)


def set_ini_file(total_qbit, num_file_qubit, size, simulation_type):
    f = open("sub_cpu.ini", "w")
    if simulation_type == "MEM":
        f.write(
            "[system]\ntotal_qbit="
            + str(total_qbit)
            + "\nfile_qbit="
            + str(num_file_qubit)
            + "\nchunk_qbit="
            + str(size)
            + "\nrunner_type=MEM\nis_subcircuit=1"
        )
    elif simulation_type == "IO":
        f.write(
            "[system]\ntotal_qbit="
            + str(total_qbit)
            + "\nfile_qbit="
            + str(num_file_qubit)
            + "\nchunk_qbit="
            + str(size)
            + "\nrunner_type=IO\nis_subcircuit=1\nstate_paths=/mnt/nvme/card0/0/path0,/mnt/nvme/card0/0/path1,/mnt/nvme/card0/0/path2,/mnt/nvme/card0/0/path3,/mnt/nvme/card0/0/path4,/mnt/nvme/card0/0/path5,/mnt/nvme/card0/0/path6,/mnt/nvme/card0/0/path7,/mnt/nvme/card0/1/path0,/mnt/nvme/card0/1/path1,/mnt/nvme/card0/1/path2,/mnt/nvme/card0/1/path3,/mnt/nvme/card0/1/path4,/mnt/nvme/card0/1/path5,/mnt/nvme/card0/1/path6,/mnt/nvme/card0/1/path7,/mnt/nvme/card0/2/path0,/mnt/nvme/card0/2/path1,/mnt/nvme/card0/2/path2,/mnt/nvme/card0/2/path3,/mnt/nvme/card0/2/path4,/mnt/nvme/card0/2/path5,/mnt/nvme/card0/2/path6,/mnt/nvme/card0/2/path7,/mnt/nvme/card0/3/path0,/mnt/nvme/card0/3/path1,/mnt/nvme/card0/3/path2,/mnt/nvme/card0/3/path3,/mnt/nvme/card0/3/path4,/mnt/nvme/card0/3/path5,/mnt/nvme/card0/3/path6,/mnt/nvme/card0/3/path7,/mnt/nvme/card1/0/path0,/mnt/nvme/card1/0/path1,/mnt/nvme/card1/0/path2,/mnt/nvme/card1/0/path3,/mnt/nvme/card1/0/path4,/mnt/nvme/card1/0/path5,/mnt/nvme/card1/0/path6,/mnt/nvme/card1/0/path7,/mnt/nvme/card1/1/path0,/mnt/nvme/card1/1/path1,/mnt/nvme/card1/1/path2,/mnt/nvme/card1/1/path3,/mnt/nvme/card1/1/path4,/mnt/nvme/card1/1/path5,/mnt/nvme/card1/1/path6,/mnt/nvme/card1/1/path7,/mnt/nvme/card1/2/path0,/mnt/nvme/card1/2/path1,/mnt/nvme/card1/2/path2,/mnt/nvme/card1/2/path3,/mnt/nvme/card1/2/path4,/mnt/nvme/card1/2/path5,/mnt/nvme/card1/2/path6,/mnt/nvme/card1/2/path7,/mnt/nvme/card1/3/path0,/mnt/nvme/card1/3/path1,/mnt/nvme/card1/3/path2,/mnt/nvme/card1/3/path3,/mnt/nvme/card1/3/path4,/mnt/nvme/card1/3/path5,/mnt/nvme/card1/3/path6,/mnt/nvme/card1/3/path7\nis_directIO=0"
        )
    elif simulation_type == "DirectIO":
        f.write(
            "[system]\ntotal_qbit="
            + str(total_qbit)
            + "\nfile_qbit="
            + str(num_file_qubit)
            + "\nchunk_qbit="
            + str(size)
            + "\nrunner_type=DirectIO\nis_subcircuit=1\nstate_paths=/mnt/nvme/card0/0/path0,/mnt/nvme/card0/0/path1,/mnt/nvme/card0/0/path2,/mnt/nvme/card0/0/path3,/mnt/nvme/card0/0/path4,/mnt/nvme/card0/0/path5,/mnt/nvme/card0/0/path6,/mnt/nvme/card0/0/path7,/mnt/nvme/card0/1/path0,/mnt/nvme/card0/1/path1,/mnt/nvme/card0/1/path2,/mnt/nvme/card0/1/path3,/mnt/nvme/card0/1/path4,/mnt/nvme/card0/1/path5,/mnt/nvme/card0/1/path6,/mnt/nvme/card0/1/path7,/mnt/nvme/card0/2/path0,/mnt/nvme/card0/2/path1,/mnt/nvme/card0/2/path2,/mnt/nvme/card0/2/path3,/mnt/nvme/card0/2/path4,/mnt/nvme/card0/2/path5,/mnt/nvme/card0/2/path6,/mnt/nvme/card0/2/path7,/mnt/nvme/card0/3/path0,/mnt/nvme/card0/3/path1,/mnt/nvme/card0/3/path2,/mnt/nvme/card0/3/path3,/mnt/nvme/card0/3/path4,/mnt/nvme/card0/3/path5,/mnt/nvme/card0/3/path6,/mnt/nvme/card0/3/path7,/mnt/nvme/card1/0/path0,/mnt/nvme/card1/0/path1,/mnt/nvme/card1/0/path2,/mnt/nvme/card1/0/path3,/mnt/nvme/card1/0/path4,/mnt/nvme/card1/0/path5,/mnt/nvme/card1/0/path6,/mnt/nvme/card1/0/path7,/mnt/nvme/card1/1/path0,/mnt/nvme/card1/1/path1,/mnt/nvme/card1/1/path2,/mnt/nvme/card1/1/path3,/mnt/nvme/card1/1/path4,/mnt/nvme/card1/1/path5,/mnt/nvme/card1/1/path6,/mnt/nvme/card1/1/path7,/mnt/nvme/card1/2/path0,/mnt/nvme/card1/2/path1,/mnt/nvme/card1/2/path2,/mnt/nvme/card1/2/path3,/mnt/nvme/card1/2/path4,/mnt/nvme/card1/2/path5,/mnt/nvme/card1/2/path6,/mnt/nvme/card1/2/path7,/mnt/nvme/card1/3/path0,/mnt/nvme/card1/3/path1,/mnt/nvme/card1/3/path2,/mnt/nvme/card1/3/path3,/mnt/nvme/card1/3/path4,/mnt/nvme/card1/3/path5,/mnt/nvme/card1/3/path6,/mnt/nvme/card1/3/path7\nis_directIO=1"
        )
    f.close()


def gen_microbenchmark(
    simulator_binary,
    tmp_circuit_dir,
    num_repeat_runs,
    total_qubit_min,
    total_qubit_max,
    chunk_min,
    chunk_max,
    simulation_type,
    num_file_qubit,
    gate_list,
    microbenchmark_result_file,
):
    f_log = open(microbenchmark_result_file, "a")
    for gate in gate_list:
        for total_qbit in range(total_qubit_min, total_qubit_max + 1):
            for size in range(chunk_min, chunk_max + 1):
                circuit_name = f"{tmp_circuit_dir}/" + gate + ".txt"
                f = open(circuit_name, "w")
                f.write(str(num_repeat_runs) + "\n")
                target = random.sample(range(size), 3)
                target.sort()
                if gate == "H" or gate == "X":
                    for i in range(num_repeat_runs):
                        f.write(gate + " " + str(target[0]) + "\n")
                elif gate == "RX" or gate == "RY" or gate == "RZ":
                    for i in range(num_repeat_runs):
                        f.write(gate + " " + str(target[0]) + " 3.141592653589793\n")
                elif gate == "U1":
                    for i in range(num_repeat_runs):
                        f.write(
                            gate
                            + " "
                            + str(target[0])
                            + " 3.141592653589793 3.141592653589793 3.141592653589793\n"
                        )
                elif gate == "CX" or gate == "CZ":
                    for i in range(num_repeat_runs):
                        f.write(
                            gate + " " + str(target[0]) + " " + str(target[1]) + "\n"
                        )
                elif gate == "CP" or gate == "RZZ":
                    for i in range(num_repeat_runs):
                        f.write(
                            gate
                            + " "
                            + str(target[0])
                            + " "
                            + str(target[1])
                            + " 3.141592653589793\n"
                        )
                elif gate == "U2":
                    for i in range(num_repeat_runs):
                        f.write(gate + " " + str(target[0]) + " " + str(target[1]))
                        for j in range(16):
                            f.write(" 3.141592653589793")
                        f.write("\n")
                elif gate == "U3":
                    for i in range(num_repeat_runs):
                        f.write(
                            gate
                            + " "
                            + str(target[0])
                            + " "
                            + str(target[1])
                            + " "
                            + str(target[2])
                        )
                        for j in range(64):
                            f.write(" 3.141592653589793")
                        f.write("\n")
                f.close()

                set_ini_file(total_qbit, num_file_qubit, size, simulation_type)
                output = subprocess.run(
                    [simulator_binary, "-i", "sub_cpu.ini", "-c", circuit_name],
                    capture_output=True,
                    text=True,
                )
                # ms
                data = (
                    float(output.stdout.split("\n")[-2].split("s")[0])
                    / num_repeat_runs
                    * 1000
                )
                f_log.write(
                    gate
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

    os.system(f"rm -f {args.microbenchmark_result_file}")
    tmp_circuit_dir = "./tmp/microbenchmark_quokka"
    os.makedirs(tmp_circuit_dir, exist_ok=True)
    os.system(f"rm -f {tmp_circuit_dir}/*")
    cpu_info = subprocess.run(
        ["lscpu | grep -E '^Core|^Socket'"], shell=True, capture_output=True, text=True
    ).stdout.split("\n")
    core_number = int(cpu_info[0].split()[-1])
    socket_number = int(cpu_info[1].split()[-1])
    num_file_qubit = int(math.log(core_number * socket_number, 2))

    gen_microbenchmark(
        args.simulator_binary,
        tmp_circuit_dir,
        args.num_repeat_runs,
        args.total_qubit_min,
        args.total_qubit_max,
        args.chunk_min,
        args.chunk_max,
        args.simulation_type,
        num_file_qubit,
        gate_list,
        args.microbenchmark_result_file,
    )


if __name__ == "__main__":
    main()
