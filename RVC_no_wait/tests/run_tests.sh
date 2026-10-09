#!/bin/sh
# Linux/macOS: sh run_tests.sh
cd "$(dirname "$0")" || exit 1
gcc -std=c11 ../main.c ../rvc.c -o rvc_test || { echo "Build failed."; exit 1; }

pass=0; fail=0
for d in case*/; do
    d=${d%/}
    (cd "$d" && ../rvc_test > output.txt)
    if diff --strip-trailing-cr -q "$d/output.txt" "$d/expected.txt" > /dev/null; then
        echo "[PASS] $d"; pass=$((pass + 1))
    else
        echo "[FAIL] $d"; fail=$((fail + 1))
    fi
done
echo
echo "PASS: $pass  FAIL: $fail"
