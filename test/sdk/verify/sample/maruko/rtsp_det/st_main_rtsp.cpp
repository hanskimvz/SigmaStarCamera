#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>
#include "st_common.h"
#include "st_rgn.h"
#include "log.h"
#include "mi_sensor.h"
#include "mi_vif.h"
#include "mi_isp.h"
#include "mi_scl.h"
#include "mi_venc.h"
#include "mi_iqserver.h"
#include "mi_ipu.h"

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "Live555RTSPServer.hh"

#include "dictionary.h"
#include "iniparser.h"
#include "sstar_detection_api.h"
// #include "mi_ipu.h"

#define ST_ISP_MAX_DEV_NUM			(1)
#define ST_ISP_MAX_CHN_NUM			(16)
#define ST_ISP_MAX_PORT_NUM			(3)

#define ST_SCL_MAX_DEV_NUM			(3)
#define ST_SCL_MAX_CHN_NUM			(4)			// really 32
#define ST_SCL_MAX_PORT_NUM			(4)

#define ST_STREAM_MAX_NUM			(6)

#define ST_FILE_OUT_MAX_NUM			(2)

#define RGN_OSD_TIME_WIDTH			(540)	// for 1920x1080
#define RGN_OSD_TIME_HEIGHT			(48)
#define RGN_OSD_HANDLE0				(0)
#define RGN_OSD_HANDLE1				(1)
#define ST_OSD_MAX_NUM				(10)
#define INVALID_RGN_HANDLE			(0xFFFF)
#define __ENABLE_PANLE__


#define RTSP_LISTEN_PORT        	554
#define ISP_ALGO_ENABLE
#ifdef ISP_ALGO_ENABLE
#include "mi_isp_iq.h"
#include "mi_isp_ae.h"
#include "mi_isp_cus3a_api.h"
static char g_IspBinPath[64] = {0};
#endif

static MI_U8 g_load_iq_bin = TRUE;
// ai
//#define DISP_WIDTH 1344
//#define DISP_HEIGHT 760
#define DISP_WIDTH 3840
#define DISP_HEIGHT 2160

#define __ENABLE_UART_FUNCTION__

#ifdef __ENABLE_UART_FUNCTION__
#include <termios.h>
#define __ENABLE_TX_POS_BY_UART__
int uart_fd=-1;
int g_baudrate = 115200;
int g_mavlink_baudrate = 115200;
int g_mavlink_flight_log_enable = 1;
int g_mavlink_ground_log_enable = 1;
pthread_t UART_RX_pthread=NULL;
#ifdef __ENABLE_TX_POS_BY_UART__
void Send_Car_Coord_Info_ByUart(char *data,int length);
#endif
#endif

MI_RGN_HANDLE rgnHandle = 5;
auto start_time = std::chrono::steady_clock::now();

typedef struct
{
	MI_ISP_HDRType_e    	eHDRType;
    MI_BOOL             	bMirror;
    MI_BOOL             	bFlip;
    MI_SYS_Rotate_e     	eRot;

	MI_BOOL					bOpenIqserver;
	char					szIspBinFile[128];
} ST_BasicAttr_S;

typedef struct
{
	MI_SNR_PADID 			ePADId;
	MI_S32 					u8ResIdx;
	MI_U32 					u32Fps;
} ST_ModSensorAttr_S;

typedef struct
{
	MI_VIF_GROUP 			GroupId;
	MI_VIF_DEV 				DevId;
	MI_VIF_PORT 			PortId;
	MI_SYS_WindowRect_t 	stCapRect;
    MI_SYS_WindowSize_t 	stDestSize;
} ST_ModVifAttr_S;

typedef struct
{
	MI_ISP_PORT				PortId;
	MI_BOOL					bEnable;
	MI_ISP_OutPortParam_t	stOutPortParam;
} ST_IspPortAttr_S;

typedef struct
{
	MI_ISP_CHANNEL 			ChnId;
	MI_SYS_WindowRect_t		stInputCrop;
	MI_ISP_ChnParam_t 		stChnParam;
	
	ST_IspPortAttr_S		stPortAttr[ST_ISP_MAX_PORT_NUM];
} ST_IspChnAttr_S;

typedef struct
{
	MI_ISP_DEV 				DevId;
	MI_U32					u32MaxChnNum;

	// input mod
	MI_SYS_ChnPort_t		stSrcChnPort;
	MI_SYS_BindType_e 		eBindType;
	
	ST_IspChnAttr_S			stChnAttr[ST_ISP_MAX_CHN_NUM];
} ST_ModIspAttr_S;

typedef struct
{
	MI_SCL_PORT 			PortId;
	MI_BOOL					bEnable;
	MI_SCL_OutPortParam_t	stOutPortParam;
} ST_SclPortAttr_S;

typedef struct
{
	MI_SCL_CHANNEL 			ChnId;
	MI_SYS_WindowRect_t		stInputCrop;

	ST_SclPortAttr_S		stPortAttr[ST_SCL_MAX_PORT_NUM];
} ST_SclChnAttr_S;

typedef struct
{
	MI_SCL_DEV 				DevId;
	MI_U32					u32MaxChnNum;

	// input mod
	MI_SYS_ChnPort_t		stSrcChnPort;
	MI_SYS_BindType_e 		eBindType;
	
	ST_SclChnAttr_S			stChnAttr[ST_SCL_MAX_CHN_NUM];
} ST_ModSclAttr_S;

typedef struct
{
	MI_BOOL					bCreate;

	MI_VENC_DEV 			VeDev;
	MI_VENC_InitParam_t		stInitParam;
} ST_ModVencDevInfo_S;

typedef struct
{
	// input mod
	MI_SYS_ChnPort_t		stSrcChnPort;
	MI_SYS_BindType_e 		eBindType;

	// venc attr
	MI_VENC_CHN 			VeChn;
	MI_U32					u32Width;
	MI_U32					u32Height;
	MI_VENC_ModType_e 		eType;
	MI_VENC_RcMode_e 		eRcMode;
	MI_U32					u32Fps;
	MI_U32 					u32Gop;
	MI_U32 					u32MaxBitRate;

	char					szURLTag[16];
} ST_ModVencAttr_S;

typedef struct
{
	MI_U32					u32MaxNum;

	ST_ModVencDevInfo_S		stVencDevH26X;
	ST_ModVencDevInfo_S		stVencDevMJPEG;
	
	ST_ModVencAttr_S		stVencAttr[ST_STREAM_MAX_NUM];
} ST_StreamAttr_S;

typedef struct
{
	// input mod
	MI_SYS_ChnPort_t		stSrcChnPort;

	int						fd;

	MI_BOOL					bDump;
} ST_DumpAttr_S;

typedef struct
{
	MI_U32					u32MaxNum;
	char					szDir[128];

	int						maxFd;

	MI_BOOL					bRunFlag;
	pthread_t 				pt;
	
	ST_DumpAttr_S			stDumpAttr[ST_FILE_OUT_MAX_NUM];
} ST_FileOutAttr_S;

typedef struct
{
	MI_BOOL					bRunFlag;
	pthread_t 				pt;

	MI_RGN_HANDLE 			hHandle[ST_OSD_MAX_NUM];
} ST_OSDAttr_S;

typedef struct{
	MI_S8					detect_enable;
	MI_S16					detect_type;
	char*					model_path;
	MI_FLOAT				detect_threshold;
	MI_S16					print_message;
}ST_IPUAttr_S;


typedef struct
{
	// basic
	ST_BasicAttr_S			stBasicAttr;

	// module
	// sensor
	ST_ModSensorAttr_S		stSensorAttr;

	// vif
	ST_ModVifAttr_S			stVifAttr;

	// isp
	ST_ModIspAttr_S			stIspAttr;

	// scl
	ST_ModSclAttr_S			stSclAttr;

	// rtsp
	ST_StreamAttr_S			stStreamAttr;

	// file to out
	ST_FileOutAttr_S		stFileOutAttr;

	// sensor
	ST_OSDAttr_S			stOSDAttr;

	// algo_detection
	ST_IPUAttr_S			stIPUAttr;
} ST_PipeLineCase_S;

class CanvasScopeLock
{
    public:
        explicit CanvasScopeLock()
        {
            pthread_mutex_lock(&_gstCanvasMutex);
        }
        ~CanvasScopeLock()
        {
            pthread_mutex_unlock(&_gstCanvasMutex);
        }
        static void CanvasLock()
        {
            pthread_mutex_lock(&_gstCanvasMutex);
        }
        static void CanvasUnlock()
        {
            pthread_mutex_unlock(&_gstCanvasMutex);
        }
    private:
        static pthread_mutex_t _gstCanvasMutex;
};
pthread_mutex_t CanvasScopeLock::_gstCanvasMutex = PTHREAD_MUTEX_INITIALIZER;

#define CANVAS_LOCK CanvasScopeLock::CanvasLock()
#define CANVAS_UNLOCK CanvasScopeLock::CanvasUnlock()

static MI_BOOL g_bExit = FALSE;
static Live555RTSPServer *g_pRTSPServer = NULL;

static dictionary *g_pstDict = NULL;
static ST_PipeLineCase_S *g_pstPipeLineCase = NULL;

void ST_Flush(void)
{
    char c;

    while((c = getchar()) != '\n' && c != EOF);
}

MI_S32 ST_CalcAfbcBufferSize(MI_SYS_WindowSize_t *pstWinSize, MI_U32 *pu32BufferSize)
{
    MI_U32 u32HeaderSize =0;
    MI_U32 u32FrameSize = 0;

    u32FrameSize = (ALIGN_UP(pstWinSize->u16Width, 128) * ALIGN_UP(pstWinSize->u16Height, 2))*3/2; //only support yuv420 pixel
    u32HeaderSize = (u32FrameSize/(64*4))*16;//block size 64x4, header size per block 16 byte

    *pu32BufferSize = u32FrameSize + u32HeaderSize ;

    return MI_SUCCESS;
}

MI_S32 ST_SysModuleInit()
{
	MI_SYS_Init(0);

	return MI_SUCCESS;
}

MI_S32 ST_SysModuleUnInit()
{
	MI_SYS_Exit(0);

	return MI_SUCCESS;
}
#ifdef __ENABLE_PANLE__
#include "mi_disp.h"
#include "mi_panel.h"
struct ST_Panel_Attr_T
{
    MI_U32              u32Width;
    MI_U32              u32Height;
    MI_VIF_DEV          s32vifDev;
};
static struct ST_Panel_Attr_T g_stPanelAttr[]=
{
    {
            .u32Width = 800,
            .u32Height = 480,
            .s32vifDev = 0,
    },

};
typedef struct 
{
    char path[100];
    MI_U8 gpio_num;
}stgpio_value_path;
enum pwm_init_status
{
    pwm_Deinit,
    pwm_Init,
};
enum pwm_enable_status
{
    pwm_Disable,
    pwm_Enable,
};
#define MAX_BUF 	(64)
#define MAXPWMINDEX 20
#define PWMPATHBASE "/sys/devices/soc0/soc/1f203200.pwm/pwm/pwmchip0/"
#define HIGH_LEVEL 1
#define LOW_LEVEL 0
#define PANEL_GPIO_NUM 5
#define  PANEL_BL_PWM_NUM        (8)
#define  PANEL_BL_PWM_ON         (100)
#define  PANEL_ENABLE_NUM        (26)
#define  PANEL_ENABLE_ON         (LOW_LEVEL)
#define  PANEL_ENABLE_OFF        (HIGH_LEVEL)
#define  PANEL_RST               (11)//(7)
#define SENSOR_RGB_FPS 25
stgpio_value_path gstgpio_value_path[PANEL_GPIO_NUM];
void _user_set_gpio_value(MI_U8 gpio_num, MI_U8 index)
{
    MI_U8 n = 0;
    char set_value[PANEL_GPIO_NUM]; 
    char *path;
    FILE *fp;
    
    for(n = 0; n < PANEL_GPIO_NUM; n++)
    {
        if(gpio_num == gstgpio_value_path[n].gpio_num)
        {
            path = gstgpio_value_path[n].path;
            break;
        }
    }
    if ((fp = fopen(path, "rb+")) == NULL)
    {
        printf("Cannot open gpio value file.\n");
        exit(1);
    }
    if (index)
    {
        rewind(fp);
        strcpy(set_value,"1");
        fwrite(&set_value, sizeof(char), 1, fp);
        fclose(fp);
    }
    else
    {
        rewind(fp);
        strcpy(set_value,"0");
        fwrite(&set_value, sizeof(char), 1, fp);
        fclose(fp);
    }
}		
void _user_export_gpio(MI_U8 gpio_num)
{
    FILE *fp;
    MI_U8 u8strlen;
    char str[10];

    if ((fp = fopen("/sys/class/gpio/export", "ab")) == NULL)
    {
        printf("Cannot open export file.\n");
        exit(1);
    }
    rewind(fp);
    sprintf(str, "%d", gpio_num);
    u8strlen = strlen(str);
    fwrite(str, sizeof(char), u8strlen, fp);
    fclose(fp);
    printf("user export gpio:%d\n",gpio_num);
}	
void _user_set_gpio_direction(MI_U8 gpio_num, char *pdirection)
{
    char path[100];
    FILE *fp;
    MI_U8 u8strlen;

    sprintf(path, "/sys/class/gpio/gpio%d/direction", gpio_num);
    if ((fp = fopen(path, "rb+")) == NULL)
    {
        printf("Cannot open direction file.\n");
        exit(1);
    }
    rewind(fp);
    u8strlen = strlen(pdirection);
    fwrite(pdirection, sizeof(char), u8strlen, fp);
    fclose(fp);
    printf("gpio:%d direction set ok\n",gpio_num);
}	
MI_S32 user_panel_gpio_init(void){
    MI_U8 n;
    char path[100];
    gstgpio_value_path[0].gpio_num = PANEL_RST;
    gstgpio_value_path[1].gpio_num = PANEL_ENABLE_NUM;
    for(n = 0; n < 2; n++)
    {
        _user_export_gpio(gstgpio_value_path[n].gpio_num);
        _user_set_gpio_direction(gstgpio_value_path[n].gpio_num, (char*)"out");
    }
    for(n = 0; n < 2; n++)
    {
        sprintf(path, "/sys/class/gpio/gpio%d/value", gstgpio_value_path[n].gpio_num);
        memcpy(gstgpio_value_path[n].path, path, sizeof(path));
    }
    _user_set_gpio_value(PANEL_ENABLE_NUM, PANEL_ENABLE_ON);
    _user_set_gpio_value(PANEL_RST, 1);
    usleep(10*1000);
    _user_set_gpio_value(PANEL_RST, 0);
    usleep(50*1000);
    _user_set_gpio_value(PANEL_RST, 1);
	usleep(120*1000);
	return MI_SUCCESS;	
}
/* PWM enable */
static int pwm_enable(char pwm)
{
    int fd;
    char path[512];
    memset(path,0,512);
    sprintf(path,"%s/pwm%d/%s",PWMPATHBASE,pwm,"enable");
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf ("\n[%s:%d] Failed enable PWM(%d)\n", __func__, __LINE__,pwm);
        return -1;
    }
    write(fd, "1", 2);
    close(fd);
    return 0;

}
static int pwm_export(char pwm)
{
    int fd;
    char path[512];
    char value[2];
    memset(path,0,512);
    sprintf(path,"%s/%s",PWMPATHBASE,"export");
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf ("\n[%s:%d] Failed export PWM(%d)\n", __func__, __LINE__,pwm);
        return -1;
    }
    sprintf(value,"%d\n",pwm);
    write(fd, value, 2);
    close(fd);
    return 0;
}
static int pwm_unexport(char pwm)
{
    int fd;
    char path[512];
    char value[2];
    memset(path,0,512);
    sprintf(path,"%s/%s",PWMPATHBASE,"unexport");
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf ("\n[%s:%d] Failed unexport PWM(%d)\n", __func__, __LINE__,pwm);
        return -1;
    }
    sprintf(value,"%d\n",pwm);
    write(fd, value, 2);
    close(fd);
    return 0;
}
/* PWM disable */
static int pwm_disable(char pwm)
{
    int fd;
    char path[512];
    memset(path,0,512);
    sprintf(path,"%s/pwm%d/%s",PWMPATHBASE,pwm,"enable");
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        printf ("\n[%s:%d] Failed disable PWM(%d)\n", __func__, __LINE__,pwm);
        return -1;
    }
    write(fd, "0", 2);
    close(fd);
    return 0;

}
int Pwm_Init(char pwm)
{
    if(pwm > MAXPWMINDEX)
    {
        printf("out of pwm number,%d > max(%d)\n",pwm,MAXPWMINDEX);
        return -1;
    }
    return pwm_export(pwm);
}


