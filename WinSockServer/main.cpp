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
#include<fcntl.h>


using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "FormatLastError.lib")

#define DEFAULT_PORT "27015"
#define DEFALT_BUFFER_LENGTH 1500

CONST CHAR g_OVERFLOW[DEFALT_BUFFER_LENGTH] = "Too many connections. Try again later.";
CONST INT MAX_CONNECTIONS = 3;
SOCKET sockets[MAX_CONNECTIONS] = {};
DWORD dwTreadsIds[MAX_CONNECTIONS] = {};
HANDLE hTreads[MAX_CONNECTIONS] = {};

VOID ClientHandler(SOCKET client_socket);

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


	//������� ������ � ������������� �����
	u_long iMode = 0;
	iResult = ioctlsocket(listen_socket, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR) {
		cout << "ioctlsocket() failed with ";
		PrintLastError(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}
	HWND cmdHwnd = GetConsoleWindow();

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

	INT i = 0;
	cout << "Waiting for client..." << endl;
	do
	{

		SOCKET client_socket = accept(listen_socket, NULL, NULL);
		if (client_socket == INVALID_SOCKET) {
			cout << "accpet() failed with";
			PrintLastError(WSAGetLastError());
			closesocket(listen_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}

		/*
		������� getpeername() ���������� � ��������� sockaddr_in ������ � IP � ����� �������
		� ���� sin_addr � sin_port. �������� inet_top() ����������� IPv4 � ������, � ntohs()
		����������� ���� � ushort. �������������� �������� ������ ���������� �� ����� ������
		���������� �� ����� � ����� ��������� � �������� ������.
		sockaddr_in client_addr;
		int client_addr_size = sizeof(client_addr);
		iResult = getpeername(client_socket, (sockaddr*)&client_addr, &client_addr_size);
		if (iResult == SOCKET_ERROR) {
			PrintLastError(WSAGetLastError());
			closesocket(listen_socket);
			closesocket(client_socket);
			freeaddrinfo(result);
			WSACleanup();
			return;
		}
		char client_IP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(client_addr.sin_addr), client_IP, INET_ADDRSTRLEN);
		u_short client_port = ntohs(client_addr.sin_port);*/
		if (i < MAX_CONNECTIONS) {
			sockets[i] = client_socket;
			hTreads[i] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ClientHandler,
				(LPVOID)sockets[i],
				0,
				&dwTreadsIds[i]
			);
			i++;
		}
		else {

			CHAR recv_buffer[DEFALT_BUFFER_LENGTH] = {};
			INT iResult = recv(client_socket, recv_buffer, DEFALT_BUFFER_LENGTH, 0);
			cout << "Extra client sends: " << recv_buffer << endl;
			send(client_socket, g_OVERFLOW, strlen(g_OVERFLOW), 0);
			closesocket(client_socket);
		}
		//ClientHandler(client_socket);
	} while (true);
	//������������ �������� ������
	WaitForMultipleObjects(MAX_CONNECTIONS, hTreads, TRUE, INFINITE);
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		CloseHandle(hTreads[i]);
		closesocket(sockets[i]);
	}
	closesocket(listen_socket);
	freeaddrinfo(result);
	WSACleanup();
}

VOID ClientHandler(SOCKET client_socket) {
	//��������� � �������� ������
	INT iResult = 0;
	CHAR recvbuffer[DEFALT_BUFFER_LENGTH] = {};
	do {
		iResult = recv(client_socket, recvbuffer, DEFALT_BUFFER_LENGTH, 0);
		if (iResult > 0) {
			//printf("Recived %i bytes from %s:%u. Message: \"%s\"\n", iResult, client_IP, client_port, recvbuffer);
			cout << "Received bytes: " << iResult << ", Message: " << recvbuffer << endl;
			for (int i = 0; i < MAX_CONNECTIONS; i++)
			{
				if (sockets[i])
				{
					if (send(sockets[i], recvbuffer, strlen(recvbuffer), 0) == SOCKET_ERROR) {
						cout << "send() failed with";
						PrintLastError(WSAGetLastError());
						break;
					}
				}
			}
		}
		else if (iResult == 0) {
			cout << "Connection closing..." << endl;
		}
		else {
			cout << "recv() failed with";
			PrintLastError(WSAGetLastError());
		}
		ZeroMemory(recvbuffer, DEFALT_BUFFER_LENGTH);
	} while (iResult > 0);

	closesocket(client_socket);
}