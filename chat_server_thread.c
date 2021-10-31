#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#define PORT 3500
#define MAX_CLIENTNUM 10
#define BUF 1024
pthread_mutex_t mutex;
int clientNum = 0;
int clientSockArr[MAX_CLIENTNUM];
char system_msg[2][6] = { "@show\n","@exit\n" }; //show, exit 명령어
char nameArr[MAX_CLIENTNUM][20];
void *connect_thread(void *arg) {
	int clientSocket = *(int*)arg;
	int bufLen;
	int cnt = 0;
	char buf[BUF];
	char message[BUF];
	char myName[20]; 
	strcpy(myName, nameArr[clientNum-1]);
	memset(buf, 0, sizeof(buf));
	while (0 != (bufLen = read(clientSocket, buf, sizeof(buf)))) {
		memset(message, 0, sizeof(message));
		buf[bufLen] = '\0';
		if (cnt == 0) {
			cnt++;
			continue;
		}
		if (!strncmp(buf, system_msg[0],6)) { //show : 현재 접속자 보여줌
			char show[BUF];
			memset(show, 0, sizeof(show));
			sprintf(show, "현재 서버에 접속중인 사용자는 아래와 같습니다.\n", clientNum);
			for (int i = 0; i < clientNum; i++) {
				strcat(show, nameArr[i]);
				strcat(show, " ");
			}
			strcat(show, "\n");
			write(clientSocket, show, sizeof(show));
		}
		else if (!strcmp(buf, system_msg[1])) { 
			write(clientSocket, system_msg[1], 6);
			memset(buf, 0, sizeof(buf));
			break;
		}
		else {
			pthread_mutex_lock(&mutex); 
			sprintf(message, "%s : ",myName);
			strcat(message, buf);
			for (int i = 0; i < clientNum; i++) {
				write(clientSockArr[i], message, sizeof(buf));
			}
			memset(buf, 0, sizeof(buf));
			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_mutex_lock(&mutex);
	for (int i = 0; i<clientNum; i++) {
		if (clientSocket == clientSockArr[i])
		{
			for (; i < clientNum - 1; i++) {
				clientSockArr[i] = clientSockArr[i + 1];
				strcpy(nameArr[i], nameArr[i + 1]); 
			}
			break;
		}
	}
	clientNum--; // client Num --
	pthread_mutex_unlock(&mutex);
	char exit_msg[100];
	sprintf(exit_msg, "%s 님이 대화방을 나가셨습니다.\n", myName); 
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < clientNum; i++) {
		write(clientSockArr[i], exit_msg, strlen(exit_msg));
	}
	pthread_mutex_unlock(&mutex); 
	printf("%s is Disconnected from the Server\n", myName); //// 접속중인 모든 클라이언트에게 대화방 나갔습니다 메세지 출력
	close(clientSocket);
	return NULL;
}
int main(int argc, char *argv[]){
	// socket 설정 //
	int serverSocket = 0 , clientSocket = 0;
	struct sockaddr_in Server,Client;
	pthread_t thread_id;
	pthread_mutex_init(&mutex, NULL);
	if (1 != argc) {
		fprintf(stderr, "Don't write arguments\n");
		exit(1);
	}
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) {
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}// 소켓 생성 실패
	else fprintf(stderr, "Socket created!\n");
	bzero(&Server, sizeof(Server));
	Server.sin_family = AF_INET;
	Server.sin_addr.s_addr = htonl(INADDR_ANY);
	Server.sin_port = htons(PORT);
	if (bind(serverSocket, (struct sockaddr *)&Server, sizeof(Server)) == 0)fprintf(stderr, "Bind completed!\n");
	else {
		fprintf(stderr, "Could not bind to address!\n");
		close(serverSocket);
		exit(1);
	}
	if (listen(serverSocket, MAX_CLIENTNUM) == -1) {
		fprintf(stderr, "Cannot listen on socket!\n"); 
		close(serverSocket);
		exit(1);
	}
	while (1) {
		int len = sizeof(Client);
		clientSocket = accept(serverSocket, (struct sockaddr *)&Client, &len);
		if (clientSocket == -1){
			printf("accept error\n");
		}
		else if(clientNum < MAX_CLIENTNUM) // 최대 client 수 설정
		{
			char hello[100] = "님 멀티 채팅방에 오신 것을 환영합니다!\n";
			char connect_msg[100];
			write(clientSocket, "1", 2); // 접속 승인
			write(clientSocket,hello , strlen(hello));
			read(clientSocket, nameArr[clientNum], sizeof(nameArr[clientNum]));
			strcpy(connect_msg, nameArr[clientNum]);
			strcat(connect_msg, " 님이 대화방에 입장하셨습니다.\n");
			
			pthread_mutex_lock(&mutex); //mutex lock
			for (int i = 0; i < clientNum; i++) {
				write(clientSockArr[i], connect_msg, sizeof(connect_msg));
			}
			pthread_mutex_unlock(&mutex); //mutex unlock
			
			pthread_mutex_lock(&mutex); //mutex lock
			clientSockArr[clientNum++] = clientSocket;
			pthread_mutex_unlock(&mutex);
			pthread_create(&thread_id, NULL, connect_thread, (void *)&clientSocket);
			printf("%s is Connected to the Server \n", nameArr[clientNum-1]);
		}
		else {
			write(clientSocket, "2", 2); //client 수 최대범위 초과
			printf("Full!\n");
			exit(1);
		}
	}
	pthread_join(thread_id, NULL);
	pthread_mutex_destroy(&mutex); 
	return 0;
}
