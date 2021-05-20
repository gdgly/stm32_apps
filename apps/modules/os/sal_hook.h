

/*================================================================
 *
 *
 *   文件名称：sal_hook.h
 *   创 建 者：肖飞
 *   创建日期：2021年05月19日 星期三 08时14分01秒
 *   修改日期：2021年05月19日 星期三 23时08分46秒
 *   描    述：
 *
 *================================================================*/
#ifndef _SAL_HOOK_H
#define _SAL_HOOK_H
#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

#if defined(SAL_HOOK)
#include "sal_socket.h"
#undef close
#undef closesocket
#undef connect
#undef accept
#undef bind
#undef listen
#undef recv
#undef recvfrom
#undef send
#undef sendto
#undef socket
#undef select
#undef fcntl
#undef setsockopt
#undef getsockopt
#undef getaddrinfo
#undef freeaddrinfo
#undef gethostbyname
#define close(s) sal_closesocket(s)
#define closesocket(s) sal_closesocket(s)
#define connect(s, name, namelen) sal_connect(s, name, namelen)
#define accept(s,addr,addrlen) sal_accept(s,addr,addrlen)
#define bind(s,name,namelen) sal_bind(s,name,namelen)
#define listen(s, backlog) sal_listen(s, backlog)
#define recv(s, mem, len, flags) sal_recvfrom(s, mem, len, flags, NULL, NULL)
#define recvfrom(s, mem, len, flags, from, fromlen) sal_recvfrom(s, mem, len, flags, from, fromlen)
#define send(s, dataptr, size, flags) sal_sendto(s, dataptr, size, flags, NULL, 0)
#define sendto(s, dataptr, size, flags, to, tolen) sal_sendto(s, dataptr, size, flags, to, tolen)
#define socket(domain, type, protocol) sal_socket(domain, type, protocol)
#define select(maxfdp1, readset, writeset, exceptset, timeout) sal_select(maxfdp1, readset, writeset, exceptset, timeout)
#define fcntl(s, cmd, val) sal_ioctlsocket(s, cmd, val)
#define setsockopt(s,level,optname,opval,optlen)  sal_setsockopt(s,level,optname,opval,optlen)
#define getsockopt(s,level,optname,opval,optlen)  sal_getsockopt(s,level,optname,opval,optlen)
#define getaddrinfo(nodname, servname, hints, res) sal_getaddrinfo(nodname, servname, hints, res)
#define freeaddrinfo(addrinfo) sal_freeaddrinfo(addrinfo)
#define gethostbyname(name) sal_gethostbyname(name)
#endif//#if defined(SAL_HOOK)
#endif //_SAL_HOOK_H
