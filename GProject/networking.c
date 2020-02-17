#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "networking.h"

#define BUFSIZE 512

#pragma comment(lib,"ws2_32.lib")

int send_score(char* name, int score, int map_number, char* play_data)
{
	int retval;
	char buf[BUFSIZE];

	//윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return -1;

	//socket()
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) return 0;

	HOSTENT* remoteHost = gethostbyname("rhythm_rinmin.kys");//웹주소를 ip로 바꾸기 위해
	//connect()
	SOCKADDR_IN serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(80);
	serveraddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) * remoteHost->h_addr_list));
	retval = connect(sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));

	if (retval == SOCKET_ERROR) return 0;

	char msg[500] = {NULL};
	sprintf(msg, "GET /Handle.php?name=%s&score=%d&map=%d&p_data=%s HTTP/1.1\r\n",name,score,map_number,play_data);
	strcat(msg, "Host: rhythm_rinmin.kys:80\r\n\r\n");//웹서버로 보낼 헤더

	send(sock, msg, strlen(msg), 0);//웹서버로 보내기

	while (1)
	{
		if ((retval = recv(sock, buf, BUFSIZE, 0)) >= 0) break;//서버에서 응답 받기

		printf("데이터 저장중..");
		//printf("%s", buf);//화면에 출력(너무 많아서 짤려서 보임)

		memset(buf, 0, BUFSIZE);//버퍼 지우기
	}
	closesocket(sock);
	//윈속 종료
	WSACleanup();
	return 0;
}