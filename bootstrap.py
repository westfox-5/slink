import subprocess
import shlex
import sys
from os import listdir
from os.path import isfile, join

import numpy as np
import pandas as pd
from scipy.spatial.distance import squareform, pdist


def print_sep(text):
    print(f"######## {text} ########")

def compile(exe):
    cmd = f"g++ --std=c++17 src/main.cpp -o {exe}"
    print(f"{cmd}")
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()
    
def run_file(exe, inFile):
    cmd = f"{exe} {inFile}"
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()
    #print(f"Execution took {end} ms")

'''
def run_tests():
    print("START all tests...")
    testFiles = [f for f in listdir(testDir) if isfile(join(testDir, f))]
    for testFile in testFiles:
        run_file(join(testDir, testFile))
    print(f"END tests...")
'''

def create_distance_matrix(outFilePath, N = 2, min = 0.1, max = 5):
    #random.seed(1337)
    # m = [[round(random.uniform(min, max), 1) if r<c else 0 for c in range(0, N, 1)] for r in range(0, N, 1)]
    m = np.random.rand(N,N) * (max - min) + min
    m_symm = (m + m.T)/2
    m_symm = np.round(m_symm, 2)
    np.fill_diagonal(m_symm, 0)

    float_formatter = "{:.1f}".format
    np.set_printoptions(formatter={'float_kind':float_formatter}, threshold=sys.maxsize)

    with open(outFilePath, "w") as outFile:
        outFile.write(str(N) + '\n')
        outFile.write(' '.join(str(m_symm[i]) for i in range(0,N,1))
            .replace("\n", '')
            .replace("[", "")
            .replace("] ", "\n")
            .replace("]", "")
            #.replace(".", "")
            #.replace(",", "")
        )


def create_distance_matrix_from_csv(input, output = None, column=None, sep=','):
    df = pd.read_csv(input, usecols=[column], sep=sep)
    # print(df)

    df = pd.DataFrame(squareform(pdist(df), 'euclidean'), columns=df.index.values, index=df.index.values) 
    # print(df)

    if output == None:
        # print to stdout
        print(df)
    else:
        # print to file
        print(f"Saved distance matrix to {output}")
        with open(output, "w") as f:
            f.write(f"{df.shape[0]}")
            f.write("\n")
            np.savetxt(output, df.values, fmt="%.2f")


if __name__ == "__main__":
    EXE_NAME = "./cluster"

    '''  CREATES A RANDOM MATRIX
    N = 2
    file = join("./examples", f'{N}.in')    
    create_distance_matrix(file, N)
    '''

    inFilename = "examples/IrisDataset/iris.data"
    outFilename = "examples/IrisDataset/iris.dist"
    column = 'sepal_length'
    
    print_sep("CREATE MATRIX")
    create_distance_matrix_from_csv(inFilename, outFilename, column)

    print_sep("COMPILE")
    compile(EXE_NAME)
    
    print_sep("EXECUTE")
    run_file(EXE_NAME, outFilename)
