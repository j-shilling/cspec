#ifdef __TEST_H__
#error "The test header has been included more than once."
#endif
#define __TEST_H__

#define TOUTPUT_BUFFER_SIZE CSPEC_TEST_OUTPUT_BUFFER_SIZE
#define TFILE_BUFFER_SIZE CSPEC_TEST_FILENAME_BUFFER_SIZE

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

} CSpecTestCase;

/*****************************************************************************/
/*                                Test Suite                                 */
/*****************************************************************************/

typedef struct _cspec_test_suite {
  const char *desc;

  CSpecTestCase *head;
  CSpecTestCase *tail;

  struct _cspec_test_suite *next;
  struct _cspec_test_suite *children_head;
  struct _cspec_test_suite *children_tail;
} CSpecTestSuite;

static CSpecTestSuite *cspec_test_suites_head = 0;
static CSpecTestSuite *cspec_test_suites_tail = 0;

__create_register_function(cspec_register_test_suite, CSpecTestSuite,
                           cspec_test_suites);

#define __SUITE__ __new_symb(suite)

#define __init_test_suite(__name, __desc)                                      \
  static CSpecTestSuite __name = {.desc = __desc,                              \
                                  .head = 0,                                   \
                                  .tail = 0,                                   \
                                  .next = 0,                                   \
                                  .children_head = 0,                          \
                                  .children_tail = 0};                         \
  static inline void __attribute__((constructor))                              \
      __concat(register_suite_, __name)(void) {                                \
    cspec_register_test_suite(&__name);                                        \
  }

#define describe(desc, body) __init_test_suite(__SUITE__, desc)

int main(int argc, char *argv[]) {
  for (CSpecTestSuite *cur = cspec_test_suites_head; cur; cur = cur->next) {
    puts(cur->desc);
      for (CSpecTestSuite *child = cur->children_head; child; child = child->next) {
	printf("  %s", child->desc);
      }
  }
}
