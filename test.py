import glob
import subprocess
import sys

tests = sorted([x for x in glob.glob("isa/*") if not x.endswith(".dmp")])
# p = subprocess.run("./test")
for test in tests:
    print(test, end="", flush=True)
    # print("\r\n!!!!!!!"+test)
    # p = subprocess.run(["./test", test])
    # p = subprocess.run(["./run_test", test], timeout=2, capture_output=True)
    p = subprocess.run(["./build/main", test])
    if p.returncode != 0:
        print("...FAIL")
        break
    else:
        print("...OK")

