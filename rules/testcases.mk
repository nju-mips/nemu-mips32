include rules/test-template.mk

# for microbench
ifeq ($(UNCORE),nemu)
export INPUT ?= REF
else
export INPUT ?= TEST
endif

AM_TESTS   != ls $(AM_HOME)/tests 2> /dev/null
AM_TESTS   := $(filter-out cputest,$(AM_TESTS))
AM_APPS    != ls $(AM_HOME)/apps 2> /dev/null
MIPS_TESTS != ls $(MIPS_TEST_HOME) 2> /dev/null
CPUTESTS   != find $(AM_HOME)/tests/cputest -name "*.c" 2> /dev/null
CPUTESTS   := $(basename $(notdir $(CPUTESTS)))

# AM apps
$(foreach app,$(AM_APPS),$(eval $(call test_template,$(AM_HOME)/apps/$(app),$(app),)))

# AM tests
$(foreach app,$(AM_TESTS),$(eval $(call test_template,$(AM_HOME)/tests/$(app),$(app),)))

# mipstest
$(foreach app,$(MIPS_TESTS),$(eval $(call test_template,$(MIPS_TEST_HOME)/$(app),$(app),)))

# nanos
# $(eval $(call test_template,$(NANOS_HOME),nanos,))

# linux
$(eval $(call test_template,$(LINUX_HOME),linux,vmlinux,!))

# u-boot
$(eval $(call test_template,$(U_BOOT_HOME),u-boot,u-boot,!))

# cputests
$(foreach c,$(CPUTESTS),$(eval $(call test_template,$(AM_HOME)/tests/cputest,$(c),,,ALL=$(c))))

.PHONY: clean-cputests run-cputests

clean-cputests:
	@make -s -C $(AM_HOME)/tests/cputest clean

run-cputests: $(foreach c,$(CPUTESTS),run-$(c))
