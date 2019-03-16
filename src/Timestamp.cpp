#include "Timestamp.h"

#include <sys/time.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

using namespace Tattoo;

Timestamp::Timestamp()
    : microSecondsSinceEpoch_(0)
{
}

Timestamp::Timestamp(int64_t microseconds)
    : microSecondsSinceEpoch_(microseconds)
{
}

std::string Timestamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
    //PRId64跨平台打印64位整数，因为int64_t用来表示64位整数，在32位系统中是long long int，64位系统中是long int
    //所以打印64位是%ld或%lld，可移植性较差，不如统一同PRID64来打印。
    snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

//把它转换成一个格式化字符串
std::string Timestamp::toFormattedString() const
{
    char buf[32] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
    return buf;
}

Timestamp Timestamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);     //获得当前时间，第二个参数是一个时区，当前不需要返回时区，就填空指针
    int64_t seconds = tv.tv_sec; //取出秒数
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

Timestamp Timestamp::invalid()
{
    return Timestamp();
}