int Pwm_Deinit(unsigned int pwm)
{
    if(pwm > MAXPWMINDEX)
    {
        printf("out of pwm number,%d > max(%d)\n",pwm,MAXPWMINDEX);
        return -1;
    }
    return pwm_unexport(pwm);
}
/* PWM configuration */
static int pwm_config(char pwm, unsigned int period, unsigned int duty_cycle)
{
	int fd,len_p,len_d;
	char buf_p[MAX_BUF];
	char buf_d[MAX_BUF];
    char path[512];
    memset(path,0,512);
    
	len_p = snprintf(buf_p, sizeof(buf_p), "%d", period);
	len_d = snprintf(buf_d, sizeof(buf_d), "%d", duty_cycle);
    sprintf(path,"%s/pwm%d/%s",PWMPATHBASE,pwm,"period");
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf ("\n[%s:%d] Failed set PWM(%d) period\n", __func__, __LINE__,pwm);
        return -1;
    }
    write(fd, buf_p, len_p);
    close(fd);
    sprintf(path,"%s/pwm%d/%s",PWMPATHBASE,pwm,"duty_cycle");
    fd = open(path, O_WRONLY);
    if (fd < 0)
    {
        printf ("\n[%s:%d] Failed set PWM(%d) duty cycle\n", __func__, __LINE__,pwm);
        return -2;
    }
    write(fd, buf_d, len_d);
    close(fd);
    return 0;
}
	
int SS_PWM_Config(char pwm_index, enum pwm_enable_status bEnable, unsigned int period, unsigned int duty_cycle)
{
    printf("pwm(%d),bEnable(%d),period(%d),duty_cycle(%d)\n",pwm_index,bEnable,period,duty_cycle);
    Pwm_Init(pwm_index);
    if (bEnable != pwm_Disable)
    {
        pwm_config(pwm_index, period, duty_cycle);
        pwm_enable(pwm_index);
    }
    else
    {
        pwm_disable(pwm_index);
    }
    return 0;
}
MI_U32 ST_PanelStart(void)
{
    ST_Panel_Attr_T *pstPanelAttr = g_stPanelAttr;
    MI_DISP_DEV DispDev = 0;
    MI_U32 u32DispWidth = pstPanelAttr->u32Width;
    MI_U32 u32DispHeight = pstPanelAttr->u32Height;    
    MI_PANEL_IntfType_e eLinkType = E_MI_PNL_INTF_TTL;
	
    /************************************************
    Step2:  set disp pub 
    *************************************************/
    MI_DISP_PubAttr_t stPubAttr;
    memset(&stPubAttr, 0x0, sizeof(MI_DISP_PubAttr_t));    
    stPubAttr.u32BgColor = YUYV_BLACK;
    stPubAttr.eIntfType = E_MI_DISP_INTF_TTL;
    stPubAttr.eIntfSync =  E_MI_DISP_OUTPUT_USER;
    STCHECKRESULT(MI_DISP_SetPubAttr(DispDev,  &stPubAttr));
    STCHECKRESULT(MI_DISP_Enable(DispDev));    
    /************************************************
    Step3:  set layer
    *************************************************/
    MI_DISP_LAYER            DispLayer = 0;
    MI_SYS_PixelFormat_e     ePixFormat = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;  
    MI_DISP_VideoLayerAttr_t stLayerAttr;
    memset(&stLayerAttr, 0x0, sizeof(MI_DISP_VideoLayerAttr_t));
    stLayerAttr.stVidLayerDispWin.u16X = 0;
    stLayerAttr.stVidLayerDispWin.u16Y = 0;
    stLayerAttr.stVidLayerDispWin.u16Width = u32DispWidth;
    stLayerAttr.stVidLayerDispWin.u16Height = u32DispHeight;
    stLayerAttr.stVidLayerSize.u16Width = u32DispWidth;
    stLayerAttr.stVidLayerSize.u16Height = u32DispHeight;
    stLayerAttr.ePixFormat = ePixFormat;
    STCHECKRESULT(MI_DISP_BindVideoLayer(DispLayer,DispDev));
    STCHECKRESULT(MI_DISP_SetVideoLayerAttr(DispLayer, &stLayerAttr));
    STCHECKRESULT(MI_DISP_EnableVideoLayer(DispLayer));
    /************************************************
    Step4:  set inputport
    *************************************************/
    MI_U8 u8DispInport = 0;    
    MI_DISP_InputPortAttr_t stInputPortAttr;
    MI_DISP_VidWinRect_t stWinRect;        
    //MI_PANEL_ParamConfig_t stPanelParam;    
    //memset(&stPanelParam, 0x0, sizeof(MI_PANEL_ParamConfig_t));
    memset(&stWinRect, 0x0, sizeof(MI_DISP_VidWinRect_t));
    memset(&stInputPortAttr, 0x0, sizeof(MI_DISP_InputPortAttr_t));
    stInputPortAttr.stDispWin.u16X = 0;
    stInputPortAttr.stDispWin.u16Y = 0;
    stInputPortAttr.stDispWin.u16Width =u32DispWidth;
    stInputPortAttr.stDispWin.u16Height = u32DispHeight;    
    stInputPortAttr.u16SrcWidth =u32DispWidth;
    stInputPortAttr.u16SrcHeight = u32DispHeight;    
    stWinRect.u16Width = u32DispWidth;
    stWinRect.u16Height = u32DispHeight;	
    STCHECKRESULT(MI_DISP_SetInputPortAttr(DispLayer, u8DispInport, &stInputPortAttr));
    STCHECKRESULT(MI_DISP_EnableInputPort(DispLayer, u8DispInport));
    STCHECKRESULT(MI_DISP_SetInputPortSyncMode(DispLayer, u8DispInport, E_MI_DISP_SYNC_MODE_FREE_RUN));

    /************************************************
    Step5:  bind scl->disp
    *************************************************/
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;	
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 2;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId =  0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = SENSOR_RGB_FPS;
    stBindInfo.u32DstFrmrate = SENSOR_RGB_FPS;
    stBindInfo.eBindType = E_MI_SYS_BIND_TYPE_FRAME_BASE;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));

	/************************************************
    Step0:  user_panel_gpio_init(by panel request)
    *************************************************/
	user_panel_gpio_init();	
    /************************************************
    Step1:  panel init
    *************************************************/
    STCHECKRESULT(MI_PANEL_Init(eLinkType));
	SS_PWM_Config(PANEL_BL_PWM_NUM,pwm_Enable,100,PANEL_BL_PWM_ON); 	
	
    return MI_SUCCESS;
}	
MI_U32 ST_PanelStop(void)
{
    MI_U32 DispLayer = 0;
    MI_U32 DispInport = 0;
    MI_U32 DispDev = 0;
    ST_Sys_BindInfo_T stBindInfo;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));
    stBindInfo.stSrcChnPort.eModId = E_MI_MODULE_ID_SCL;
    stBindInfo.stSrcChnPort.u32DevId = 0;
    stBindInfo.stSrcChnPort.u32ChnId = 0;
    stBindInfo.stSrcChnPort.u32PortId = 2;
    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_DISP;
    stBindInfo.stDstChnPort.u32DevId = 0;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;
    stBindInfo.u32SrcFrmrate = SENSOR_RGB_FPS;
    stBindInfo.u32DstFrmrate = SENSOR_RGB_FPS;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));    
    STCHECKRESULT(MI_DISP_DisableInputPort(DispLayer, DispInport));    
    STCHECKRESULT(MI_DISP_DisableVideoLayer(DispLayer));
    STCHECKRESULT(MI_DISP_UnBindVideoLayer(DispLayer, DispDev));    
    STCHECKRESULT(MI_DISP_Disable(DispLayer));
    STCHECKRESULT(MI_PANEL_DeInit(E_MI_PNL_INTF_TTL));
	Pwm_Deinit(PANEL_BL_PWM_NUM);
	
    return 0;
}	
#endif


	
MI_S32 ST_SnrModuleInit(ST_BasicAttr_S *pstBasicAttr, ST_ModSensorAttr_S *pstSensorAttr)
{
	MI_SNR_Res_t stRes;
	MI_U32 u32ChocieRes = 0;

	if (pstBasicAttr->eHDRType != E_MI_ISP_HDR_TYPE_OFF)
	{
		STCHECKRESULT(MI_SNR_SetPlaneMode(pstSensorAttr->ePADId, TRUE));
	}
	else
	{
		STCHECKRESULT(MI_SNR_SetPlaneMode(pstSensorAttr->ePADId, FALSE));
	}

	if (pstSensorAttr->u8ResIdx < 0)
	{
		MI_U32 u32ResCount = 0;
		MI_U8 u8ResIndex = 0;
		
		STCHECKRESULT(MI_SNR_QueryResCount(pstSensorAttr->ePADId, &u32ResCount));
		for(u8ResIndex=0; u8ResIndex < u32ResCount; u8ResIndex++)
	    {
	        MI_SNR_GetRes(pstSensorAttr->ePADId, u8ResIndex, &stRes);
	        printf("index %d, Crop(%d,%d,%d,%d), outputsize(%d,%d), maxfps %d, minfps %d, ResDesc %s\n",
			        u8ResIndex,
			        stRes.stCropRect.u16X, stRes.stCropRect.u16Y, stRes.stCropRect.u16Width,stRes.stCropRect.u16Height,
			        stRes.stOutputSize.u16Width, stRes.stOutputSize.u16Height,
			        stRes.u32MaxFps,stRes.u32MinFps,
			        stRes.strResDesc);
	    }

		printf("choice which resolution use, cnt %d\n", u32ResCount);
	    do
	    {
	        scanf("%d", &u32ChocieRes);
	        ST_Flush();
	        MI_SNR_QueryResCount(pstSensorAttr->ePADId, &u32ResCount);
	        if(u32ChocieRes >= u32ResCount)
	        {
	            printf("choice err res %d > =cnt %d\n", u32ChocieRes, u32ResCount);
	        }
	    } while(u32ChocieRes >= u32ResCount);

		printf("You select %d res\n", u32ChocieRes);

		pstSensorAttr->u8ResIdx = u32ChocieRes;
	}
	else
	{
		ST_DBG("error\n");
	}

	STCHECKRESULT(MI_SNR_SetRes(pstSensorAttr->ePADId, pstSensorAttr->u8ResIdx));
	STCHECKRESULT(MI_SNR_Enable(pstSensorAttr->ePADId));

	// set fps, adjust
	STCHECKRESULT(MI_SNR_GetRes(pstSensorAttr->ePADId, pstSensorAttr->u8ResIdx, &stRes));

	if ((pstSensorAttr->u32Fps > stRes.u32MinFps) &&
		(pstSensorAttr->u32Fps < stRes.u32MaxFps))
	{
		STCHECKRESULT(MI_SNR_SetFps(pstSensorAttr->ePADId, pstSensorAttr->u32Fps));	
	}

    return MI_SUCCESS;
}

MI_S32 ST_SnrModuleUnInit(ST_ModSensorAttr_S *pstSensorAttr)
{
	STCHECKRESULT(MI_SNR_Disable(pstSensorAttr->ePADId));

	return MI_SUCCESS;
}

MI_S32 ST_VifModuleInit(ST_BasicAttr_S *pstBasicAttr, MI_SNR_PADID SnrPadId, ST_ModVifAttr_S *pstVifAttr)
{
    MI_U32 u32PlaneId = 0;

    MI_SNR_PADInfo_t  stPad0Info;
    MI_SNR_PlaneInfo_t stSnrPlane0Info;
    memset(&stPad0Info, 0x0, sizeof(MI_SNR_PADInfo_t));
    memset(&stSnrPlane0Info, 0x0, sizeof(MI_SNR_PlaneInfo_t));

    MI_VIF_GroupAttr_t stGroupAttr;
    memset(&stGroupAttr, 0x0, sizeof(MI_VIF_GroupAttr_t));

    STCHECKRESULT(MI_SNR_GetPadInfo(SnrPadId, &stPad0Info));
    STCHECKRESULT(MI_SNR_GetPlaneInfo(SnrPadId, u32PlaneId, &stSnrPlane0Info));

    stGroupAttr.eIntfMode = (MI_VIF_IntfMode_e)stPad0Info.eIntfMode;
    stGroupAttr.eWorkMode = E_MI_VIF_WORK_MODE_1MULTIPLEX;
    stGroupAttr.eHDRType = (MI_VIF_HDRType_e)pstBasicAttr->eHDRType;
    if(stGroupAttr.eIntfMode == E_MI_VIF_MODE_BT656)
        stGroupAttr.eClkEdge = (MI_VIF_ClkEdge_e)stPad0Info.unIntfAttr.stBt656Attr.eClkEdge;
    else
        stGroupAttr.eClkEdge = E_MI_VIF_CLK_EDGE_DOUBLE;

    //stGroupAttr.u32GroupStitchMask = u32GroupStitchMask;

    STCHECKRESULT(MI_VIF_CreateDevGroup(pstVifAttr->GroupId, &stGroupAttr));

    MI_VIF_DevAttr_t stVifDevAttr;
    memset(&stVifDevAttr, 0x0, sizeof(MI_VIF_DevAttr_t));

    stVifDevAttr.stInputRect.u16X = stSnrPlane0Info.stCapRect.u16X;
    stVifDevAttr.stInputRect.u16Y = stSnrPlane0Info.stCapRect.u16Y;
    stVifDevAttr.stInputRect.u16Width = stSnrPlane0Info.stCapRect.u16Width;
    stVifDevAttr.stInputRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
    if(stSnrPlane0Info.eBayerId >= E_MI_SYS_PIXEL_BAYERID_MAX)
    {
        stVifDevAttr.eInputPixel = stSnrPlane0Info.ePixel;
    }
    else
        stVifDevAttr.eInputPixel = (MI_SYS_PixelFormat_e)RGB_BAYER_PIXEL(stSnrPlane0Info.ePixPrecision, stSnrPlane0Info.eBayerId);

    printf("setchnportattr (%d,%d,%d,%d) \n", stVifDevAttr.stInputRect.u16X, stVifDevAttr.stInputRect.u16Y, stVifDevAttr.stInputRect.u16Width, stVifDevAttr.stInputRect.u16Height);
    STCHECKRESULT(MI_VIF_SetDevAttr(pstVifAttr->DevId, &stVifDevAttr));
    STCHECKRESULT(MI_VIF_EnableDev(pstVifAttr->DevId));

    MI_VIF_OutputPortAttr_t stVifPortInfo;
    memset(&stVifPortInfo, 0, sizeof(MI_VIF_OutputPortAttr_t));

	if ((pstVifAttr->stCapRect.u16X == 0) &&
		(pstVifAttr->stCapRect.u16Y == 0) &&
		(pstVifAttr->stCapRect.u16Width == 0) &&
		(pstVifAttr->stCapRect.u16Height == 0))
	{
		stVifPortInfo.stCapRect.u16X = stSnrPlane0Info.stCapRect.u16X;
	    stVifPortInfo.stCapRect.u16Y = stSnrPlane0Info.stCapRect.u16Y;
	    stVifPortInfo.stCapRect.u16Width =  stSnrPlane0Info.stCapRect.u16Width;
	    stVifPortInfo.stCapRect.u16Height = stSnrPlane0Info.stCapRect.u16Height;
	}
	else
	{
		memcpy(&stVifPortInfo.stCapRect, &pstVifAttr->stCapRect, sizeof(MI_SYS_WindowRect_t));
	}

	if ((pstVifAttr->stDestSize.u16Width == 0) &&
		(pstVifAttr->stDestSize.u16Height == 0))
	{
		stVifPortInfo.stDestSize.u16Width = stSnrPlane0Info.stCapRect.u16Width;
    	stVifPortInfo.stDestSize.u16Height = stSnrPlane0Info.stCapRect.u16Height;
	}
	else
	{
		stVifPortInfo.stDestSize.u16Width = pstVifAttr->stDestSize.u16Width;
		stVifPortInfo.stDestSize.u16Height = pstVifAttr->stDestSize.u16Height;
	}
    
    printf("sensor bayerid %d, bit mode %d \n", stSnrPlane0Info.eBayerId, stSnrPlane0Info.ePixPrecision);
    stVifPortInfo.ePixFormat = stVifDevAttr.eInputPixel;
    //stVifPortInfo.u32FrameModeLineCount for lowlantancy mode
    stVifPortInfo.eFrameRate = E_MI_VIF_FRAMERATE_FULL;

    STCHECKRESULT(MI_VIF_SetOutputPortAttr(pstVifAttr->DevId, pstVifAttr->PortId, &stVifPortInfo));
    STCHECKRESULT(MI_VIF_EnableOutputPort(pstVifAttr->DevId, pstVifAttr->PortId));

    return MI_SUCCESS;
}

