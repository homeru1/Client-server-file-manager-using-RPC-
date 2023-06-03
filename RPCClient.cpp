#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "Source_h.h"
#include <locale.h>
#include <conio.h>
#include <cstdio>

#pragma comment (lib, "Rpcrt4.lib")
using namespace std;

#define SIZE 100
#define MAX_BUFF 1048576

int BUFFER[MAX_BUFF];

int GetName(char* path) {
	int i = 0,j=0;
	while (path[i]!='\0') {
		if (path[i] == '\\')j = i;
		i++;
	}
	return j;
}

void REmove(int id) {
	char path[512];// = "C:\\Users\\vaasa\\source\\repos\\RPCServer\\RPCServer\\test.txt";
	cout << "Please enter the path to the file:";
	cin >> path;
	int state = Remove((const unsigned char*)path, id);
	if (state == 1) {
		cout << "Done!" << endl;
	}
	else if (state == -1) {
		cout << "Impersonation error" << endl;
		return;
	}
	else if (state == -2) {
		cout << "Not enough privileges" << endl;
		return;
	}
}

void DOwnload(int id) {
	char path[512];// = "C:\\Users\\vaasa\\source\\repos\\RPCServer\\RPCServer\\test.txt";
	cout << "Please enter the path to the file:";
	cin >> path;
	int tmp = GetName(path);
	char name[128];
	strcpy(name, &path[tmp + 1]);
	int count = 1024, result = 1;
	int buffer[1024];
	count = Download(buffer, (const unsigned char*)path, id);
	if (count == -1) {
		cout << "Impersonation error" << endl;
		return;
	}
	else if (count == -2) {
		cout << "Access denied" << endl;
		return;
	}
	FILE* open = fopen(name, "w");
	fwrite(buffer, sizeof(int), count, open);
	while (count == 1024) {
		count = Download(buffer, (const unsigned char*)path, id);
			fwrite(buffer, sizeof(int), count, open);
	}
	fclose(open);
}

void UPload(int id) {
	char path[512];// = "C:\\Users\\vaasa\\source\\repos\\RPCClient\\RPCClient\\test.txt";
	char dest[512];
	cout << "Please enter the path to the file:";
	cin >> path;
	cout << "Please enter the destination of the file:";
	cin >> dest;
	FILE* open = fopen(path, "r");
	int tmp = GetName(path);
	char name[128];
	strcpy(name, &path[tmp]);
	char buffer[1024];
	bool uploading = 1;
	int count = 1024, result = 1, size = 0;
	char a[] = "\\";
	strcat(dest, a);
	strcat(dest, name);
	while (count==1024) {
		count = fread(buffer, 1, 1024, open);
		result = Upload((const unsigned char*)buffer, (const unsigned char*)dest, count, size,id);
		if (result == -1) {
			cout << "Impersonation error" << endl;
			fclose(open);
			return;
		}
		else if (result == -2) {
			cout << "Access denied" << endl;
			fclose(open);
			return;
		}
		size += count;
	}
	//Upload((const unsigned char*)buffer, (const unsigned char*)name, 1, -1, id);
	fclose(open);
}

int main() {
	setlocale(LC_ALL, "Rus");
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	RPC_STATUS status;
	RPC_CSTR szStringBinding = NULL;

	//Функция RpcStringBindingCompose создает дескриптор привязки строки.
	char IP[32];
	char Port[8];
	//cout << "IP: ";
	//cin >> IP;
	//cout << "Port: ";
	//cin >> Port;
	status = RpcStringBindingComposeA(
		NULL,                                        // UUID to bind to.
		(RPC_CSTR)("ncacn_ip_tcp"),					// Use TCP/IP protocol.
		(RPC_CSTR)("192.168.31.69"),				// TCP/IP network address to use.  // 192.168.31.69  localhost
		(RPC_CSTR)("9000"),					// TCP/IP port to use.
		NULL,										// Protocol dependent network options to use.
		&szStringBinding);							// String binding output.

	if (status)
	{
		cout << "Connect error." << endl;
		exit(status);
	}

	//Функция RpcBindingFromStringBinding возвращает дескриптор привязки из строкового представления дескриптора привязки.
	status = RpcBindingFromStringBindingA(
		szStringBinding,        // The string binding to validate.
		&hRPCBinding);     // Put the result in the implicit binding
								// handle defined in the IDL file.
	if (status)
		exit(status);
	char login[100] = "\0";
	char password[100] = "\0";
	int state = -1;//-1;
	while (state<0) {
		cout << "login: ";
		cin >> login;
		cout << "Password: ";
		cin >> password;
		cout << "Entering the server...: ";
		state = Log_in((const unsigned char*)login, (const unsigned char*)password);
		if (state == -1) {
			cout << "Too many users, what a bit and try again" << endl;
			Sleep(100);
		}
		else if(state == -2) {
			cout << "Incorrect username or password try again" << endl;
			cout << "login: ";
			cin >> login;
			cout << "Password: ";
			cin >> password;
		}
		else if (state == -3) {
			cout << "Impersonation error" << endl;
			cout << "login: ";
			cin >> login;
			cout << "Password: ";
			cin >> password;
		}
	}
	system("cls");
	cout << "Enter is succesful!, Hello " << login << endl;
	int tmp = 1;
	RpcTryExcept
	{
		while (tmp!=4) {
			cout << "What to do: \n1)Upload file \n2)Download file\n3)Delete file from the server\n4)Exit";
			cin >> tmp;
			system("cls");
			switch (tmp)
			{
			case 1:
				UPload(state);
				break;
			case 2:
				DOwnload(state);
				break;
			case 3:
				REmove(state);
				break;
			case 4:
			break;
			default:
			break;
			}
		}
		Output((const unsigned char*)"Hello RPC World!");
	}
		RpcExcept(1)
	{
		std::cerr << "Runtime reported exception " << RpcExceptionCode()
			<< std::endl;
	}
	RpcEndExcept

		// Free the memory allocated by a string.
		status = RpcStringFreeA(
			&szStringBinding); // String to be freed.

	if (status)
		exit(status);

	// Releases binding handle resources and disconnects from the server.
	status = RpcBindingFree(
		&hRPCBinding); // Frees the implicit binding handle defined in the IDL file.

	if (status)
		exit(status);
}

void* __RPC_USER midl_user_allocate(size_t size)
{
	return malloc(size);
}

// Memory deallocation function for RPC.
void __RPC_USER midl_user_free(void* p)
{
	free(p);
}