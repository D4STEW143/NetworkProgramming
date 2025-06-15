#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // !WIN32_LEAN_AND_MEAN

#include<Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<iphlpapi.h>
#include<stdio.h>
#include<iostream>
#include<FormatLastError.h>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "FormatLastError.lib")

#define DEFAULT_PORT "27015"
#define DEFALT_BUFFER_LENGTH 1500

void main() {
	setlocale(LC_ALL, "Russian");

	//������������� ������
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		PrintLastError(WSAGetLastError());
		return;
	}

	//��������� �� ����� �� ����
	addrinfo* result = NULL;
	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
	if (iResult) {
		cout << "getaddrinfo() failed with ";
		PrintLastError(WSAGetLastError());
		WSACleanup();
		return;
	}

	//������� �����, ������� ����� ������� � ������� ����������� �� �������
	SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listen_socket == INVALID_SOCKET) {
		cout << "socket() failed with ";
		PrintLastError(WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
	}

	//BindSocket ��������� ����� � ������� IP � ������
	iResult = bind(listen_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		cout << "bind() failed with ";
		PrintLastError(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//�������� ������������� ������
	iResult = listen(listen_socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		cout << "listen() failed with";
		PrintLastError(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//��������� ������� �� ���������� �� ��������
	cout << "Waiting for client..." << endl;
	SOCKET client_socket = accept(listen_socket, NULL, NULL);
	if (client_socket == INVALID_SOCKET) {
		cout << "accpet() failed with";
		PrintLastError(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//��������� � �������� ������
	CHAR recvbuffer[DEFALT_BUFFER_LENGTH] = {};
	do {
		iResult = recv(client_socket, recvbuffer, DEFALT_BUFFER_LENGTH, 0);
		if (iResult > 0) {
			cout << "Received bytes: " << iResult << ", Message: " << recvbuffer << endl;
			if (send(client_socket, recvbuffer, strlen(recvbuffer), 0) == SOCKET_ERROR) {
				cout << "send() failed with";
				PrintLastError(WSAGetLastError());
				break;
			}
		}
		else if (iResult == 0) {
			cout << "Connection closing..." << endl;
		}
		else {
			cout << "recv() failed with";
			PrintLastError(WSAGetLastError());
		}
	} while(iResult>0);

	//������������ �������� ������
	closesocket(client_socket);
	closesocket(listen_socket);
	freeaddrinfo(result);
	WSACleanup();
}