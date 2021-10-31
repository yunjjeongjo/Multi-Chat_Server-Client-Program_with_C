#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#define BUF 1024
#define PORT 3500
char system_msg[2][6] = { "@show\n","@exit\n" };
void *rcv_msg(void * arg) {
	int sock = *((int *)arg);
	char name_message[BUF];
	memset(name_message, 0, sizeof(name_message));
	int recv_size;
	int rtn;
	while (1) {
		memset(name_message, 0, sizeof(name_message));
		recv_size = read(sock, name_message, BUF);
		if (recv_size == -1)
			break;
		else if (!strcmp(name_message, "@exit\n")) //exit	
			break;
		name_message[recv_size] = '\0';
		fputs(name_message, stdout);
	}
}
void *send_msg(void * arg) {
	
	int sock = *((int*)arg);
	char message[BUF];
	memset(message, 0, sizeof(message));
	while (1) {
		fgets(message, BUF, stdin);
		if (!strcmp(message, "@exit\n")) { //exit : 연결 종료
			write(sock, message, strlen(message));
			break;
		}
		else
			write(sock, message, strlen(message));
		memset(message, 0, sizeof(message));
	}
}
int main(int argc, char *argv[]) {
	int sock;
	char *addr;
	char flag[2];
	char buf[BUF];
	char name[30];
	struct sockaddr_in serv_addr;
	pthread_t recv_thread, send_thread;
	if (argc != 3) {
		printf("Usage : %s <IP> <ID> \n", argv[0]);
		exit(1);
	}
	strcpy(name, argv[2]);
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == sock) {
		printf("SOCKET ERROR\n");
		exit(1);
	}
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(PORT);
	if (-1 == connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
		printf("CONNECT ERROR\n");
		exit(1);
	}
	memset(buf, 0, sizeof(buf));
	read(sock, flag, 2); // 접속 승인 flag read
	flag[1] = '\0';
	read(sock, buf, sizeof(buf));
	write(sock, name, sizeof(name)); // 이름 wirte
	if (!strcmp(flag, "1")) {
		printf("%s, %s \n", argv[2], buf);
		memset(buf, 0, sizeof(buf));
		pthread_create(&recv_thread, NULL, rcv_msg, (void *)&sock);
		pthread_create(&send_thread, NULL, send_msg, (void *)&sock);
		//보내는 메시지 처리하는 스레드 생성
		pthread_join(recv_thread, NULL);
		pthread_join(send_thread, NULL);
	}
	else if (!strcmp(flag, "0")) printf("Server is fulled\n");
	// 클라이언트 접속자 최대범위 이탈
	close(sock);
	return 0;
}
