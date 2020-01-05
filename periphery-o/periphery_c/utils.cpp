#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include "utils.h"

using namespace std;

wstring StringToWString(string s)
{
	// ��ȡ��������С
	int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), NULL, 0);
	// ����ռ䣬��������С���ַ�����
	TCHAR* buf = new TCHAR[len + 1];
	// ���ֽڱ���ת���ɿ��ֽڱ���
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size(), buf, len);
	// ����ַ�����β
	buf[len] = '\0';
	wstring ws;
	ws.append(buf);
	// ɾ��������
	if (buf) delete[] buf;

	return ws;
}


string WStringToString(wstring ws)
{
	// ��ȡ��������С
	int len = WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.size(), NULL, 0, NULL, NULL);
	// ����ռ䣬��������С���ֽڼ���
	char* buf = new char[len + 1];
	// ���ֽڱ���ת���ɶ��ֽڱ���
	WideCharToMultiByte(CP_ACP, 0, ws.c_str(), ws.size(), buf, len, NULL, NULL);
	// ����ַ�����β
	buf[len] = '\0';
	string s;
	s.append(buf);
	// ɾ��������
	if (buf) delete[] buf;

	return s;
}



//ָ��SYSTEMTIME���͵�ʱ�䣬��ʽ��ΪYYYYMMDDHH24MISS�͵��ַ���
string StTime2StrTime(SYSTEMTIME &tm1)
{
	char  szTime[128];
	sprintf_s(szTime, "%4.4hd-%2.2hd-%2.2hd %2.2hd:%2.2hd:%2.2hd",
		tm1.wYear,
		tm1.wMonth + 1,
		tm1.wDay,
		tm1.wHour,
		tm1.wMinute,
		tm1.wSecond);

	return szTime;
}

//ָ��YYYYMMDDHH24MISS�͵�ʱ�䣬��ʽ��ΪSYSTEMTIME�͵�ʱ��
SYSTEMTIME StrTime2StTime(string time)
{
	SYSTEMTIME tm1;

	ZeroMemory((void*)&tm1,sizeof(tm1));
	sscanf_s(time.c_str(), "%4hd-%2hd-%2hd %2hd:%2hd:%2hd",
		&tm1.wYear,
		&tm1.wMonth,
		&tm1.wDay,
		&tm1.wHour,
		&tm1.wMinute,
		&tm1.wSecond);
	return tm1;
}

ULONGLONG StrTime2LongTime(string time)
{
	// Windows�ļ�ʱ����һ��64λ��ֵ�����Ǵ�1601��1��1������12:00������ʱ������
	// ��λ��1/10,000,000�롣��1000���֮1��(100-nanosecond)
	FILETIME ft;
	SYSTEMTIME st;
	st = StrTime2StTime(time);
	SystemTimeToFileTime(&st, &ft);

	// Ȼ��Time Protocolʹ�õĻ�׼ʱ�������ȥ��ʱ�䣨ulTime��
	ULONGLONG *pLLong = (ULONGLONG *)&ft;

	FILETIME ft1;
	SYSTEMTIME st1;
	ZeroMemory((void*)&st1, sizeof(st1));

	// ���Ƚ���׼ʱ�䣨1900��1��1��0��0��0��0���룩ת��ΪWindows�ļ�ʱ��
	st1.wYear = 1900;
	st1.wMonth = 1;
	st1.wDay = 1;
	st1.wHour = 0;
	st1.wMinute = 0;
	st1.wSecond = 0;
	st1.wMilliseconds = 0;

	SystemTimeToFileTime(&st1, &ft1);

	// Ȼ��Time Protocolʹ�õĻ�׼ʱ�������ȥ��ʱ�䣨ulTime��
	ULONGLONG *pLLong1 = (ULONGLONG *)&ft1;

	ULONGLONG ulTimehl = ((*pLLong) - (*pLLong1)) / 10000000;
	SYSTEMTIME st2 = LongTime2StTime(ulTimehl);
	return ulTimehl;
}

