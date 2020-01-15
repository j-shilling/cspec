#include <stdio.h>
#include <test.h>

it("test 1",
   assert(0 == 0);
);

describe("First Describe Block",
  it("test 2",
     assert(1 == 1);
  );

  it("test 3",
  );
);

describe("Second Describe Block",
	 it("test 4",
  );

	 it("test 5", );
);