MI_S32 ST_VifModuleUnInit(ST_ModVifAttr_S *pstVifAttr)
{
	// disable port
	STCHECKRESULT(MI_VIF_DisableOutputPort(pstVifAttr->DevId, pstVifAttr->PortId));

	// disable dev
	STCHECKRESULT(MI_VIF_DisableDev(pstVifAttr->DevId));

	// disable group
	STCHECKRESULT(MI_VIF_DestroyDevGroup(pstVifAttr->GroupId));
	
	return MI_SUCCESS;
}
MI_BOOL ST_DoSetIqBin(MI_ISP_DEV IspDev, MI_ISP_CHANNEL IspChnId, char *pConfigPath)
{
    MI_ISP_IQ_ParamInitInfoType_t status;
	
    MI_U8 u8ispreadycnt = 0;
    if (NULL == pConfigPath || strlen(pConfigPath) == 0)
    {
        printf("IQ Bin File path NULL!\n");
        return FALSE;
    }
    do
    {
        if(u8ispreadycnt > 100)
        {
            printf("%s:%d, isp ready time out \n", __FUNCTION__, __LINE__);
            u8ispreadycnt = 0;
            break;
        }
        memset(&status, 0, sizeof(status));
        MI_ISP_IQ_GetParaInitStatus(IspDev, IspChnId, &status);
        if(status.stParaAPI.bFlag != 1)
        {
            usleep(300*1000);
            u8ispreadycnt++;
            continue;
        }
        u8ispreadycnt = 0;
        printf("loading api bin...path:%s\n",pConfigPath);
        STCHECKRESULT(MI_ISP_ApiCmdLoadBinFile(IspDev, IspChnId, (char *)pConfigPath, 1234));
        usleep(10*1000);
    }while(!status.stParaAPI.bFlag);

    MI_ISP_AE_FlickerType_e Flicker = E_SS_AE_FLICKER_TYPE_50HZ;
    STCHECKRESULT(MI_ISP_AE_SetFlicker(IspDev, IspChnId, &Flicker));
    printf("MI_ISP_AE_SetFlicker!\n");	

    return MI_SUCCESS;
}

MI_S32 ST_IspModuleinit(ST_BasicAttr_S *pstBasicAttr, ST_ModIspAttr_S *pstIspAttr)
{
    MI_ISP_PORT IspOutPortId = 0;
	MI_U32 u32IspDevStitchMask = E_MI_ISP_DEVICEMASK_ID0;
	MI_ISP_BindSnrId_e u32SensorBindId = E_MI_ISP_SENSOR0;
	MI_U8 i = 0, j = 0;
	MI_ISP_CHANNEL IspChnId = 0;

    MI_ISP_DevAttr_t stCreateDevAttr;
    memset(&stCreateDevAttr, 0x0, sizeof(MI_ISP_DevAttr_t));
    stCreateDevAttr.u32DevStitchMask = u32IspDevStitchMask;
	ExecFunc(MI_ISP_CreateDevice(pstIspAttr->DevId, &stCreateDevAttr), MI_SUCCESS);

    MI_ISP_ChannelAttr_t  stIspChnAttr;
	MI_ISP_ChnParam_t stIspChnParam;
	MI_ISP_OutPortParam_t stIspOutputParam;
	
    memset(&stIspChnAttr, 0x0, sizeof(MI_ISP_ChannelAttr_t));
    stIspChnAttr.u32SensorBindId = u32SensorBindId;
    stIspChnAttr.u32Sync3AType = E_MI_ISP_SYNC3A_AE|E_MI_ISP_SYNC3A_AWB|E_MI_ISP_SYNC3A_IQ;

	for (i = 0; i < pstIspAttr->u32MaxChnNum; i ++)
	{
		IspChnId = i;

		ExecFunc(MI_ISP_CreateChannel(pstIspAttr->DevId, IspChnId, &stIspChnAttr), MI_SUCCESS);

		memset(&stIspChnParam, 0x0, sizeof(MI_ISP_ChnParam_t));
		stIspChnParam.eHDRType = pstBasicAttr->eHDRType;
	    stIspChnParam.e3DNRLevel = pstIspAttr->stChnAttr[i].stChnParam.e3DNRLevel;
		stIspChnParam.bMirror = pstBasicAttr->bMirror;
		stIspChnParam.bFlip = pstBasicAttr->bFlip;
		stIspChnParam.eRot = pstBasicAttr->eRot;
		ExecFunc(MI_ISP_SetChnParam(pstIspAttr->DevId, IspChnId, &stIspChnParam), MI_SUCCESS);
	    ExecFunc(MI_ISP_StartChannel(pstIspAttr->DevId, IspChnId), MI_SUCCESS);

		//
		for (j = 0; j < ST_ISP_MAX_PORT_NUM; j ++)
		{
			if (pstIspAttr->stChnAttr[i].stPortAttr[j].bEnable == 0)
			{
				continue;
			}
			
			IspOutPortId = pstIspAttr->stChnAttr[i].stPortAttr[j].PortId;

			memset(&stIspOutputParam, 0x0, sizeof(MI_ISP_OutPortParam_t));
			stIspOutputParam.ePixelFormat = pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.ePixelFormat;

			if ((pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16X != 0) ||
				(pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Y != 0) ||
				(pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Width != 0) ||
				(pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Height != 0))
			{
				stIspOutputParam.stCropRect.u16X = pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16X;
				stIspOutputParam.stCropRect.u16Y = pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Y;
				stIspOutputParam.stCropRect.u16Width = pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Width;
				stIspOutputParam.stCropRect.u16Height = pstIspAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Height;
			}
			ExecFunc(MI_ISP_SetOutputPortParam(pstIspAttr->DevId, IspChnId, IspOutPortId, &stIspOutputParam), MI_SUCCESS);
		    ExecFunc(MI_ISP_EnableOutputPort(pstIspAttr->DevId, IspChnId, IspOutPortId), MI_SUCCESS);

			// only for debug
			#if 0
			MI_SYS_ChnPort_t stChnPort;
		    stChnPort.eModId = E_MI_MODULE_ID_ISP;
		    stChnPort.u32DevId = pstIspAttr->DevId;
		    stChnPort.u32ChnId = IspChnId;
		    stChnPort.u32PortId = IspOutPortId;
			ExecFunc(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 2, 4), MI_SUCCESS);
			#endif
		}
	}

	ST_Sys_BindInfo_T stBindInfo;
	
	// bind(VIF->ISP)
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));

    stBindInfo.stSrcChnPort.eModId = pstIspAttr->stSrcChnPort.eModId;
    stBindInfo.stSrcChnPort.u32DevId = pstIspAttr->stSrcChnPort.u32DevId;
    stBindInfo.stSrcChnPort.u32ChnId = pstIspAttr->stSrcChnPort.u32ChnId;
    stBindInfo.stSrcChnPort.u32PortId = pstIspAttr->stSrcChnPort.u32PortId;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
    stBindInfo.stDstChnPort.u32DevId = pstIspAttr->DevId;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = pstIspAttr->eBindType;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
	
    return MI_SUCCESS;
}

MI_S32 ST_IspModuleUninit(ST_ModIspAttr_S *pstIspAttr)
{
	MI_U8 i = 0, j = 0;
	MI_ISP_CHANNEL IspChnId = 0;
	MI_ISP_PORT IspOutPortId = 0;

	for (i = 0; i < pstIspAttr->u32MaxChnNum; i ++)
	{
		IspChnId = i;

		for (j = 0; j < ST_ISP_MAX_PORT_NUM; j ++)
		{
			if (pstIspAttr->stChnAttr[i].stPortAttr[j].bEnable == 0)
			{
				continue;
			}
			
			IspOutPortId = pstIspAttr->stChnAttr[i].stPortAttr[j].PortId;

			// disable port
			ExecFunc(MI_ISP_DisableOutputPort(pstIspAttr->DevId, IspChnId, IspOutPortId), MI_SUCCESS);
		}

		// stop channel
		ExecFunc(MI_ISP_StopChannel(pstIspAttr->DevId, IspChnId), MI_SUCCESS);

		// destory channel
		ExecFunc(MI_ISP_DestroyChannel(pstIspAttr->DevId, IspChnId), MI_SUCCESS);
	}

	// destory device
	ExecFunc(MI_ISP_DestoryDevice(pstIspAttr->DevId), MI_SUCCESS);
	
	return MI_SUCCESS;
}

MI_S32 ST_SclCreateChn(ST_BasicAttr_S *pstBasicAttr, ST_ModSclAttr_S *pstSclAttr)
{
	MI_U8 i = 0, j = 0;
	MI_SCL_CHANNEL SclChnId;
	
	MI_SCL_ChannelAttr_t  stSclChnAttr;
	MI_SYS_WindowRect_t stInputCropWin;
	MI_SCL_ChnParam_t stChnParam;

	MI_SCL_PORT SclPortId = 0;
	MI_SCL_OutPortParam_t stSclOutputParam;

	for (i = 0; i < pstSclAttr->u32MaxChnNum; i ++)
	{
		SclChnId = pstSclAttr->stChnAttr[i].ChnId;
		
		memset(&stSclChnAttr, 0x0, sizeof(MI_SCL_ChannelAttr_t));
    	STCHECKRESULT(MI_SCL_CreateChannel(pstSclAttr->DevId, SclChnId, &stSclChnAttr));

		memset(&stInputCropWin, 0, sizeof(MI_SYS_WindowRect_t));
		if ((pstSclAttr->stChnAttr[i].stInputCrop.u16X != 0) ||
			(pstSclAttr->stChnAttr[i].stInputCrop.u16Y != 0) ||
			(pstSclAttr->stChnAttr[i].stInputCrop.u16Width != 0) ||
			(pstSclAttr->stChnAttr[i].stInputCrop.u16Height != 0))
		{
			stInputCropWin = pstSclAttr->stChnAttr[i].stInputCrop;
		}
		STCHECKRESULT(MI_SCL_SetInputPortCrop(pstSclAttr->DevId, SclChnId, &stInputCropWin));

		memset(&stChnParam, 0, sizeof(MI_SCL_ChnParam_t));

		// this chip scl module not support rotation
		// stChnParam.eRot = pstBasicAttr->eRot;
		STCHECKRESULT(MI_SCL_SetChnParam(pstSclAttr->DevId, SclChnId, &stChnParam));

		STCHECKRESULT(MI_SCL_StartChannel(pstSclAttr->DevId, SclChnId));

		for (j = 0; j < ST_SCL_MAX_PORT_NUM; j ++)
		{
			if (pstSclAttr->stChnAttr[i].stPortAttr[j].bEnable == 0)
			{
				continue;
			}

			SclPortId = pstSclAttr->stChnAttr[i].stPortAttr[j].PortId;
			ST_DBG("SclPortId : %d\n",SclPortId);
			memset(&stSclOutputParam, 0, sizeof(MI_SCL_OutPortParam_t));

			stSclOutputParam.stSCLOutCropRect.u16X = pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16X;
			stSclOutputParam.stSCLOutCropRect.u16Y = pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Y;
			stSclOutputParam.stSCLOutCropRect.u16Width = 
									pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Width;
			stSclOutputParam.stSCLOutCropRect.u16Height = 
									pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Height;

			if ((pstBasicAttr->eRot == E_MI_SYS_ROTATE_NONE) ||
				(pstBasicAttr->eRot == E_MI_SYS_ROTATE_180))
			{
				stSclOutputParam.stSCLOutputSize.u16Width = 
									pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Width;
				stSclOutputParam.stSCLOutputSize.u16Height =
									pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Height;
			}
			else if ((pstBasicAttr->eRot == E_MI_SYS_ROTATE_90) ||
					 (pstBasicAttr->eRot == E_MI_SYS_ROTATE_270))
			{
				stSclOutputParam.stSCLOutputSize.u16Width = 
									pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Height;
				stSclOutputParam.stSCLOutputSize.u16Height =
									pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Width;
			}
			
		    stSclOutputParam.ePixelFormat = pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.ePixelFormat;
            if(SclPortId==0)    
                stSclOutputParam.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
		    else
                stSclOutputParam.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
		    stSclOutputParam.bMirror = 0;
		    stSclOutputParam.bFlip = 0;

			ST_DBG("devId:%d, chnId:%d, portId:%d\n", pstSclAttr->DevId, SclChnId, SclPortId);
			STCHECKRESULT(MI_SCL_SetOutputPortParam(pstSclAttr->DevId, SclChnId, SclPortId, &stSclOutputParam));
			STCHECKRESULT(MI_SCL_EnableOutputPort(pstSclAttr->DevId, SclChnId, SclPortId));

			// only port0
			if (SclPortId == 0)
			{
				// OSD
				MI_RGN_Attr_t stRgnAttr;
				MI_RGN_ChnPortParam_t stChnPortParam;
				MI_RGN_ChnPort_t stChnPort;

				stChnPort.eModId = E_MI_MODULE_ID_SCL;
				stChnPort.s32DevId = pstSclAttr->DevId;
				stChnPort.s32ChnId = SclChnId;
				stChnPort.s32PortId = SclPortId;

				// OSD0
				memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
			    stRgnAttr.eType = E_MI_RGN_TYPE_OSD;
			    stRgnAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_I4;
			    stRgnAttr.stOsdInitParam.stSize.u32Width = 3840;//g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Width;
			    stRgnAttr.stOsdInitParam.stSize.u32Height = 2160;//g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u32Height;
			    ExecFunc(ST_OSD_Create(RGN_OSD_HANDLE0, &stRgnAttr), MI_RGN_OK);
				printf("stSCLOutputSize.u16Width = %d\n",g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Width);
				printf("stSCLOutputSize.u32Height = %d\n",g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Height);				
				memset(&stChnPortParam, 0, sizeof(MI_RGN_ChnPortParam_t));
		        stChnPortParam.bShow = TRUE;
		        stChnPortParam.stPoint.u32X = 10;
		        stChnPortParam.stPoint.u32Y = 10;
		        stChnPortParam.unPara.stOsdChnPort.u32Layer = 100;
		        stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
		        stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
		        stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xAA;			
		        ExecFunc(MI_RGN_AttachToChn(0, RGN_OSD_HANDLE0, &stChnPort, &stChnPortParam), MI_RGN_OK);

				// OSD1
				#if 0
				memset(&stRgnAttr, 0, sizeof(MI_RGN_Attr_t));
			    stRgnAttr.eType = E_MI_RGN_TYPE_OSD;
			    stRgnAttr.stOsdInitParam.ePixelFmt = E_MI_RGN_PIXEL_FORMAT_I4;
			    stRgnAttr.stOsdInitParam.stSize.u32Width = RGN_OSD_TIME_WIDTH;
			    stRgnAttr.stOsdInitParam.stSize.u32Height = RGN_OSD_TIME_HEIGHT;
			    ExecFunc(ST_OSD_Create(RGN_OSD_HANDLE1, &stRgnAttr), MI_RGN_OK);

				memset(&stChnPortParam, 0, sizeof(MI_RGN_ChnPortParam_t));
		        stChnPortParam.bShow = TRUE;
		        stChnPortParam.stPoint.u32X = 
					pstSclAttr->stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Width - RGN_OSD_TIME_WIDTH - 100;
		        stChnPortParam.stPoint.u32Y = 10;
		        stChnPortParam.unPara.stOsdChnPort.u32Layer = 100;
		        stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.eAlphaMode = E_MI_RGN_PIXEL_ALPHA;
		        stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8BgAlpha = 0;
		        stChnPortParam.unPara.stOsdChnPort.stOsdAlphaAttr.stAlphaPara.stArgb1555Alpha.u8FgAlpha = 0xAA;			
		        ExecFunc(MI_RGN_AttachToChn(0, RGN_OSD_HANDLE1, &stChnPort, &stChnPortParam), MI_RGN_OK);
				#endif
			}
			
			#if 0
			MI_SYS_ChnPort_t stChnPort;
		    stChnPort.eModId = E_MI_MODULE_ID_SCL;
		    stChnPort.u32DevId = pstSclAttr->DevId;
		    stChnPort.u32ChnId = SclChnId;
		    stChnPort.u32PortId = SclPortId;
			STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 1, 5));
			#endif
		}
	}

	return MI_SUCCESS;
}

