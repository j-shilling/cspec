#ifdef __TEST_H__
#error "The test header has been included more than once."
#endif
#define __TEST_H__

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

typedef int (*test_t)(int);

static const int TOUTPUT_BUFFER_SIZE = 512;
static const int TFILE_BUFFER_SIZE = 128;

static const int TPASS = 0;
static const int TFAIL = 1;
static const int TERROR = 2;

typedef struct _test_case {
  test_t func;
  int status;
  int line_number;
  const char *name;
  int pipe[2];
  char output[TOUTPUT_BUFFER_SIZE];
  char file[TFILE_BUFFER_SIZE];
} test_case_t;

typedef struct _test_case_list {
  test_case_t *item;
  struct _test_case_list *next;
} test_case_list_t;

typedef struct _test_suite {
  const char *name;
  test_case_list_t *head;
  test_case_list_t *tail;
} test_suite;

static struct _test_suite suite = {.name = NULL, .head = NULL, .tail = NULL};

static inline void register_test_case(test_case_list_t *list) {
  if (suite.tail) {
    suite.tail->next = list;
    suite.tail = list;
  } else {
    suite.head = list;
    suite.tail = list;
  }
}

#define __xconcat(x, y) x##y
#define __concat(x, y) __xconcat(x, y)
#define __to_symb(x) __concat(x, __COUNTER__)
#define __TEST_FUNCTION__ __to_symb(test)

#define __xinit_test(tfunc, tname)                                             \
  static inline int tfunc(int);                                                \
  static test_case_t tfunc##_test_case = {.func = tfunc, .name = tname};       \
  static test_case_list_t tfunc##_test_case_node = {                           \
      .item = &tfunc##_test_case, .next = NULL};                               \
  static inline void __attribute__((constructor))                              \
      register_test_##tfunc(void) {                                            \
    register_test_case(&tfunc##_test_case_node);                               \
  }                                                                            \
  static inline int tfunc(int fd) {                                            \
    int line;                                                                  \
    const char filename[] = __FILE__;

#define __init_test(x, y) __xinit_test(x, y)

#define START_TEST(tname) __init_test(__TEST_FUNCTION__, tname)

#define END_TEST                                                               \
  return TPASS;                                                                \
  }

#define DESCRIBE(sname)                                                        \
  static inline void __attribute__((constructor)) describe_suite(void) {       \
    suite.name = sname;                                                        \
  }

#define IT(message, body) START_TEST("it " message) body END_TEST

#define fail(args...)                                                          \
  do {                                                                         \
    printf("fd = %d", fd);                                                     \
    line = __LINE__;                                                           \
    write(fd, &line, sizeof(line));                                            \
    write(fd, filename, sizeof(filename));                                     \
    dprintf(fd, args);                                                         \
    return TFAIL;                                                              \
  } while (0);

#define assert(expr)                                                           \
  do {                                                                         \
    if (!(expr)) {                                                             \
      fail("Failure/Error: assert()\n\n"                                       \
           "  expected: true\n"                                                \
           "       got: false\n");                                             \
    }                                                                          \
  } while (0);

static inline void __print_tests_with_status(const char *message,
                                             const int status) {
  printf("%s:\n\n", message);
  int i = 0;
  for (test_case_list_t *cur = suite.head; cur; cur = cur->next) {
    if (status == cur->item->status) {
      printf("%4d) %s\n", ++i, cur->item->name);

      printf("      \x1b[31m%s\x1b[0m\n", cur->item->output);

      printf("\x1b[36m    // %s:%d\x1b[0m\n\n", cur->item->file,
             cur->item->line_number);
    }
  }
}

int main(int argc, char *argv[]) {
  int examples = 0;
  int failures = 0;
  int errors = 0;

  if (suite.name) {
    printf("%s:\n\n", suite.name);
  }

  for (test_case_list_t *cur = suite.head; cur; cur = cur->next) {
    examples++;
    test_case_t *test = cur->item;

    fflush(stdout);
    pipe(test->pipe);

    int pid = fork();
    if (0 == pid) {
      // We are in the child process
      close(test->pipe[0]);
      exit(test->func(test->pipe[1]));
    } else if (-1 == pid) {
      // The fork failed
      perror("Could not start child process");
    } else {
      // We are in the parent process

      close(test->pipe[1]);
      read(test->pipe[0], &test->line_number, sizeof(test->line_number));
      read(test->pipe[0], test->file, TFILE_BUFFER_SIZE);
      read(test->pipe[0], test->output, TOUTPUT_BUFFER_SIZE);
      close(test->pipe[0]);

      int status;
      waitpid(pid, &status, 0);
      test->status = WEXITSTATUS(status);
      if (TFAIL == test->status) {
        failures++;
        printf("\x1b[31mF\x1b[0m");
      } else if (TERROR == test->status) {
        errors++;
        printf("\x1b[31mE\x1b[0m");
      } else {
        printf("\x1b[32m.\x1b[0m");
      }
    }
  }

  putchar('\n');

  if (failures) {
    __print_tests_with_status("Failures", TFAIL);
  }
  if (errors) {
    __print_tests_with_status("Errors", TERROR);
  }

  if (failures || errors)
    printf("\x1b[31m");
  else
    printf("\x1b[32m");
  printf("%d examples, %d failures, %d errors\x1b[0m\n", examples, failures,
         errors);

  return 0;
}
