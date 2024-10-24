for i in range(0, 38):
    with open(f"./Hq{str(i)}.txt", "w") as f:
        f.write(f"1\n3 0 1 4 {i}\n")