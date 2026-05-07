ifneq ($(ARCH),x86)
-include $(ALKAID_PROJ_CONFIG)
ifneq ($(PROJ_ROOT), )
ifeq ($(PRODUCT), xvr)
CUSTOMER_ENABLED := snr9931
endif
INTERFACE_MODULES := vif isp scl disp vdec venc rgn vdisp sys ai ao gfx ipu sensor vdf ldc shadow panel hdmi isp iqserver pcie mipitx fb nir
INTERFACE_ENABLED:=$(patsubst %_enable_,%,$(filter %_enable_, $(foreach n,$(INTERFACE_MODULES),$(n)_$(interface_$(n))_)))
INTERFACE_DISABLED:=$(filter-out $(INTERFACE_ENABLED),$(INTERFACE_MODULES))
CODEDEFINE += $(foreach n,$(INTERFACE_ENABLED),-DINTERFACE_$(shell tr 'a-z' 'A-Z' <<< $(n))) $(foreach n,$(CUSTOMER_ENABLED),-DCUSTOMER_$(shell tr 'a-z' 'A-Z' <<< $(n)))
CODEDEFINE += -DSSTAR_CHIP_$(shell tr 'a-z' 'A-Z' <<< $(CHIP))
else
CC := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AR := $(CROSS_COMPILE)ar
STRIP := $(CROSS_COMPILE)strip
TOOLCHAIN ?= glibc
TOOLCHAIN_REL := $(CROSS_COMPILE)
TOOLCHAIN_VERSION := $(shell $(TOOLCHAIN_REL)gcc -dumpversion)
endif
else
CC := gcc
CXX := g++
AR := ar
STRIP := strip
endif

