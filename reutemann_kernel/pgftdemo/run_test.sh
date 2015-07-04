#!/bin/bash
testfiles=(copy_page.txt mapping.txt swap.txt writeback.txt 
segfault.txt)

#rm -rf tests/TestResult_Kernel/*

for i in ${testfiles[@]}
do
#./run_serial.sh < tests/Testfiles_Kernel/$i > 
tests/exspected_TestResult_Kernel/$i
./run_serial.sh < tests/Testfiles_Kernel/$i > tests/TestResult_Kernel/$i
done
diff -uqr tests/TestResult_Kernel/ tests/exspected_TestResult_Kernel/