MI_S32 ST_SclModuleInit(ST_BasicAttr_S *pstBasicAttr, ST_ModSclAttr_S *pstSclAttr, ST_StreamAttr_S *pstStreamAttr)
{
	MI_SCL_DevAttr_t stCreateDevAttr;
	MI_U8 i = 0;
	MI_BOOL bNeedConfigPrivatePool = FALSE;
	
    memset(&stCreateDevAttr, 0x0, sizeof(MI_SCL_DevAttr_t));

	// default map relation
	// port0->E_MI_SCL_HWSCL0, port1->E_MI_SCL_HWSCL1
	// port2->E_MI_SCL_HWSCL2, port3->E_MI_SCL_HWSCL3
    stCreateDevAttr.u32NeedUseHWOutPortMask = E_MI_SCL_HWSCL0 | E_MI_SCL_HWSCL1 |
												E_MI_SCL_HWSCL2 | E_MI_SCL_HWSCL3;

    STCHECKRESULT(MI_SCL_CreateDevice(pstSclAttr->DevId, &stCreateDevAttr));

	// when scl->venc is E_MI_SYS_BIND_TYPE_HW_RING mode, config the privae pool
	for (i = 0; i < ST_STREAM_MAX_NUM; i ++)
	{
		if (pstStreamAttr->stVencAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING)
		{
			bNeedConfigPrivatePool = TRUE;
			break;
		}
	}

	if (bNeedConfigPrivatePool == TRUE)
	{
		MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
	    memset(&stPrivPoolCfg, 0x0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
	    stPrivPoolCfg.bCreate = TRUE;
	    stPrivPoolCfg.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
	    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule = E_MI_MODULE_ID_SCL;
	    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = pstSclAttr->DevId;
	    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = pstStreamAttr->stVencAttr[i].u32Width;
	    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = pstStreamAttr->stVencAttr[i].u32Height;
	    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = pstStreamAttr->stVencAttr[i].u32Height/2;
		//strcpy((char *)stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u8MMAHeapName, "mma_heap_name0");
		STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg));
	}

	STCHECKRESULT(ST_SclCreateChn(pstBasicAttr, pstSclAttr));

	ST_Sys_BindInfo_T stBindInfo;

	// bind(ISP->SCL)
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));

    stBindInfo.stSrcChnPort.eModId = pstSclAttr->stSrcChnPort.eModId;
    stBindInfo.stSrcChnPort.u32DevId = pstSclAttr->stSrcChnPort.u32DevId;
    stBindInfo.stSrcChnPort.u32ChnId = pstSclAttr->stSrcChnPort.u32ChnId;
    stBindInfo.stSrcChnPort.u32PortId = pstSclAttr->stSrcChnPort.u32PortId;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
    stBindInfo.stDstChnPort.u32DevId = pstSclAttr->DevId;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = pstSclAttr->eBindType;
    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
	
	return MI_SUCCESS;
}

MI_S32 ST_SclModuleUnInit(ST_ModSclAttr_S *pstSclAttr)
{
	MI_U8 i = 0, j =0;
	MI_SCL_CHANNEL SclChnId;
	MI_SCL_PORT SclPortId;

	for (i = 0; i < pstSclAttr->u32MaxChnNum; i ++)
	{
		SclChnId = pstSclAttr->stChnAttr[i].ChnId;
		
		for (j = 0; j < ST_SCL_MAX_PORT_NUM; j ++)
		{
			if (pstSclAttr->stChnAttr[i].stPortAttr[j].bEnable == 0)
			{
				continue;
			}

			SclPortId = pstSclAttr->stChnAttr[i].stPortAttr[j].PortId;

			STCHECKRESULT(MI_SCL_DisableOutputPort(pstSclAttr->DevId, SclChnId, SclPortId));
		}

		// stop
		STCHECKRESULT(MI_SCL_StopChannel(pstSclAttr->DevId, SclChnId));

		// destory
		STCHECKRESULT(MI_SCL_DestroyChannel(pstSclAttr->DevId, SclChnId));
	}

	//destory device
	STCHECKRESULT(MI_SCL_DestroyDevice(pstSclAttr->DevId));
	
	return MI_SUCCESS;
}

MI_S32 ST_VencModuleInit(ST_BasicAttr_S *pstBasicAttr, ST_StreamAttr_S *pstStreamAttr)
{
	MI_U8 i = 0;
	MI_VENC_ChnAttr_t stChnAttr;
	MI_VENC_DEV VeDevId;
	MI_VENC_InputSourceConfig_t stVencSourceCfg;
	MI_U32 u32Width;
	MI_U32 u32Height;

	memset(&pstStreamAttr->stVencDevH26X, 0, sizeof(ST_ModVencDevInfo_S));
	pstStreamAttr->stVencDevH26X.bCreate = FALSE;
	pstStreamAttr->stVencDevH26X.VeDev = MI_VENC_DEV_ID_H264_H265_0;

	memset(&pstStreamAttr->stVencDevMJPEG, 0, sizeof(ST_ModVencDevInfo_S));
	pstStreamAttr->stVencDevMJPEG.bCreate = FALSE;
	pstStreamAttr->stVencDevMJPEG.VeDev = MI_VENC_DEV_ID_JPEG_0;

	for (i = 0; i < pstStreamAttr->u32MaxNum; i ++)
	{
		if ((pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H264E) ||
			(pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H265E))
		{
			pstStreamAttr->stVencDevH26X.stInitParam.u32MaxWidth = 
				MAX(pstStreamAttr->stVencDevH26X.stInitParam.u32MaxWidth, pstStreamAttr->stVencAttr[i].u32Width);
			pstStreamAttr->stVencDevH26X.stInitParam.u32MaxHeight = 
				MAX(pstStreamAttr->stVencDevH26X.stInitParam.u32MaxHeight, pstStreamAttr->stVencAttr[i].u32Height);
		}

		if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
		{
			pstStreamAttr->stVencDevMJPEG.stInitParam.u32MaxWidth = 
				MAX(pstStreamAttr->stVencDevMJPEG.stInitParam.u32MaxWidth, pstStreamAttr->stVencAttr[i].u32Width);
			pstStreamAttr->stVencDevMJPEG.stInitParam.u32MaxHeight = 
				MAX(pstStreamAttr->stVencDevMJPEG.stInitParam.u32MaxHeight, pstStreamAttr->stVencAttr[i].u32Height);
		}
	}

	for (i = 0; i < pstStreamAttr->u32MaxNum; i ++)
	{
		memset(&stChnAttr, 0, sizeof(MI_VENC_ChnAttr_t));

		if ((pstBasicAttr->eRot == E_MI_SYS_ROTATE_NONE) ||
			(pstBasicAttr->eRot == E_MI_SYS_ROTATE_180))
		{
			u32Width = pstStreamAttr->stVencAttr[i].u32Width;
			u32Height = pstStreamAttr->stVencAttr[i].u32Height;
		}
		else if ((pstBasicAttr->eRot == E_MI_SYS_ROTATE_90) ||
					(pstBasicAttr->eRot == E_MI_SYS_ROTATE_270))
		{
			u32Width = pstStreamAttr->stVencAttr[i].u32Height;
			u32Height = pstStreamAttr->stVencAttr[i].u32Width;
		}
		
		if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H264E)
		{
			stChnAttr.stVeAttr.stAttrH264e.u32PicWidth = u32Width;
	        stChnAttr.stVeAttr.stAttrH264e.u32PicHeight = u32Height;
	        stChnAttr.stVeAttr.stAttrH264e.u32MaxPicWidth = u32Width;
			stChnAttr.stVeAttr.stAttrH264e.u32MaxPicHeight = u32Height;
	        stChnAttr.stVeAttr.stAttrH264e.u32BFrameNum = 2;
	        stChnAttr.stVeAttr.stAttrH264e.bByFrame = TRUE;

			if (pstStreamAttr->stVencAttr[i].eRcMode == E_MI_VENC_RC_MODE_H264CBR)
			{
				stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H264CBR;
				stChnAttr.stRcAttr.stAttrH264Cbr.u32BitRate = pstStreamAttr->stVencAttr[i].u32MaxBitRate;
				stChnAttr.stRcAttr.stAttrH264Cbr.u32FluctuateLevel = 0;
				stChnAttr.stRcAttr.stAttrH264Cbr.u32Gop = pstStreamAttr->stVencAttr[i].u32Gop;
				stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateNum = pstStreamAttr->stVencAttr[i].u32Fps;
				stChnAttr.stRcAttr.stAttrH264Cbr.u32SrcFrmRateDen = 1;
				stChnAttr.stRcAttr.stAttrH264Cbr.u32StatTime = 0;
			}
		}
		else if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H265E)
		{
			stChnAttr.stVeAttr.stAttrH265e.u32PicWidth = u32Width;
	        stChnAttr.stVeAttr.stAttrH265e.u32PicHeight = u32Height;
	        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicWidth = u32Width;
	        stChnAttr.stVeAttr.stAttrH265e.u32MaxPicHeight = u32Height;
	        stChnAttr.stVeAttr.stAttrH265e.bByFrame = TRUE;

			if (pstStreamAttr->stVencAttr[i].eRcMode == E_MI_VENC_RC_MODE_H265CBR)
			{
				stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_H265CBR;
				stChnAttr.stRcAttr.stAttrH265Cbr.u32BitRate = pstStreamAttr->stVencAttr[i].u32MaxBitRate;
				stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateNum = pstStreamAttr->stVencAttr[i].u32Fps;
				stChnAttr.stRcAttr.stAttrH265Cbr.u32SrcFrmRateDen = 1;
				stChnAttr.stRcAttr.stAttrH265Cbr.u32Gop = pstStreamAttr->stVencAttr[i].u32Gop;
				stChnAttr.stRcAttr.stAttrH265Cbr.u32FluctuateLevel = 0;
				stChnAttr.stRcAttr.stAttrH265Cbr.u32StatTime = 0;
			}
		}
		else if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
		{
			stChnAttr.stVeAttr.eType = E_MI_VENC_MODTYPE_JPEGE;
	        stChnAttr.stVeAttr.stAttrJpeg.u32PicWidth = u32Width;
	        stChnAttr.stVeAttr.stAttrJpeg.u32PicHeight = u32Height;
	        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicWidth = u32Width;
	        stChnAttr.stVeAttr.stAttrJpeg.u32MaxPicHeight = u32Height;

	        stChnAttr.stRcAttr.eRcMode = E_MI_VENC_RC_MODE_MJPEGFIXQP;
	        stChnAttr.stRcAttr.stAttrMjpegFixQp.u32Qfactor = 50;
	        stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateNum = pstStreamAttr->stVencAttr[i].u32Fps;
	        stChnAttr.stRcAttr.stAttrMjpegFixQp.u32SrcFrmRateDen = 1;
		}

		// create dev
		if ((pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H264E) ||
			(pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H265E))
		{
			if (pstStreamAttr->stVencDevH26X.bCreate == FALSE)
			{
				STCHECKRESULT(MI_VENC_CreateDev(pstStreamAttr->stVencDevH26X.VeDev, &pstStreamAttr->stVencDevH26X.stInitParam));

				pstStreamAttr->stVencDevH26X.bCreate = TRUE;
			}

			VeDevId = pstStreamAttr->stVencDevH26X.VeDev;
		}
		else if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
		{
			if (pstStreamAttr->stVencDevMJPEG.bCreate == FALSE)
			{
				STCHECKRESULT(MI_VENC_CreateDev(pstStreamAttr->stVencDevMJPEG.VeDev, &pstStreamAttr->stVencDevMJPEG.stInitParam));

				pstStreamAttr->stVencDevMJPEG.bCreate = TRUE;
			}

			VeDevId = pstStreamAttr->stVencDevMJPEG.VeDev;
		}

		// config private pool
		if ((pstStreamAttr->stVencAttr[i].stSrcChnPort.eModId == E_MI_MODULE_ID_VENC) &&
			(pstStreamAttr->stVencAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING))
		{
			MI_SYS_GlobalPrivPoolConfig_t stPrivPoolCfg;
		    memset(&stPrivPoolCfg, 0x0, sizeof(MI_SYS_GlobalPrivPoolConfig_t));
		    stPrivPoolCfg.bCreate = TRUE;
		    stPrivPoolCfg.eConfigType = E_MI_SYS_PER_DEV_PRIVATE_RING_POOL;
		    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.eModule = E_MI_MODULE_ID_VENC;
		    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u32Devid = VeDevId;
		    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxWidth = u32Width;
		    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16MaxHeight = u32Height;
		    stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u16RingLine = u32Height;
			strcpy((char *)stPrivPoolCfg.uConfig.stpreDevPrivRingPoolConfig.u8MMAHeapName, "mma_heap_name0");
			STCHECKRESULT(MI_SYS_ConfigPrivateMMAPool(0, &stPrivPoolCfg));
		}

		stChnAttr.stVeAttr.eType = pstStreamAttr->stVencAttr[i].eType;
		STCHECKRESULT(MI_VENC_CreateChn(VeDevId, pstStreamAttr->stVencAttr[i].VeChn, &stChnAttr));

		if ((pstStreamAttr->stVencAttr[i].eBindType == E_MI_SYS_BIND_TYPE_HW_RING) &&
			(pstStreamAttr->stVencAttr[i].stSrcChnPort.eModId == E_MI_MODULE_ID_SCL))
		{
			memset(&stVencSourceCfg, 0, sizeof(MI_VENC_InputSourceConfig_t));

			stVencSourceCfg.eInputSrcBufferMode = E_MI_VENC_INPUT_MODE_RING_UNIFIED_DMA;
			STCHECKRESULT(MI_VENC_SetInputSourceConfig(VeDevId, pstStreamAttr->stVencAttr[i].VeChn, &stVencSourceCfg));
		}

		STCHECKRESULT(MI_VENC_StartRecvPic(VeDevId, pstStreamAttr->stVencAttr[i].VeChn));

		// bind
		ST_Sys_BindInfo_T stBindInfo;

		// bind (SCL->VENC)
		memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));

		stBindInfo.stSrcChnPort.eModId = pstStreamAttr->stVencAttr[i].stSrcChnPort.eModId;
	    stBindInfo.stSrcChnPort.u32DevId = pstStreamAttr->stVencAttr[i].stSrcChnPort.u32DevId;
	    stBindInfo.stSrcChnPort.u32ChnId = pstStreamAttr->stVencAttr[i].stSrcChnPort.u32ChnId;
	    stBindInfo.stSrcChnPort.u32PortId = pstStreamAttr->stVencAttr[i].stSrcChnPort.u32PortId;

	    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
	    stBindInfo.stDstChnPort.u32DevId = VeDevId;
	    stBindInfo.stDstChnPort.u32ChnId = pstStreamAttr->stVencAttr[i].VeChn;
	    stBindInfo.stDstChnPort.u32PortId = 0;

	    stBindInfo.u32SrcFrmrate = 30;
	    stBindInfo.u32DstFrmrate = pstStreamAttr->stVencAttr[i].u32Fps;
	    stBindInfo.eBindType = pstStreamAttr->stVencAttr[i].eBindType;

		ST_DBG("src(%d-%d-%d-%d)  dst(%d-%d-%d-%d)  %d...\n", stBindInfo.stSrcChnPort.eModId, stBindInfo.stSrcChnPort.u32DevId,
        stBindInfo.stSrcChnPort.u32ChnId, stBindInfo.stSrcChnPort.u32PortId,
        stBindInfo.stDstChnPort.eModId, stBindInfo.stDstChnPort.u32DevId, stBindInfo.stDstChnPort.u32ChnId,
        stBindInfo.stDstChnPort.u32PortId, stBindInfo.eBindType);

	    STCHECKRESULT(ST_Sys_Bind(&stBindInfo));
	}

	return MI_SUCCESS;
}

MI_S32 ST_VencModuleUnInit(ST_StreamAttr_S *pstStreamAttr)
{
	MI_U8 i = 0;
	MI_VENC_DEV VeDevId;

	for (i = 0; i < pstStreamAttr->u32MaxNum; i ++)
	{
		if ((pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H264E) ||
			(pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H265E))
		{
			VeDevId = MI_VENC_DEV_ID_H264_H265_0;
		}
		else if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
		{
			VeDevId = MI_VENC_DEV_ID_JPEG_0;
		}

		// stop
		STCHECKRESULT(MI_VENC_StopRecvPic(VeDevId, pstStreamAttr->stVencAttr[i].VeChn));

		// destory
		STCHECKRESULT(MI_VENC_DestroyChn(VeDevId, pstStreamAttr->stVencAttr[i].VeChn));
	}

	// destory device
	if (pstStreamAttr->stVencDevH26X.bCreate == TRUE)
	{
		STCHECKRESULT(MI_VENC_DestroyDev(pstStreamAttr->stVencDevH26X.VeDev));
		pstStreamAttr->stVencDevH26X.bCreate = FALSE;
	}

	if (pstStreamAttr->stVencDevMJPEG.bCreate == TRUE)
	{
		STCHECKRESULT(MI_VENC_DestroyDev(pstStreamAttr->stVencDevMJPEG.VeDev));
		pstStreamAttr->stVencDevMJPEG.bCreate = FALSE;
	}

	return MI_SUCCESS;
}

