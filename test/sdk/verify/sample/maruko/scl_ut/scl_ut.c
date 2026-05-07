#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <poll.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include "mi_sys.h"
#include "mi_scl.h"

#include "st_common.h"

#define ST_SCL_USER_STRETCHBUFF_DEVID (1)
#define ST_SCL_USER_STRETCHBUFF_PORTID E_MI_SCL_HWSCL3

#define scl_ut_dbg(fmt, args...) {do{printf("[%s][%d]"fmt,__FUNCTION__,__LINE__,##args);}while(0);}

#ifndef ALIGN_UP
#define ALIGN_UP(val, alignment) ((((val)+(alignment)-1)/(alignment))*(alignment))
#endif
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(val, alignment) (((val)/(alignment))*(alignment))
#endif

#ifndef ExecFuncResult
#define ExecFuncResult(_func_, _ret_) \
        do{ \
            _ret_ = _func_; \
            if (_ret_ != MI_SUCCESS) \
            { \
                printf("[%s %d]exec function failed, error:%x\n", __func__, __LINE__, _ret_); \
                goto EXIT; \
            } \
        } while(0)
#endif

typedef struct
{
    pthread_t pt;
    
    int src_width;
    int src_height;
    MI_SYS_PixelFormat_e eInputPixel;

    MI_SYS_WindowRect_t stCropWin;
    
    int dst_width;
    int dst_height;
    MI_SYS_PixelFormat_e eOutputPixel;
    
    char szFile[64];
} ST_Scale_File_S;

MI_S32 ST_OpenSourceFile(const char *pFileName, int *pSrcFd)
{
    int src_fd = open(pFileName, O_RDONLY);
    if (src_fd < 0)
    {
        perror("open");
        return -1;
    }
    *pSrcFd = src_fd;

    return TRUE;
}

MI_S32 ST_GetOneFrameByStride(int srcFd, char *pData,int width, int stride, int yuvsize)
{
    off_t current = lseek(srcFd,0L, SEEK_CUR);
    off_t end = lseek(srcFd,0L, SEEK_END);
    int bufferheight = yuvsize / stride;
    int i = 0;

    if((end - current) == 0 || (end - current) < yuvsize)
    {
        lseek(srcFd,0,SEEK_SET);
        current = lseek(srcFd,0,SEEK_CUR);
        //end = lseek(srcFd,0L,SEEK_END);
    }
    /*if ((end - current) < yuvSize)
    {
        return -1;
    }*/
    lseek(srcFd, current, SEEK_SET);

    // printf("width %d, stride %d, buffer height %d \n", width, stride, bufferheight);
    for (i = 0; i < bufferheight; i++)
    {
        if (read(srcFd, pData+ i * stride, width) < width)
        {
            printf("linecnt %d read size err \n", i);
            return 0;
        }
    }

    if(i == bufferheight)
        return 1;

    return 0;
}

MI_S32 ST_WriteFile(char *pFilePath, FILE **fp, MI_U8 *pData, MI_U32 u32BufSize, MI_U16 *pu16DumpNum, MI_BOOL bRecover)//pFilePath=/path/name
{
    MI_S32 s32Ret = MI_SUCCESS;

    if(*fp == NULL)
    {
        const char *mode = bRecover ? "wb":"ab";

        *fp = fopen(pFilePath,mode);
        if(*fp == NULL)
        {
            printf("file %s open fail\n", pFilePath);
            *pu16DumpNum = 0;
            return -1;
        }
    }

    fwrite(pData, u32BufSize, 1, *fp);

    if(*pu16DumpNum > 0)
    {
        *pu16DumpNum -=1;
    }

    if(*pu16DumpNum == 0)
    {
        fclose(*fp);
        *fp=NULL;
    }

    return s32Ret;
}

