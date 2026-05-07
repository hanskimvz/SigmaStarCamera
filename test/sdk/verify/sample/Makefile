BUILD_TOP:=$(shell pwd)
ALKAID_PATH ?= $(BUILD_TOP)/../../..
OUT_PATH:=$(BUILD_TOP)/out
ALKAID_PROJ:=$(ALKAID_PATH)/project
ALKAID_PROJ_CONFIG:=$(ALKAID_PATH)/project/configs/current.configs
3PARTY_PATH:=$(BUILD_TOP)/../3rdparty
IMAGE_PATH := $(BUILD_TOP)/release

include $(BUILD_TOP)/mi_dep.mk

LIBS_PATH := internal
APP_PATH := maruko
MODULES_ALL ?= $(patsubst $(BUILD_TOP)/%/,%,$(foreach m,$(shell find $(foreach n, ${APP_PATH}, $(BUILD_TOP)/${n}) -type d),$(dir $(wildcard $(m)/$(notdir $(m)).mk))))

FILTER_OUT_MK := $(shell find $(foreach n, ${APP_PATH}, $(BUILD_TOP)/${n}) -name filterout.mk)
-include $(FILTER_OUT_MK)
MODULES := $(filter-out $(foreach q,$(FILTER_OUT_MODULE), %/$(q)), $(MODULES_ALL))

$(warning MODULES := $(MODULES))

-include $(foreach mod,$(MODULES_IN),$(BUILD_TOP)/$(mod)/dep.mk)

INTERNAL_LIBS := $(notdir $(wildcard $(foreach m,$(LIBS_PATH),$(m)/*)))
MODULES_CLEAN := $(foreach m,$(MODULES),$(m)_clean)
MODULES_APP_ALL := $(foreach m,$(MODULES),$(m)_app_all)
MODULES_APP_CLEAN := $(foreach m,$(MODULES),$(m)_app_clean)
MODULES_OBJS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_all)
MODULES_OBJS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_obj_clean)
MODULES_LIBS_ALL := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_all)
MODULES_LIBS_CLEAN := $(foreach m,$(INTERNAL_LIBS),$(m)_lib_clean)
MODULES_INSTALL := $(foreach m,$(MODULES),$(m)_install)

export PROJ_ROOT CHIP PRODUCT BOARD TOOLCHAIN TOOLCHAIN_VERSION TOOLCHAIN_REL KERNEL_VERSION CUSTOMER_TAILOR CUSTOMER_OPTIONS MOD_PREFIX INTERFACE_ENABLED INTERFACE_DISABLED MHAL_ENABLED MHAL_DISABLED DUAL_OS CHIP_ALIAS MY_TOOLCHAIN
export BUILD_TOP
export OUT_PATH IMAGE_PATH
export ALKAID_PROJ
export ALKAID_PROJ_CONFIG
export TARGET_OUT
export LIBS_PATH
export ARCH CROSS_COMPILE

.PHONY: all lib obj clean install depend_internal depend_internal_clean $(MODULES) $(MODULES_CLEAN) $(MODULES_APP_ALL) $(MODULES_APP_CLEAN) $(MODULES_OBJS_ALL) $(MODULES_OBJS_CLEAN) $(MODULES_LIBS_ALL) $(MODULES_LIBS_CLEAN)

all:
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal
ifneq ($(MODULES_APP_ALL), )
	@$(MAKE) $(MODULES_APP_ALL)
endif

clean:
	@$(MAKE) $(foreach mod,$(MODULES),$(patsubst %,%_app_clean,$(mod)))
	@$(MAKE) MODULES_IN="$(MODULES)" depend_internal_clean
	@rm $(OUT_PATH) -rf
	@rm $(IMAGE_PATH) -rf

install:
	@rm -rvf $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo
	@mkdir -p $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/
	@$(MAKE) $(MODULES_INSTALL)
	@if [ -d $(IMAGE_PATH) ]; then  \
		cp -rvf $(IMAGE_PATH)/* $(PROJ_ROOT)/release/chip/$(CHIP)/$(PRODUCT)/common/$(TOOLCHAIN)/$(TOOLCHAIN_VERSION)/bin/mi_demo/;  \
	fi;

depend_internal:
ifneq ($(DEP), )
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_all)
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_all)
endif

depend_internal_clean:
ifneq ($(DEP), )
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_obj_clean)
	@$(MAKE) $(foreach m,$(sort $(DEP)),$(m)_lib_clean)
endif

$(MODULES_INSTALL):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(foreach m,$(APP_PATH),$(patsubst $(m)/%,%,$(filter $(m)/%,$(patsubst %_install,%,$@)))))) MODULE_PATH=$(patsubst %_install,%,$@) -f ./build.mk module_install

$(MODULES):
	@$(MAKE) MODULES_IN=$@ depend_internal
	@$(MAKE) $(patsubst %,%_app_all,$@)

$(MODULES_CLEAN):
	@$(MAKE) $(patsubst %_clean,%_app_clean,$@)
	@$(MAKE) MODULES_IN=$(patsubst %_clean,%,$@) depend_internal_clean

$(MODULES_APP_ALL):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(foreach m,$(APP_PATH),$(patsubst $(m)/%,%,$(filter $(m)/%,$(patsubst %_app_all,%,$@)))))) MODULE_PATH=$(patsubst %_app_all,%,$@) -f ./build.mk gen_exe all

$(MODULES_APP_CLEAN):
	@$(MAKE) MODULE_NAME=$(strip $(subst /,_,$(foreach m,$(APP_PATH),$(patsubst $(m)/%,%,$(filter $(m)/%,$(patsubst %_app_clean,%,$@)))))) MODULE_PATH=$(patsubst %_app_clean,%,$@) -f ./build.mk clean_files clean

$(MODULES_OBJS_ALL):
	@$(MAKE) MODULE_PATH=$(filter %$(patsubst %_obj_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./build.mk gen_obj

$(MODULES_OBJS_CLEAN):
	@$(MAKE) MODULE_PATH=$(filter %$(patsubst %_obj_clean,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) -f ./build.mk clean_files

$(MODULES_LIBS_ALL):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=static -f ./build.mk gen_lib
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_all,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=dynamic -f ./build.mk gen_lib

$(MODULES_LIBS_CLEAN):
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=static -f ./build.mk clean_files
	@$(MAKE) MODULE_NAME=$(patsubst %_lib_clean,%,$@) MODULE_PATH=$(filter %$(patsubst %_lib_all,%,$@),$(wildcard $(foreach m,$(LIBS_PATH),$(m)/*))) LIB_TYPE=dynamic -f ./build.mk clean_files