void *ST_OpenStream(char const *szStreamName, void *arg)
{
	MI_U32 i = 0;
    MI_S32 s32Ret = MI_SUCCESS;
	ST_StreamAttr_S *pstStreamAttr = &g_pstPipeLineCase->stStreamAttr;
	ST_ModVencAttr_S *pstVencAttr = NULL;
	
    for(i = 0; i < ST_STREAM_MAX_NUM; i ++)
    {
        if(!strncmp(szStreamName, pstStreamAttr->stVencAttr[i].szURLTag,
                    strlen(szStreamName)))
        {
            break;
        }
    }

    if(i >= ST_STREAM_MAX_NUM)
    {
        ST_ERR("not found this stream, \"%s\"", szStreamName);
        return NULL;
    }

	pstVencAttr = &pstStreamAttr->stVencAttr[i];
	if ((pstVencAttr->eType == E_MI_VENC_MODTYPE_H264E) ||
		(pstVencAttr->eType == E_MI_VENC_MODTYPE_H265E))
	{
		// this chip only one H26X dev
		s32Ret = MI_VENC_RequestIdr(MI_VENC_DEV_ID_H264_H265_0, pstVencAttr->VeChn, TRUE);

		if(MI_SUCCESS != s32Ret)
	    {
	        ST_WARN("request IDR fail, error:%x\n", s32Ret);
	    }
	}

    ST_DBG("open stream \"%s\" success, chn:%d\n", szStreamName, pstVencAttr->VeChn);

    return pstVencAttr;
}

int ST_VideoReadStream(void *handle, unsigned char *ucpBuf, int BufLen, struct timeval *p_Timestamp, void *arg)
{
    MI_SYS_BufInfo_t stBufInfo;
    MI_S32 s32Ret = MI_SUCCESS;
    MI_S32 len = 0;
    MI_VENC_Stream_t stStream;
    MI_VENC_Pack_t stPack;
    MI_VENC_ChnStat_t stStat;
    MI_VENC_CHN vencChn ;
    MI_VENC_DEV DevId =0;

    if(handle == NULL)
    {
        return -1;
    }

    ST_ModVencAttr_S *pstVencAttr = (ST_ModVencAttr_S *)handle;

    vencChn = pstVencAttr->VeChn;
	if ((pstVencAttr->eType == E_MI_VENC_MODTYPE_H264E) ||
		(pstVencAttr->eType == E_MI_VENC_MODTYPE_H265E))
	{
		// this chip only one H26X dev
		DevId = MI_VENC_DEV_ID_H264_H265_0;
	}
	else if (pstVencAttr->eType == E_MI_VENC_MODTYPE_JPEGE)
	{
		// this chip only one MJPEG dev
		DevId = MI_VENC_DEV_ID_JPEG_0;
	}

    memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
    memset(&stStream, 0, sizeof(stStream));
    memset(&stPack, 0, sizeof(stPack));
    stStream.pstPack = &stPack;
    stStream.u32PackCount = 1;
    s32Ret = MI_VENC_Query(DevId, vencChn, &stStat);

    if(s32Ret != MI_SUCCESS || stStat.u32CurPacks == 0)
    {
        return 0;
    }

    s32Ret = MI_VENC_GetStream(DevId, vencChn, &stStream, 40);

    if(MI_SUCCESS == s32Ret)
    {
        len = stStream.pstPack[0].u32Len;
        memcpy(ucpBuf, stStream.pstPack[0].pu8Addr, MIN(len, BufLen));

        s32Ret = MI_VENC_ReleaseStream(DevId, vencChn, &stStream);
        if(s32Ret != MI_SUCCESS)
        {
            ST_WARN("RELEASE venc buffer fail\n");
        }

        return len;
    }

    return 0;
}

int ST_CloseStream(void *handle, void *arg)
{
    if(handle == NULL)
    {
        return -1;
    }

	ST_ModVencAttr_S *pstVencAttr = (ST_ModVencAttr_S *)handle;

    ST_DBG("close \"%s\" success\n", pstVencAttr->szURLTag);

    return 0;
}

MI_S32 ST_RtspServerStart(ST_StreamAttr_S *pstStreamAttr)
{
	unsigned int rtspServerPortNum = RTSP_LISTEN_PORT;
    int iRet = 0;
    char *urlPrefix = NULL;
    unsigned int i = 0;
    ServerMediaSession *mediaSession = NULL;
    ServerMediaSubsession *subSession = NULL;
    Live555RTSPServer *pRTSPServer = NULL;

    pRTSPServer = new Live555RTSPServer();

    if(pRTSPServer == NULL)
    {
        ST_ERR("malloc error\n");
        return -1;
    }

	iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);

    while(iRet < 0)
    {
        rtspServerPortNum++;

        if(rtspServerPortNum > 65535)
        {
            ST_INFO("Failed to create RTSP server: %s\n", pRTSPServer->getResultMsg());
            delete pRTSPServer;
            pRTSPServer = NULL;
            return -2;
        }

        iRet = pRTSPServer->SetRTSPServerPort(rtspServerPortNum);
    }

	urlPrefix = pRTSPServer->rtspURLPrefix();
	printf("=================URL===================\n");
	for (i = 0; i < pstStreamAttr->u32MaxNum; i ++)
	{
		printf("%s%s\n", urlPrefix, pstStreamAttr->stVencAttr[i].szURLTag);
	}
    printf("=================URL===================\n");

	for (i = 0; i < pstStreamAttr->u32MaxNum; i ++)
	{
		pRTSPServer->createServerMediaSession(mediaSession,
                                  			  pstStreamAttr->stVencAttr[i].szURLTag,
                                          	  NULL, NULL);

		if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H264E)
	    {
	        subSession = WW_H264VideoFileServerMediaSubsession::createNew(
	                         *(pRTSPServer->GetUsageEnvironmentObj()),
	                         pstStreamAttr->stVencAttr[i].szURLTag,
	                         ST_OpenStream,
	                         ST_VideoReadStream,
	                         ST_CloseStream, 30);
	    }
	    else if(pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H265E)
	    {
	        subSession = WW_H265VideoFileServerMediaSubsession::createNew(
	                         *(pRTSPServer->GetUsageEnvironmentObj()),
	                         pstStreamAttr->stVencAttr[i].szURLTag,
	                         ST_OpenStream,
	                         ST_VideoReadStream,
	                         ST_CloseStream, 30);
	    }
	    else if(pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
	    {
	        subSession = WW_JPEGVideoFileServerMediaSubsession::createNew(
	                         *(pRTSPServer->GetUsageEnvironmentObj()),
	                         pstStreamAttr->stVencAttr[i].szURLTag,
	                         ST_OpenStream,
	                         ST_VideoReadStream,
	                         ST_CloseStream, 30);
	    }

	    pRTSPServer->addSubsession(mediaSession, subSession);
	    pRTSPServer->addServerMediaSession(mediaSession);
	}
	
	pRTSPServer->Start();

    g_pRTSPServer = pRTSPServer;

    return 0;
}

MI_S32 ST_RtspServerStop(void)
{
    if(g_pRTSPServer)
    {
        g_pRTSPServer->Join();
        delete g_pRTSPServer;
        g_pRTSPServer = NULL;
    }

    return 0;
}

void ST_DumpBufinfoToFile(const char *pszDir, ST_DumpAttr_S *pstDumpAttr, MI_SYS_BufInfo_t *pstBufInfo)
{
	int fd = -1;
	char szFilePath[256] = {0,};
	static int count = 0;
	MI_U32 u32FrameSize = 0;

	snprintf(szFilePath, sizeof(szFilePath) - 1, "%s/%s_dev%d_chn%d_port%d_%04d.dat", pszDir, 
						(pstDumpAttr->stSrcChnPort.eModId == E_MI_MODULE_ID_SCL) ? "scl" :
						(pstDumpAttr->stSrcChnPort.eModId == E_MI_MODULE_ID_ISP) ? "isp" : "unknow", 
						pstDumpAttr->stSrcChnPort.u32DevId, pstDumpAttr->stSrcChnPort.u32ChnId,
						pstDumpAttr->stSrcChnPort.u32PortId, count ++);

	//ST_DBG("bCrcCheck: %d\n", pstBufInfo->bCrcCheck);
	// fd = open(szFilePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	fd = open("./temp.yuv", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0)
    {
        perror("open file fail\n");
		return;
    }

	// frame data
	if(pstBufInfo->eBufType == E_MI_SYS_BUFDATA_FRAME)
	{
		// yuv420
		u32FrameSize = pstBufInfo->stFrameData.u32Stride[0] * pstBufInfo->stFrameData.u16Height * 3 /2;
		int size = write(fd, (MI_U8 *)pstBufInfo->stFrameData.pVirAddr[0], u32FrameSize);
		ST_DBG("size is %d", size);
	}

	close(fd);
}


void ST_dumps_image(MI_SYS_BufInfo_t *pstBufInfo)
{
	int fd = -1;
	static int count = 0;
	MI_U32 u32FrameSize = 0;

	//ST_DBG("bCrcCheck: %d\n", pstBufInfo->bCrcCheck);
	// fd = open(szFilePath, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	fd = open("./temp.yuv", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd < 0)
    {
        perror("open file fail\n");
		return;
    }
	ST_DBG("pstBufInfo->eBufType:%d\n",pstBufInfo->eBufType);
	// frame data
	if(pstBufInfo->eBufType == E_MI_SYS_BUFDATA_FRAME)
	{
		// argb8888
		// u32FrameSize = pstBufInfo->stFrameData.u32Stride[0] * pstBufInfo->stFrameData.u16Height * 4;
		// u32FrameSize = pstBufInfo->stFrameData.u16Width * pstBufInfo->stFrameData.u16Height * 4;
		int size = write(fd, (MI_U8 *)pstBufInfo->stFrameData.pVirAddr[0], pstBufInfo->stFrameData.u32BufSize);
		ST_DBG("size is %d\n", size);
	}

	close(fd);
}

void *ST_FileoutDumpProc(void * args)
{
	ST_DBG("ST_FileoutDumpProc\n");
	ST_FileOutAttr_S *pstFileOutAttr = (ST_FileOutAttr_S *)args;
	fd_set read_fds;
    struct timeval TimeoutVal;
	int maxFd = pstFileOutAttr->maxFd;
	MI_S32 s32Ret = MI_SUCCESS;
	MI_U8 i = 0;
	ST_DumpAttr_S *pstDumpAttr = NULL;
	MI_SYS_ChnPort_t stChnPort;
	MI_SYS_BufInfo_t stBufInfo;
	MI_SYS_BUF_HANDLE stBufHandle;
	
	while(pstFileOutAttr->bRunFlag)
	{
		FD_ZERO(&read_fds);
        FD_SET(maxFd, &read_fds);

		TimeoutVal.tv_sec  = 1;
        TimeoutVal.tv_usec = 0;

        s32Ret = select(maxFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		if(s32Ret < 0)
        {
            ST_ERR("select failed!\n");
            //  usleep(10 * 1000);
            continue;
        }
        else if(s32Ret == 0)
        {
            ST_ERR("get frame time out\n");
            //usleep(10 * 1000);
            continue;
        }
		else
		{
			// find avlid fd
			pstDumpAttr = NULL;
			for (i = 0; i < pstFileOutAttr->u32MaxNum; i ++)
			{
				if (FD_ISSET(pstFileOutAttr->stDumpAttr[i].fd, &read_fds))
				{
					pstDumpAttr = &pstFileOutAttr->stDumpAttr[i];
					break;
				}
			}

			if (pstDumpAttr == NULL)
			{
				ST_ERR("invalid dump attr\n");
				continue;
			}

			memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
			memset(&stBufInfo, 0, sizeof(MI_SYS_BufInfo_t));

			stChnPort.eModId = pstDumpAttr->stSrcChnPort.eModId;
			stChnPort.u32DevId = pstDumpAttr->stSrcChnPort.u32DevId;
			stChnPort.u32ChnId = pstDumpAttr->stSrcChnPort.u32ChnId;
			stChnPort.u32PortId = pstDumpAttr->stSrcChnPort.u32PortId;
			s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnPort, &stBufInfo, &stBufHandle);
			if(MI_SUCCESS != s32Ret)
            {
                ST_ERR("MI_SYS_ChnOutputPortGetBuf err, %x\n", s32Ret);
                continue;
            }

			if (pstDumpAttr->bDump)
			{
				// save to file
				ST_DumpBufinfoToFile(pstFileOutAttr->szDir, pstDumpAttr, &stBufInfo);
			}

			MI_SYS_ChnOutputPortPutBuf(stBufHandle);
		}
	}

	return NULL;
}

MI_S32 ST_FileoutDumpInit(ST_FileOutAttr_S *pstFileOutAttr)
{
	MI_U8 i = 0;
	MI_SYS_ChnPort_t stChnPort;
	int fd = -1;

	pstFileOutAttr->maxFd = 0;
	
	for (i = 0; i < pstFileOutAttr->u32MaxNum; i ++)
	{
		ST_DBG("dump file!\n");
		memset(&stChnPort, 0, sizeof(MI_SYS_ChnPort_t));
	    stChnPort.eModId = pstFileOutAttr->stDumpAttr[i].stSrcChnPort.eModId;
	    stChnPort.u32DevId = pstFileOutAttr->stDumpAttr[i].stSrcChnPort.u32DevId;
	    stChnPort.u32ChnId = pstFileOutAttr->stDumpAttr[i].stSrcChnPort.u32ChnId;
	    stChnPort.u32PortId = pstFileOutAttr->stDumpAttr[i].stSrcChnPort.u32PortId;
		STCHECKRESULT(MI_SYS_SetChnOutputPortDepth(0, &stChnPort, 2, 6));

		STCHECKRESULT(MI_SYS_GetFd(&stChnPort, &fd));

		pstFileOutAttr->stDumpAttr[i].fd = fd;
		pstFileOutAttr->maxFd = MAX(pstFileOutAttr->maxFd, fd);
	}

	// create thread
	if (pstFileOutAttr->u32MaxNum > 0)
	{
		pstFileOutAttr->bRunFlag = TRUE;
		pthread_create(&pstFileOutAttr->pt, NULL, ST_FileoutDumpProc, (void *)pstFileOutAttr);	
	}
	
	return MI_SUCCESS;
}

MI_S32 ST_FileoutDumpUnInit(ST_FileOutAttr_S *pstFileOutAttr)
{
	pstFileOutAttr->bRunFlag = FALSE;
	pthread_join(pstFileOutAttr->pt, NULL);

	return MI_SUCCESS;
}

void *ST_UpdateRgnOsdProc(void *args)
{
    time_t now = 0;
    struct tm *tm = NULL;
    char szTime[64];
    int len = 0;
    struct timeval tv;
    MI_RGN_CanvasInfo_t* pstCanvasInfo = NULL;
    ST_Point_T stPoint;
    time_t defaultInterval = 980 * 1000;
	time_t timeTmp = 0;
    struct timeval timeBegin, timeEnd;
	ST_OSDAttr_S *pstOSDAttr = (ST_OSDAttr_S *)args;
	int i = 0;

    while(pstOSDAttr->bRunFlag)
    {
        tv.tv_sec = 0;
        tv.tv_usec = defaultInterval;

        if(0 == select(0, NULL, NULL, NULL, &tv))
        {
            // time out
            gettimeofday(&timeBegin, NULL);

            now = time(NULL);

            if((tm = localtime(&now)) == NULL)
            {
                printf("localtime error\n");
                return NULL;
            }

            memset(szTime, 0, sizeof(szTime));
            len = 0;

            len += sprintf(szTime + len, "%d-", tm->tm_year + 1900);
            len += sprintf(szTime + len, "%02d-", tm->tm_mon + 1);
            len += sprintf(szTime + len, "%02d ", tm->tm_mday);
            len += sprintf(szTime + len, "%02d:", tm->tm_hour);
            len += sprintf(szTime + len, "%02d:", tm->tm_min);
            len += sprintf(szTime + len, "%02d", tm->tm_sec);

            stPoint.u32X = 0;
            stPoint.u32Y = 0;

            CANVAS_LOCK;

			for (i = 0; i < ST_OSD_MAX_NUM; i ++)
			{
				if (pstOSDAttr->hHandle[i] == INVALID_RGN_HANDLE)
				{
					continue;
				}

				(void)ST_OSD_GetCanvasInfo(pstOSDAttr->hHandle[i], &pstCanvasInfo);

	            (void)ST_OSD_Clear(pstOSDAttr->hHandle[i], NULL);

	            // (void)ST_OSD_DrawText(pstOSDAttr->hHandle[i], stPoint, szTime, RGB2PIXEL1555(255,0,0), DMF_Font_Size_48x48);
	            (void)ST_OSD_DrawText(pstOSDAttr->hHandle[i], stPoint, szTime, I4_RED, DMF_Font_Size_48x48);

	            (void)ST_OSD_Update(pstOSDAttr->hHandle[i]);
			}
            CANVAS_UNLOCK;

            gettimeofday(&timeEnd, NULL);

            //defaultInterval = 1000 * 1000 - ((timeEnd.tv_sec * 1000000 + timeEnd.tv_usec) -
             //                   (timeBegin.tv_sec * 1000000 + timeBegin.tv_usec));
             timeTmp = (timeEnd.tv_sec * 1000000 + timeEnd.tv_usec) - (timeBegin.tv_sec * 1000000 + timeBegin.tv_usec);
			if(timeTmp > 0 && timeTmp < 1000 * 1000)
			{
				defaultInterval = 1000 * 1000 - timeTmp;
			}
			else if(timeTmp >= 1000 * 1000)
			{
				defaultInterval = 10 * 1000;
			}
			else
			{
				defaultInterval = 980 * 1000;
			}
           	// ST_DBG("defaultInterval:%d, szTime:%s\n", defaultInterval, szTime);
        }
    }

    return NULL;
}

