import subprocess
import shlex
import datetime
import time
from os import listdir
from os.path import isfile, join

testDir = './examples'
exe = './cluster'

def compile():
    cmd = f"./build.sh"
    print(f"Compiling: `{cmd}`")
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()
    
def runTest(testFile):
    cmd = f"{exe} {testFile}"
    print(f"Test: `{testFile}`...", end='')
    start = datetime.datetime.now()
    process = subprocess.Popen(shlex.split(cmd))
    process.wait()
    end = (datetime.datetime.now() - start).microseconds / 10

    print(f" ({end} ms)")

def runAllTests():
    print("START all tests...")
    testFiles = [f for f in listdir(testDir) if isfile(join(testDir, f))]
    start = datetime.datetime.now()
    for testFile in testFiles:
        runTest(join(testDir, testFile))
    end = (datetime.datetime.now() - start).microseconds / 10
    print(f"END tests... ({end} ms)")


if __name__=='__main__':
    compile()
    runAllTests()    

