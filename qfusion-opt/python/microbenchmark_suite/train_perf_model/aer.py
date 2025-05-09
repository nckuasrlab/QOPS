import argparse
import os
import pickle

import pandas as pd
from python.common import gate_list_aer as gate_list
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_percentage_error
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

# Random Seed for Reproducibility
random_seed = 0


def get_args():
    parser = argparse.ArgumentParser(
        description="Train performance model Aer",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-i",
        "--microbenchmark_result",
        help="Path to microbenchmark result",
        metavar="CSV_FILENAME",
        default="./log/microbenchmark_result_aer.csv",
    )
    parser.add_argument(
        "-o",
        "--model_folder",
        help="Folder path to output model",
        metavar="MODEL_FOLDER",
        default="./model/aer",
    )
    return parser.parse_args()


def main():
    args = get_args()
    print(args)

    # load microbenchmark result
    column_names = ["gate_type", "total_qubit", "target_qubit", "execution_time"]
    df_microbenchmark = pd.read_csv(args.microbenchmark_result, names=column_names)

    # data preprocessing
    df_microbenchmark["gate_type"] = pd.Categorical(
        df_microbenchmark["gate_type"], categories=gate_list
    )
    df_microbenchmark = pd.get_dummies(df_microbenchmark, columns=["gate_type"])
    x = df_microbenchmark.drop(labels=["execution_time"], axis=1)
    y = df_microbenchmark["execution_time"]
    x_train, x_test, y_train, y_test = train_test_split(
        x, y, test_size=0.2, random_state=random_seed
    )
    sc = StandardScaler()
    x_train = sc.fit_transform(x_train)
    x_test = sc.transform(x_test)

    # train model
    model = RandomForestRegressor(n_estimators=500, random_state=random_seed)
    model.fit(x_train, y_train)
    y_test_predict = model.predict(x_test)
    print(
        "model error rate: "
        + str(mean_absolute_percentage_error(y_test, y_test_predict))
    )

    # store model and scaler
    os.makedirs(args.model_folder, exist_ok=True)
    with open(f"{args.model_folder}/gate_model.pkl", "wb") as f:
        pickle.dump(model, f)
    with open(f"{args.model_folder}/scaler.pkl", "wb") as f:
        pickle.dump(sc, f)


if __name__ == "__main__":
    main()
