# Copyright (c) 2014 Phil Mansfield
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

from __future__ import print_function

import math
import os
import subprocess
import sys
import time


ESTIMATE_FRACTION = 0.1
MAX_ESTIMATE_ITERS = 30

MIN_INT_WIDTH = 15
MIN_FLOAT_WIDTH = 15

def hasTail(fileName, tail):
    return tail == fileName[len(fileName) - len(tail):]

def isTest(fileName):
    return hasTail(fileName, "_test")

def isBenchmark(fileName):
    return hasTail(fileName, "_bench")

def getAllTests(testDir):
    return [f for f in os.listdir(testDir) if
            os.path.isfile(os.path.join(testDir, f)) and 
            isTest(f)]

def getAllBenchmarks(benchDir):
    return [f for f in os.listdir(benchDir) if
            os.path.isfile(os.path.join(benchDir, f)) and 
            isBenchmark(f)]

def runAllTests(testDir):
    tests = getAllTests(testDir)
    failSum = 0
    if len(tests) == 0: return

    col0Name = "Test Name"
    col1Name = "Passed"
    col2Name = "Failed"
    col0Width = max(max(map(len, tests)), len(col0Name))
    col1Width = len(col1Name)
    col2Width = len(col2Name)

    print("%*s %*s %*s" % (col0Width, col0Name,
                           col1Width, col1Name,
                           col2Width, col2Name))

    for test in tests:
        print("%*s" % (col0Width, test), end=" ")
        if subprocess.call(["%s" % os.path.join(testDir, test)]) == 0:
            print("%*s %*s" % (col1Width, "X", col2Width, ""))
        else:
            failSum += 1
            print("%*s %*s" % (col1Width, "", col2Width, "X"))
    if failSum == 0:
        print("All tests passed!")
    else:
        print("Failed %d/%d tests." % (failSum, len(tests)))
        

def runAllBenchmarks(benchDir):
    benches = getAllBenchmarks(benchDir)
    if len(benches) == 0: return

    for i, bench in enumerate(benches):
        if i > 0: print()
        print("%s:" % bench)
        subprocess.call(["%s" % os.path.join(benchDir, bench)])

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: '$ python runTests.py <input-dir> [test | bench]")
        sys.exit(1)

    inDir = sys.argv[1]
    testType = "test" if len(sys.argv) == 2 else sys.argv[2]

    if testType == "test":
        print("Running all tests...")
        runAllTests(inDir)
    elif testType == "bench":
        print("Running all benchmarks...")
        runAllBenchmarks(inDir)
