// SERVER.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include < iostream >
#pragma comment( lib, "ws2_32.lib" )
#include < Windows.h >
#include < conio.h >
using namespace std;

#define MY_PORT    9090 // Порт, который слушает сервер

// заведём глобальную переменную  ниже для обмена данным между нитями сервера   
char* cbuff; // общий буфер для записи сообщений от всех клиентов


string decript(string text, int key) {
	string newWord = "";
	for (char ch : text) {
		if (ch < 65 || ch > 122) {
			newWord += ch;
		}
		else if (ch >= 97 && ch <= 122) {
			if (ch - key < 97) {
				newWord += ch - key + 26;
			}
			else {
				newWord += ch - key;
			}
		}
		else if (ch >= 65 || ch <= 90) {
			if (ch - key < 65) {
				newWord += ch - key + 26;
			}
			else {
				newWord += ch - key;
			}
		}
	}
	return newWord;
}


// 	макрос для печати количества активных пользователей 
#define PRINTNUSERS if (nclients)\
  printf("%d user on-line\n",nclients);\
  else printf("No User on line\n");

// прототип функции, обслуживающий подключившихся пользователей
DWORD WINAPI WorkWithClient(LPVOID client_socket);

//будет просто циклически проверять знкачение глобальной переменной
//DWORD WINAPI CheckCommonBuffer(LPVOID client_socket);

// глобальная переменная – количество активных пользователей 
int nclients = 0;
SOCKET massiv_socket[64];

int main(int argc, char* argv[])
{
	char buff[1024];    // Буфер для различных нужд
	string aa;
	int key, sock;

	printf("TCP SERVER \n");
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		// Ошибка!
		printf("Error WSAStartup %d\n",
			WSAGetLastError());
		return -1;
	}

	//  создание сокета
	SOCKET mysocket;
	// AF_INET     - сокет Интернета
	// SOCK_STREAM  - потоковый сокет (с установкой соединения)
	// 0      - по умолчанию выбирается TCP протокол
	//mysocket = socket(AF_INET, SOCK_STREAM, 0);
	if ((mysocket = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		// Ошибка!
		printf("Error socket %d\n", WSAGetLastError());
		WSACleanup();
		// Деиницилизация библиотеки Winsock
		return -1;
	}

	// связываем сокет с локальным адресом
	sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(MY_PORT);
	local_addr.sin_addr.s_addr = 0;  // сервер принимает подключения на все IP-адреса

	// вызываем bind для связывания
	if (bind(mysocket, (sockaddr *)&local_addr,
		sizeof(local_addr)))
	{
		// Ошибка
		perror("bind");
		//exit(2);
		return -1;
	}

	// Подключаем слушателя
	listen(mysocket, 1);
	while (1)
	{
		sock = accept(mysocket, NULL, NULL);
		if (sock < 0)
		{
			perror("accept");
			exit(3);
		}
		else {
			cout << "The connection is established" << endl;
		}

		while (1)
		{

			recv(mysocket, (char*)&key, sizeof(int), 0);
			recv(mysocket, buff, 1024, 0);

			printf("Key:\n", key);
			printf("Slovo\n", aa);

			int k = key;
			string t = buff;
			string tt = decript(buff, k);
			printf("Decript server:\n ", tt);
			printf("Key: (serv)\n", k);
			break;
		}


		// ожидание подключений
		// размер очереди – 0x100
		if (listen(mysocket, 20))
		{
			// Ошибка
			printf("Error listen %d\n", WSAGetLastError());
			closesocket(mysocket);
			WSACleanup();
			return -1;
		}
		
		//printf("Waiting for connection\n");


		//  извлекаем сообщение из очереди
		SOCKET client_socket;	    // сокет для клиента
		sockaddr_in client_addr;    // адрес клиента (заполняется системой)

		// функции accept необходимо передать размер структуры
		int client_addr_size = sizeof(client_addr);

		// цикл извлечения запросов на подключение из  очереди
		/* accept - держит управление и не даёт циклу вращаться
		(то есть не даёт потоку- нити выполняться вообще)
		пока не поступит очередной запрос на соединение*/
		while ((client_socket = accept(mysocket, (sockaddr *)
			&client_addr, &client_addr_size)))
		{
			massiv_socket[nclients] = client_socket;
			nclients++;      // увеличиваем счетчик  подключившихся клиентов

			// пытаемся получить имя хоста
			HOSTENT *hst;
			hst = gethostbyaddr((char *)
				&client_addr.sin_addr.s_addr, 4, AF_INET);

			// вывод сведений о клиенте
			printf("+%s [%s] new connect!\n",
				(hst) ? hst->h_name : "",
				inet_ntoa(client_addr.sin_addr));
			PRINTNUSERS

				// Вызов нового потока для обслужвания клиента
				DWORD thID;
			CreateThread(NULL, NULL, WorkWithClient,
				&client_socket, NULL, &thID);

		}
	
		return 0;

	}
}


// Эта функция создается в отдельном потоке и  обсуживает очередного подключившегося клиента
// независимо от остальных
DWORD WINAPI WorkWithClient(LPVOID client_socket)
{
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	char buff[1024] = "123124";
	//  char buff2[20*1024];
#define sHELLO "Hello, new member!\r\n"
	printf(" \n new thread is started connect!\n");

	// отправляем клиенту приветствие 
	send(my_sock, sHELLO, sizeof(sHELLO), 0);

	// цикл эхо-сервера: прием строки от клиента и возвращение ее клиенту
	int bytes_recv;
	while ((bytes_recv = recv(my_sock, &buff[0], sizeof(buff), 0)) && (bytes_recv != SOCKET_ERROR))
	{
		bytes_recv = recv(my_sock, buff, sizeof(buff), 0);
		cbuff = buff; // пишем в глобальную переменную чтобы другие участники  узнали о сообщении
		send(my_sock, &buff[0], bytes_recv, 0);
	}

	// если мы здесь, то произошел выход из цикла по  причине возращения функцией recv ошибки –
	// соединение клиентом разорвано
	nclients--; // уменьшаем счетчик активных клиентов
	printf("%d-disconnect\n", client_socket); PRINTNUSERS

		closesocket(my_sock);	// закрываем сокет
	return 0;
}