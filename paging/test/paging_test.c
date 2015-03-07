#include <stdio.h>
#include "seatest.h"
#include "paging.h"

/* HOW TO ADD TESTS:
1) add a new test function:
  void test_<NAME>() { }
2) add test function to fixture
3) make test in root directory
4) ./build/test/paging_test to run tests
*/

/* Test the add function of paging.c*/
void test_one()
{
        assert_int_equal( 1, 1);
}

void test_two()
{
        assert_int_equal( 2, 2);
}

/* create a test fixture */

void test_fixture_math( void )
{
        test_fixture_start();
        run_test(test_one);
        run_test(test_two);
        test_fixture_end();
}

/* create a test suite */
void all_tests( void )
{
        test_fixture_math();
}


/* run the test suite */
int main( int argc, char** argv )
{
        return run_tests(all_tests);
}
