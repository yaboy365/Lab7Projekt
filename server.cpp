#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "pch.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <bitset>
#include <stdexcept>
//#include <random>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 23 //23 bytse - 184 bits

class message {
public:
	//std::string mes;
	//std::bitset<280> pack;
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
		switch (oper) {
			//con_neg - fields empty
		case 0: {
			c_id = arg = arg2 = two_arg_oper = result = history = 0;
			if (s_id == 0) std::cout << "Otrzymano zadanie zakonczenia polaczenia: " << std::endl;
			break;
		}
				//two_arg_oper - two_arg_oper, arg, arg2
		case 1: {
			std::bitset<72> pak2(0);
			//first bit in pak
			temp = 71;
			for (int i = 9; i < 18; i++) {
				//divide into bytes
				std::bitset<8> temp_bitset(buffer[i]);
				for (int j = 7; j >= 0; j--) {
					pak2[temp] = temp_bitset[j];
					temp--;
				}
			}
			std::bitset<32> bs_arg(0);
			for (int i = 66; i >= 35; i--) bs_arg[i - 35] = pak2[i];
			arg = static_cast<int>(bs_arg.to_ulong());
			std::bitset<32> bs_arg2(0);
			for (int i = 34; i >= 3; i--) bs_arg2[i - 3] = pak2[i];
			arg2 = static_cast<int>(bs_arg2.to_ulong());
			std::bitset<2> bs_tao(0);
			for (int i = 2; i >= 1; i--) bs_tao[i - 1] = pak2[i];
			two_arg_oper = static_cast<int>(bs_tao.to_ulong());
			c_id = result = history = 0;
			std::cout << "Otrzymano zadanie wykonania ";
			switch (two_arg_oper) {
			case 0: std::cout << "mnozenia "; break;
			case 1: std::cout << "dzielenia "; break;
			case 2: std::cout << "dodawania "; break;
			case 3: std::cout << "odejmowania "; break;
			}
			std::cout << "liczb " << arg << " oraz " << arg2 << std::endl;
			break;
		}
				//factorial - arg
		case 2: {
			std::bitset<40> pak2(0);
			//first bit in pak
			temp = 39;
			for (int i = 9; i < 14; i++) {
				//divide into bytes
				std::bitset<8> temp_bitset(buffer[i]);
				for (int j = 7; j >= 0; j--) {
					pak2[temp] = temp_bitset[j];
					temp--;
				}
			}
			std::bitset<32> bs_arg(0);
			for (int i = 34; i >= 3; i--) bs_arg[i - 3] = pak2[i];
			arg = static_cast<int>(bs_arg.to_ulong());
			c_id = arg2 = two_arg_oper = result = history = 0;
			std::cout << "Otrzymano zadanie obliczenia silni dla " << arg << std::endl;
			break;
		}
				//history - fields empty
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
			arg = arg2 = two_arg_oper = result = 0;
			std::cout << "Otrzymano zadanie wyswietlenia historii" << std::endl;
			break;
		}
		}
	}

	~message() {}

	//pack data to string
	std::string pack_to_string() {
		//char temp_ch = 0;
		//any errors
		if (status == 0) {
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
				std::bitset<120> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
					std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + std::bitset<8>(c_id).to_string() +
					std::bitset<32>(result).to_string() + "000"));
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
				std::bitset<120> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
					std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + std::bitset<8>(c_id).to_string()) +
					std::bitset<32>(result).to_string() + "000");
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
				//for two_arg_oper
				if (history == 0) {
					std::bitset<184> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
						std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + std::bitset<8>(c_id).to_string()) +
						std::bitset<1>(history).to_string() + std::bitset<32>(arg).to_string() + std::bitset<32>(arg2).to_string() +
						std::bitset<2>(two_arg_oper).to_string() + std::bitset<32>(result).to_string());
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
				//for factorial oper
				else if (history == 1) {
					std::bitset<152> pak(std::string(std::bitset<2>(oper).to_string() + std::bitset<3>(status).to_string() +
						std::bitset<64>(data_length).to_string() + std::bitset<8>(s_id).to_string() + std::bitset<8>(c_id).to_string()) +
						std::bitset<1>(history).to_string() + std::bitset<32>(arg).to_string() + std::bitset<32>(result).to_string() +
						"00");
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
		}
		else {//error occur
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
			std::cout << "Podczas pracy serwera wystapil blad." << std::endl;
			return temp;
		}
	}
};

class session {
public:
	int user_id; //user id
	int s_id; //session id
	int last;  //last index
	std::vector<message> history; //operations, c_id-1 = index
};

