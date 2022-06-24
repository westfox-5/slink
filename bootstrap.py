from importlib.resources import path
import getopt
from logging import exception
import subprocess
import shlex
import sys
from os import makedirs
from os.path import isfile, join, dirname, normpath, basename
import numpy as np

VERBOSE = False
FORCE_RECREATE = False
FILE_TYPE = ""
MATRIX_TYPE = "linear"
NUM_THREADS = 1
EXE_NAME = "./slink"

LOG_PREFIX = " > "


def print_help():
    print("Usage: bootstrap.py [MODE] [MODE ARGS] [OPTIONS] [ARGS]")
    print("  MODE:")
    print("   -r --random <matrix dimension>            generate a distance matrix with random numbers")
    print("   -i --input <input file>                   compute the distance matrix for the specified file")
    print("  OPTIONS:")
    print("   -h --help                                 print this helper")
    print("   -v --verbose                              enable verbose mode")
    print("   -f --force                                force recreation of the distance matrix")
    print("   -n --num-threads <N>                      specify number of threads for the parallel execution. If not provided, sequential execution is performed")
    print("   -m --matrix-type <linear, col_major>      specify type of matrix to store data")


def print_header():
    print()
    print("   $$$$$$\  $$\       $$$$$$\ $$\   $$\ $$\   $$\ ")
    print("   $$  __$$\ $$ |      \_$$  _|$$$\  $$ |$$ | $$  |")
    print("   $$ /  \__|$$ |        $$ |  $$$$\ $$ |$$ |$$  / ")
    print("   \$$$$$$\  $$ |        $$ |  $$ $$\$$ |$$$$$  /  ")
    print("    \____$$\ $$ |        $$ |  $$ \$$$$ |$$  $$<   ")
    print("   $$\   $$ |$$ |        $$ |  $$ |\$$$ |$$ |\$$\  ")
    print("   \$$$$$$  |$$$$$$$$\ $$$$$$\ $$ | \$$ |$$ | \$$\ ")
    print("    \______/ \________|\______|\__|  \__|\__|  \__|")
    print()
    print(" A Single Linkage Hierarchical Clustering algorithm")
    print(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~")


def print_sep(text):
    print()
    print(f"{LOG_PREFIX}{text}")


def log(text):
    global VERBOSE

    if VERBOSE:
        print(text)


def compile() -> int:
    global EXE_NAME
    cmd = f"make"
    log(f"{LOG_PREFIX}{cmd}")
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()
    return process.returncode


def run_file(inFile):
    global EXE_NAME
    global NUM_THREADS
    global MATRIX_TYPE
    global FILE_TYPE

    # cmd = f"perf stat -d {EXE_NAME} {FILE_TYPE} {inFile} {NUM_THREADS}"
    cmd = f"{EXE_NAME} {FILE_TYPE} {inFile} {MATRIX_TYPE} {NUM_THREADS}"
    log(f"{LOG_PREFIX}{cmd}")
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()


def create_distance_matrix(outFilePath, N=2, min=0.1, max=5):
    global FORCE_RECREATE

    if (isfile(outFilePath) and not FORCE_RECREATE):
        log(f"file {outFilePath} already exists")
        return

    # random.seed(1337)
    # m = [[round(random.uniform(min, max), 1) if r<c else 0 for c in range(0, N, 1)] for r in range(0, N, 1)]
    m = np.random.rand(N, N) * (max - min) + min

    m_symm = (m + m.T)/2
    m_symm = np.round(m_symm, 2)
    np.fill_diagonal(m_symm, 0)

    log("created matrix")
    float_formatter = "{:.1f}".format
    np.set_printoptions(
        formatter={'float_kind': float_formatter}, threshold=sys.maxsize)

    log("start file writing")
    with open(outFilePath, "w") as outFile:
        outFile.write(str(N) + '\n')
        outFile.write(' '.join(str(m_symm[i]) for i in range(0, N, 1))
                      .replace("\n", '')
                      .replace("[", "")
                      .replace("] ", "\n")
                      .replace("]", "")
        )

def run_from_random(N):
    outFilepath = join('./', f'{N}.in')
    print_sep(f"CREATE MATRIX {outFilepath}")
    create_distance_matrix(outFilepath, N)

    compile_and_run_file(outFilepath)


def run_from_file(inFilepath):
    compile_and_run_file(inFilepath)


def compile_and_run_file(outFilepath):
    print_sep("COMPILE")
    return_code = compile()
    if return_code != 0:
        log(f"Compilation failed (return code: {return_code})")
        return

    print_sep("EXECUTE")
    run_file(outFilepath)


if __name__ == "__main__":
    print_header()

    input_type = None
    matrix_dim = None
    input_file = None
    column_identifier = None
    sep = ","

    options, remainder = None, None
    try:
        options, remainder = getopt.getopt(sys.argv[1:], 'r:i:m:n:vfh', ["random=", "input=", "matrix-type=", "num-threads=", "verbose", "force", "help"])
    except Exception:
        print_help()
        print_sep(f"ERROR: Invalid argument")
        exit(1)

    for opt, arg in options:
        if opt in ('-r', '--random'):
            FILE_TYPE = 'dist'
            input_type = "random"
            matrix_dim = arg
        elif opt in ('-i', '--input'):
            FILE_TYPE = 'csv'
            input_type = "file"
            input_file = arg
        elif opt in ('-m', '--matrix-type'):
            MATRIX_TYPE = arg
        elif opt in ('-n', '--num-threads'):
            NUM_THREADS = int(arg)
        elif opt in ('-f', '--force'):
            FORCE_RECREATE = True
        elif opt in ('-v', '--verbose'):
            VERBOSE = True
        elif opt in ('-h', '--help'):
            print_help()
            exit(0)
        else:
            print_help()
            print_sep(f"ERROR: Invalid argument {opt}")
            exit(1)

    if input_type == "random":
        run_from_random(int(matrix_dim))
    elif input_type == "file":
        run_from_file(input_file)
    else:
        print_help()
        print_sep(
            f"ERROR: Invalid input type {input_type}" if input_type != None else "Input type not provided.")
        exit(1)

# Datasets
# https://archive.ics.uci.edu/ml/datasets.php
