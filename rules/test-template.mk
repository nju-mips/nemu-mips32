# note INPUT=REF is designed for microbench
# arg1 dir           eg. $(AM_HOME)/apps/microbench
# arg2 name          eg. videotest microbench
# arg3 orig_app      eg. add instead of cputest
# arg4 orig_app deps eg. nothing for linux
# arg5 additional env
define test_template
ifneq ($(3),)
$(2)_ORIG_APP := $(1)/$(3)
else
$(2)_ORIG_APP := $(1)/build/$(2)-$(ARCH).elf
endif

ifneq ($(3),)
$(2)_OBJDIR := $(3)
else
$(2)_OBJDIR := $(OBJ_DIR)/$(2)
endif

ifeq ($(4),!)
$(2)_DEPS :=
else
$(2)_DEPS != find $(1) -regex ".*\.\(c\|h\|cc\|cpp\|S\)" &> /dev/null
endif

$(2)_ELF  := $$($(2)_OBJDIR)/$(2).elf

.PHONY: compile-$(2) clean-$(2)

$$($(2)_ORIG_APP): $$($(2)_DEPS)
	make -s -C $(1) ARCH=$(ARCH) $(5)

compile-$(2): $$($(2)_ORIG_APP)

run-$(2): $$($(2)_ORIG_APP)
	@build/nemu -b -e $$<

diff-$(2): $$($(2)_ORIG_APP)
	@build/nemu --diff-test -b -e $$<

clean-$(2):
	make -s -C $(1) ARCH=$(ARCH) $(5) clean
endef
