
#define MAX_REQUEST 1024
#define BUF_SIZE 4096
#define Rev_Buf 32

wchar_t* c2w(const char *str);

class SayHello_C
{
public:
	SayHello_C();
public:
	void SayHello_f(int &bytesSent,int rev_num,char *szRequest);
	void Name_Check_by_id_f(int &bytesSent,int rev_num,char *szRequest);
	void Close_Socket_f(int &bytesSent);
public:
	SOCKET *ClientSocket;
};

SayHello_C::SayHello_C()
{
	//ClientSocket = INVALID_SOCKET;
}

void SayHello_C::SayHello_f(int &bytesSent,int rev_num,char *szRequest)
{
	char name[20];
	char age[10];
	char CS1[60];
	int count = 1;

	do
	{
		if(szRequest[count] == '#')
		{
			count++;
			int count_name = 0;
			int count_age = 0;
			while(szRequest[count] != '#')//获取信息中的姓名信息
			{
				name[count_name] = szRequest[count];
				count_name++;
				count++;
			}
			name[count_name] = '\0';
			count++;

			while(szRequest[count] != '#')//获取信息中的年龄信息
			{
				age[count_age] = szRequest[count];
				count_age++;
				count++;
			}
			age[count_age] = '\0';
		}
		count++;
	}while( szRequest[count] != '\0');

	strcpy(CS1,"Hello,\0");
	strcat(CS1,age);
	strcat(CS1," years old \0");
	strcat(CS1,name);
	strcat(CS1,"!\0");
				
	bytesSent = send(*ClientSocket,CS1,strlen(CS1)+1,0);//返回指定信息给Client,格式为"Hello," + "age" + "years old" + "name"\0	
}

void SayHello_C::Name_Check_by_id_f(int &bytesSent,int rev_num,char *szRequest)
{
	char *id_check = (char *)malloc(rev_num+1);
	char id_check_temp;
	char CS1[20];
	int count = 1;
	do
	{
		id_check_temp = id_check[count-1] = szRequest[count];
		count++;
	}while(id_check_temp != '\0');
				
	if(strcmp(id_check,"15063049\0") == 0)
		strcpy(CS1,"张新明\0"); 
	//此处添加代码，增加更多的学号信息
	else
		strcpy(CS1,"unknow id\0");
	bytesSent = send(*ClientSocket,CS1,strlen(CS1)+1,0);
	free(id_check);
}

void SayHello_C::Close_Socket_f(int &bytesSent)
{
	char ser_close[] = "Close_Connect\0";
	bytesSent = send(*ClientSocket,ser_close,strlen(ser_close)+1,0);
}

DWORD WINAPI CommunicationThread(LPVOID lpParameter)//接受处理线程
{
	DWORD dwTid = GetCurrentThreadId();
	printf("create Thread (%d)\n",dwTid);
	SayHello_C *say_temp = (SayHello_C *) lpParameter;
	char szRequest[MAX_REQUEST];

	char *p = new char[Rev_Buf];
	int iResult;
	int bytesSent;		//The number of data to send

	while(lstrcmpi(LPCWSTR(c2w((const char *)p)),_T("close connect")) != 0)  //circle to rec & send
	{
		//rec
		iResult = recv(*(say_temp->ClientSocket),(char *)szRequest,MAX_REQUEST,0);

		p  = (char *)szRequest;
		printf("no waiting\n");
		if(iResult == 0)			//the client socket is closed
		{
			printf("Connection closing...\n");
			closesocket(*(say_temp->ClientSocket));
			return 1;
		}

		else if(iResult == SOCKET_ERROR)		//the communication failed,becasue 
		{
			printf("recv failed%d\n",WSAGetLastError());
			closesocket(*(say_temp->ClientSocket));
			return 1;
		}

		else if(iResult > 0)			//sucess to rec
		{
			printf("\tCommunicationThread(%d)\tByte recive :%d\n",dwTid,iResult);
			wprintf(_T("\tCommunicationThread(%d)\trequest string is (%s)\n"),dwTid,c2w((const char *)p));
			if(szRequest[0] == '1')
			{
				say_temp->Name_Check_by_id_f(bytesSent,iResult,szRequest);
				if(bytesSent == SOCKET_ERROR)
				{
					printf("\tsend error:%d\n",WSAGetLastError());
					closesocket(*(say_temp->ClientSocket));
					return 1;
				}
			}

			else if(szRequest[0] == '2')
			{
				say_temp->SayHello_f(bytesSent,iResult,szRequest);
				if(bytesSent == SOCKET_ERROR)
				{
					printf("\tsend error:%d\n",WSAGetLastError());
					closesocket(*(say_temp->ClientSocket));
					return 1;
				}
				printf("\tCommunicationThread(%d)\tsend %d bytes\n",dwTid,bytesSent);
			}

			else if(lstrcmpi(LPCWSTR(c2w((const char *)p)),_T("close connect")) == 0)	//close the ser_socket
			{
				say_temp->Close_Socket_f(bytesSent);
				if(bytesSent == SOCKET_ERROR)
				{
					printf("\tsend error:%d\n",WSAGetLastError());
					closesocket(*(say_temp->ClientSocket));
					return 1;
				}
				printf("\tCommunicationThread(%d)\tsend %d bytes\n",dwTid,bytesSent);
				closesocket(*(say_temp->ClientSocket));
			}
			//add other rec here
		}
		else
			return 1;
	}

	printf("The Thread (%d) end\n",dwTid);
	free(say_temp->ClientSocket);
	delete say_temp;
	return 0;
}

wchar_t* c2w(const char *str)

{

	int length = strlen(str)+1;

	wchar_t *t = (wchar_t*)malloc(sizeof(wchar_t)*length);

	memset(t,0,length*sizeof(wchar_t));

	MultiByteToWideChar(CP_ACP,0,str,strlen(str),t,length);

	return t;
}

