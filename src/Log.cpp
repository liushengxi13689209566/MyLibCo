/*************************************************************************
	> File Name: Log.cpp
	> Author: 
	> Mail: 
	> Created Time: 2018年02月13日 星期二 17时02分24秒
 ************************************************************************/

#include "Log.h"

namespace Tattoo
{

namespace LogInfo
{
namespace LevelType
{ //less is more important.
const int TRACE = 90;
const int INFO = 50;
const int ERROR = 30;
const int DEBUG = 0;
} // namespace LevelType
} // namespace LogInfo

//构造函数
Log::Log(std::string projectName)
    : projectName_(projectName)
{
    fileName_ = projectName_ + ".log";
    fileOldName_ = projectName_ + ".old.log";
    fileMaxSize_ = 128 * 1024; //默认大小128kb
    outputLevel_ = LogInfo::LevelType::TRACE;
}

Log::~Log()
{
}

//得到文件大小
long Log::getFileSize()
{
    FILE *pFile;
    long size;

    pFile = fopen(fileName_.c_str(), "rb");
    if (pFile == NULL)
        return 0l;

    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    fclose(pFile);
    return size;
}

void Log::limitFileSize()
{
    if (getFileSize() > fileMaxSize_)
    {
        //if old name file exist, remove it.
        remove(fileOldName_.c_str());

        //rename the current file to old name file.
        rename(fileName_.c_str(), fileOldName_.c_str());
    }
}

//写时间
void Log::writeTime(std::ofstream &file)
{
    time_t t;
    tm *lt;
    t = time(NULL);
    lt = localtime(&t);

    file << (lt->tm_year + 1900) << "-" << (lt->tm_mon + 1) << "-" << lt->tm_mday << " ";
    file << lt->tm_hour << ":" << lt->tm_min << ":" << lt->tm_sec << "  ";
}

void Log::outputInfo(int _outputLevel, std::string filePos, std::string function, int line, std::string strInfo)
{
    static std::mutex mInfo;
    std::lock_guard<std::mutex> lock(mInfo);

    limitFileSize();

    std::string strLevel;
    switch (_outputLevel)
    {
    case LogInfo::LevelType::TRACE:
        strLevel = "trace";
        break;
    case LogInfo::LevelType::INFO:
        strLevel = "info";
        break;
    case LogInfo::LevelType::ERROR:
        strLevel = "error";
        break;
    }

    std::ofstream file(fileName_.c_str(), std::ios::app | std::ios::out);
    writeTime(file);
    file << strLevel.c_str() << " ";
    file << filePos << " " << function << " " << line << " :";
    file << " " << strInfo.c_str() << std::endl;
    file.close();
}

void Log::trace(std::string file, std::string function, int line, std::string strTrace)
{
    if (outputLevel_ >= LogInfo::LevelType::TRACE)
    {
        outputInfo(LogInfo::LevelType::TRACE, file, function, line, strTrace);
    }
}

void Log::info(std::string file, std::string function, int line, std::string strInfo)
{
    if (outputLevel_ >= LogInfo::LevelType::INFO)
    {
        outputInfo(LogInfo::LevelType::INFO, file, function, line, strInfo);
    }
}

void Log::error(std::string file, std::string function, int line, std::string strError)
{
    if (outputLevel_ >= LogInfo::LevelType::ERROR)
    {
        outputInfo(LogInfo::LevelType::ERROR, file, function, line, strError);
    }
}

void Log::debug(std::string file, std::string function, int line, std::string strDebug)
{
    std::cout << "DEBUG-->" << file << " " << function << " " << line << " :" << strDebug << std::endl;
}

} // namespace Tattoo
