.PHONY: app clean menuconfig

AUTO_CONF  := include/config/auto.conf
AUTOCONF_H := include/generated/autoconf.h
_CONFIG    := .config
CONF       := kconfig/conf
MCONF      := kconfig/mconf

CFLAGS += -include $(AUTOCONF_H)

-include $(AUTO_CONF)

FIRST_GOAL := $(firstword $(MAKECMDGOALS))
ifneq ($(FIRST_GOAL),menuconfig)
ifeq ($(filter %_defconfig,$(FIRST_GOAL)),)
$(AUTO_CONF): $(_CONFIG)
	@$(CONF) --syncconfig ./Kconfig
endif
endif

menuconfig: $(MCONF)
	@$(MCONF) ./Kconfig

$(CONF) $(MCONF): %:
	@cd $(@D) && make -s $(@F)

$(_CONFIG): Kconfig | $(CONF)
	@$(CONF) $<

$(AUTOCONF_H): $(_CONFIG)
	@$(CONF) --syncconfig ./Kconfig

%_defconfig:
	@cp configs/$@ ./.config