MI_S32 ST_OSDUpdateInit(ST_OSDAttr_S *pstOSDAttr)
{
	MI_U8 i = 0;

	for (i = 0; i < ST_OSD_MAX_NUM; i ++)
	{
		pstOSDAttr->hHandle[i] = INVALID_RGN_HANDLE;
	}

	//
	pstOSDAttr->hHandle[0] = RGN_OSD_HANDLE0;
	
	pstOSDAttr->bRunFlag = TRUE;
	pthread_create(&pstOSDAttr->pt, NULL, ST_UpdateRgnOsdProc, (void *)pstOSDAttr);
	
	return MI_SUCCESS;
}

MI_S32 ST_OSDUpdateUnInit(ST_OSDAttr_S *pstOSDAttr)
{
	pstOSDAttr->bRunFlag = TRUE;
	pthread_join(pstOSDAttr->pt, NULL);

	return MI_SUCCESS;
}

MI_S32 ST_BaseModuleInit()
{
	ST_PipeLineCase_S *pstPipeLineCase = g_pstPipeLineCase;

	// sys
	STCHECKRESULT(ST_SysModuleInit());

	// osd
	STCHECKRESULT(ST_OSD_Init());
	
	// sensor
	STCHECKRESULT(ST_SnrModuleInit(&pstPipeLineCase->stBasicAttr,
									&pstPipeLineCase->stSensorAttr));

	// vif
	STCHECKRESULT(ST_VifModuleInit(&pstPipeLineCase->stBasicAttr,
									pstPipeLineCase->stSensorAttr.ePADId,
									&pstPipeLineCase->stVifAttr));

	// isp
	STCHECKRESULT(ST_IspModuleinit(&pstPipeLineCase->stBasicAttr,
									&pstPipeLineCase->stIspAttr));

	// ST_DBG("pass isp\n");
	// scl
	STCHECKRESULT(ST_SclModuleInit(&pstPipeLineCase->stBasicAttr, &pstPipeLineCase->stSclAttr,
									&pstPipeLineCase->stStreamAttr));

	ST_DBG("pass venc\n");
	// venc
	STCHECKRESULT(ST_VencModuleInit(&pstPipeLineCase->stBasicAttr,
									&pstPipeLineCase->stStreamAttr));
	ST_DBG("pass rtsp\n");
	// rtsp start
	ST_RtspServerStart(&pstPipeLineCase->stStreamAttr);

	// // file to out
	// STCHECKRESULT(ST_FileoutDumpInit(&pstPipeLineCase->stFileOutAttr));

	// osd thread
	// STCHECKRESULT(ST_OSDUpdateInit(&pstPipeLineCase->stOSDAttr));

	// misc
	if (pstPipeLineCase->stBasicAttr.bOpenIqserver)
	{
		STCHECKRESULT(MI_IQSERVER_Open());
	}
	if(g_load_iq_bin)
	{
		ST_DoSetIqBin(0, 0, g_IspBinPath);
	}		
	return MI_SUCCESS;
}

MI_S32 ST_BaseModuleUnbind(ST_PipeLineCase_S *pstPipeLineCase)
{
	ST_Sys_BindInfo_T stBindInfo;
	MI_U8 i = 0;
	
	// unbind venc
	ST_StreamAttr_S *pstStreamAttr = &pstPipeLineCase->stStreamAttr;
	MI_VENC_DEV VeDevId;
	for (i = 0; i < pstStreamAttr->u32MaxNum; i ++)
	{
		if ((pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H264E) ||
			(pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_H265E))
		{
			VeDevId = MI_VENC_DEV_ID_H264_H265_0;
		}
		else if (pstStreamAttr->stVencAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE)
		{
			VeDevId = MI_VENC_DEV_ID_JPEG_0;
		}
		
		// unbind
		memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));

		stBindInfo.stSrcChnPort.eModId = pstStreamAttr->stVencAttr[i].stSrcChnPort.eModId;
	    stBindInfo.stSrcChnPort.u32DevId = pstStreamAttr->stVencAttr[i].stSrcChnPort.u32DevId;
	    stBindInfo.stSrcChnPort.u32ChnId = pstStreamAttr->stVencAttr[i].stSrcChnPort.u32ChnId;
	    stBindInfo.stSrcChnPort.u32PortId = pstStreamAttr->stVencAttr[i].stSrcChnPort.u32PortId;

	    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_VENC;
	    stBindInfo.stDstChnPort.u32DevId = VeDevId;
	    stBindInfo.stDstChnPort.u32ChnId = pstStreamAttr->stVencAttr[i].VeChn;
	    stBindInfo.stDstChnPort.u32PortId = 0;

	    stBindInfo.u32SrcFrmrate = 30;
	    stBindInfo.u32DstFrmrate = pstStreamAttr->stVencAttr[i].u32Fps;
	    stBindInfo.eBindType = pstStreamAttr->stVencAttr[i].eBindType;
	    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
	}

	// unbind(ISP->SCL)
	ST_ModSclAttr_S *pstSclAttr = &pstPipeLineCase->stSclAttr;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));

    stBindInfo.stSrcChnPort.eModId = pstSclAttr->stSrcChnPort.eModId;
    stBindInfo.stSrcChnPort.u32DevId = pstSclAttr->stSrcChnPort.u32DevId;
    stBindInfo.stSrcChnPort.u32ChnId = pstSclAttr->stSrcChnPort.u32ChnId;
    stBindInfo.stSrcChnPort.u32PortId = pstSclAttr->stSrcChnPort.u32PortId;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_SCL;
    stBindInfo.stDstChnPort.u32DevId = pstSclAttr->DevId;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = pstSclAttr->eBindType;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));

	// unbind(VIF->ISP)
	ST_ModIspAttr_S *pstIspAttr = &pstPipeLineCase->stIspAttr;
    memset(&stBindInfo, 0x0, sizeof(ST_Sys_BindInfo_T));

    stBindInfo.stSrcChnPort.eModId = pstIspAttr->stSrcChnPort.eModId;
    stBindInfo.stSrcChnPort.u32DevId = pstIspAttr->stSrcChnPort.u32DevId;
    stBindInfo.stSrcChnPort.u32ChnId = pstIspAttr->stSrcChnPort.u32ChnId;
    stBindInfo.stSrcChnPort.u32PortId = pstIspAttr->stSrcChnPort.u32PortId;

    stBindInfo.stDstChnPort.eModId = E_MI_MODULE_ID_ISP;
    stBindInfo.stDstChnPort.u32DevId = pstIspAttr->DevId;
    stBindInfo.stDstChnPort.u32ChnId = 0;
    stBindInfo.stDstChnPort.u32PortId = 0;

    stBindInfo.u32SrcFrmrate = 30;
    stBindInfo.u32DstFrmrate = 30;
    stBindInfo.eBindType = pstIspAttr->eBindType;
    STCHECKRESULT(ST_Sys_UnBind(&stBindInfo));
	
	return MI_SUCCESS;
}

MI_S32 ST_BaseModuleUnInit()
{
	ST_PipeLineCase_S *pstPipeLineCase = g_pstPipeLineCase;

	// rtsp stop
	ST_RtspServerStop();

	// unbind all
	STCHECKRESULT(ST_BaseModuleUnbind(pstPipeLineCase));
	
	// release venc
	STCHECKRESULT(ST_VencModuleUnInit(&pstPipeLineCase->stStreamAttr));

	// // release scl
	// STCHECKRESULT(ST_SclModuleUnInit(&pstPipeLineCase->stSclAttr));

	// release isp
	STCHECKRESULT(ST_IspModuleUninit(&pstPipeLineCase->stIspAttr));

	//release vif
	STCHECKRESULT(ST_VifModuleUnInit(&pstPipeLineCase->stVifAttr));
	
	// release sensor
	STCHECKRESULT(ST_SnrModuleUnInit(&pstPipeLineCase->stSensorAttr));
	
	// release sys
	STCHECKRESULT(ST_SysModuleUnInit());

	return MI_SUCCESS;
}