SYSTEMTIME LongTime2StTime(ULONGLONG ulTime)
{
	// Windows�ļ�ʱ����һ��64λ��ֵ�����Ǵ�1601��1��1������12:00������ʱ������
	// ��λ��1/10,000,000�롣��1000���֮1��(100-nanosecond)
	FILETIME ft;
	SYSTEMTIME st;
	ZeroMemory((void*)&st, sizeof(st));
	// ���Ƚ���׼ʱ�䣨1900��1��1��0��0��0��0���룩ת��ΪWindows�ļ�ʱ��
	st.wYear = 1900;
	st.wMonth = 1;
	st.wDay = 1;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0;

	SystemTimeToFileTime(&st, &ft);

	// Ȼ��Time Protocolʹ�õĻ�׼ʱ�������ȥ��ʱ�䣨ulTime��
	ULONGLONG *pLLong = (ULONGLONG *)&ft;

	/* ע�⣺
	   �ļ�ʱ�䵥λ��1/1000 0000��(��100ns)��
	   ��Ҫ����ʱ��server�ϻ�ȡ������Ϊ��λ��ulTime��һ��ת��
	*/
	*pLLong += (ULONGLONG)10000000 * ulTime;

	// �ٽ�ʱ��ת������������ϵͳʱ��
	FileTimeToSystemTime(&ft, &st);

	//printf("%04d%02d%02d %02d:%02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	return st;
}
ULONGLONG GetNetTime(string srv)
{

	//-----------------------------------------
	// Declare and initialize variables
	WSADATA wsaData;
	int iResult;
	SOCKET sockfd;
	DWORD dwRetval;
	int nTry = 0;
	int i = 1;
	int nRet;
	unsigned long ulTime = 0;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;


	DWORD ipbufferlength = 46;


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		//printf("WSAStartup failed: %d\n", iResult);
		return 0;
	}

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	if (srv.empty())
	{
		dwRetval = getaddrinfo("time.nist.gov", "37", &hints, &result);
	}
	else
	{
		dwRetval = getaddrinfo(srv.c_str(), "37", &hints, &result);
	}
	if (dwRetval != 0) {
		//printf("getaddrinfo failed with error: %d\n", dwRetval);
		WSACleanup();
		return 0;
	}


	// Retrieve each address and print out the hex bytes
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{

		sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (INVALID_SOCKET == sockfd)
			continue;

		int TimeOut = 3000;//���ý��ճ�ʱ6��
		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

		do
		{
			if (5 == nTry++)
			{
				break;
			}

			nRet = connect(sockfd, ptr->ai_addr, ptr->ai_addrlen);
			if (SOCKET_ERROR == nRet)
			{
				continue;
			}

			nRet = recv(sockfd, (char *)&ulTime, sizeof(ulTime), 0);
			if ((SOCKET_ERROR != nRet) && (0 != nRet))
			{
				closesocket(sockfd);
				freeaddrinfo(result);
				WSACleanup();

				ULONGLONG ulTimehl = ntohl(ulTime) + 60 * 60 * 8;
				//SYSTEMTIME st = LongTime2StTime(ulTimehl);
				return ulTimehl;
			}

		} while (1);

		closesocket(sockfd);
	}


	freeaddrinfo(result);
	WSACleanup();
	return 0;



#if 0

	int nRet;
	/* Initialize Winsock */
	WORD wVersionRequested;
	WSADATA wsaData;
	int nErrCode;

	wVersionRequested = MAKEWORD(2, 2);
	nErrCode = WSAStartup(wVersionRequested, &wsaData);
	if (0 != nErrCode)
	{
		return 0;
	}


	

	/* Get server IP */
	struct hostent *host;
	char *pServerIP;

	if (srv.empty())
	{
		host = gethostbyname("time.nist.gov");
	}
	else
	{
		host = gethostbyname(srv.c_str());
	}
	if (NULL == host)
	{
		return 0;
	}

	pServerIP = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);

	/* Connect to time server, and get time */
	SOCKET sockfd;

	char cTimeBuf[40] = { 0 };
	unsigned long ulTime = 0;
	int nTry = 0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == sockfd)
	{
		return 0;
	}
	int TimeOut = 3000;//���ý��ճ�ʱ6��
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&TimeOut, sizeof(TimeOut));

	sockaddr_in    addr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(37);
	addr.sin_addr.s_addr = inet_addr(pServerIP);

	do
	{
		if (5 == nTry++)
		{
			return 0;
		}

		nRet = connect(sockfd, (sockaddr *)&addr, sizeof(addr));
		if (SOCKET_ERROR == nRet)
		{
			continue;
		}

		nRet = recv(sockfd, (char *)&ulTime, sizeof(ulTime), 0);
		if ((SOCKET_ERROR != nRet) && (0 != nRet))
		{
			break;
		}

	} while (1);

	closesocket(sockfd);
	ULONGLONG ulTimehl = ntohl(ulTime) + 60 * 60 * 8;

	//SYSTEMTIME st = LongTime2StTime(ulTimehl);

	return ulTimehl ;
#endif
}