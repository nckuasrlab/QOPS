def generate_env():
    env_content = """\
# microbenchmark
TIMES=100
TOTAL_QUBIT=24
CHUNK_SIZE=18
CHUNK_START=10
TEST_SIZE=9
RUNNER_TYPE=MEM
# other setting
RUN_MICROBENCHMARK=0
RE_TRAIN=1
MODE=1
    """
    with open("./.env", "w") as env_file:
        env_file.write(env_content)
    print(".env file generated successfully")

if __name__ == "__main__":
    generate_env()