MI_S32 ST_LoadIni(char *pIniPath)
{
	ST_PipeLineCase_S *pstPipeLineCase = NULL;
	MI_U8 i = 0, j = 0;
	char szBuf[64];
	char *pString = NULL;
	
	ST_DBG("inipath %s \n", pIniPath);

	pstPipeLineCase = (ST_PipeLineCase_S *)malloc(sizeof(ST_PipeLineCase_S));
	if (pstPipeLineCase == NULL)
	{
		ST_ERR("malloc %d bytes fail\n", sizeof(ST_PipeLineCase_S));
		return -1;
	}
	memset(pstPipeLineCase, 0, sizeof(ST_PipeLineCase_S));
	
    dictionary *pstDict = iniparser_load(pIniPath);

	// basic
	pstPipeLineCase->stBasicAttr.eHDRType = (MI_ISP_HDRType_e)iniparser_getint(pstDict, "basic:eHDRType", 0);
	pstPipeLineCase->stBasicAttr.bMirror = iniparser_getint(pstDict, "basic:bMirror", 0);
	pstPipeLineCase->stBasicAttr.bFlip = iniparser_getint(pstDict, "basic:bFlip", 0);
	pstPipeLineCase->stBasicAttr.eRot = (MI_SYS_Rotate_e)iniparser_getint(pstDict, "basic:eRot", 0);
	pstPipeLineCase->stBasicAttr.bOpenIqserver = (MI_SYS_Rotate_e)iniparser_getint(pstDict, "basic:bOpenIqserver", 0);
	printf("sensor info: \n");
	printf(" eHDRType: %d\n", pstPipeLineCase->stBasicAttr.eHDRType);
	printf(" bMirror: %d\n", pstPipeLineCase->stBasicAttr.bMirror);
	printf(" bFlip: %d\n", pstPipeLineCase->stBasicAttr.bFlip);
	printf(" eRot: %d\n", pstPipeLineCase->stBasicAttr.eRot);
	printf(" bOpenIqserver: %d\n", pstPipeLineCase->stBasicAttr.bOpenIqserver);
#ifdef ISP_ALGO_ENABLE
	pString = iniparser_getstring(pstDict, "basic:IqBinPath", "/customer/imx415_iqfile.bin");
	if (pString != NULL)
	{
		memset(g_IspBinPath, 0, sizeof(g_IspBinPath));
		snprintf(g_IspBinPath, sizeof(g_IspBinPath) - 1, "%s", pString);
	}
#endif	
	// sensor
	pstPipeLineCase->stSensorAttr.ePADId = iniparser_getint(pstDict, "sensor:ePADId", 0);
	pstPipeLineCase->stSensorAttr.u8ResIdx = iniparser_getint(pstDict, "sensor:u8ResIdx", 0);
	pstPipeLineCase->stSensorAttr.u32Fps = iniparser_getint(pstDict, "sensor:u32Fps", 0);
	printf("\r\n\r\nsensor info: \n");
	printf(" ePADId: %d\n", pstPipeLineCase->stSensorAttr.ePADId);
	printf(" u8ResIdx: %d\n", pstPipeLineCase->stSensorAttr.u8ResIdx);
	printf(" u32Fps: %d\n", pstPipeLineCase->stSensorAttr.u32Fps);

	// vif
	pstPipeLineCase->stVifAttr.GroupId = iniparser_getint(pstDict, "vif:GroupId", 0);
	pstPipeLineCase->stVifAttr.DevId = iniparser_getint(pstDict, "vif:DevId", 0);
	pstPipeLineCase->stVifAttr.PortId = iniparser_getint(pstDict, "vif:PortId", 0);
	pstPipeLineCase->stVifAttr.stCapRect.u16X = iniparser_getint(pstDict, "vif:stCapRect.u16X", 0);
	pstPipeLineCase->stVifAttr.stCapRect.u16Y = iniparser_getint(pstDict, "vif:stCapRect.u16Y", 0);
	pstPipeLineCase->stVifAttr.stCapRect.u16Width = iniparser_getint(pstDict, "vif:stCapRect.u16Width", 0);
	pstPipeLineCase->stVifAttr.stCapRect.u16Height = iniparser_getint(pstDict, "vif:stCapRect.u16Height", 0);
	pstPipeLineCase->stVifAttr.stDestSize.u16Width = iniparser_getint(pstDict, "vif:stDestSize.u16Width", 0);
	pstPipeLineCase->stVifAttr.stDestSize.u16Height = iniparser_getint(pstDict, "vif:stDestSize.u16Height", 0);
	printf("\r\n\r\nvif info: \n");
	printf(" GroupId: %d\n", pstPipeLineCase->stVifAttr.GroupId);
	printf(" DevId: %d\n", pstPipeLineCase->stVifAttr.DevId);
	printf(" PortId: %d\n", pstPipeLineCase->stVifAttr.PortId);
	printf(" stCapRect.u16X: %d\n", pstPipeLineCase->stVifAttr.stCapRect.u16X);
	printf(" stCapRect.u16Y: %d\n", pstPipeLineCase->stVifAttr.stCapRect.u16Y);
	printf(" stCapRect.u16Width: %d\n", pstPipeLineCase->stVifAttr.stCapRect.u16Width);
	printf(" stDestSize.u16Width: %d\n", pstPipeLineCase->stVifAttr.stDestSize.u16Width);
	printf(" stDestSize.u16Height: %d\n", pstPipeLineCase->stVifAttr.stDestSize.u16Height);

	// isp
	pstPipeLineCase->stIspAttr.DevId = iniparser_getint(pstDict, "isp:DevId", 0);
	pstPipeLineCase->stIspAttr.u32MaxChnNum = iniparser_getint(pstDict, "isp:u32MaxChnNum", 0);
	pstPipeLineCase->stIspAttr.stSrcChnPort.eModId = (MI_ModuleId_e)iniparser_getint(pstDict, "isp:stSrcChnPort.eModId", 0);
	pstPipeLineCase->stIspAttr.stSrcChnPort.u32DevId = iniparser_getint(pstDict, "isp:stSrcChnPort.u32DevId", 0);
	pstPipeLineCase->stIspAttr.stSrcChnPort.u32ChnId = iniparser_getint(pstDict, "isp:stSrcChnPort.u32ChnId", 0);
	pstPipeLineCase->stIspAttr.stSrcChnPort.u32PortId = iniparser_getint(pstDict, "isp:stSrcChnPort.u32PortId", 0);
	pstPipeLineCase->stIspAttr.eBindType = (MI_SYS_BindType_e)iniparser_getint(pstDict, "isp:eBindType", 0);
	for (i = 0; i < pstPipeLineCase->stIspAttr.u32MaxChnNum; i ++)
	{
		if (i >= ST_ISP_MAX_CHN_NUM)
		{
			ST_ERR("isp max chn num is %d\n", ST_ISP_MAX_CHN_NUM);
			pstPipeLineCase->stIspAttr.u32MaxChnNum = ST_ISP_MAX_CHN_NUM;
			break;
		}

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_ChnId", i);
		pstPipeLineCase->stIspAttr.stChnAttr[i].ChnId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16X", i);
		pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16X = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16Y", i);
		pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16Y = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16Width", i);
		pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16Width = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16Height", i);
		pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16Height = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stChnParam.e3DNRLevel", i);
		pstPipeLineCase->stIspAttr.stChnAttr[i].stChnParam.e3DNRLevel = (MI_ISP_3DNR_Level_e)iniparser_getint(pstDict, szBuf, 0);

		for (j = 0; j < ST_ISP_MAX_PORT_NUM; j ++)
		{
			pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].PortId = j;

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_bEnable", i, j);
			pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].bEnable = iniparser_getint(pstDict, szBuf, 0);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16X", i, j);
			pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16X
														= iniparser_getint(pstDict, szBuf, 0);
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16Y", i, j);
			pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Y
														= iniparser_getint(pstDict, szBuf, 0);
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16Width", i, j);
			pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Width
														= iniparser_getint(pstDict, szBuf, 0);
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16Height", i, j);
			pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Height
														= iniparser_getint(pstDict, szBuf, 0);
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.ePixelFormat", i, j);
			pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.ePixelFormat
														= (MI_SYS_PixelFormat_e)iniparser_getint(pstDict, szBuf, 0);
		}
	}
	printf("\r\n\r\nisp dev info: \n");
	printf(" ePADId: %d\n", pstPipeLineCase->stIspAttr.DevId);
	printf(" u32MaxChnNum: %d\n", pstPipeLineCase->stIspAttr.u32MaxChnNum);
	printf(" stSrcChnPort.eModId: %d\n", pstPipeLineCase->stIspAttr.stSrcChnPort.eModId);
	printf(" stSrcChnPort.u32DevId: %d\n", pstPipeLineCase->stIspAttr.stSrcChnPort.u32DevId);
	printf(" stSrcChnPort.u32ChnId: %d\n", pstPipeLineCase->stIspAttr.stSrcChnPort.u32ChnId);
	printf(" stSrcChnPort.u32PortId: %d\n", pstPipeLineCase->stIspAttr.stSrcChnPort.u32PortId);
	printf(" eBindType: %d\n", pstPipeLineCase->stIspAttr.eBindType);
	for (i = 0; i < pstPipeLineCase->stIspAttr.u32MaxChnNum; i ++)
	{
		printf("\r\n");
		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_ChnId", i);
		printf(" %s:%d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].ChnId);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16X", i);
		printf(" %s:%d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16X);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16Y", i);
		printf(" %s:%d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16Y);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16Width", i);
		printf(" %s:%d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16Width);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stInputCrop.u16Height", i);
		printf(" %s:%d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stInputCrop.u16Height);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_stChnParam.e3DNRLevel", i);
		printf(" %s:%d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stChnParam.e3DNRLevel);

		for (j = 0; j < ST_ISP_MAX_PORT_NUM; j ++)
		{
			printf("\r\n");
		
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_PortId", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].PortId);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_bEnable", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].bEnable);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16X", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16X);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16Y", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Y);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16Width", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Width);
			
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.stCropRect.u16Height", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stCropRect.u16Height);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "isp:chn%d_port%d_stOutPortParam.ePixelFormat", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stIspAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.ePixelFormat);
		}
	}
	
	// scl
	pstPipeLineCase->stSclAttr.DevId = iniparser_getint(pstDict, "scl:DevId", 0);
	pstPipeLineCase->stSclAttr.u32MaxChnNum = iniparser_getint(pstDict, "scl:u32MaxChnNum", 0);
	pstPipeLineCase->stSclAttr.stSrcChnPort.eModId = (MI_ModuleId_e)iniparser_getint(pstDict, "scl:stSrcChnPort.eModId", 0);
	pstPipeLineCase->stSclAttr.stSrcChnPort.u32DevId = iniparser_getint(pstDict, "scl:stSrcChnPort.u32DevId", 0);
	pstPipeLineCase->stSclAttr.stSrcChnPort.u32ChnId = iniparser_getint(pstDict, "scl:stSrcChnPort.u32ChnId", 0);
	pstPipeLineCase->stSclAttr.stSrcChnPort.u32PortId = iniparser_getint(pstDict, "scl:stSrcChnPort.u32PortId", 0);
	pstPipeLineCase->stSclAttr.eBindType = (MI_SYS_BindType_e)iniparser_getint(pstDict, "scl:eBindType", 0);
	for (i = 0; i < pstPipeLineCase->stSclAttr.u32MaxChnNum; i ++)
	{
		if (i >= ST_SCL_MAX_CHN_NUM)
		{
			ST_ERR("scl max chn num is %d\n", ST_SCL_MAX_CHN_NUM);
			pstPipeLineCase->stSclAttr.u32MaxChnNum = ST_SCL_MAX_CHN_NUM;
			break;
		}

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_ChnId", i);
		pstPipeLineCase->stSclAttr.stChnAttr[i].ChnId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16X", i);
		pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16X = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16Y", i);
		pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16Y = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16Width", i);
		pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16Width = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16Height", i);
		pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16Height = iniparser_getint(pstDict, szBuf, 0);

		for (j = 0; j < ST_SCL_MAX_PORT_NUM; j ++)
		{
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].PortId = j;

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_bEnable", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].bEnable = iniparser_getint(pstDict, szBuf, 0);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16X", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16X
														= iniparser_getint(pstDict, szBuf, 0);
			
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16Y", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Y
														= iniparser_getint(pstDict, szBuf, 0);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16Width", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Width
														= iniparser_getint(pstDict, szBuf, 0);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16Height", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Height
														= iniparser_getint(pstDict, szBuf, 0);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutputSize.u16Width", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Width
														= iniparser_getint(pstDict, szBuf, 0);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutputSize.u16Height", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Height
														= iniparser_getint(pstDict, szBuf, 0);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.ePixelFormat", i, j);
			pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.ePixelFormat 
														= (MI_SYS_PixelFormat_e)iniparser_getint(pstDict, szBuf, 0);
		}
	}
	printf("\r\n\r\nscl module info: \n");
	printf(" ePADId: %d\n", pstPipeLineCase->stSclAttr.DevId);
	printf(" u32MaxChnNum: %d\n", pstPipeLineCase->stSclAttr.u32MaxChnNum);
	printf(" stSrcChnPort.eModId: %d\n", pstPipeLineCase->stSclAttr.stSrcChnPort.eModId);
	printf(" stSrcChnPort.u32DevId: %d\n", pstPipeLineCase->stSclAttr.stSrcChnPort.u32DevId);
	printf(" stSrcChnPort.u32ChnId: %d\n", pstPipeLineCase->stSclAttr.stSrcChnPort.u32ChnId);
	printf(" stSrcChnPort.u32PortId: %d\n", pstPipeLineCase->stSclAttr.stSrcChnPort.u32PortId);
	printf(" eBindType: %d\n", pstPipeLineCase->stSclAttr.eBindType);
	for (i = 0; i < pstPipeLineCase->stSclAttr.u32MaxChnNum; i ++)
	{
		printf("\r\n");
		
		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_ChnId", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].ChnId);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16X", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16X);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16Y", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16Y);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16Width", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16Width);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_stInputCrop.u16Height", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stInputCrop.u16Height);

		for (j = 0; j < ST_SCL_MAX_PORT_NUM; j ++)
		{
			printf("\r\n");
		
			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_PortId", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].PortId);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_bEnable", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].bEnable);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16X", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16X);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16Y", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Y);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16Width", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Width);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutCropRect.u16Height", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutCropRect.u16Height);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutputSize.u16Width", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Width);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.stSCLOutputSize.u16Height", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.stSCLOutputSize.u16Height);

			memset(szBuf, 0, sizeof(szBuf));
			snprintf(szBuf, sizeof(szBuf) - 1, "scl:chn%d_port%d_stOutPortParam.ePixelFormat", i, j);
			printf(" %s: %d\n", szBuf, pstPipeLineCase->stSclAttr.stChnAttr[i].stPortAttr[j].stOutPortParam.ePixelFormat);
		}
	}

	// venc && rtsp
	pstPipeLineCase->stStreamAttr.u32MaxNum = iniparser_getint(pstDict, "stream:u32MaxNum", 0);
	for (i = 0; i < pstPipeLineCase->stStreamAttr.u32MaxNum; i ++)
	{
		if (i >= ST_STREAM_MAX_NUM)
		{
			ST_ERR("stream max chn num is %d\n", ST_STREAM_MAX_NUM);
			pstPipeLineCase->stStreamAttr.u32MaxNum = ST_STREAM_MAX_NUM;
			break;
		}

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.eModId", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.eModId = (MI_ModuleId_e)iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.u32DevId", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.u32DevId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.u32ChnId", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.u32ChnId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.u32PortId", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.u32PortId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.eBindType", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].eBindType = (MI_SYS_BindType_e)iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.VeChn", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].VeChn = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Width", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Width = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Height", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Height = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.eType", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].eType = (MI_VENC_ModType_e)iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.eRcMode", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].eRcMode = (MI_VENC_RcMode_e)iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Fps", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Fps = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Gop", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Gop = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32MaxBitRate", i);
		pstPipeLineCase->stStreamAttr.stVencAttr[i].u32MaxBitRate = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_szURLTag", i);
		pString = iniparser_getstring(pstDict, szBuf, "video0");
		if (pString != NULL)
		{
			memset(pstPipeLineCase->stStreamAttr.stVencAttr[i].szURLTag, 0, 
					sizeof(pstPipeLineCase->stStreamAttr.stVencAttr[i].szURLTag));
			snprintf(pstPipeLineCase->stStreamAttr.stVencAttr[i].szURLTag, 
					sizeof(pstPipeLineCase->stStreamAttr.stVencAttr[i].szURLTag) - 1, "%s", pString);
		}
	}
	printf("\r\n\r\nstream info: \n");
	printf(" u32MaxNum: %d\n", pstPipeLineCase->stStreamAttr.u32MaxNum);
	for (i = 0; i < pstPipeLineCase->stStreamAttr.u32MaxNum; i ++)
	{
		printf("\r\n");
	
		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.eModId", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.eModId);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.u32DevId", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.u32DevId);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.u32ChnId", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.u32ChnId);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.stSrcChnPort.u32PortId", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].stSrcChnPort.u32PortId);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.eBindType", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].eBindType);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.VeChn", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].VeChn);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Width", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Width);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Height", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Height);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.eType", i);
		printf(" %s: %s\n", szBuf, (pstPipeLineCase->stStreamAttr.stVencAttr[i].eType == E_MI_VENC_MODTYPE_H264E) ? "H264" :
								   	(pstPipeLineCase->stStreamAttr.stVencAttr[i].eType == E_MI_VENC_MODTYPE_H265E) ? "H265" :
								   	(pstPipeLineCase->stStreamAttr.stVencAttr[i].eType == E_MI_VENC_MODTYPE_JPEGE) ? "MJPEG" : "ERR");

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.eRcMode", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].eRcMode);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Fps", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Fps);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32Gop", i);
		printf(" %s: %d\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].u32Gop);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_stVencAttr.u32MaxBitRate", i);
		printf(" %s: %d kbps\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].u32MaxBitRate / 1024);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "stream:stream%d_szURLTag", i);
		printf(" %s: %s\n", szBuf, pstPipeLineCase->stStreamAttr.stVencAttr[i].szURLTag);
	}

	// save module data to file
	pstPipeLineCase->stFileOutAttr.u32MaxNum = iniparser_getint(pstDict, "file_out:u32MaxNum", 0);
	pString = iniparser_getstring(pstDict, "file_out:szDir", "./");
	if (pString != NULL)
	{
		memset(pstPipeLineCase->stFileOutAttr.szDir, 0, sizeof(pstPipeLineCase->stFileOutAttr.szDir));
		snprintf(pstPipeLineCase->stFileOutAttr.szDir, 
				sizeof(pstPipeLineCase->stFileOutAttr.szDir) - 1, "%s", pString);
	}
	for (i = 0; i < pstPipeLineCase->stFileOutAttr.u32MaxNum; i ++)
	{
		if (i >= ST_FILE_OUT_MAX_NUM)
		{
			ST_ERR("file out max num is %d\n", ST_FILE_OUT_MAX_NUM);
			pstPipeLineCase->stFileOutAttr.u32MaxNum = ST_FILE_OUT_MAX_NUM;
			break;
		}

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "file_out:file_out%d_stDumpAttr.stSrcChnPort.eModId", i);
		pstPipeLineCase->stFileOutAttr.stDumpAttr[i].stSrcChnPort.eModId = (MI_ModuleId_e)iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "file_out:file_out%d_stDumpAttr.stSrcChnPort.u32DevId", i);
		pstPipeLineCase->stFileOutAttr.stDumpAttr[i].stSrcChnPort.u32DevId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "file_out:file_out%d_stDumpAttr.stSrcChnPort.u32ChnId", i);
		pstPipeLineCase->stFileOutAttr.stDumpAttr[i].stSrcChnPort.u32ChnId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "file_out:file_out%d_stDumpAttr.stSrcChnPort.u32PortId", i);
		pstPipeLineCase->stFileOutAttr.stDumpAttr[i].stSrcChnPort.u32PortId = iniparser_getint(pstDict, szBuf, 0);

		memset(szBuf, 0, sizeof(szBuf));
		snprintf(szBuf, sizeof(szBuf) - 1, "file_out:file_out%d_stDumpAttr.bDump", i);
		pstPipeLineCase->stFileOutAttr.stDumpAttr[i].bDump = iniparser_getint(pstDict, szBuf, 0);
	}
	

	// algo_detection
	pstPipeLineCase->stIPUAttr.detect_enable = iniparser_getint(pstDict, "ipu:detect_enable", 0);
	pstPipeLineCase->stIPUAttr.detect_type = iniparser_getint(pstDict, "ipu:detect_type", 99);
	pstPipeLineCase->stIPUAttr.model_path = iniparser_getstring(pstDict, "ipu:model_path", "./");
	pstPipeLineCase->stIPUAttr.detect_threshold = iniparser_getdouble(pstDict, "ipu:detect_threshold", 0.6);
	pstPipeLineCase->stIPUAttr.print_message = iniparser_getint(pstDict, "ipu:print_message", 99);

	g_pstPipeLineCase = pstPipeLineCase;
	g_pstDict = pstDict;
	
	return MI_SUCCESS;
}

void ST_UnLoadIni()
{
	if (g_pstPipeLineCase != NULL)
	{
		free(g_pstPipeLineCase);
		g_pstPipeLineCase = NULL;
	}

	if (g_pstDict != NULL)
	{
		iniparser_freedict(g_pstDict);
		g_pstDict = NULL;
	}
}

void ST_HandleSig(MI_S32 signo)
{
    if(signo == SIGINT)
    {
        ST_INFO("catch Ctrl + C, exit normally\n");

        g_bExit = TRUE;
    }
}

pthread_t     g_detectThread;
pthread_t     g_alertThread;
MI_S32        s32DivpFd;
MI_S32        m_s32MaxFd;
void * detection_manager;

MI_S32 create_device_callback(MI_IPU_DevAttr_t* stDevAttr, char *pFirmwarePath)
{
	return MI_IPU_CreateDevice(stDevAttr, NULL, pFirmwarePath, 0);
}

void destory_device_callback()
{
	MI_IPU_DestroyDevice();
}

void detect_callback(std::vector<Box_t> results)
{
	// count time
	if(g_pstPipeLineCase->stIPUAttr.print_message == 1){
		auto end_time = std::chrono::steady_clock::now();
    	printf(GREEN "avg time: %f s, fps: %f\n" NONE, std::chrono::duration<double>(end_time - start_time).count(), 1.0 / std::chrono::duration<double>(end_time - start_time).count());
		start_time = std::chrono::steady_clock::now();
		for(int i = 0; i < results.size(); i++)
			printf("--->sorce: %f \n  ---> classid : %d  w:%d, h:%d\n", results[i].score, results[i].class_id, results[i].width, results[i].height);
		return ;
	}
	#define VPE_CROP_STEP_THRESHOLD 128
    #define VPE_CROP_STEPS 30
    #define NOBODY_CHEK_COUNTS 100
    #define RECT_BORDER_WIDTH 2
	#ifdef __ENABLE_TX_POS_BY_UART__
	char txbuffer[500];
	int count_index = 0;
	#endif
	int isvalid = 0;
    MI_RGN_CanvasInfo_t *pstCanvasInfo;
    ST_OSD_GetCanvasInfo(RGN_OSD_HANDLE0, &pstCanvasInfo);

	ST_Rect_T rgnRect = {0,0,DISP_WIDTH,DISP_HEIGHT};
    ST_OSD_Clear(RGN_OSD_HANDLE0, &rgnRect);
    
    int i = 0;
    rgnRect = {0,0,DISP_WIDTH,DISP_HEIGHT};

    for(i = 0; i < results.size(); i++)
    {
        rgnRect.u32X = results[i].x;
        rgnRect.u32Y = results[i].y;
        rgnRect.u16PicW = results[i].width;
        rgnRect.u16PicH = results[i].height;
		isvalid = 0;			
		printf("--->sorce: %f \n  ---> classid : %d  rgnRect: %d, %d, %d, %d\n", results[i].score, results[i].class_id, rgnRect.u32X, rgnRect.u32Y, rgnRect.u16PicW, rgnRect.u16PicH);
		if(results[i].class_id == 0){
			if (g_pstPipeLineCase->stIPUAttr.detect_type != 2){
				ST_OSD_DrawRect(RGN_OSD_HANDLE0, rgnRect, RECT_BORDER_WIDTH, 8);   //brown
				isvalid = 1;
			}
		}else if(results[i].class_id == 1){
			if (g_pstPipeLineCase->stIPUAttr.detect_type != 2){
				ST_OSD_DrawRect(RGN_OSD_HANDLE0, rgnRect, RECT_BORDER_WIDTH, 2);  // green
				isvalid = 1;				
			}
		}else if(results[i].class_id == 2){
			if (g_pstPipeLineCase->stIPUAttr.detect_type != 2){
				ST_OSD_DrawRect(RGN_OSD_HANDLE0, rgnRect, RECT_BORDER_WIDTH, 3);   // blue
				isvalid = 1;
			}
		}else if(results[i].class_id == 3){
			if (g_pstPipeLineCase->stIPUAttr.detect_type != 2){
				ST_OSD_DrawRect(RGN_OSD_HANDLE0, rgnRect, RECT_BORDER_WIDTH, 4);	 // yellow
				isvalid = 1;
			}
		}else if(results[i].class_id == 4){
			ST_OSD_DrawRect(RGN_OSD_HANDLE0, rgnRect, RECT_BORDER_WIDTH, 1);	 // red
			isvalid = 1;
		}else if(results[i].class_id == 5){
			isvalid = 1;
			ST_OSD_DrawRect(RGN_OSD_HANDLE0, rgnRect, RECT_BORDER_WIDTH, 10);	 // voilet
		}
		#ifdef __ENABLE_TX_POS_BY_UART__
		if (isvalid){
			txbuffer[count_index * 5] = count_index;
			txbuffer[count_index * 5 + 1] = (rgnRect.u32X + rgnRect.u16PicW / 2) & 0xFF;
			txbuffer[count_index * 5 + 2] = ((rgnRect.u32X + rgnRect.u16PicW / 2) >> 8) & 0xFF;
			txbuffer[count_index * 5 + 3] = (rgnRect.u32Y + rgnRect.u16PicH / 2) & 0xFF;
			txbuffer[count_index * 5 + 4] = ((rgnRect.u32Y+ rgnRect.u16PicH / 2) >> 8) & 0xFF;	
			count_index ++;
		}
		#endif
    }
    ST_OSD_Update(RGN_OSD_HANDLE0);
	if(g_pstPipeLineCase->stIPUAttr.print_message == 2){
		auto end_time = std::chrono::steady_clock::now();
    	printf(GREEN "avg time: %f s, fps: %f\n" NONE, std::chrono::duration<double>(end_time - start_time).count(), 1.0 / std::chrono::duration<double>(end_time - start_time).count());
		start_time = std::chrono::steady_clock::now();
	}
	#ifdef __ENABLE_TX_POS_BY_UART__
	if (count_index > 0){
		printf("Send_Car_Coord_Info_ByUart:count_index = %d\n",count_index);
		Send_Car_Coord_Info_ByUart(txbuffer,count_index * 5);
	}
	#endif
}

