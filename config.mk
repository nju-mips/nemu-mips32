.PHONY: app clean menuconfig

AUTO_CONF  := include/config/auto.conf
AUTOCONF_H := include/generated/autoconf.h
_CONFIG    := .config
CONF       := kconfig/conf
MCONF      := kconfig/mconf

CFLAGS += -include $(AUTOCONF_H)

-include $(AUTO_CONF)

ifneq ($(MAKECMDGOALS),menuconfig)
$(AUTO_CONF): $(_CONFIG)
	@$(CONF) --syncconfig ./Kconfig
endif

menuconfig: $(MCONF)
	@$(MCONF) ./Kconfig

$(CONF) $(MCONF): %:
	@cd $(@D) && make -s $(@F)

$(_CONFIG): $(CONF)
	@$(CONF) ./Kconfig

$(AUTOCONF_H): $(_CONFIG)
	@$(CONF) --syncconfig ./Kconfig

%_defconfig:
	@cp configs/$@ ./.config
