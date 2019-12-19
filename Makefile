TESTDIR   := test
BUILDDIR  := obj
TARGETDIR := bin
DEPDIR    := dep

TEST_SRC  := $(shell find $(TESTDIR) -name '*.test.c')
TESTS     := $(patsubst $(TESTDIR)/%.test.c, %, $(TEST_SRC))

WARNINGS  := -Werror -Wall -Wextra -Wshadow -Wdouble-promotion -Wformat=2 \
             -Wundef -fno-common -Wno-unused-parameter

.PHONY: check clean

check: $(TESTS)

clean:
	-rm -rf $(TARGETDIR)
	-rm -rf $(BUILDDIR)
	-rm -rf $(DEPDIR)

define TEST_template
$(1)_PROG = $$(TARGETDIR)/$(1).test
$(1)_SRC  = $$(TESTDIR)/$(1).test.c
$(1)_OBJ  = $$(BUILDDIR)/$(1).test.o
$(1)_DEP  = $$(DEPDIR)/$(1).test.d

$(1)_DEPFLAGS = -MT $$@ -MMD -MP -MF $$($(1)_DEP)

$(1): $$($(1)_PROG)
	@echo "  [TEST] $$@"
	@./$$($(1)_PROG) &> test.log
	-@rm test.log

$$($(1)_PROG): $$($(1)_OBJ)
	@mkdir -p $$(dir $$@)
	@echo "  [LINK] $$@"
	@$$(CC) $$(LDFLAGS) -o $$@ $$^

$$($(1)_OBJ): $$($(1)_SRC)
	@mkdir -p $$(dir $$@)
	@mkdir -p $$(dir $$($(1)_DEP))
	@echo "  [CC] $$@"
	@$$(CC) $$(WARNINGS) $$(CFLAGS) $$($(1)_DEPFLAGS) -I. -c -o $$@ $$<

.PHONY: $(1)

endef

$(foreach test,$(TESTS),$(eval $(call TEST_template,$(test))))

####
## DEPENDENCY FILES
####

$(DEPS):

include $(wildcard $(DEPS))
