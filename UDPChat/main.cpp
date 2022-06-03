
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//에러 나와서 경고 안받는다고 전처리 
#include<stdio.h>
#include<WinSock2.h>
#include<conio.h>
#include<string.h>
#pragma comment(lib, "ws2_32.lib")
#define L 3 // 틱택토가 3x3 



//todo -> 차례를 번갈아하면서 하도록 통제, 누구 차례인지 알려주기(그래도 얼마 안남음)

void initBoard(char board[L][L]) {
	for (int y = 0; y < L; y++)
		for (int x = 0; x < L; x++)
			board[y][x] = ' ';
}

// 판을 격자와 함께 출력
void printBoard(const char board[L][L]) {
	// 가로 줄 번호
	printf("  ");
	for (int x = 0; x < L; x++)
		printf("%d ", x + 1);
	printf("\n");

	for (int y = 0; y < L; y++) {
		// 세로 줄 번호와 플레이어 말 출력
		printf("%d ", y + 1);
		for (int x = 0; x < L; x++) {
			printf("%c", board[y][x]);
			if (x != L - 1) printf("|");
		}
		printf("\n");

		// 세로 격자 줄 출력
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

// 판 범위 밖으로 나가는지 검사
int isInRange(int x, int y) {
	return x >= 0 && x < L&& y >= 0 && y < L;
}

// 판의 해당 칸에 새로운 말을 놓을 수 있는지 검사
int isValid(const char board[L][L], int x, int y) {
	return isInRange(x, y) && board[y][x] == ' ';
}

// 전달받은 플레이어에게서 말을 놓을 위치를 입력받음
// 1: 입력 성공
// 0: 잘못된 입력
int playerInput(const char* name, const char board[L][L], int* x, int* y) {
	(*x)--;
	(*y)--;
	if (!isValid(board, *x, *y)) return 0;
	return 1;
}

// 가로, 세로, 우하 대각선, 우상 대각선 방향에 대한 x, y 변화량
int dx[] = { -1, 1, 0, 0, -1, 1, 1, -1 };
int dy[] = { 0, 0, -1, 1, -1, 1, -1, 1 };
// 해당 칸이 승리한 줄에 속해있는지 검사하는 함수
int isWinner(const char board[L][L], int x, int y) {
	// 모든 방향 검사
	for (int d = 0; d < 4; d++) {
		// 같은 말의 개수를 셈
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
		// 말의 개수가 한 줄을 모두 채웠으면 승리
		if (count == L)
			return 1;
	}
	return 0;
}


// 실제 게임 로직
int play(char board[L][L], const char* playerNames[], const char marks[]) {
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata); // 초기화 

	char ip_address[20];
	printf("My ip address :");
	scanf("%s", ip_address); // ip 주소 할당 

	unsigned short port; // 포트 번호 담을 변수 

	printf("My port #:");
	scanf("%hu", &port); // 포트 번호 할당
	

	SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); //ipv4로, 비연결 지향형,udp로 소켓 생성
	if (s == INVALID_SOCKET) { // 소켓 생성 실패하면 
		fprintf(stderr, "[!] Failed to create a socket!\n");
		return 1; // 리턴 
	}

	sockaddr_in sin = { 0 }; //소켓의 주소를 담는 기본 구조체 역할 (ipv4)
	sin.sin_family = AF_INET; // 구조체 원소 값 할당 ipv4
	sin.sin_addr.s_addr = inet_addr(ip_address); // 해당 주소를 ip_address로  하는데, 타입변환 
	sin.sin_port = htons(port); // 포트 번호는 이걸로 하는데, 바이트 변환 변환 
	if (bind(s, (const sockaddr*)&sin, sizeof sin) == SOCKET_ERROR) { // 소켓 바인딩 , s라는 소켓에 주소 구조체 포인터, 구조체의 크기 
		fprintf(stderr, "[!] Failed to bind the socket!\n"); // 만약 에러뜨면 반환 
		return 1;
	}
	
	// 몇번째 라운드인지를 저장하는 변수
	int round = 0;
	// 누구의 차례인지를 저장하는 변수
	int turn = 0;
	// 말을 놓을 위치
	int x, y;
	int count = 0;
	// 게임이 끝날 때까지 반복
	while (1) {
		fd_set readfds; // 파일 디스크립터 ? 
		FD_ZERO(&readfds); //초기화 
		FD_SET(s, &readfds); // 셋팅 

		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000; //기다릴시간 

		if (select(0, &readfds, NULL, NULL, &tv) > 0) { //소켓의 패킷 전송/수신 가능 
			
			for (int i = 0; i < 2; i++) {
				char message[256]; // 가능하다면 메시지 담아서 
				sockaddr_in sin_from; // 소켓의 주소를 담는 기본 구조체 
				int fromlen = sizeof sin_from; // 위 구조체의 크기를 담고 
				const int message_len = recvfrom(s, message, sizeof message - 1, 0, (sockaddr*)&sin_from, &fromlen); //udp에서 메시지 수신 
				

				if (message_len >= 0) {
					message[message_len] = '\0'; // 마지막은 널값. 
					printf("From %s:%hu: %s \n", inet_ntoa(sin_from.sin_addr), ntohs(sin_from.sin_port), message); // 해당 문자열을 출력 
					int result = atoi(message);
					printf("틱택토 넘어온 값 : %d, count = %d \n", result, count);
					if (count == 0) {
						x = result; // 좌표값을 2번에 걸쳐서 받는 로직으로 하기로 했습니다. 
						count = 1;
					}

					else if (count == 1) {
						y = result; // 이번엔 y값 받기 
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
				printf("%s 승리!\n", playerNames[turn]);
				break;
			}
			else if (round == L * L) {
				printf("무승부!\n");
				break;
			}
			turn = !turn; // 0아니면 1이니까 이걸로 순서 통제

		}
		/*

		*/
		if (_kbhit() && _getch() == 13) { // 보내는 과정, 마지막 send말고는 다 위랑 동일함, 에러 뜨길래 _getch()로 변경하였습니다. 
				
				printf("접속할 주소값을 먼저 기입한 후 입력할 좌표값을 정수로 2개 입력하세요 ex/ 127.0.0.1 portnum input \n");
				
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
					

					sendto(s, message_to_send, strlen(message_to_send), 0, (const sockaddr*)&sin_to, sizeof sin_to);//이건 상대한테 보내는거고 
					sendto(s, message_to_send, strlen(message_to_send), 0, (const sockaddr*)&sin, sizeof sin);//동기화를 위해서 나한테도 보내기(상태 유지)


				}
			

			//전송 끝나면 차례 통제가 필요함


		}

	}




}


int main(void) {



	char board[L][L];
	initBoard(board);
	 const char* names[] = {
			"Player 1", "Player 2"
	};
	// 플레이어 말
	const char marks[] = { 'O', 'X' };
	play(board, names, marks);
	return 0;
}
