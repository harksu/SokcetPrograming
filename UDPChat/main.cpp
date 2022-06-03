
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//���� ���ͼ� ��� �ȹ޴´ٰ� ��ó�� 
#include<stdio.h>
#include<WinSock2.h>
#include<conio.h>
#include<string.h>
#pragma comment(lib, "ws2_32.lib")
#define L 3 // ƽ���䰡 3x3 



//todo -> ���ʸ� �������ϸ鼭 �ϵ��� ����, ���� �������� �˷��ֱ�(�׷��� �� �ȳ���)

void initBoard(char board[L][L]) {
	for (int y = 0; y < L; y++)
		for (int x = 0; x < L; x++)
			board[y][x] = ' ';
}

// ���� ���ڿ� �Բ� ���
void printBoard(const char board[L][L]) {
	// ���� �� ��ȣ
	printf("  ");
	for (int x = 0; x < L; x++)
		printf("%d ", x + 1);
	printf("\n");

	for (int y = 0; y < L; y++) {
		// ���� �� ��ȣ�� �÷��̾� �� ���
		printf("%d ", y + 1);
		for (int x = 0; x < L; x++) {
			printf("%c", board[y][x]);
			if (x != L - 1) printf("|");
		}
		printf("\n");

		// ���� ���� �� ���
		if (y != L - 1) {
			printf("  ");
			for (int x = 0; x < L; x++) {
				printf("-");
				if (x != L - 1) printf("+");
			}
			printf("\n");
		}
	}
}

// �� ���� ������ �������� �˻�
int isInRange(int x, int y) {
	return x >= 0 && x < L&& y >= 0 && y < L;
}

// ���� �ش� ĭ�� ���ο� ���� ���� �� �ִ��� �˻�
int isValid(const char board[L][L], int x, int y) {
	return isInRange(x, y) && board[y][x] == ' ';
}

// ���޹��� �÷��̾�Լ� ���� ���� ��ġ�� �Է¹���
// 1: �Է� ����
// 0: �߸��� �Է�
int playerInput(const char* name, const char board[L][L], int* x, int* y) {
	(*x)--;
	(*y)--;
	if (!isValid(board, *x, *y)) return 0;
	return 1;
}

