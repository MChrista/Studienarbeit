#!/bin/bash
testfiles=(copy_page.txt mapping.txt simple_replace_test.txt writeback.txt swap.txt segfault.txt extended_replace_test.txt statetest.txt)

rm tests/TestResult_Kernel/*

for i in ${testfiles[@]}
do
#./run_serial.sh < tests/Testfiles_Kernel/$i > tests/exspected_TestResult_Kernel/$i
./run_serial.sh < tests/Testfiles_Kernel/$i > tests/TestResult_Kernel/$i
done
diff -uqr tests/TestResult_Kernel/ tests/exspected_TestResult_Kernel/

