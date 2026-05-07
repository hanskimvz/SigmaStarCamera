.PHONY :all clean gen_exe gen_obj clean_files gen_lib

GCCFLAGS ?= -Wall -g -pipe -fPIC
ifeq ($(ARCH),arm)
API_EX_CFLAGS+= -mthumb
endif
ifeq ($(DEBUG_ASAN), 1)
GCCFLAGS += -fsanitize=address -fno-omit-frame-pointer
endif
CODEDEFINE += -DLINUX_OS
CXXFLAGS := $(GCCFLAGS) $(LOCAL_CXXFLAGS)
CXXFLAGS += $(CODEDEFINE) -DLINUX_OS -std=gnu++11
CXXFLAGS += $(foreach dir,$(INC),-I$(dir))

CFLAGS := $(GCCFLAGS) $(LOCAL_CFLAGS)
CFLAGS += $(CODEDEFINE)
CFLAGS += $(foreach dir,$(INC),-I$(dir))

SRC      +=  $(sort $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c)) $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.cpp)))
OBJS_CXX := $(patsubst %.cpp,%.$(ARCH).o,$(filter %.cpp, $(SRC)))
OBJS     := $(patsubst %.c,%.$(ARCH).o,$(filter %.c, $(SRC)))
SRC_CHECK := $(filter-out $(realpath $(MODULE_PATH))%,$(foreach file,$(SRC),$(realpath $(file))))
ifneq ($(SRC_CHECK),)
$(warning files->$(SRC_CHECK))
$(error $(MODULE_NAME) include extern src files, please use submodule or parvate submodule for instead.)
endif

ifeq ($(LIB_TYPE), static)
LIB_SUFFIX := a
endif
ifeq ($(LIB_TYPE), dynamic)
LIB_SUFFIX := so
endif
LIB_OUT:= $(OUT_PATH)/$(ARCH)/libs/$(LIB_TYPE)
APP_OUT:= $(OUT_PATH)/$(ARCH)/app

DFILES := $(foreach f,$(OBJS_CXX) $(OBJS),$(patsubst %.o,%.d,$(f)))
sinclude $(DFILES)

$(OBJS):%.$(ARCH).o:%.c
	@echo compile $<...
	@$(CC) $(CFLAGS) -MM $< -MT $@ > $(@:.o=.d)
	@$(CC) $(CFLAGS) -c -ffunction-sections -fdata-sections $< -o $@
$(OBJS_CXX):%.$(ARCH).o:%.cpp
	@echo compile $<...
	@$(CC) $(CXXFLAGS) -MM $< -MT $@ > $(@:.o=.d)
	@$(CXX) $(CXXFLAGS) -c -ffunction-sections -fdata-sections $< -o $@

gen_exe: $(OBJS_CXX) $(OBJS)
ifneq ($(OBJS_CXX), )
	@mkdir -p $(APP_OUT)
	@$(CXX) $(CXXFLAGS) -Wl,--gc-sections $(OBJS_CXX) $(OBJS) $(LIBS) -o $(APP_OUT)/$(EXEFILE)
else
ifneq ($(OBJS), )
	@mkdir -p $(APP_OUT)
	@$(CC) $(CFLAGS) -Wl,--gc-sections $(OBJS) $(LIBS) -o $(APP_OUT)/$(EXEFILE)
endif
endif

gen_obj:$(OBJS_CXX) $(OBJS)

%.a: $(OBJS_CXX) $(OBJS)
	@mkdir -p $(LIB_OUT)
	@$(AR) sqD $@ $(OBJS_CXX) $(OBJS)

%.so:$(OBJS_CXX) $(OBJS)
	@mkdir -p $(LIB_OUT)
	@$(CXX) -shared -fPIC -o $@ $(OBJS_CXX) $(OBJS)

gen_lib: $(LIB_OUT)/lib$(LIB_NAME).$(LIB_SUFFIX)

clean_files:
	@rm -rf $(OBJS_CXX) $(OBJS) $(OBJS_CXX:.o=.d) $(OBJS:.o=.d)
ifneq ($(EXEFILE), )
	@rm -rf $(APP_OUT)/$(EXEFILE)
endif
