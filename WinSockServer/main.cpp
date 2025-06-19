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

void main() {
	setlocale(LC_ALL, "Russian");

	//Инициализация винсок
	WSADATA wsaData;
	INT iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		PrintLastError(WSAGetLastError());
		return;
	}

	//Проверяем не занят ли порт
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

	//Создаем сокет, который будет слушать и ожидать подключения от клиента
	SOCKET listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listen_socket == INVALID_SOCKET) {
		cout << "socket() failed with ";
		PrintLastError(WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
	}


	//Перевод сокета в неблокирующий режим
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

	//BindSocket связываем сокет с целевым IP и портом
	iResult = bind(listen_socket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		cout << "bind() failed with ";
		PrintLastError(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//Включаем прослушивание сокета
	iResult = listen(listen_socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		cout << "listen() failed with";
		PrintLastError(WSAGetLastError());
		closesocket(listen_socket);
		freeaddrinfo(result);
		WSACleanup();
		return;
	}

	//Принимаем запросы на соединение от клиентов
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

	/*
	Функция getpeername() записывает в структуру sockaddr_in данные о IP и порте клиента
	в поля sin_addr и sin_port. Функцией inet_top() преобразуем IPv4 в строку, а ntohs()
	преобразует порт в ushort. Форматирование выходной строки происходит на этапе вывода
	информации на экран в пунке получение и отправка данных.
	*/
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
	u_short client_port = ntohs(client_addr.sin_port);

	//Получение и отправка данных
	CHAR recvbuffer[DEFALT_BUFFER_LENGTH] = {};
	do {
		iResult = recv(client_socket, recvbuffer, DEFALT_BUFFER_LENGTH, 0);
		if (iResult > 0) {
			printf("Recived %i bytes from %s:%u. Message: \"%s\"\n", iResult, client_IP, client_port, recvbuffer);
			//cout << "Received bytes: " << iResult << ", Message: " << recvbuffer << endl;
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

	//Освобождение ресурсов Винсок
	closesocket(client_socket);
	closesocket(listen_socket);
	freeaddrinfo(result);
	WSACleanup();
}