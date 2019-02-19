/*************************************************************************
	> File Name: deebug.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年02月19日 星期二 22时34分28秒
 ************************************************************************/

#ifndef _DEEBUG_H
#define _DEEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

namespace Tattoo
{

#define DEBUG(M, ...) fprintf(stdout, " DEBUG %s : %s: %d: " M "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__) /*  */

#define INFO(M, ...) fprintf(stdout, "  " M "\n", ##__VA_ARGS__) /*  */

} // namespace Tattoo

#endif
