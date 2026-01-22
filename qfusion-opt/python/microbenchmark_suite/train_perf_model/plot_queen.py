import argparse

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from python.common import gate_list_queen as gate_list
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_absolute_percentage_error
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler

# Random Seed for Reproducibility
random_seed = 0


def get_args():
    parser = argparse.ArgumentParser(
        description="Train performance model Queen",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-i",
        "--microbenchmark_result",
        help="Path to microbenchmark result",
        metavar="CSV_FILENAME",
        default="./log/microbenchmark_result_queen.csv",
    )
    parser.add_argument(
        "-o",
        "--model_folder",
        help="Folder path to output model",
        metavar="MODEL_FOLDER",
        default="./model/queen",
    )
    return parser.parse_args()


def main():
    args = get_args()
    print(args)

    # load microbenchmark result
    column_names = ["gate_type", "total_qubit", "chunk_size", "execution_time"]
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

    # 1. train model
    model = RandomForestRegressor(n_estimators=500, random_state=random_seed)
    model.fit(x_train, y_train)
    y_test_predict = model.predict(x_test)
    print(
        "model error rate: "
        + str(mean_absolute_percentage_error(y_test, y_test_predict))
    )

    # 2. Plot
    y_test_predict = model.predict(x_test)
    plt.figure(figsize=(3, 3))
    plt.scatter(y_test, y_test_predict, alpha=0.5, color="blue", label="Predictions")

    # Plot the perfect prediction line (y=x)
    min_val = min(y_test.min(), y_test_predict.min())
    max_val = max(y_test.max(), y_test_predict.max()) * 1.3
    plt.plot(
        [min_val, max_val],
        [min_val, max_val],
        color="red",
        linestyle="--",
        lw=2,
        label="Perfect Fit",
    )

    # Annotate the MAPE on the plot
    mape = mean_absolute_percentage_error(y_test, y_test_predict)
    plt.text(
        min_val + (max_val - min_val) * 0.6,
        max_val - (max_val - min_val) * 0.95,
        f"MAPE: {mape:.1%}",
        fontsize=10,
        bbox=dict(facecolor="white", alpha=0.5),
    )

    plt.xlabel("Actual Execution Time")
    plt.ylabel("Predicted Execution Time")
    # plt.title('Actual vs. Predicted Values')
    plt.legend()
    plt.grid(True)
    plt.savefig(f"{args.model_folder}/rf_actual_vs_pred_queen.pdf", bbox_inches="tight")
    plt.savefig(f"{args.model_folder}/rf_actual_vs_pred_queen.png", bbox_inches="tight")


if __name__ == "__main__":
    main()
