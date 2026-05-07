#ifndef LOG_
#define LOG_
#define NONE "\033[m\n"
#define GREEN "\033[0;32;32m"
#define RED "\033[0;32;31m"
#define YELLOW "\033[1;33m"

#define DBG_INFO(fmt, args...) printf(GREEN "%s[%d]: " fmt NONE, __FUNCTION__, __LINE__, ##args);
#define DBG_ERR(fmt, args...) printf(RED "%s[%d]: " fmt NONE, __FUNCTION__, __LINE__, ##args);

#endif 