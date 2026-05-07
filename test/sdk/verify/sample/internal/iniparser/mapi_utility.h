/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/


#ifndef __MAPI_UTILITY_H__
#define __MAPI_UTILITY_H__


// headers of standard C libs
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include "mi_common.h"
// headers of the same layer's
#include "iniparser.h"

// headers of underlying layer's

/// the helper utility class for ini file parsing and reading
        //-------------------------------------------------------------------------------------------------
        /// constructor
        /// @param s    \b IN: config path
        /// @return  None
        //-------------------------------------------------------------------------------------------------
        dictionary *mapi_config_helper(const char* s);
        //-------------------------------------------------------------------------------------------------
        /// Get U32 value from ini
        /// @param s          \b IN: ini attribute name
        /// @param  pu32Num         \b OUT: U32 value get from ini
        /// @param  notfound         \b IN: *pu32Num will be notfound if read error or not found
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL   mapi_config_helper_GetU32(dictionary *ini, const char* s, MI_U32 *pu32Num, MI_U32 notfound);
        //-------------------------------------------------------------------------------------------------
        /// Get BOOL value from ini
        /// @param s          \b IN: ini attribute name
        /// @param  pu8Bool         \b OUT: BOOL value get from ini
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL    mapi_config_helper_GetBOOL(dictionary *ini, const char* s, MI_BOOL *pu8Bool);
        //-------------------------------------------------------------------------------------------------
        /// Get char string from ini, the string is created by strdup, so user must free it if no more use
        /// @param s          \b IN: ini attribute name
        /// @param  ppu8String         \b OUT:  get char string from ini
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL  mapi_config_helper_GetString(dictionary *ini, const char* s, MI_U8 **ppu8String);
        //-------------------------------------------------------------------------------------------------
        /// Get Each Items in Array from ini
        /// @param s          \b IN: ini attribute name
        /// @param  thelist         \b OUT: list<string> that contain each row in the array.
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL  mapi_config_helper_GetArrayItems(dictionary *ini, const char* s,const char **arr, int *len);
        //-------------------------------------------------------------------------------------------------
        /// Get U8 value from ini
        /// @param s          \b IN: ini attribute name
        /// @param  u8Num         \b OUT: U8 value get from ini
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL mapi_config_helper_GetU8(dictionary *ini, const char* s, MI_U8 *u8Num);
        //-------------------------------------------------------------------------------------------------
        /// Get 1-Dimensional value from ini, the array must malloc by user then this API will fill the value in it
        /// @param s          \b IN: ini attribute name
        /// @param  pu8Array         \b OUT: u8 Array get from ini
        /// @param  size         \b OUT: Array Length
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL mapi_config_helper_Get1DArray(dictionary *ini, const char* s, MI_U8 * pu8Array,int size);
        //-------------------------------------------------------------------------------------------------
        /// Get 2-Dimensional value from ini, the array must malloc by user then this API will fill the value in it
        /// @param s          \b IN: ini attribute name
        /// @param  ppu8Array         \b OUT: u8 2-D Array get from ini
        /// @param dimension1          \b IN: dimension1 rows of array (user must indicate explictly number for lower level to malloc)
        /// @param dimension2          \b IN: dimension2 column of array (user must indicate explictly number for lower level to malloc )
        /// @return  MI_BOOL  \b OUT: indicate this operation success or not
        //-------------------------------------------------------------------------------------------------
        MI_BOOL mapi_config_helper_Get2DArray(dictionary *ini, const char* s, MI_U8 ** ppu8Array,int dimension1, int dimension2);
#endif
