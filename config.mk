.PHONY: app clean prepare

AUTO_CONF := include/config/auto.conf
AUTOCONF_H := include/generated/autoconf.h
-include $(AUTO_CONF)

CONF := menuconfig/conf
MCONF := menuconfig/mconf
$(CONF) $(MCONF):
	@cd $(@D) && make -s $(@F)

ifeq ($(wildcard $(AUTO_CONF)),)
prepare: $(AUTO_CONF)
	@make $(MAKEGOALS)
else
prepare: $(AUTO_CONF) $(AUTOCONF_H)
	@echo + .config
endif

$(AUTO_CONF): $(CONF)
	@$(CONF) --syncconfig ./Kconfig

menuconfig:
	@$(MCONF) ./Kconfig

%_defconfig:
	@cp configs/$@ ./.config
