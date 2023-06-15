#include <iostream>
#include "vector"
#include <cstdlib>
#include <ctime>
#include "thread"
#include "chrono"
#include "math.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // link with ws2_32.lib
using namespace std;

int** createMatrix(int matrixSize) {
	int** matrix = new int* [matrixSize];
	for (int i = 0; i < matrixSize; i++) {
		matrix[i] = new int[matrixSize];
		for (int j = 0; j < matrixSize; j++) {
			matrix[i][j] = rand() % 15;
		}
	}
	return matrix;
}

void printMatrix(int** matrix, int matrixSize) {
	for (int i = 0; i < matrixSize; i++) {
		cout << "[ ";
		for (int j = 0; j < matrixSize; j++) {
			cout << matrix[i][j] << " ";
		}
		cout << "]" << endl;
	}
}

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed.\n";
		return 1;
	}

	cout << "Matrixes created. Connecting to the server..." << endl;
	// create a socket
	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cerr << "Error creating socket: " << WSAGetLastError() << "\n";
		WSACleanup();
		return 1;
	}

	// connect to server
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr.s_addr);
	serverAddr.sin_port = htons(8080); // replace with actual server port
	if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
		SOCKET_ERROR) {
		std::cerr << "Error connecting to server: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	cout << "Enter n(for matrix nxn):";
	int matrixSize;
	cin >> matrixSize;
	int** matrixA = createMatrix(matrixSize);
	int** matrixB = createMatrix(matrixSize);
	printMatrix(matrixA, matrixSize);
	cout << endl;
	printMatrix(matrixB, matrixSize);
	int numOfThreads;
	cout << "Enter amount of threads: ";
	cin >> numOfThreads;
	cout << "Connected to the server. Sending matrix size..." << endl;

	// sending matrix size
	int bytesSent = send(clientSocket, (char*)&matrixSize, sizeof(matrixSize), 0);
	if (bytesSent == SOCKET_ERROR) {
		cerr << "Error sending data: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	cout << "Matrix size sent. Sending matrixes..." << endl;

	//sending matrixes to the server
	for (int i = 0; i < matrixSize; i++) {
		int bytesSentForRow = send(clientSocket, (char*)matrixA[i], sizeof(int) * matrixSize, 0);
		if (bytesSentForRow == SOCKET_ERROR) {
			std::cerr << "Error sending row of matrix: " << WSAGetLastError() << "\n";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		bytesSentForRow = send(clientSocket, (char*)matrixB[i], sizeof(int) * matrixSize, 0);
		if (bytesSentForRow == SOCKET_ERROR) {
			std::cerr << "Error sending row of matrix: " << WSAGetLastError() << "\n";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
	}
	cout << "Matrixes sent. Sending threads..." << endl;

	//sending amount of threads to the server
	bytesSent = send(clientSocket, (char*)&numOfThreads, sizeof(numOfThreads), 0);
	if (bytesSent == SOCKET_ERROR) {
		std::cerr << "Error sending row of matrix: " << WSAGetLastError() << "\n";
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}
	
	cout << "All the information sent. Waiting for result..." << endl;
	// receive result from server
	int** result = new int*[matrixSize];
	for (int i = 0; i < matrixSize; i++) {
		result[i] = new int[matrixSize];
		int bytesReceived = recv(clientSocket, (char*)result[i], sizeof(int) * matrixSize, 0);
		if (bytesReceived == SOCKET_ERROR) {
			cerr << "Error receiving data: " << WSAGetLastError() << "\n";
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
	}

	cout << "Result reveived. Printing the result..." << endl;

	printMatrix(result, matrixSize);
	// cleanup
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}