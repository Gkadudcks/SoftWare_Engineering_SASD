#!/bin/sh
# Linux/macOS: sh run_tests.sh
cd "$(dirname "$0")" || exit 1
gcc -std=c11 ../main.c ../rvc.c -o rvc_test || { echo "Build failed."; exit 1; }

pass=0; fail=0; log=all_outputs.txt; : > "$log"
for d in TC*/; do
    d=${d%/}
    (cd "$d" && ../rvc_test "$d" > output.txt && sed -E 's/ASYNC STOP @ [0-9]+ms/ASYNC STOP @ Xms/' output.txt > output_norm.txt)
    if diff --strip-trailing-cr -q "$d/output_norm.txt" "$d/expected.txt" > /dev/null; then
        r=PASS; pass=$((pass + 1))
    else
        r=FAIL; fail=$((fail + 1))
    fi
    { echo; echo "############################################################"; echo "# $d  -  $r"; echo "############################################################"; cat "$d/output.txt"; } | tee -a "$log"
done
echo; echo "============================================================"
echo "PASS: $pass  FAIL: $fail" | tee -a "$log"
echo "All outputs saved to all_outputs.txt"
