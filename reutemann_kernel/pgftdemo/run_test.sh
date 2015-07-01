#!/bin/bash
testfiles=(copy_page.txt kerneltest.txt)

rm -rf tests/TestResult_Kernel/*

for i in ${testfiles[@]}
do
./run_serial.sh < tests/Testfiles_Kernel/$i > tests/TestResult_Kernel/$i &
done

sleep 2
ps -ef | grep qemu | grep -v grep | awk '{print $2}' | xargs kill -9
sleep 1
diff -qr tests/TestResult_Kernel/ tests/exspected_TestResult_Kernel/

