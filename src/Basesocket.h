/*************************************************************************
	> File Name: Basesocket.h
	> Author: Liu Shengxi 
	> Mail: 13689209566@163.com
	> Created Time: 2019年01月20日 星期日 19时51分33秒
 ************************************************************************/

#ifndef _BASESOCKET_H
#define _BASESOCKET_H
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
namespace Tattoo
{
namespace BaseSocket
{

void close(int sockfd);
void bind(int sockfd, struct sockaddr_in addr);
int listen(int sockfd);
int accept(int sockfd, struct sockaddr_in *addr);
void shutDownWrite(int sockfd);
} // namespace BaseSocket
} // namespace Tattoo
#endif
