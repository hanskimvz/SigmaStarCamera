INC  += $(BUILD_TOP)/internal/live555/UsageEnvironment/include
INC  += $(BUILD_TOP)/internal/live555/groupsock/include
INC  += $(BUILD_TOP)/internal/live555/liveMedia/include
INC  += $(BUILD_TOP)/internal/live555/BasicUsageEnvironment/include
INC  += $(BUILD_TOP)/internal/live555/mediaServer/include
INC  += $(BUILD_TOP)/internal/iniparser
INC  += $(BUILD_TOP)/maruko/rtsp_det/inc

LIBS += -L$(BUILD_TOP)/libs
##LIBS += -llive555
LIBS += -L$(BUILD_TOP)/internal/iniparser
LIBS += -liniparser -ldictionary
LIBS += -L$(BUILD_TOP)/maruko/rtsp_det/libs
LIBS += -lsstar_algo_detection -lsstar_algo
 
LIBS += -lmi_sys -lmi_sensor -lmi_vif -lmi_iqserver -lfbc_decode
LIBS += -lmi_ipu -lmi_isp -lmi_scl -lmi_venc -lmi_rgn -lmi_disp -lmi_panel

ifeq ($(INTERFACE_CUS3A), y)
LIBS += -lcus3a -lispalgo
endif