int DoScaleProcess(ST_Scale_File_S *pstScaleFile)
{
    MI_S32 s32Ret = MI_SUCCESS;

    char sDestFilePath[128];
    FILE *fp = NULL;
    MI_U16 u16DumpBufferNum = 1;
    int src_fd = 0;
    MI_U32  u32SrcSize=0, u32DestSize=0;
    struct stat statbuff;
    MI_PHY  inputphyaddr =0, outputaddr=0;
    void *pviraddr = NULL, *pviroutaddr=NULL;
    time_t stTime = 0;
    MI_U32 u32SrcAddrOffset[2]={0};
    MI_U32 u32DstAddrOffset[2]={0};
    MI_U32 u32DestStride[3] ={0};
    MI_U16 u16ValidStride[3] = {0};

    MI_U32 u32InputStride = 0;
    struct timeval tpstart, tpend;

    if(stat(pstScaleFile->szFile, &statbuff) < 0)
    {
        ST_ERR("file not exit, %s!\n", pstScaleFile->szFile);
        s32Ret = -1;
        goto EXIT;
    }
    else
    {
        if (statbuff.st_size == 0)
        {
            ST_ERR("File size is zero!\n");
            s32Ret = -1;
            goto EXIT;
        }
        u32SrcSize = statbuff.st_size;
    }

    if(ST_OpenSourceFile(pstScaleFile->szFile, &src_fd) < 0)
    {
        ST_ERR("open file fail!\n");
        s32Ret = -1;
        goto EXIT;
    }

    #if 0
    printf("input pixel %d, size(%d,%d), crop(%d,%d,%d,%d), output pixel %d, size(%d,%d)\n", 
            pstScaleFile->eInputPixel, pstScaleFile->src_width, pstScaleFile->src_height,
            pstScaleFile->stCropWin.u16X, pstScaleFile->stCropWin.u16Y, pstScaleFile->stCropWin.u16Width, pstScaleFile->stCropWin.u16Height,
            pstScaleFile->eOutputPixel, pstScaleFile->dst_width, pstScaleFile->dst_height);
    #endif
    
    u32InputStride = ALIGN_UP(pstScaleFile->src_width, 16);
    
    if(pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV
        || pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY
        || pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU
        || pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY)
    {
        u32SrcSize = u32InputStride * pstScaleFile->src_height;
        u16ValidStride[0] = pstScaleFile->src_width * 2;
        u32SrcAddrOffset[0] = 0;
    }
    else if(pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420
        || pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21
           )
    {
        u32SrcSize = u32InputStride * pstScaleFile->src_height * 3/2;
        u16ValidStride[0] = pstScaleFile->src_width;
        u16ValidStride[1] = pstScaleFile->src_width;
        u32SrcAddrOffset[0] = u32InputStride * pstScaleFile->src_height;
    }
    else if(pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_ARGB8888
        || pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_ABGR8888
        || pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_BGRA8888)
    {
        u32SrcSize = u32InputStride * pstScaleFile->src_height;
        u16ValidStride[0] = pstScaleFile->src_width * 4;
        u32SrcAddrOffset[0] = 0;
    }
    else if(pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        u32SrcSize = u32InputStride * pstScaleFile->src_height * 2;
        u16ValidStride[0] = pstScaleFile->src_width;
        u16ValidStride[1] = pstScaleFile->src_width;
        u32SrcAddrOffset[0] = u32InputStride * pstScaleFile->src_height;
    }
    else if(pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR)
    {
        u32SrcSize = u32InputStride * pstScaleFile->src_height * 3/2;
        u16ValidStride[0] = pstScaleFile->src_width;
        u16ValidStride[1] = pstScaleFile->src_width / 2;
        u16ValidStride[2] = pstScaleFile->src_width / 2;
        u32SrcAddrOffset[0] = u32InputStride * pstScaleFile->src_height;
        u32SrcAddrOffset[1] = u32SrcAddrOffset[0] + u32InputStride / 2 * pstScaleFile->src_height / 2;
    }
    else if(pstScaleFile->eInputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR)
    {
        u32SrcSize = u32InputStride * pstScaleFile->src_height * 2;
        u16ValidStride[0] = pstScaleFile->src_width;
        u16ValidStride[1] = pstScaleFile->src_width / 2;
        u16ValidStride[2] = pstScaleFile->src_width / 2;
        u32SrcAddrOffset[0] = u32InputStride * pstScaleFile->src_height;
        u32SrcAddrOffset[1] = u32SrcAddrOffset[0] + u32InputStride / 2 * pstScaleFile->src_height;
    }

    pstScaleFile->dst_width = ALIGN_UP(pstScaleFile->dst_width, 8);
    pstScaleFile->dst_height = ALIGN_UP(pstScaleFile->dst_height, 2);
    if(pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YUYV
        || pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_UYVY
        || pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_YVYU
        || pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_VYUY)
    {
        u32DestStride[0] = ALIGN_UP(pstScaleFile->dst_width, 16)*2;
        u32DestSize = u32DestStride[0] * pstScaleFile->dst_height;
        u32DstAddrOffset[0]=0;
    }
    else if(pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420
        || pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420_NV21
        )
    {
        u32DestStride[0] = ALIGN_UP(pstScaleFile->dst_width, 16);
        u32DestStride[1] = ALIGN_UP(pstScaleFile->dst_width, 16);
        u32DestSize = u32DestStride[0] * pstScaleFile->dst_height * 3/2;
        u32DstAddrOffset[0] = u32DestStride[0] * pstScaleFile->dst_height;
    }
    else if(pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_ARGB8888
        || pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_ABGR8888
        || pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_BGRA8888)
    {
        u32DestStride[0] = ALIGN_UP(pstScaleFile->dst_width, 16) * 4;
        u32DestSize = u32DestStride[0] * pstScaleFile->dst_height;
        u32DstAddrOffset[0] = 0;
    }
    else if(pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_422)
    {
        u32DestStride[0] = ALIGN_UP(pstScaleFile->dst_width, 32);
        u32DestSize = u32DestStride[0] * pstScaleFile->dst_height * 3/2;
        u32DstAddrOffset[0]=u32DestStride[0] * pstScaleFile->dst_height;
    }
    else if(pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV420_PLANAR)
    {
        u32DestStride[0] = ALIGN_UP(pstScaleFile->dst_width, 32);
        u32DestSize = u32DestStride[0] * pstScaleFile->dst_height * 3/2;
        u32DestStride[1] = u32DestStride[0]/2;
        u32DestStride[2] = u32DestStride[0]/2;
        u32DstAddrOffset[0] = u32DestStride[0] * pstScaleFile->dst_height;
        u32DstAddrOffset[1] = u32DstAddrOffset[0] + u32DestStride[1] * pstScaleFile->dst_height / 2;
    }
    else if(pstScaleFile->eOutputPixel == E_MI_SYS_PIXEL_FRAME_YUV422_PLANAR)
    {
        u32DestStride[0] = ALIGN_UP(pstScaleFile->dst_width, 32);
        u32DestSize = u32DestStride[0] * pstScaleFile->dst_height * 2;
        u32DestStride[1] = u32DestStride[0]/2;
        u32DestStride[2] = u32DestStride[0]/2;
        u32DstAddrOffset[0] = u32DestStride[0] * pstScaleFile->dst_height;
        u32DstAddrOffset[1] = u32DstAddrOffset[0] + u32DestStride[1] * pstScaleFile->dst_height;
    }

    ExecFuncResult(MI_SYS_MMA_Alloc(0, (MI_U8*)"mma_heap_name0", u32SrcSize, &inputphyaddr), s32Ret);
    ExecFuncResult(MI_SYS_Mmap(inputphyaddr, u32SrcSize, &pviraddr, FALSE), s32Ret);
    ExecFuncResult(MI_SYS_MMA_Alloc(0, (MI_U8*)"mma_heap_name0", u32DestSize, &outputaddr), s32Ret);
    ExecFuncResult(MI_SYS_Mmap(outputaddr, u32DestSize, &pviroutaddr, FALSE), s32Ret);
    if(1 != ST_GetOneFrameByStride(src_fd, (char *)pviraddr, u16ValidStride[0], u32InputStride, u32SrcSize))
    {
        ST_ERR("read %s size %d fail \n", pstScaleFile->szFile, u32SrcSize);
        s32Ret = -1;
        goto EXIT;
    }

    MI_SCL_DirectBuf_t  stSrcBuff;
    memset(&stSrcBuff, 0x0, sizeof(MI_SCL_DirectBuf_t));
    stSrcBuff.u32Width = pstScaleFile->src_width;
    stSrcBuff.u32Height = pstScaleFile->src_height;
    stSrcBuff.ePixelFormat = pstScaleFile->eInputPixel;
    stSrcBuff.phyAddr[0] = inputphyaddr;
    stSrcBuff.phyAddr[1] = inputphyaddr+u32SrcAddrOffset[0];
    //stSrcBuff.phyAddr[2] = inputphyaddr+u32SrcAddrOffset[1];
    stSrcBuff.u32BuffSize = u32SrcSize;
    stSrcBuff.u32Stride[0] = u16ValidStride[0];
    stSrcBuff.u32Stride[1] = u16ValidStride[1];
    //stSrcBuff.u32Stride[2] = u16ValidStride[2];

    MI_SCL_DirectBuf_t  stDestBuff;
    memset(&stDestBuff, 0x0, sizeof(MI_SCL_DirectBuf_t));
    stDestBuff.u32Width = pstScaleFile->dst_width;
    stDestBuff.u32Height = pstScaleFile->dst_height;
    stDestBuff.ePixelFormat = pstScaleFile->eOutputPixel;
    stDestBuff.phyAddr[0] = outputaddr;
    stDestBuff.phyAddr[1] = outputaddr+u32DstAddrOffset[0];
    //stDestBuff.phyAddr[2] = outputaddr+u32DstAddrOffset[1];
    stDestBuff.u32BuffSize = u32DestSize;
    stDestBuff.u32Stride[0]=u32DestStride[0];
    stDestBuff.u32Stride[1]=u32DestStride[1];
    //stDestBuff.u32Stride[2]=u32DestStride[2];

    MI_SYS_WindowRect_t stSrcCropWin;
    memset(&stSrcCropWin, 0x0, sizeof(MI_SYS_WindowRect_t));
    stSrcCropWin.u16X = pstScaleFile->stCropWin.u16X;
    stSrcCropWin.u16Y = pstScaleFile->stCropWin.u16Y;
    stSrcCropWin.u16Width = pstScaleFile->stCropWin.u16Width;
    stSrcCropWin.u16Height = pstScaleFile->stCropWin.u16Height;

    scl_ut_dbg("Scale process start, (%d, %d)->(%d, %d)\n", pstScaleFile->src_width, pstScaleFile->src_height, 
                 pstScaleFile->dst_width, pstScaleFile->dst_height);
    gettimeofday(&tpstart, NULL);
    ExecFuncResult(MI_SCL_StretchBuf(&stSrcBuff, &stSrcCropWin, &stDestBuff, E_MI_SCL_FILTER_TYPE_AUTO), s32Ret);
    gettimeofday(&tpend, NULL);
    scl_ut_dbg("Scale process end, (%d, %d)->(%d, %d), cost time:%ld\n", 
            pstScaleFile->src_width, pstScaleFile->src_height, pstScaleFile->dst_width, 
            pstScaleFile->dst_height, 1000000 * (tpend.tv_sec - tpstart.tv_sec) + tpend.tv_usec - tpstart.tv_usec);

    sprintf(sDestFilePath, "stretch_%dx%d_pixel%d_%ld.yuv", pstScaleFile->dst_width, pstScaleFile->dst_height, 
                            pstScaleFile->eOutputPixel, time(&stTime));

    ST_WriteFile(sDestFilePath, &fp, (MI_U8 *)pviroutaddr, u32DestSize, &u16DumpBufferNum, FALSE);

EXIT:
    if(src_fd)
    {
        close(src_fd);
    }
    MI_SYS_Munmap(pviraddr, u32SrcSize);
    MI_SYS_Munmap(pviroutaddr, u32DestSize);

    MI_SYS_MMA_Free(0,inputphyaddr);
    MI_SYS_MMA_Free(0,outputaddr);

    return s32Ret;
}

