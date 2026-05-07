LINK_TYPE ?= static
INTER_LINK_TYPE ?= static

ifneq ($(MODULE_PATH),)
export CUR_DIR=./$(MODULE_PATH)
EXEFILE=prog_$(MODULE_NAME)
LIB_NAME=$(MODULE_NAME)
SUBDIRS:=$(MODULE_PATH)
include $(BUILD_TOP)/mi_dep.mk
#$(notdir $(MODULE_PATH)).mk is for app use only
-include $(MODULE_PATH)/$(notdir $(MODULE_PATH)).mk
-include $(MODULE_PATH)/dep.mk
#lib.mk used for internal libs, common libs, app self use libs
-include $(MODULE_PATH)/lib.mk
LIBS := $(foreach m,$(DEP),-l$(m)) $(LIBS)
INC += $(filter $(foreach d,$(DEP),%$(d)), $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
endif
INC  += $(BUILD_TOP)/common
INC  += $(ALKAID_PROJ)/release/include
INC += $(foreach m,$(PREBUILD_LIBS),$(BUILD_TOP)/prebuild_libs/$(m)/include/)
ifneq ($(PROJ_ROOT), )
INC  += $(PROJ_ROOT)/release/include
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/include/uapi/mstar
INC  += $(PROJ_ROOT)/kbuild/$(KERNEL_VERSION)/drivers/sstar/include
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/mi_libs/$(LINK_TYPE)
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/3rd_party_libs/$(LINK_TYPE)
LIBS += -L$(PROJ_ROOT)/release/chip/$(CHIP)/sigma_common_libs/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(LINK_TYPE)
LIBS += -lmi_sys -lmi_common
LIBS +=  -lcam_fs_wrapper -lcam_os_wrapper
ifeq ($(DUAL_OS), on)
CODEDEFINE += -DLINUX_FLOW_ON_DUAL_OS
endif
ifeq ($(CHIP), i6)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I6=1
else ifeq ($(CHIP), i2m)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I2M=1
else ifeq ($(CHIP), i6e)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I6E=1
else ifeq ($(CHIP), i6b0)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I6B0=1
else ifeq ($(CHIP), p3)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_P3=1
else ifeq ($(CHIP), m6)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_M6=1
else ifeq ($(CHIP), i7)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I7=1
else ifeq ($(CHIP), m6p)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_M6P=1
else ifeq ($(CHIP), i6c)
CODEDEFINE += -DCONFIG_SIGMASTAR_CHIP_I6C=1
endif
endif
LIBS += -lrt -lpthread -lm -ldl
LIBS += $(foreach m,$(PREBUILD_LIBS),-L$(BUILD_TOP)/prebuild_libs/$(m)/lib/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/$(LINK_TYPE)/)

LIBS += -L$(OUT_PATH)/$(ARCH)/libs/$(INTER_LINK_TYPE)/
LIBS += -L libs/
CODEDEFINE += -DTRANS_BUFFER=480 -DSOCKET_ADDR='"/tmp/cmd_base"'
MODULE_RELEASE_PACKAGE ?= off
#APP_REL_PREFIX ?= $(MODULE_NAME)

module_install:
ifneq ($(APP_REL_PREFIX), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/
	@cp -vrf $(OUT_PATH)/$(ARCH)/app/$(EXEFILE) $(IMAGE_PATH)/$(APP_REL_PREFIX)/
ifneq ($(DEBUG_ASAN), 1)
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/$(EXEFILE)
endif
ifneq ($(MODULE_REL_FILES), )
	$(foreach n,$(MODULE_REL_FILES),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/;)
endif
ifneq ($(MODULE_REL_LIB), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib/
	$(foreach n,$(MODULE_REL_LIB),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib;)
ifneq ($(DEBUG_ASAN), 1)
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/lib/*
endif
endif
ifneq ($(MODULE_REL_BIN), )
	@mkdir -p $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin/
	$(foreach n,$(MODULE_REL_BIN),cp -rfvd $(n) $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin;)
ifneq ($(DEBUG_ASAN), 1)
	@$(STRIP) --strip-unneeded $(IMAGE_PATH)/$(APP_REL_PREFIX)/bin/*
endif
endif
ifeq ($(MODULE_RELEASE_PACKAGE), on)
	@rm -rfv $(IMAGE_PATH)/$(APP_REL_PREFIX).sqfs
	@mksquashfs $(APP_REL_PREFIX) $(IMAGE_PATH)/$(APP_REL_PREFIX).sqfs -comp xz -all-root
endif
endif

include $(BUILD_TOP)/compile.mk
