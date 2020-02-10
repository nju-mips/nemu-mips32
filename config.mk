.PHONY: app clean prepare menuconfig

AUTO_CONF := include/config/auto.conf
AUTOCONF_H := include/generated/autoconf.h
_CONFIG := .config
-include $(AUTO_CONF)

CONF := kconfig/conf
MCONF := kconfig/mconf
$(CONF) $(MCONF):
	@cd $(@D) && make -s $(@F)

ifeq ($(wildcard $(AUTO_CONF)),)
config-dep := prepare
prepare: $(AUTO_CONF)
	@make $(MAKEGOALS)
else
config-dep := $(AUTO_CONF) $(AUTOCONF_H)
endif

$(_CONFIG):
	@echo "You are suggested to make xx_defconfig/menuconfig firstly"

$(AUTO_CONF) $(AUTOCONF_H): $(_CONFIG) $(CONF)
	@$(CONF) --syncconfig ./Kconfig

menuconfig: $(MCONF)
	@$(MCONF) ./Kconfig

%_defconfig:
	@cp configs/$@ ./.config