void *ST_DoScaleProcess(void *args)
{
    ST_Scale_File_S *pstScaleFile = (ST_Scale_File_S *)args;

    DoScaleProcess(pstScaleFile);
    
    return NULL;
}

#define TEST_NUM 4

MI_S32 main(int argc, char **argv)
{
    int i = 0;
    ST_Scale_File_S stScaleFile[TEST_NUM];

    MI_SYS_Init(0);

    MI_SCL_DevAttr_t stDevAttr;
    stDevAttr.u32NeedUseHWOutPortMask = ST_SCL_USER_STRETCHBUFF_PORTID;

    if (MI_SUCCESS != MI_SCL_CreateDevice(ST_SCL_USER_STRETCHBUFF_DEVID, &stDevAttr))
    {
        ST_ERR("MI_SCL_CreateDevice fail\n");
        return -1;
    }

    for(i = 0; i < TEST_NUM; i ++)
    {
        switch (i)
        {
            case 0:
                // src
                stScaleFile[i].src_width = 3840;
                stScaleFile[i].src_height = 2160;
                stScaleFile[i].eInputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

                // dst
                stScaleFile[i].dst_width = 64;
                stScaleFile[i].dst_height = 64;
                stScaleFile[i].eOutputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                snprintf(stScaleFile[i].szFile, sizeof(stScaleFile[i].szFile) - 1, "3840x2160.yuv");
                break;
            case 1:
                // src
                stScaleFile[i].src_width = 3840;
                stScaleFile[i].src_height = 2160;
                stScaleFile[i].eInputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

                // dst
                stScaleFile[i].dst_width = 1280;
                stScaleFile[i].dst_height = 720;
                stScaleFile[i].eOutputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                snprintf(stScaleFile[i].szFile, sizeof(stScaleFile[i].szFile) - 1, "3840x2160.yuv");
                break;
            case 2:
                // src
                stScaleFile[i].src_width = 1920;
                stScaleFile[i].src_height = 1080;
                stScaleFile[i].eInputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

                //dst
                stScaleFile[i].dst_width = 112;
                stScaleFile[i].dst_height = 112;
                stScaleFile[i].eOutputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                snprintf(stScaleFile[i].szFile, sizeof(stScaleFile[i].szFile) - 1, "1920x1080_yuv420.yuv");
                break;
            case 3:
                // src
                stScaleFile[i].src_width = 1920;
                stScaleFile[i].src_height = 1080;
                stScaleFile[i].eInputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;

                //dst
                stScaleFile[i].dst_width = 192;
                stScaleFile[i].dst_height = 192;
                stScaleFile[i].eOutputPixel = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
                snprintf(stScaleFile[i].szFile, sizeof(stScaleFile[i].szFile) - 1, "1920x1080_yuv420.yuv");
                break;
        }

        pthread_create(&stScaleFile[i].pt, NULL, ST_DoScaleProcess, &stScaleFile[i]);
    }
    
    for(i = 0; i < TEST_NUM; i ++)
    {
        pthread_join(stScaleFile[i].pt, NULL);   
    }

    if (MI_SUCCESS != MI_SCL_DestroyDevice(ST_SCL_USER_STRETCHBUFF_DEVID))
    {
        ST_ERR("MI_SCL_DestroyDevice fail\n");
    }

    MI_SYS_Exit(0);
    
    return 0;
}

