#include <stdio.h>
#include <inttypes.h>
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

// MMU Function
void  test_convertVirtualToPhysical( int virtualAddr, uint32_t *page_directory ) {
  //printf("Page Dir Address %p\n",page_directory);
  int page_offset = virtualAddr & 0x00000FFF;
  //printf("Page Offset      : %08X\n", page_offset);
  int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
  //printf("Page Table Offset: %08X\n", page_table_offset);
  int page_dir_offset = virtualAddr >> 22;
  //printf("Page Dir Offset  : %08X\n", page_dir_offset);


  // if page_table cannot be found, throw page fault
  if (!page_directory[page_dir_offset])
  {
      pageFault(virtualAddr);
  }
  uint32_t * page_table = (void *) page_directory[page_dir_offset];

  //printf ("Page Dir Entry   : %p\n", page_table);
  int page_entry = * ((page_table) + page_table_offset);
  //printf ("Page Table Entry : %p\n", page_entry);
}

/* create a test fixture */

void test_fixture_math( void )
{
        test_fixture_start();
        run_test(test_one);
        run_test(test_two);
        //run_test(test_convertVirtualToPhysical);
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
