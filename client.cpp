#define WIN32_LEAN_AND_MEAN

#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <array>
#include <vector>
#include <bitset>
#include <string>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 23

class message {
public:
	//char mes1[80];
	//char mes2[144];
	//char mes3[112];
	//char mes4[88];
	//header
	int oper; //2b: 00 - con_neg; 01 - 2arg_oper; 10 - factorial, 11 - history
	int status; //3b: 000 - correct; 001 - incorrect_arg; 010 - result_beyond_the_scope; 011 - too_many_calculations_for_session; 100 - incorrect_session_id; 101 - incorrect_calculation_id; 110 - message_can't_be_read; 111 - server_internal_error;
	long long data_length; //64b: data field in bits
	//data
	int s_id; //session id 8b
	int c_id; //calculation id 8b
	int history; //1b : (ONLY FOR oper = 11 and answer) 0 - two_arg_oper; 1 - factorial 
	int arg; //(ONLY FOR oper={01;11;10}) 32b
	int arg2; //(ONLY FOR oper={01;11}) 32b
	int two_arg_oper;  //(ONLY FOR oper={01;11}) 2b: 00 - multiplication; 01 - division; 10 - addition; 11 - substraction
	int result; //(ONLY FOR oper={01;10;11}) 32b

	//constructors and destructors
	message() : oper(0), status(0), data_length(0), two_arg_oper(0), arg(0), arg2(0), result(0), s_id(0), c_id(0), history(0) {}

