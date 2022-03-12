import subprocess
import shlex
import sys
from os import listdir
from os.path import isfile, join

import numpy as np

testDir = './examples'
exe = './cluster'

def compile():
    cmd = f"./build.sh"
    print(f"Compilation start [{cmd}]")
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()
    print("Compilation ended.")
    print()
    
def run_file(inFile):
    cmd = f"{exe} {inFile}"
    print(f"Input file: `{inFile}`...")
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()
    #print(f"Execution took {end} ms")

def run_tests():
    print("START all tests...")
    testFiles = [f for f in listdir(testDir) if isfile(join(testDir, f))]
    for testFile in testFiles:
        run_file(join(testDir, testFile))
    print(f"END tests...")


def create_distance_matrix(outFilePath, N = 2, min = 0.1, max = 5):
    #random.seed(1337)
    # m = [[round(random.uniform(min, max), 1) if r<c else 0 for c in range(0, N, 1)] for r in range(0, N, 1)]
    m = np.random.rand(N,N) * (max - min) + min
    m_symm = (m + m.T)/2
    m_symm = np.round(m_symm, 2)
    np.fill_diagonal(m_symm, 0)

    float_formatter = "{:.1f}".format
    np.set_printoptions(formatter={'float_kind':float_formatter}, threshold=sys.maxsize)
    #print(m_symm)

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

if __name__=='__main__':
    N = 2
    file = join(testDir, f'{N}.in')
    
    #create_distance_matrix(file, N)

    #compile()
    #run_file(file)