int __cdecl main()
{
	std::vector<ULONG> users_table; //IP(value) -> user id(index)
	std::vector<session> sessions_container;   //all sessions
	int session_id = 1;
	int calculation_id = 1;

	WSADATA wsaData;
	SOCKET ClientSocket = INVALID_SOCKET;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	// Create a client socket
	ClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (ClientSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in service, client;
	service.sin_family = AF_INET;
	service.sin_port = htons(1234);
	service.sin_addr.s_addr = INADDR_ANY;

	char buffer[DEFAULT_BUFLEN];
	int len = sizeof(sockaddr_in);

	// Setup the UDP socket
	iResult = bind(ClientSocket, (sockaddr*)&service, len);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	//waiting for request
	iResult = recvfrom(ClientSocket, buffer, DEFAULT_BUFLEN, 0, (sockaddr*)&client, &len);
	if (iResult == SOCKET_ERROR) {
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	message* request = new message(buffer);
	std::cout << "Otrzymano zadanie nawiazania polaczenia" << std::endl;
	//matching user id an creating new session
	session new_session;
	bool found = false;
	ULONG ip_addr = client.sin_addr.s_addr;
	//checking in the users_table
	for (int i = 0; i < users_table.size(); i++) {
		if (users_table[i] == ip_addr) {
			found = true;
			new_session.user_id = i;
			break;
		}
	}
	//initialize new session and if there is a need, update users table 
	if (found) {
		new_session.s_id = session_id;
		new_session.last = 0;
	}
	else {
		new_session.user_id = users_table.size();
		users_table.push_back(ip_addr);
		new_session.s_id = session_id;
		new_session.last = 0;
	}
	session_id++;
	//create accept answer for request
	message* answer = new message();
	answer->oper = 0;
	answer->status = 0;
	answer->data_length = 8;
	answer->s_id = new_session.s_id;
	//send accept answer
	std::string temp_str = answer->pack_to_string();
	memcpy(buffer, temp_str.c_str(), 10);
	iResult = sendto(ClientSocket, buffer, 10, 0, (sockaddr*)&client, len);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	sessions_container.push_back(new_session);
	bool end = false;
	//connection
	do {
		//waiting for request
		iResult = recvfrom(ClientSocket, buffer, DEFAULT_BUFLEN, 0, (sockaddr*)&client, &len);
		if (iResult == SOCKET_ERROR) {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
		//calculating and update history
		request = new message(buffer);
		answer = new message();
		switch (request->oper) {
			//end communication
		case 0: {
			answer->oper = 0;
			answer->status = 0;
			answer->data_length = 8;
			answer->s_id = new_session.s_id;
			end = true;
			break;
		}
				//calculate two_arg_oper
		case 1: {
			answer->oper = 1;
			switch (request->two_arg_oper) {
				//multipilcation
			case 0: {
				//result beyond the scope
				//wynik poza zakresem
				if ((request->arg == -1) && (request->arg2 == INT_MIN)) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				else if ((request->arg2 == -1) && (request->arg == INT_MIN)) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				else if (request->arg > INT_MAX / request->arg2) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				else if (request->arg < INT_MIN / request->arg2) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//too many calculations (max 255 per session)
				else if (calculation_id == 255) {
					answer->status = 3;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				else {
					answer->status = 0;
					answer->data_length = 48;
					int temp_int = request->arg * request->arg2;
					answer->result = (request->arg * request->arg2);
					answer->s_id = new_session.s_id;
					answer->c_id = calculation_id;
				}
				break;
			}
					//division
			case 1: {
				//dzielenie przez 0
				if (request->arg2 == 0) {
					answer->status = 1;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//wynik poza zakresem
				else if (request->arg > INT_MAX * request->arg2) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				else if (request->arg < INT_MIN * request->arg2) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//przekroczony limit obliczen
				else if (calculation_id == 255) {
					answer->status = 3;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//mozna poprawnie wykonac operacje
				else {
					answer->status = 0;
					answer->data_length = 48;
					answer->result = (request->arg / request->arg2);
					answer->s_id = new_session.s_id;
					answer->c_id = calculation_id;
				}
				break;
			}
					//addition
			case 2: {
				//wynik poza zakresem
				if (request->arg > (INT_MAX - request->arg2)) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				else if (request->arg < INT_MIN + request->arg2) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//przekroczony limit obliczen
				else if (calculation_id == 255) {
					answer->status = 3;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//mozna poprawnie wykonac operacje
				else {
					answer->status = 0;
					answer->data_length = 48;
					answer->result = (request->arg + request->arg2);
					answer->s_id = new_session.s_id;
					answer->c_id = calculation_id;
				}
				break;
			}
					//substraction
			case 3: {
				//wynik poza zakresem
				if (request->arg > INT_MAX + request->arg2) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				else if (request->arg < INT_MIN + request->arg2) {
					answer->status = 2;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//przekroczony limit obliczen
				if (calculation_id == 255) {
					answer->status = 3;
					answer->data_length = 8;
					answer->s_id = new_session.s_id;
				}
				//mozna poprawnie wykonac operacje
				else {
					answer->status = 0;
					answer->data_length = 48;
					answer->result = (request->arg - request->arg2);
					answer->s_id = new_session.s_id;
					answer->c_id = calculation_id;
				}
				break;
			}
			}
			break;
		}
				//calculate factorial
		case 2: {
			answer->oper = 2;
			if (((request->arg) > 12) || (request->arg < 0)) {
				answer->status = 2;
				answer->data_length = 8;
				answer->s_id = new_session.s_id;
			}
			else if (calculation_id == 255) {
				answer->status = 3;
				answer->data_length = 8;
				answer->s_id = new_session.s_id;
			}
			else {
				int factorial = 1;
				std::cout << "arg: " << request->arg << std::endl;
				for (int i = 1; i <= request->arg; i++) { factorial *= i; }
				answer->status = 0;
				answer->data_length = 48;
				answer->result = factorial;
				answer->s_id = new_session.s_id;
				answer->c_id = calculation_id;
			}
			break;
		}
				//send history
		case 3: {
			answer->oper = 3;
			std::cout << "Dlugosc wektora: " << sessions_container.size() << std::endl;
			std::cout << "Ilosc dzialan: " << new_session.last << std::endl;
			int ses = request->s_id;
			bool ses_found = false;
			//checking the session_id from the request in  the session_container
			for (session s : sessions_container) {
				if (s.s_id == ses) {
					ses_found = true;
					//checking user_id
					if (new_session.user_id == s.user_id) {
						//request of view the whole history
						if (request->c_id == 0) {
							for (message &m : s.history) {
								answer->status = 0;
								answer->s_id = m.s_id;
								answer->c_id = m.c_id;
								//two_arg_oper
								if (m.oper == 1) {
									answer->history = 0;
									answer->data_length = 115;
									answer->arg = m.arg;
									answer->arg2 = m.arg2;
									answer->two_arg_oper = m.two_arg_oper;
									answer->result = m.result;
									temp_str = answer->pack_to_string();
									//send one calculation
									memcpy(buffer, temp_str.c_str(), 23);
									iResult = sendto(ClientSocket, buffer, 23, 0, (sockaddr*)&client, len);
									if (iResult == SOCKET_ERROR) {
										printf("send failed with error: %d\n", WSAGetLastError());
										closesocket(ClientSocket);
										WSACleanup();
										return 1;
									}
								}
								//factorial
								else if (m.oper == 2) {
									answer->history = 1;
									answer->data_length = 81;
									answer->arg = m.arg;
									answer->result = m.result;
									temp_str = answer->pack_to_string();
									//send one calculation
									memcpy(buffer, temp_str.c_str(), 19);
									iResult = sendto(ClientSocket, buffer, 19, 0, (sockaddr*)&client, len);
									if (iResult == SOCKET_ERROR) {
										printf("send failed with error: %d\n", WSAGetLastError());
										closesocket(ClientSocket);
										WSACleanup();
										return 1;
									}
								}
							}
							//at the end send the message with beyond the scope error
							answer->status = 2;
							answer->data_length = 8;
							answer->s_id = new_session.s_id;
							temp_str = answer->pack_to_string();
							memcpy(buffer, temp_str.c_str(), 10);
							iResult = sendto(ClientSocket, buffer, 10, 0, (sockaddr*)&client, len);
							if (iResult == SOCKET_ERROR) {
								printf("send failed with error: %d\n", WSAGetLastError());
								closesocket(ClientSocket);
								WSACleanup();
								return 1;
							}
						}
						//request of view only one calculation
						else {
							bool calc_found = false;
							for (message m : s.history) {
								if (m.c_id == request->c_id) {
									answer->status = 0;
									answer->s_id = m.s_id;
									answer->c_id = m.c_id;
									//two_arg_oper
									if (m.oper == 1) {
										answer->history = 0;
										answer->data_length = 115;
										answer->arg = m.arg;
										answer->arg2 = m.arg2;
										answer->two_arg_oper = m.two_arg_oper;
										answer->result = m.result;
										temp_str = answer->pack_to_string();
										//send one calculation
										memcpy(buffer, temp_str.c_str(), 23);
										iResult = sendto(ClientSocket, buffer, 23, 0, (sockaddr*)&client, len);
										if (iResult == SOCKET_ERROR) {
											printf("send failed with error: %d\n", WSAGetLastError());
											closesocket(ClientSocket);
											WSACleanup();
											return 1;
										}
									}
									//factorial
									else {
										answer->history = 1;
										answer->data_length = 81;
										answer->arg = m.arg;
										answer->result = m.result;
										temp_str = answer->pack_to_string();
										//send one calculation
										memcpy(buffer, temp_str.c_str(), 19);
										iResult = sendto(ClientSocket, buffer, 19, 0, (sockaddr*)&client, len);
										if (iResult == SOCKET_ERROR) {
											printf("send failed with error: %d\n", WSAGetLastError());
											closesocket(ClientSocket);
											WSACleanup();
											return 1;
										}
									}
									calc_found = true;
									break;
								}
							}
							//calculation not found
							if (!calc_found) {
								answer->status = 5;
								answer->data_length = 8;
								answer->s_id = new_session.s_id;
								temp_str = answer->pack_to_string();
								memcpy(buffer, temp_str.c_str(), 10);
								iResult = sendto(ClientSocket, buffer, 10, 0, (sockaddr*)&client, len);
								if (iResult == SOCKET_ERROR) {
									printf("send failed with error: %d\n", WSAGetLastError());
									closesocket(ClientSocket);
									WSACleanup();
									return 1;
								}
							}
						}
						break;
					}
					//other user session
					else {
						answer->status = 4;
						answer->data_length = 8;
						answer->s_id = new_session.s_id;
						temp_str = answer->pack_to_string();
						memcpy(buffer, temp_str.c_str(), 10);
						iResult = sendto(ClientSocket, buffer, 10, 0, (sockaddr*)&client, len);
						if (iResult == SOCKET_ERROR) {
							printf("send failed with error: %d\n", WSAGetLastError());
							closesocket(ClientSocket);
							WSACleanup();
							return 1;
						}
					}
					break;
				}
			}
			//session not found
			if (!ses_found) {
				answer->status = 4;
				answer->data_length = 8;
				answer->s_id = new_session.s_id;
				temp_str = answer->pack_to_string();
				memcpy(buffer, temp_str.c_str(), 10);
				iResult = sendto(ClientSocket, buffer, 10, 0, (sockaddr*)&client, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
			}
		}
		}
		//answer for request for operations other than history
		if (answer->oper != 3) {
			temp_str = answer->pack_to_string();
			if (answer->oper == 0) {
				memcpy(buffer, temp_str.c_str(), 10);
				iResult = sendto(ClientSocket, buffer, 10, 0, (sockaddr*)&client, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
			}
			else if (answer->oper == 1) {
				memcpy(buffer, temp_str.c_str(), 15);
				iResult = sendto(ClientSocket, buffer, 15, 0, (sockaddr*)&client, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
			}
			else if (answer->oper == 2) {
				memcpy(buffer, temp_str.c_str(), 15);
				iResult = sendto(ClientSocket, buffer, 15, 0, (sockaddr*)&client, len);
				if (iResult == SOCKET_ERROR) {
					printf("send failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
			}
		}
		//update history if oper = 1 or 2 and thera was any errors
		if (((request->oper == 1) || (request->oper == 2)) && (answer->status == 0)) {
			if (request->oper == 1) {
				message* temp = answer;
				temp->history = 0;
				temp->arg = request->arg;
				temp->arg2 = request->arg2;
				temp->two_arg_oper = request->two_arg_oper;
				new_session.history.push_back(*temp);
			}
			else if (request->oper == 2) {
				message* temp = answer;
				temp->history = 1;
				temp->arg = request->arg;
				new_session.history.push_back(*temp);
			}
			new_session.last++;
			bool temp = false;
			for (session &s : sessions_container) {
				if (s.s_id == new_session.s_id) {
					s = new_session;
					s.history = new_session.history;
					temp = true;
					break;
				}
			}
			if (!temp)sessions_container.push_back(new_session);
			calculation_id++;
		}
	} while (!end);

	printf("Connection closed\n");
	//session_id for next session
	session_id++;
	//clear calculation_id for next session
	calculation_id = 1;

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}