	message(char buffer[]) {
		//unpack data from char[]
		std::bitset<80> pak(0);
		//first bit in pak
		int temp = 79;
		for (int i = 0; i < 10; i++) {
			//divide into bytes
			std::bitset<8> temp_bitset(buffer[i]);
			for (int j = 7; j >= 0; j--) {
				pak[temp] = temp_bitset[j];
				temp--;
			}
		}
		//unpack header fields
		std::bitset<2> bs_oper(0);
		std::bitset<3> bs_status(0);
		std::bitset<64> bs_length(0);
		for (int i = 79; i >= 78; i--) bs_oper[i - 78] = pak[i];
		for (int i = 77; i >= 75; i--) bs_status[i - 75] = pak[i];
		for (int i = 74; i >= 11; i--) bs_length[i - 11] = pak[i];
		oper = static_cast<int>(bs_oper.to_ulong());
		status = static_cast<int>(bs_status.to_ulong());
		data_length = static_cast<long long>(bs_length.to_ulong());
		//unpack data fields
		std::bitset<8> bs_sid(0);
		for (int i = 10; i >= 3; i--) bs_sid[i - 3] = pak[i];
		s_id = static_cast<int>(bs_sid.to_ulong());
		if (status == 0) {
			//unpack data fields
			switch (oper) {
				//con_neg - fields empty
			case 0: {
				c_id = arg = arg2 = two_arg_oper = result = history = 0;
				break;
			}
					//two_arg_oper - res
			case 1: {
				std::bitset<48> pak2(0);
				//first bit in pak
				temp = 47;
				for (int i = 9; i < 15; i++) {
					//divide into bytes
					std::bitset<8> temp_bitset(buffer[i]);
					for (int j = 7; j >= 0; j--) {
						pak2[temp] = temp_bitset[j];
						temp--;
					}
				}
				std::bitset<8> bs_cid(0);
				for (int i = 42; i >= 35; i--) bs_cid[i - 35] = pak2[i];
				c_id = static_cast<int>(bs_cid.to_ulong());
				std::bitset<32> bs_res(0);
				for (int i = 34; i >= 3; i--) bs_res[i - 3] = pak2[i];
				result = static_cast<int>(bs_res.to_ulong());
				arg = arg2 = two_arg_oper = history = 0;
				std::cout << "Otrzymano wynik: " << result << std::endl;
				break;
			}
					//factorial - res
			case 2: {
				std::bitset<48> pak2(0);
				//first bit in pak
				temp = 47;
				for (int i = 9; i < 15; i++) {
					//divide into bytes
					std::bitset<8> temp_bitset(buffer[i]);
					for (int j = 7; j >= 0; j--) {
						pak2[temp] = temp_bitset[j];
						temp--;
					}
				}
				std::bitset<8> bs_cid(0);
				for (int i = 42; i >= 35; i--) bs_cid[i - 35] = pak2[i];
				c_id = static_cast<int>(bs_cid.to_ulong());
				std::bitset<32> bs_res(0);
				for (int i = 34; i >= 3; i--) bs_res[i - 3] = pak2[i];
				result = static_cast<int>(bs_res.to_ulong());
				arg = arg2 = two_arg_oper = history = 0;
				std::cout << "Otrzymano wynik: " << result << std::endl;
				break;
			}
					//history - all fields
			case 3: {
				std::bitset<16> pak2(0);
				//first bit in pak
				temp = 15;
				for (int i = 9; i < 11; i++) {
					//divide into bytes
					std::bitset<8> temp_bitset(buffer[i]);
					for (int j = 7; j >= 0; j--) {
						pak2[temp] = temp_bitset[j];
						temp--;
					}
				}
				std::bitset<8> bs_cid(0);
				for (int i = 10; i >= 3; i--) bs_cid[i - 3] = pak2[i];
				c_id = static_cast<int>(bs_cid.to_ulong());
				std::bitset<1> bs_hist;
				bs_hist[0] = pak2[2];
				history = static_cast<int>(bs_hist.to_ulong());
				//two arg oper
				if (history == 0) {
					std::bitset<104> pak2(0);
					//first bit in pak
					temp = 103;
					for (int i = 10; i < 23; i++) {
						//divide into bytes
						std::bitset<8> temp_bitset(buffer[i]);
						for (int j = 7; j >= 0; j--) {
							pak2[temp] = temp_bitset[j];
							temp--;
						}
					}
					std::bitset<32> bs_arg(0);
					for (int i = 97; i >= 66; i--) bs_arg[i - 66] = pak2[i];
					arg = static_cast<int>(bs_arg.to_ulong());
					std::bitset<32> bs_arg2(0);
					for (int i = 65; i >= 34; i--) bs_arg2[i - 34] = pak2[i];
					arg2 = static_cast<int>(bs_arg2.to_ulong());
					std::bitset<2> bs_tao(0);
					for (int i = 33; i >= 32; i--) bs_tao[i - 32] = pak2[i];
					two_arg_oper = static_cast<int>(bs_tao.to_ulong());
					std::bitset<32> bs_res(0);
					for (int i = 31; i >= 0; i--) bs_res[i] = pak2[i];
					result = static_cast<int>(bs_res.to_ulong());
					std::cout << "Otrzymano wpis z historii: " << arg;
					switch (two_arg_oper) {
					case 0: std::cout << " * "; break;
					case 1: std::cout << " / "; break;
					case 2: std::cout << " + "; break;
					case 3: std::cout << " - "; break;
					}
					std::cout << arg2 << " = " << result << std::endl;
				}
				//factorial
				else {
					std::bitset<72> pak2(0);
					//first bit in pak
					temp = 71;
					for (int i = 10; i < 19; i++) {
						//divide into bytes
						std::bitset<8> temp_bitset(buffer[i]);
						for (int j = 7; j >= 0; j--) {
							pak2[temp] = temp_bitset[j];
							temp--;
						}
					}
					std::bitset<32> bs_arg(0);
					for (int i = 65; i >= 34; i--) bs_arg[i - 34] = pak2[i];
					arg = static_cast<int>(bs_arg.to_ulong());
					std::bitset<32> bs_res(0);
					for (int i = 33; i >= 2; i--) bs_res[i - 2] = pak2[i];
					result = static_cast<int>(bs_res.to_ulong());
					arg2 = two_arg_oper = 0;
					std::cout << "Otrzymano wpis z historii: " << arg << "! = " << result << std::endl;
				}
				break;
			}
			}
		}
		else {
			c_id = arg = arg2 = two_arg_oper = result = history = 0;
			switch (status) {
			case 1: std::cout << "Otrzymano komunikat bledu: Niepoprawny argument" << std::endl; break;
			case 2: {
				if (oper == 3) {
					std::cout << "Zakonczono wysylanie historii sesji" << std::endl;
				}
				else {
					std::cout << "Otrzymano komunikat bledu: Rezultat poza zakresem" << std::endl;
				}
				break;
			}
			case 3: std::cout << "Otrzymano komunikat bledu: Przekroczono limit obliczen na sesje" << std::endl; break;
			case 4: std::cout << "Otrzymano komunikat bledu: Niepoprawne id sesji" << std::endl; break;
			case 5: std::cout << "Otrzymano komunikat bledu: Niepoprawne id dzialania" << std::endl; break;
			case 6: std::cout << "Otrzymano komunikat bledu: Wiadomosc nie moze byc odczytana" << std::endl; break;
			case 7: std::cout << "Otrzymano komunikat bledu: Wewnetrzny blad serwera" << std::endl; break;
			}
		}
	}