// ����, ����, ���� �밢��, ��� �밢�� ���⿡ ���� x, y ��ȭ��
int dx[] = { -1, 1, 0, 0, -1, 1, 1, -1 };
int dy[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
// �ش� ĭ�� �¸��� �ٿ� �����ִ��� �˻��ϴ� �Լ�
int isWinner(const char board[L][L], int x, int y) {
	// ��� ���� �˻�
	for (int d = 0; d < 4; d++) {
		// ���� ���� ������ ��
		int count = 1;
		for (int b = 0; b < 2; b++) {
			int index = 2 * d + b;

			int cx = x + dx[index];
			int cy = y + dy[index];
			while (isInRange(cx, cy)) {
				if (board[cy][cx] == board[y][x])
					count++;
				cx += dx[index];
				cy += dy[index];
			}
		}
		// ���� ������ �� ���� ��� ä������ �¸�
		if (count == L)
			return 1;
	}
	return 0;
}


// ���� ���� ����
int play(char board[L][L], const char* playerNames[], const char marks[]) {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata); // �ʱ�ȭ 

	char ip_address[20];
	printf("My ip address :");
	scanf("%s", ip_address); // ip �ּ� �Ҵ� 

	unsigned short port; // ��Ʈ ��ȣ ���� ���� 

	printf("My port #:");
	scanf("%hu", &port); // ��Ʈ ��ȣ �Ҵ�
	

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //ipv4��, �񿬰� ������,udp�� ���� ����
	if (s == INVALID_SOCKET) { // ���� ���� �����ϸ� 
		fprintf(stderr, "[!] Failed to create a socket!\n");
		return 1; // ���� 
	}

	sockaddr_in sin = { 0 }; //������ �ּҸ� ��� �⺻ ����ü ���� (ipv4)
	sin.sin_family = AF_INET; // ����ü ���� �� �Ҵ� ipv4
	sin.sin_addr.s_addr = inet_addr(ip_address); // �ش� �ּҸ� ip_address��  �ϴµ�, Ÿ�Ժ�ȯ 
	sin.sin_port = htons(port); // ��Ʈ ��ȣ�� �̰ɷ� �ϴµ�, ����Ʈ ��ȯ ��ȯ 
	if (bind(s, (const sockaddr*)&sin, sizeof sin) == SOCKET_ERROR) { // ���� ���ε� , s��� ���Ͽ� �ּ� ����ü ������, ����ü�� ũ�� 
		fprintf(stderr, "[!] Failed to bind the socket!\n"); // ���� �����߸� ��ȯ 
		return 1;
	}
	
	// ���° ���������� �����ϴ� ����
	int round = 0;
	// ������ ���������� �����ϴ� ����
	int turn = 0;
	// ���� ���� ��ġ
	int x, y;
	int count = 0;
	// ������ ���� ������ �ݺ�
	while (1) {
		fd_set readfds; // ���� ��ũ���� ? 
		FD_ZERO(&readfds); //�ʱ�ȭ 
		FD_SET(s, &readfds); // ���� 

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000; //��ٸ��ð� 

		if (select(0, &readfds, NULL, NULL, &tv) > 0) { //������ ��Ŷ ����/���� ���� 
			
			for (int i = 0; i < 2; i++) {
				char message[256]; // �����ϴٸ� �޽��� ��Ƽ� 
				sockaddr_in sin_from; // ������ �ּҸ� ��� �⺻ ����ü 
				int fromlen = sizeof sin_from; // �� ����ü�� ũ�⸦ ��� 
				const int message_len = recvfrom(s, message, sizeof message - 1, 0, (sockaddr*)&sin_from, &fromlen); //udp���� �޽��� ���� 
				

				if (message_len >= 0) {
					message[message_len] = '\0'; // �������� �ΰ�. 
					printf("From %s:%hu: %s \n", inet_ntoa(sin_from.sin_addr), ntohs(sin_from.sin_port), message); // �ش� ���ڿ��� ��� 
					int result = atoi(message);
					printf("ƽ���� �Ѿ�� �� : %d, count = %d \n", result, count);
					if (count == 0) {
						x = result; // ��ǥ���� 2���� ���ļ� �޴� �������� �ϱ�� �߽��ϴ�. 
						count = 1;
					}

					else if (count == 1) {
						y = result; // �̹��� y�� �ޱ� 
						count = 0;
					}


				}
			}

			if (!playerInput(playerNames[turn], board, &x, &y))
				continue;

			board[y][x] = marks[turn];
			printBoard(board);
			round++;

			if (isWinner(board, x, y)) {
				printf("%s �¸�!\n", playerNames[turn]);
				break;
			}
			else if (round == L * L) {
				printf("���º�!\n");
				break;
			}
			turn = !turn; // 0�ƴϸ� 1�̴ϱ� �̰ɷ� ���� ����

		}
		/*

		*/
		if (_kbhit() && _getch() == 13) { // ������ ����, ������ send����� �� ���� ������, ���� �߱淡 _getch()�� �����Ͽ����ϴ�. 
				
				printf("������ �ּҰ��� ���� ������ �� �Է��� ��ǥ���� ������ 2�� �Է��ϼ��� ex/ 127.0.0.1 portnum input \n");
				
				for (int i = 0; i < 2; i++) {
					char target_ip_addr[20];
					unsigned short target_port;
					printf("ip port message > ");
					scanf("%s %hu", target_ip_addr, &target_port);

					char message_to_send[256];
					fgets(message_to_send, sizeof message_to_send, stdin);

					sockaddr_in sin_to{ 0 };
					sin_to.sin_family = AF_INET;
					sin_to.sin_addr.s_addr = inet_addr(target_ip_addr);
					sin_to.sin_port = htons(target_port);
					

					sendto(s, message_to_send, strlen(message_to_send), 0, (const sockaddr*)&sin_to, sizeof sin_to);//�̰� ������� �����°Ű� 
					sendto(s, message_to_send, strlen(message_to_send), 0, (const sockaddr*)&sin, sizeof sin);//����ȭ�� ���ؼ� �����׵� ������(���� ����)


				}
			

			//���� ������ ���� ������ �ʿ���


		}

	}




}


int main(void) {



	char board[L][L];
	initBoard(board);
	 const char* names[] = {
			"Player 1", "Player 2"
	};
	// �÷��̾� ��
	const char marks[] = { 'O', 'X' };
	play(board, names, marks);
	return 0;
}
