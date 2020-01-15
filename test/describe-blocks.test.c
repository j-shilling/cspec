#include <stdio.h>
#include <test.h>

it("test 1",
   return 1;
);

describe("First Describe Block",
  it("test 2",
     CSpecTestCase *test = NULL;
     test->run();
  );

  it("test 3",
     return 1;
  );
);

describe("Second Describe Block",
  it("test 4"
,     CSpecTestCase *test = NULL;
     test->run();
  );

	 it("test 5", );
);