	~message() {}

	//pack data to string
	std::string pack_to_string() {
		//match message form to type of operation
		switch (oper) {
			//con_neg
		case 0: {
			//all fields into one bitset
			std::bitset<80> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
				std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + "000"));
			std::string temp;
			std::bitset<8> tempBitset(0);
			//podział dużego bitsetu na bajty
			for (int i = pak.size() - 1; i >= 0; i--)
			{
				tempBitset[i % 8] = pak[i];
				if (i % 8 == 0)
				{
					//zamiana bajtów na chary
					char t = static_cast<char> (tempBitset.to_ulong());
					//łączenie w pojedynczy string
					temp.push_back(t);
				}
			}
			return temp;
		}
		case 1: {//2arg_oper
			//all fields into one bitset
			std::bitset<144> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
				std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + std::bitset<32>(arg).to_string() +
				std::bitset<32>(arg2).to_string() + std::bitset<2>(two_arg_oper).to_string() + "0"));
			std::string temp;
			std::bitset<8> tempBitset(0);
			//podział dużego bitsetu na bajty
			for (int i = pak.size() - 1; i >= 0; i--)
			{
				tempBitset[i % 8] = pak[i];
				if (i % 8 == 0)
				{
					//zamiana bajtów na chary
					char t = static_cast<char> (tempBitset.to_ulong());
					//łączenie w pojedynczy string
					temp.push_back(t);
				}
			}
			return temp;
		}
		case 2: {//factorial
			//all fields into one bitset
			std::bitset<112> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
				std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + std::bitset<32>(arg).to_string() +
				"000"));
			std::string temp;
			std::bitset<8> tempBitset(0);
			//podział dużego bitsetu na bajty
			for (int i = pak.size() - 1; i >= 0; i--)
			{
				tempBitset[i % 8] = pak[i];
				if (i % 8 == 0)
				{
					//zamiana bajtów na chary
					char t = static_cast<char> (tempBitset.to_ulong());
					//łączenie w pojedynczy string
					temp.push_back(t);
				}
			}
			return temp;
		}
		case 3: {//history
			//all fields into one bitset
			std::bitset<88> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
				std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + std::bitset<8>(c_id).to_string() +
				"000"));
			std::string temp;
			std::bitset<8> tempBitset(0);
			//podział dużego bitsetu na bajty
			for (int i = pak.size() - 1; i >= 0; i--)
			{
				tempBitset[i % 8] = pak[i];
				if (i % 8 == 0)
				{
					//zamiana bajtów na chary
					char t = static_cast<char> (tempBitset.to_ulong());
					//łączenie w pojedynczy string
					temp.push_back(t);
				}
			}
			return temp;
		}
		}
	}
};

