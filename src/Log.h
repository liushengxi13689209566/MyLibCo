/*************************************************************************
	> File Name: Log.h
	> Author: chudongfang 
    > Git: https://github.com/chudongfang 
	> Created Time: 2018年02月13日 星期二 17时02分15秒
 ************************************************************************/

//使用方法
//创建 : Log Mylog("项目名");
//TRACE: LOG_TRACE(日志名,"内容");
//INFO : LOG_INFO(日志名,"内容");
//ERROR: LOG_ERROR(日志名,"内容");
//DEBUG: LOG_DEBUG(日志名,"内容");

#ifndef _LOG_H
#define _LOG_H

#include <string>
#include <fstream>
#include <thread>
#include <mutex>
#include <iostream>
namespace Tattoo
{

#define LOG(projectName) Log Logname(projectName)
#define LOG_TRACE(strTrace) (Logname).trace(__FILE__, __FUNCTION__, __LINE__, (strTrace))
#define LOG_INFO(strTrace)  (Logname).info(__FILE__,__FUNCTION__,__LINE__,(strTrace)
#define LOG_ERROR(strTrace) (Logname).error(__FILE__,__FUNCTION__,__LINE__,(strTrace)
#define LOG_DEBUG(strTrace) (Logname).debug(__FILE__, __FUNCTION__, __LINE__, (strTrace))
namespace LogInfo
{
namespace LevelType
{ //less is more important.
extern const int TRACE;
extern const int INFO;
extern const int ERROR;
extern const int DEBUG; //no output
} // namespace LevelType
} // namespace LogInfo

class Log
{
  public:
	Log(std::string projectName);
	~Log();
	void trace(std::string file, std::string function, int line, std::string strTrace);
	void info(std::string file, std::string function, int line, std::string strInfo);
	void error(std::string file, std::string function, int line, std::string strError);
	void debug(std::string file, std::string function, int line, std::string strTrace);

	std::string getfileName() const
	{
		return fileName_;
	}

	std::string getfileOldName() const
	{
		return fileOldName_;
	}

	std::string getprojectName() const
	{
		return projectName_;
	}

	int getoutputLevel()
	{
		return outputLevel_;
	}

	void setfileName(const std::string fileName)
	{
		fileName_ = fileName;
	}

	void setprojectName(const std::string projectName)
	{
		projectName_ = projectName;
	}

  private:
	void outputInfo(int _outputLevel, std::string filePos, std::string function, int line, std::string strInfo);
	void limitFileSize();
	long getFileSize();
	void writeTime(std::ofstream &file);

	std::string fileName_;
	std::string fileOldName_;
	std::string projectName_;

	long fileMaxSize_;
	int outputLevel_;
};

} // namespace Tattoo

#endif
