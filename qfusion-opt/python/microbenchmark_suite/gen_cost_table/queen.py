import argparse
import os
import pickle

import pandas as pd
from python.common import gate_list_queen as gate_list
from sklearn.ensemble import RandomForestRegressor
from sklearn.preprocessing import StandardScaler


def add_shared_arguments(parser):
    parser.add_argument(
        "-i",
        "--model_folder",
        help="Folder path to model",
        metavar="PATH",
        default="./model/queen",
    )


def get_args():
    parser = argparse.ArgumentParser(
        description="Generate cost table Queen",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )

    subparsers = parser.add_subparsers(dest="mode", help="Available modes")

    # --- Subparser for the 'gen_table' mode ---
    gen_table_parser = subparsers.add_parser("gen_table", help="Generate cost table")
    add_shared_arguments(gen_table_parser)
    gen_table_parser.add_argument(
        "input_total_qubit", help="Total number of qubits", type=int
    )
    gen_table_parser.add_argument(
        "--input_chunk_size",
        help="Chunk size (default: best_chunk_size)",
        type=int,
        default=-1,
    )
    gen_table_parser.add_argument(
        "-o",
        "--output_file",
        help="Output file (default: %(default)s)",
        metavar="FILE",
        default="./log/gate_exe_time_queen.csv",
    )

    # --- Subparser for the 'predict' mode ---
    predict_parser = subparsers.add_parser("predict", help="Predict cost for a gate")
    add_shared_arguments(predict_parser)
    predict_parser.add_argument("input_gate_type", help="Gate type", choices=gate_list)
    predict_parser.add_argument(
        "input_total_qubit", help="Total number of qubits", type=int
    )
    predict_parser.add_argument(
        "--input_chunk_size",
        help="Chunk size (default: best_chunk_size)",
        type=int,
        default=-1,
    )
    return parser.parse_args()


def get_best_chunk_size(benchmark_filename: str, total_qubit: int):
    chunk_sizes = {}
    with open(benchmark_filename, "r") as f:
        for line in f.readlines():
            if not line.strip():
                continue
            parts = line.split(",")
            if int(parts[1]) != total_qubit:
                continue
            if parts[2] not in chunk_sizes:
                chunk_sizes[parts[2]] = float(parts[3])
            else:
                chunk_sizes[parts[2]] += float(parts[3])
    index_min = min(chunk_sizes, key=chunk_sizes.get)
    return index_min


def main():
    args = get_args()
    print(args)
    if args.input_chunk_size == -1:
        args.input_chunk_size = get_best_chunk_size(
            "./log/microbenchmark_result_queen.csv", args.input_total_qubit
        )
        print(f"Best chunk size: {args.input_chunk_size}")

    # load pretrained performance model
    with open(f"{args.model_folder}/gate_model.pkl", "rb") as f:
        model: RandomForestRegressor = pickle.load(f)
    with open(f"{args.model_folder}/scaler.pkl", "rb") as f:
        sc: StandardScaler = pickle.load(f)

    if args.mode == "gen_table":
        input_total_qubit = args.input_total_qubit
        input_chunk_size = args.input_chunk_size
        os.system(f"rm -f {args.output_file}")
        f_exe = open(f"{args.output_file}", "w")
        for gate in gate_list:
            df_input = pd.DataFrame(
                [[gate, input_total_qubit, input_chunk_size]],
                columns=["gate_type", "total_qubit", "chunk_size"],
            )
            df_input["gate_type"] = pd.Categorical(
                df_input["gate_type"], categories=gate_list
            )
            df_input = pd.get_dummies(df_input, columns=["gate_type"])
            df_input = sc.transform(df_input)
            f_exe.write(f"{gate},{input_chunk_size},{model.predict(df_input)[0]}\n")
        f_exe.close()

    elif args.mode == "predict":
        input_gate_type = args.input_gate_type
        input_total_qubit = args.input_total_qubit
        input_chunk_size = args.input_chunk_size
        df_input = pd.DataFrame(
            [[input_gate_type, input_total_qubit, input_chunk_size]],
            columns=["gate_type", "total_qubit", "chunk_size"],
        )
        df_input["gate_type"] = pd.Categorical(
            df_input["gate_type"], categories=gate_list
        )
        df_input = pd.get_dummies(df_input, columns=["gate_type"])
        df_input = sc.transform(df_input)
        print(model.predict(df_input)[0])


if __name__ == "__main__":
    main()
