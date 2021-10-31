#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
// socket & bind & listen & accept & connect
#include <sys/types.h>
#include <sys/socket.h>
// sockaddr_in
#include <netinet/in.h>
// read & write
#include <unistd.h>
// htonl
#include <arpa/inet.h>
// errno, perror
#include <errno.h>
// open
#include <fcntl.h>
#include <sys/stat.h>
//select
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#define PORT      3500
#define IP        "127.0.0.1"
#define MAX_USER     3
#define NIC_NAME_SIZE   9
#define NIC_NAME_MSG   (9+2)
#define BUF_SIZE  255              //only message
#define MSG_SIZE  (BUF_SIZE+1+NIC_NAME_MSG)  //message + Nick Name
#define MSG_END    "\x01\x02\x03"     
int main(int iRtn, char *argv[]) 
{
  int iFd;
  struct sockaddr_in stAddr;
  int iLen;
  char cBuf[BUF_SIZE];  
  fd_set fdRead;
  char cMSG[MSG_SIZE];
  char cNick[NIC_NAME_SIZE];
  unsigned short usPORT=PORT; 
  char system_msg[2][6] = { "@show\n","@exit\n" };
  strcpy(cNick, argv[2]);
  /*** socket ***/
  iFd = socket(AF_INET, SOCK_STREAM, 0);
  if(-1 == iFd)
  {
    perror("socket:");
    return 100;
  }
  /*** structure setting ***/
  stAddr.sin_family = AF_INET;
  stAddr.sin_addr.s_addr = inet_addr(argv[1]);
  stAddr.sin_port = htons(usPORT);
  iLen = sizeof(struct sockaddr_in);
  /*** connect ***/
  iRtn = connect(iFd, (struct sockaddr *)&stAddr, iLen);
  if(-1 == iRtn)
  {
    perror("connect:");
    close(iFd);
    return 200;
  }
  write(iFd, cNick, NIC_NAME_SIZE);
  while(1)
  {
    FD_ZERO(&fdRead);  
    FD_SET(0, &fdRead);
    FD_SET(iFd, &fdRead);      
    select(iFd+1,&fdRead, 0, 0, 0);
    if(0 != (FD_ISSET(0, &fdRead) ) )
    {
      iRtn = read(0, cBuf, BUF_SIZE);      
      if(iRtn == 0)
      {
        write(iFd, MSG_END, sizeof(MSG_END));
        break;
      }
      cBuf[iRtn - 1] = 0;
      sprintf(cMSG, "%s : %s ", cNick, cBuf);
      write(iFd, cMSG, MSG_SIZE);
  
    }
    if(0 != (FD_ISSET(iFd, &fdRead) ))
    {
      iRtn = read(iFd, cMSG, sizeof(cMSG));      
      if(iRtn == 0)
      {
        printf("Server does not respond\n");
        break;
      }
      if( 0 == strncmp(cMSG, MSG_END, sizeof(MSG_END) ))
      {
        break;
      }
      printf("%s\n", cMSG);    
    }
  }
  /*** read & write ***/
  //memset(cBuf, 0, BUF_SIZE);
  //iRtn = read(0, cBuf, BUF_SIZE);
  close(iFd);
  return 0;
}