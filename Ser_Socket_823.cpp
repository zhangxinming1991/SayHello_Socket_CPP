// Ser_Socket_823.cpp : Defines the entry point for the console application.


#include "stdafx.h"
#include <winsock2.h>
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT <= 0x0500)
#define _WSPIAPI_COUNTOF
#endif

#include "Ser_Socket_823.h"
#include <ws2tcpip.h>

#include <iostream>
using namespace std;

#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_PORT "5000"

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA wsaData;
	SOCKET ListenSocket = INVALID_SOCKET;

	int iResult;
	//Load the ws2_32.dll
	iResult = WSAStartup(MAKEWORD(2,2),&wsaData);
	if(iResult != 0)
	{
		printf("WSAStartup Failed:%d",WSAGetLastError());
		return 1;
	}
////////////////////////////
	hostent host;
	ZeroMemory(&host,sizeof(hostent));
	LPCTSTR wHost = _T("");
	char *hostname = new char[20];
	size_t i;
	wcstombs_s(&i,hostname,20,wHost,20);
	host = *gethostbyname(hostname);
	struct in_addr *host_addr = new struct in_addr;
	ZeroMemory(host_addr,sizeof(in_addr));
	host_addr->S_un.S_addr = *(unsigned long *)(*(host.h_addr_list));
	char *hostaddr_pri;
	hostaddr_pri = inet_ntoa(*host_addr);
	printf("Server IP: %s\n",inet_ntoa(*host_addr));
	printf("Server Port:%d\n",5000);
////////////////////////
	struct addrinfo *hints = new struct addrinfo;
	ZeroMemory(hints,sizeof(struct addrinfo));
	hints->ai_family = AF_INET;			//IPV4
	hints->ai_socktype = SOCK_STREAM;	//TCP
	hints->ai_protocol = IPPROTO_TCP;	//TCP
	hints->ai_flags = 0x01;				//Use the address in the bind
	struct addrinfo *result = new struct addrinfo;
	ZeroMemory(result,sizeof(struct addrinfo));
	iResult = getaddrinfo(host.h_name, DEFAULT_PORT, hints, &result);
	if(iResult != 0)
	{
		printf("getaddrinfo failed :%d\n",iResult);
		//WSACleanup();
		goto END1;
	}
	//socket
	ListenSocket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
	if(ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed:%ld\n",WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	//bind
	iResult = bind(ListenSocket,result->ai_addr,(int)result->ai_addrlen);
	if(iResult == SOCKET_ERROR)
	{
		printf("bind failed:%d\n",WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		goto END1;
	}
	printf("bind\n");
	//listen
	iResult = listen(ListenSocket,SOMAXCONN);
	printf("start listen...\n");
	if(iResult == SOCKET_ERROR)
	{
		printf("listen failed %d\n",GetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		goto END1;
	}
	int count = 0;
	while(1)
	{
		printf("ready to accept\n");

		SayHello_C *say_object = new SayHello_C();
		say_object->ClientSocket = (SOCKET *)malloc(sizeof(SOCKET));

		*(say_object->ClientSocket) = accept(ListenSocket,NULL,NULL);
		if(*(say_object->ClientSocket) == INVALID_SOCKET)
		{
			printf("accept failed:%d\n",GetLastError());
			closesocket(ListenSocket);
			free(say_object->ClientSocket);
			break;
		}

		printf("accept a connetion\n");

		//Create the Thread to rec & send
		if(!CreateThread(NULL,0,CommunicationThread,(LPVOID)say_object,0,NULL))
		{
			printf("Create Thread error(%d)\n",GetLastError());
			free(say_object->ClientSocket);
			break;
		}
	}


END1:
	freeaddrinfo(result);
	delete hints;
	delete []hostname;
	delete host_addr;
	WSACleanup();
	return 0;	
}