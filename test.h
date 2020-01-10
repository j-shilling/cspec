#ifdef __TEST_H__
#error "The test header has been included more than once."
#endif
#define __TEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// Generates a function that appends and element to a linked list
#define __create_register_function(function_name, node_type, list_prefix)      \
  static inline void function_name(node_type *node) {                          \
    if (list_prefix##_tail) {                                                  \
      list_prefix##_tail->next = node;                                         \
      list_prefix##_tail = node;                                               \
    } else {                                                                   \
      list_prefix##_head = node;                                               \
      list_prefix##_tail = node;                                               \
    }                                                                          \
  }

#define __xconcat(x, y) x##y
#define __concat(x, y) __xconcat(x, y)
#define __new_symb(x) __concat(x, __COUNTER__)

/*****************************************************************************/
/*                                 Test Case                                 */
/*****************************************************************************/
typedef struct _cspec_test_case {
  const char *desc;

  int (*run)(void);

  int status;

  struct _cspec_test_case *next;
} CSpecTestCase;

/*****************************************************************************/
/*                                Test Suite                                 */
/*****************************************************************************/

typedef struct _cspec_test_suite {
  const char *desc;

  CSpecTestCase *tests_head;
  CSpecTestCase *tests_tail;

  struct _cspec_test_suite *next;
} CSpecTestSuite;

static CSpecTestSuite cspec_global_tests = {
    .desc = 0, .tests_head = 0, .tests_tail = 0, .next = 0};

static CSpecTestSuite *cspec_test_suites_head = &cspec_global_tests;
static CSpecTestSuite *cspec_test_suites_tail = &cspec_global_tests;

/*****************************************************************************/
/*                             Test Suite Methods                            */
/*****************************************************************************/

__create_register_function(cspec_register_test_suite, CSpecTestSuite,
                           cspec_test_suites);

#define __SUITE__ __new_symb(suite)

#define __init_test_suite(__name, __desc)                                      \
  static CSpecTestSuite __name = {                                             \
      .desc = __desc, .tests_head = 0, .tests_tail = 0, .next = 0};            \
  static inline void __attribute__((constructor))                              \
      __concat(register_suite_, __name)(void) {                                \
    cspec_register_test_suite(&__name);                                        \
  }

#define describe(desc, body) __init_test_suite(__SUITE__, desc) body

/*****************************************************************************/
/*                             Test Case Methods                             */
/*****************************************************************************/

__create_register_function(cspec_register_test_case, CSpecTestCase,
                           cspec_test_suites_tail->tests);

#define __TEST__ __new_symb(suite)

#define __init_test_case(__name, __desc)                                       \
  static inline int __concat(run_test_, __name)(void);                         \
  static CSpecTestCase __name = {                                              \
      .desc = __desc, .next = 0, .run = __concat(run_test_, __name)};          \
  static inline void __attribute__((constructor))                              \
      __concat(register_test_, __name)(void) {                                 \
    cspec_register_test_case(&__name);                                         \
  }                                                                            \
  static inline int __concat(run_test_, __name)(void)

#define it(desc, body)                                                         \
  __init_test_case(__TEST__, desc) {                                           \
    body;                                                                      \
    return 0;                                                                  \
  }

/*****************************************************************************/
/*                                    Main                                   */
/*****************************************************************************/

int main(int argc, char *argv[]) {
  int nerrors = 0;
  int nfailures = 0;
  int npasses = 0;

  for (CSpecTestSuite *cur = cspec_test_suites_head; cur; cur = cur->next) {
    for (CSpecTestCase *test = cur->tests_head; test; test = test->next) {
      int pid = fork();
      if (-1 == pid) {
        // fork failed
        puts("Fork failed");
      } else if (0 == pid) {
        // we are in the child process
        int result = test->run();
        exit(result);
      } else {

        // we are in the parent process

        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);

        // Figure out if it crashed, failed, or passed
        if (!WIFEXITED(status)) {
          nerrors++;
          puts("E");
          test->status = -1;
        } else if (WEXITSTATUS(status)) {
          nfailures++;
          puts("F");
          test->status = WEXITSTATUS(status);
        } else {
          npasses++;
          puts(".");
          test->status = 0;
        }
      }
    }
  }
}