void *DetectThread(void *argc)
{
    printf("detect thread start\n");
    int type = *(int *)argc;
    fd_set read_fds;
    struct timeval tv;
    MI_S32 s32Ret = 0;
    MI_SYS_ChnPort_t stChnOutputPort;
    MI_SYS_BufInfo_t        stBufInfo;
    MI_SYS_BUF_HANDLE       stBufHandle;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int count = 0;
	int fd = -1;

	// 获取
	stChnOutputPort.eModId = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.eModId;
	stChnOutputPort.u32DevId = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.u32DevId;
	stChnOutputPort.u32ChnId    = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.u32ChnId;
	stChnOutputPort.u32PortId   = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.u32PortId;
	if(MI_SYS_SetChnOutputPortDepth(0, &stChnOutputPort, 2, 6) != MI_SUCCESS){
		ST_ERR("MI_SYS_SetChnOutputPortDepth error\n");
	}else{
		ST_DBG("MI_SYS_SetChnOutputPortDepth success\n");
	}
	if(MI_SYS_GetFd(&stChnOutputPort, &fd) != MI_SUCCESS){
		ST_ERR("MI_SYS_GetFd error\n");
	}else{
		ST_DBG("MI_SYS_GetFd success\n");
	}
	m_s32MaxFd = fd;
	
    while(1)
    {
        FD_ZERO(&read_fds);
        FD_SET(m_s32MaxFd, &read_fds);
        s32Ret = select(m_s32MaxFd + 1, &read_fds, NULL, NULL, &tv);
        if (s32Ret < 0)
        {
            printf("select failed\n");
        }
        else if (0 == s32Ret)
        {
			// ST_ERR("get frame time out\n");
            continue;
        }
        else
        {
         if(FD_ISSET(m_s32MaxFd, &read_fds))
            {
                stChnOutputPort.eModId      = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.eModId;
                stChnOutputPort.u32DevId    = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.u32DevId;
                stChnOutputPort.u32ChnId    = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.u32ChnId;
                stChnOutputPort.u32PortId   = g_pstPipeLineCase->stFileOutAttr.stDumpAttr[0].stSrcChnPort.u32PortId;
                memset(&stBufInfo, 0x0, sizeof(MI_SYS_BufInfo_t));
                s32Ret = MI_SYS_ChnOutputPortGetBuf(&stChnOutputPort, &stBufInfo, &stBufHandle);
				//printf("MI_SYS_ChnOutputPortGetBuf \n");
                if (MI_SUCCESS != s32Ret)
                {
                    printf("get divp buffer fail,ret:%x\n",s32Ret);
                    continue;
                }
				if(count == 10)  ST_dumps_image(&stBufInfo);
				// ST_FileoutDumpProc();
				// ST_DumpBufinfoToFile(pstFileOutAttr->szDir, pstDumpAttr, &stBufInfo);
                if(type==0)
                    doDetectFace(detection_manager, &stBufInfo);
                else if(type ==1)
                    doDetectPerson(detection_manager, &stBufInfo);
                else if(type ==2)
                    doDetect(detection_manager, &stBufInfo);
                MI_SYS_ChnOutputPortPutBuf(stBufHandle);
				count += 1;
            }
        }
   }
}

DetectionInfo_t face_detection_info = 
{
    "/config/dla/ipu_firmware.bin",
    "syfd360102_fixed_argb.sim_sgsimg.img",
    0.5, //threshold
    {DISP_WIDTH, DISP_HEIGHT},
	create_device_callback,
    detect_callback,
	destory_device_callback
};

DetectionInfo_t person_detection_info = 
{
    "/config/dla/ipu_firmware.bin",
    "./model/yuv/sypdy2.3603013_V2.1_20220714_fixed.sim_sgsimg.img",
    0.6, //threshold
    {DISP_WIDTH, DISP_HEIGHT},
    create_device_callback,
    detect_callback,
	destory_device_callback
};


DetectionInfo_t pcn_detection_info = 
{
    "/config/dla/ipu_firmware.bin",
    "./model/argb/sypcna2.3603013_V2.0_20220722_fixed.sim_sgsimg.img",
    0.6, //threshold
    {DISP_WIDTH, DISP_HEIGHT},
	create_device_callback,
    detect_callback,
	destory_device_callback
};
#ifdef __ENABLE_UART_FUNCTION__
void ST_open_uart(char *file)
{
	uart_fd =open(file,O_RDWR|O_NOCTTY|O_NDELAY);

	if(uart_fd==-1){
		printf("uart:open %s fail\n",file);
		return; 
	}       
	if(fcntl(uart_fd,F_SETFL,0)<0){
		printf("uart:fcntl faild!\n"); 
	}
	isatty(STDIN_FILENO);
}
void ST_init_uart(void)
{
	struct termios opt;
	tcgetattr(uart_fd,&opt);
	if (g_baudrate == 115200){
		cfsetispeed(&opt,B115200);
		cfsetospeed(&opt,B115200);
		printf("smc:baudrate = B115200 \n");		
	}
	if (g_baudrate == 230400){
		cfsetispeed(&opt,B230400);
		cfsetospeed(&opt,B230400);
		printf("smc:baudrate = B230400 \n");			
	}
	if (g_baudrate == 460800){
		cfsetispeed(&opt,B460800);
		cfsetospeed(&opt,B460800);
		printf("smc:baudrate = B460800 \n");			
	}	

	opt.c_cflag &= ~CSIZE;
	opt.c_cflag |= CS8;
	opt.c_cflag |= CLOCAL|CREAD;
	opt.c_cflag &= ~PARENB;
	opt.c_iflag &= ~INPCK;
	opt.c_iflag &= ~(IXON);//清BIT位，关闭流控字符
	opt.c_iflag &= ~ICRNL; //禁止将输入的CR转换为NL		
	opt.c_cflag &= ~CSTOPB;
	opt.c_cflag &= ~ICRNL;//禁止将输入的CR转换为NL
	opt.c_oflag &= ~ONLCR; //在输出时将换行符NL 映射成回车-换行符CR-NL
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	
	opt.c_oflag &= ~OPOST;	
	opt.c_cc[VTIME]=0;
	opt.c_cc[VMIN]=0;
	tcflush(uart_fd,TCIOFLUSH);
	tcsetattr(uart_fd,TCSANOW,&opt);
}	
void ST_close_uart(){
    if(uart_fd!=-1)
        close(uart_fd);  
}
typedef enum
{
   WAIT_HEAD = 0,
   STAT_LENGTH = 1,
   STAT_DATA = 2,
}FRMA_RX_STATE_e;

typedef struct
{
	MI_U16 Index;				/*!< 接收数据索引号      */
    MI_U8 RxBuffer[512];
	FRMA_RX_STATE_e RxPhase;				/*!< 接收帧状态      */
	MI_U8 back;					/*!<       */
	MI_U8 length;
}tFrmA_RxState;
#define X25_INIT_CRC 0xffff
int packet_sequence = 0;
MI_U8 Frm_Check_Sum(MI_U8 *Buf, MI_U8 Pos_Bin, MI_U8 Pos_CRC)
{
	MI_U8 i;
	MI_U8 sum = 0;
	for(i = Pos_Bin; i < Pos_CRC ; i++)
	{
		sum += Buf[i];
	}
	if(sum == Buf[Pos_CRC])
		return TRUE;
	else
		return FALSE;
}
void crc_accumulate(MI_U8 data, MI_U16 *crcAccum)
{
	MI_U8 tmp;
	tmp = data ^ (MI_U8)(*crcAccum &0xff);
	tmp ^= (tmp<<4);
	*crcAccum = (*crcAccum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);
}
MI_U16 crc_calculate(MI_U8* pBuffer, MI_U16 length)
{
	MI_U16 crcTmp,crcTmp_ext;
	int i;
	crcTmp = X25_INIT_CRC;	
	for(i=0;i<length;i++)
	{
		crc_accumulate(pBuffer[i+1], &crcTmp);
	}
	crcTmp_ext = crcTmp;
	if((pBuffer[length+1] == (crcTmp_ext&0x00ff))&&(pBuffer[length+2] == ((crcTmp_ext>>8)&0x00ff)))
		return TRUE;
	for(i=0;i<256;i++)
	{
		crcTmp_ext = crcTmp;
		crc_accumulate(i&0xff, &crcTmp_ext);
		if((pBuffer[length+1] == (crcTmp_ext&0x00ff))&&(pBuffer[length+2] == ((crcTmp_ext>>8)&0x00ff)))
			return TRUE;
	}
    return FALSE;
}


bool FrmA_RxData_MavL_1(tFrmA_RxState *state, MI_U8 data, bool reset,MI_U8 uart_num)
{
	MI_U8 *buf;
	buf = state->RxBuffer;
	if(reset == TRUE)
	{
		state->Index		= 0;
		state->RxPhase	= WAIT_HEAD;
		return FALSE;
	}
	switch(state->RxPhase)
	{
		case WAIT_HEAD:
			if (data == 0xFE)	
			{
				state->Index = 1;
				buf[0] = data;
				state->RxPhase	= STAT_LENGTH;
			}
			break;
		case STAT_LENGTH:	//
			buf[state->Index++]	= data;							
			state->RxPhase	= STAT_DATA;
		break;
		case STAT_DATA:		// Êý¾ÝÇøÄÚÈÝ
			buf[state->Index++] = data;
			if (state->Index >= (buf[1]+8))
			{
				state->length = state->Index;
				state->RxPhase	= WAIT_HEAD; 
				state->Index = 0;			
				if(crc_calculate(buf, buf[1] + 5) == TRUE) 
				{						
					printf("Receive one frame FrMA data success\n");
					write(uart_fd,buf,state->length);
					memset(state->RxBuffer,0,512);	
					return TRUE;
				}
				memset(state->RxBuffer,0,512);
			}
			break;
			default:
			break;
	}
	return FALSE;
}
#ifdef __ENABLE_TX_POS_BY_UART__
void Send_Car_Coord_Info_ByUart(char *data,int length){
	char txbuffer[512];
	MI_U16 crcTmp,i;
	txbuffer[0] = 0xFE;
	txbuffer[1] = length;
	txbuffer[2] = packet_sequence;
	packet_sequence ++;
	txbuffer[3] = 0;//system id
	txbuffer[4] = 0;//Component ID
	txbuffer[5] = 0x3A;//Message ID
	memcpy(txbuffer + 6, data, length);
	crcTmp = X25_INIT_CRC;	
	for(i = 0; i < length + 5; i++)
	{
		crc_accumulate(txbuffer[i+1], &crcTmp);
	}	
	txbuffer[length + 6] = crcTmp & 0x0FF;
	txbuffer[length + 7] = (crcTmp >> 8) & 0x0FF;
	write(uart_fd,txbuffer,length + 8);
}
#endif
void * ST_uart_rx_func(void * argv)
{
	int i,data_length;
	tFrmA_RxState frma_uart1_rx_state;
	int packet_cnt = 0;
	MI_U8 rec_buf[512] = {0};
	memset(&frma_uart1_rx_state,0,sizeof(frma_uart1_rx_state));
	FrmA_RxData_MavL_1(&frma_uart1_rx_state,NULL,TRUE,1);
    while(!g_bExit)
    {
        memset(rec_buf,0,sizeof(rec_buf));
		data_length = read(uart_fd,rec_buf,sizeof(rec_buf));
        if(data_length > 0){	
			printf("mavlink_rx_data[%d] = ",packet_cnt);
			for (i = 0; i < data_length; i ++){
				printf("0x%x,",rec_buf[i]);
			}	
			printf("\n");
			packet_cnt ++;		
			for (i = 0; i < data_length; i ++){
				FrmA_RxData_MavL_1((tFrmA_RxState *)&frma_uart1_rx_state,rec_buf[i],FALSE,1);
			}

		}else{
			usleep(5 * 1000);
		}		
    }

}

#endif
int main(int argc, char **argv)
{
	
	if (argc < 2)
	{
		printf("%s param_snr0.ini\n", argv[0]);
		return 0;
	}

	// load ini
	ST_LoadIni(argv[1]);

	ST_BaseModuleInit();
	#ifdef __ENABLE_PANLE__
	STCHECKRESULT(ST_PanelStart());
	#endif
	if (g_pstPipeLineCase->stIPUAttr.detect_enable != 0){
		// 添加ai
		STCHECKRESULT(getDetectionManager(&detection_manager));
		
	    int type = g_pstPipeLineCase->stIPUAttr.detect_type;
		if(type == 0){
			strcpy(face_detection_info.Model, g_pstPipeLineCase->stIPUAttr.model_path);
			face_detection_info.threshold = g_pstPipeLineCase->stIPUAttr.detect_threshold;
			face_detection_info.disp_size.width = g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Width;
			face_detection_info.disp_size.height = g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Height;
			STCHECKRESULT(initDetection(detection_manager, &face_detection_info));
		}else if(type==1){
			strcpy(person_detection_info.Model, g_pstPipeLineCase->stIPUAttr.model_path);
			person_detection_info.threshold = g_pstPipeLineCase->stIPUAttr.detect_threshold;
			person_detection_info.disp_size.width = g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Width;
			person_detection_info.disp_size.height = g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Height;
			STCHECKRESULT(initDetection(detection_manager, &person_detection_info));
		}else if(type==2){
			strcpy(pcn_detection_info.Model, g_pstPipeLineCase->stIPUAttr.model_path);
			pcn_detection_info.threshold = g_pstPipeLineCase->stIPUAttr.detect_threshold;
			pcn_detection_info.disp_size.width = g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Width;
			pcn_detection_info.disp_size.height = g_pstPipeLineCase->stSclAttr.stChnAttr[0].stPortAttr[0].stOutPortParam.stSCLOutputSize.u16Height;
			STCHECKRESULT(initDetection(detection_manager, &pcn_detection_info));
		}
	    STCHECKRESULT(startDetect(detection_manager));
	    pthread_create(&g_detectThread, NULL, DetectThread, &type);
	    pthread_setname_np(g_detectThread, "Detect_Task");
	}
#ifdef __ENABLE_UART_FUNCTION__
	ST_open_uart("/dev/ttyS1");
	ST_init_uart();
	pthread_create(&UART_RX_pthread,NULL,ST_uart_rx_func,NULL);
#endif

	while(1)
    {
//		ST_DBG("pass over\n");
        usleep(10000 * 1000);
    }
#ifdef __ENABLE_UART_FUNCTION__
	ST_close_uart();
	pthread_cancel(UART_RX_pthread);	
	pthread_join(UART_RX_pthread,NULL);	
#endif	
	if (g_pstPipeLineCase->stIPUAttr.detect_enable != 0){

	}

	ST_DBG("normal exit\n");
	#ifdef __ENABLE_PANLE__
	STCHECKRESULT(ST_PanelStop());
	#endif
	ST_BaseModuleUnInit();

	ST_UnLoadIni();
	
	return 0;
}

