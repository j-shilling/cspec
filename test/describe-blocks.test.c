#include <test.h>

static inline int fib(int n) {
  if (n < 2)
    return 0;

  return fib(n-1) + fib(n-2);
}

describe("fib",

    it("returns 1 when n is 0",

        expect(fib(0)).to.be(1);

    );

);
