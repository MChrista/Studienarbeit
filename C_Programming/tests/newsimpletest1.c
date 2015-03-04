/*
 * File:   newsimpletest1.c
 * Author: michael
 *
 * Created on Mar 4, 2015, 9:49:59 AM
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * Simple C Test Suite
 */

void test1() {
    printf("newsimpletest1 test 1\n");
}

void test2() {
    printf("newsimpletest1 test 2\n");
    printf("%%TEST_FAILED%% time=0 testname=test2 (newsimpletest1) message=error message sample\n");
}

int main(int argc, char** argv) {
    printf("%%SUITE_STARTING%% newsimpletest1\n");
    printf("%%SUITE_STARTED%%\n");

    printf("%%TEST_STARTED%% test1 (newsimpletest1)\n");
    test1();
    printf("%%TEST_FINISHED%% time=0 test1 (newsimpletest1) \n");

    printf("%%TEST_STARTED%% test2 (newsimpletest1)\n");
    test2();
    printf("%%TEST_FINISHED%% time=0 test2 (newsimpletest1) \n");

    printf("%%SUITE_FINISHED%% time=0\n");

    return (EXIT_SUCCESS);
}