int user_interface(int s_id) {
	int temp;
	std::cout << "\nNumer obecnej sesji: " << s_id << std::endl;
	std::cout << "Co chcesz zrobic?\n0 - zakonczyc polaczenie\n1 - wykonac operacje dwuargumentowa\n2 - obliczyc silnie\n"
		"3 - przejrzec historie obliczen\n";
	std::cin >> temp;
	return temp;
}

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	int iResult;
	int session_id = 0;

	std::string ip_addr;

	std::cout << "Podaj adres IP serwera" << std::endl;
	std::cin >> ip_addr;
	const char* ip = ip_addr.c_str();
	//const char* ip = "127.0.0.1";        
	ULONG* IPaddr = new ULONG;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	//Create a Socket for connecting to server
	ConnectSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	inet_pton(AF_INET, ip, IPaddr);
	addr.sin_addr.s_addr = *IPaddr;

	//char buffer[DEFAULT_BUFLEN];
	int len = sizeof(sockaddr_in);
	char buffer[DEFAULT_BUFLEN];

	//initialize connection
	message* request = new message();
	request->oper = 0;
	request->status = 0;
	request->data_length = 8;
	request->s_id = 0;

	//send request
	std::string temp_str = request->pack_to_string();
	memcpy(buffer, temp_str.c_str(), 10);
	iResult = sendto(ConnectSocket, buffer, 10, 0, (sockaddr*)&addr, len);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	//receive s_id
	iResult = recvfrom(ConnectSocket, buffer, DEFAULT_BUFLEN, 0, (sockaddr*)&addr, &len);
	if (iResult == SOCKET_ERROR) {
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	message* answer = new message(buffer);
	session_id = answer->s_id;

	bool end = false;
	//connection
	do {
		int option = user_interface(session_id);
		std::cout << "Wybrano: " << option << std::endl;
		request = new message();
		switch (option) {
		case 0: {
			request->oper = 0;
			request->status = 0;
			request->data_length = 8;
			request->s_id = session_id;
			if (option == 0) end = true;
			break;
		}
		case 1: {
			int ar;
			int ar2;
			int op;
			std::cout << "Wybierz dzialanie\n0 - mnozenie\n1 - dzielenie\n2 - dodawanie\n3 - odejmowanie\n";
			std::cin >> op;
			std::cout << "Podaj pierwszy argument\n";
			std::cin >> ar;
			std::cout << "Podaj drugi argument\n";
			std::cin >> ar2;
			request->oper = 1;
			request->status = 0;
			request->data_length = 74;
			request->two_arg_oper = op;
			request->arg = ar;
			request->arg2 = ar2;
			request->s_id = session_id;
			break;
		}
		case 2: {
			long long arg;
			std::cout << "Podaj liczbe ktorej silnie chcesz obliczyc (maks 12)\n";
			std::cin >> arg;
			request->oper = 2;
			request->status = 0;
			request->data_length = 40;
			request->arg = arg;
			request->s_id = session_id;
			break;
		}
		case 3: {
			int ses;
			int calc;
			int option;
			std::cout << "Wybierz opcje\n0 - wyswietlanie calej sesji\n1 - wyswietlenie pojedynczego dzialania\n";
			std::cin >> option;
			if (option == 0) {
				std::cout << "Podaj numer sesji ktora chcesz przejrzec\n";
				std::cin >> ses;
				calc = 0;
				request->oper = 3;
				request->status = 0;
				request->data_length = 16;
				request->s_id = ses;
				request->c_id = calc;
				//send request
				std::string temp_str = request->pack_to_string();
				memcpy(buffer, temp_str.c_str(), 11);
				iResult = sendto(ConnectSocket, buffer, 11, 0, (sockaddr*)&addr, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				bool temp = true;
				do {
					//receiv answer
					iResult = recvfrom(ConnectSocket, buffer, DEFAULT_BUFLEN, 0, (sockaddr*)&addr, &len);
					if (iResult == SOCKET_ERROR) {
						printf("recv failed with error: %d\n", WSAGetLastError());
						closesocket(ConnectSocket);
						WSACleanup();
						return 1;
					}
					answer = new message(buffer);
					if (answer->status != 0) temp = false;

				} while (temp);
			}
			else if (option == 1) {
				std::cout << "Podaj numer sesji w ktorej zlecone bylo interesujace cie dzialanie" << std::endl;
				std::cin >> ses;
				std::cout << "Podaj numer dzialania" << std::endl;
				std::cin >> calc;
				request->oper = 3;
				request->status = 0;
				request->data_length = 16;
				request->s_id = ses;
				request->c_id = calc;
				//send request
				std::string temp_str = request->pack_to_string();
				memcpy(buffer, temp_str.c_str(), 11);
				iResult = sendto(ConnectSocket, buffer, 11, 0, (sockaddr*)&addr, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				iResult = recvfrom(ConnectSocket, buffer, DEFAULT_BUFLEN, 0, (sockaddr*)&addr, &len);
				if (iResult == SOCKET_ERROR) {
					printf("recv failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				answer = new message(buffer);
			}
		}
		}
		if (request->oper != 3) {
			//send request
			std::string temp_str = request->pack_to_string();
			if (request->oper == 0) {
				memcpy(buffer, temp_str.c_str(), 10);
				iResult = sendto(ConnectSocket, buffer, 18, 0, (sockaddr*)&addr, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			else if (request->oper == 1) {
				memcpy(buffer, temp_str.c_str(), 18);
				iResult = sendto(ConnectSocket, buffer, 18, 0, (sockaddr*)&addr, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			else if (request->oper == 2) {
				memcpy(buffer, temp_str.c_str(), 14);
				iResult = sendto(ConnectSocket, buffer, 14, 0, (sockaddr*)&addr, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}
			//receiv answer
			iResult = recvfrom(ConnectSocket, buffer, DEFAULT_BUFLEN, 0, (sockaddr*)&addr, &len);
			if (iResult == SOCKET_ERROR) {
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			answer = new message(buffer);
		}
	} while (!end);

	printf("Connection closed\n");
	// cleanup
	delete IPaddr;
	closesocket(ConnectSocket);
	WSACleanup();

	return 0;
}