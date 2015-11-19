//
//  main.c
//  client
//
//  Created by liuxt on 15/11/18.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "header.h"

int main(int argc, const char * argv[]) {
    FILE* fp = fopen(argv[1], "r");
    int returnVal = 0;
    returnVal = client(fp);
    return returnVal;
}
int client(FILE* fp) {
    int sockfd = 0;
    char recvBuff[BUFFSIZE];
    char sendBuff[BUFFSIZE];
    struct sockaddr_in serv_addr;
    memset(recvBuff, '0' ,sizeof(recvBuff));
    memset(sendBuff, '0', sizeof(sendBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)                               // Create socket
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(23);
    serv_addr.sin_addr.s_addr = inet_addr("140.112.172.1");                         // Use telnet to ptt.cc
    
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0)         // Connect server
    {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    
    
    char c;
    char tag[TAGSIZE] = {0};
    STATE state = STATE_NONE;
    
    while ((c = getc(fp)) != EOF) {                                                 // Scan input, suppose all inputs are in standard formats
        if (c == '<') {
            int count = 0;                                                          // # of TAG characters, BOARD is 5 for example
            while((c = getc(fp)) != '>'){
                tag[count++] = c;
            }
            tag[count] = '\0';
            if (strcmp(tag, "ID") == 0) {
                user(fp, count, sockfd);
            }
            else if (strcmp(tag, "PASS") == 0) {
                pass(fp, count, sockfd);
            }
            else if (strcmp(tag, "EXIT") == 0) {
                logout(fp, count, sockfd);
            }
            else if (strcmp(tag, "W") == 0) {
                sendMessageTo(fp, count, sockfd);
                state = STATE_SEND;                                                 // "CONTENT" means sendMessageContent
            }
            else if (strcmp(tag, "CONTENT") == 0){
                switch(state){
                    case STATE_SEND:
                        sendMessageContent(fp, count, sockfd);
                        break;
                    case STATE_POST:
                        sendPostContent(fp, count, sockfd);
                        break;
                    case STATE_MAIL:
                        sendMailContent(fp, count, sockfd);
                        break;
                    default:
                        printf("ERROR! Need<CONTENT>data</CONTENT>\n");
                }
            }
            else if(strcmp(tag, "M") == 0){                                         // "CONTENT" means sendMailContent
                sendMailTo(fp, count, sockfd);
                state = STATE_MAIL;
            }
            else if(strcmp(tag, "TITLE") == 0){
                sendMailTitle(fp, count, sockfd);
            }
            else if(strcmp(tag, "BOARD") == 0){
                sendPostTo(fp, count, sockfd);
                state = STATE_POST;                                                 // "CONTENT" means sendPostContent
            }
            else if(strcmp(tag, "P") == 0){
                sendPost(fp, count, sockfd);
            }
            else{
                printf("ERROR! Need<TAG>data</TAG>\n");
                exit(1);
            }
        }
        else if(isspace(c)){                                                        // Deal with space
            //do nothing
        }
        else{
            printf("ERROR! Standard input: <TAG>data</TAG>\n");                     // The input is INVALID!
            exit(1);
        }
    }
    return 0;
}
void sendEnter(int sockfd){
    char sendBuff[BUFFSIZE] = {0};
    strcpy(sendBuff, "\r");
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
        printf("Send failed! \n");
        exit(1);
    }
    
}
void sendBackwards(int sockfd){
    char sendBuff[BUFFSIZE] = {0};
    strcpy(sendBuff, "\033OD");
    for (int i = 0; i < 5; i++) {
        usleep(SLEEPTIME * 2);
        if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
            printf("Send failed! \n");
            exit(1);
        }
    }
}
void sendCtrlC(int sockfd){
    char sendBuff[BUFFSIZE] = {0};
    usleep(SLEEPTIME);
    strcpy(sendBuff, "\003");
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
        printf("Send failed! \n");
        exit(1);
    }
}
void sendCtrlP(int sockfd){
    char sendBuff[BUFFSIZE] = {0};
    usleep(SLEEPTIME);
    strcpy(sendBuff, "\020");
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
        printf("Send failed! \n");
        exit(1);
    }
}
void sendCtrlX(int sockfd){
    char sendBuff[BUFFSIZE] = {0};
    usleep(SLEEPTIME);
    strcpy(sendBuff, "\030");
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
        printf("Send failed! \n");
        exit(1);
    }
}
void sendCtrlU(int sockfd){
    char sendBuff[BUFFSIZE] = {0};
    usleep(SLEEPTIME);
    strcpy(sendBuff, "\025");
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
        printf("Send failed! \n");
        exit(1);
    }
}
void sendChar(char c, int sockfd){
    char sendBuff[BUFFSIZE] = {0};
    sendBuff[0] = c;
    sendBuff[1] = '\0';
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
        printf("Send failed! \n");
        exit(1);
    }
}
void user(FILE* fp, int count, int sockfd){
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                                        // the end tag
                for( ; count >= 0; count--){                                    // Deal with </TAG>
                    c = getc(fp);
                }
                printf("\nUSERNAME OK!\n");
                sendEnter(sockfd);
                usleep(SLEEPTIME);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);                                          // Deal with '<' character in the inputs
                ungetc(c, fp);
            }
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);                                                // Send characters to server
        }
    }
}
void pass(FILE* fp, int count, int sockfd){
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nPASSWORD OK!\n");
                sendEnter(sockfd);
                usleep(SLEEPTIME);
                sleep(1);                                                       // The login process takes about 0.5s!!!!
                sendCtrlC(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void logout(FILE* fp, int count, int sockfd){
    usleep(SLEEPTIME);
    sendBackwards(sockfd);
    usleep(SLEEPTIME);
    sendEnter(sockfd);
    usleep(SLEEPTIME);
    sendChar('y', sockfd);
    sendEnter(sockfd);
    usleep(SLEEPTIME);
    sendCtrlC(sockfd);
    printf("\nLOGOUT OK!\n");
    return;
}
void sendMessage(FILE* fp, int count, int sockfd){
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_MESSAGE OK!\n");
                return;
            }
            else{
                putchar('<');
                ungetc(c, fp);
            }
        }
        else{
            putchar(c);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendMessageTo(FILE* fp, int count, int sockfd){
    usleep(SLEEPTIME);
    sendBackwards(sockfd);
    usleep(SLEEPTIME);
    sendCtrlU(sockfd);
    usleep(SLEEPTIME);
    sendChar('s', sockfd);
    usleep(SLEEPTIME);
    
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_MESSAGE_TO OK!\n");
                sendEnter(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendMessageContent(FILE* fp, int count, int sockfd){
    usleep(SLEEPTIME);
    sendChar('w', sockfd);
    usleep(SLEEPTIME);
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_MESSAGE_CONTENT OK!\n");
                sendEnter(sockfd);
                usleep(SLEEPTIME);
                sendChar('y', sockfd);
                sendEnter(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else{
            sendChar(c, sockfd);
            //putchar(c);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendMailContent(FILE* fp, int count, int sockfd){
    usleep(SLEEPTIME);
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_MAIL_CONTENT OK!\n");
                usleep(SLEEPTIME);
                sendCtrlX(sockfd);
                usleep(SLEEPTIME);
                sendChar('s', sockfd);
                sendEnter(sockfd);
                usleep(SLEEPTIME);
                sendChar('y', sockfd);
                sendEnter(sockfd);
                usleep(SLEEPTIME);
                sendCtrlC(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else if(c == '\n'){
            sendEnter(sockfd);
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendPostContent(FILE* fp, int count, int sockfd){
    usleep(SLEEPTIME);
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_POST_CONTENT OK!\n");
                sendCtrlX(sockfd);
                usleep(SLEEPTIME);
                sendChar('s', sockfd);
                sendEnter(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else if(c == '\n'){
            sendEnter(sockfd);
        }
        else{
            //putchar(c);
            sendChar(c , sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendMailTo(FILE* fp, int count, int sockfd){
    usleep(SLEEPTIME);
    sendBackwards(sockfd);
    usleep(SLEEPTIME);
    sendChar('m', sockfd);
    sendEnter(sockfd);
    usleep(SLEEPTIME);
    sendChar('s', sockfd);
    sendEnter(sockfd);
    usleep(SLEEPTIME);
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_Mail_To OK!\n");
                sendEnter(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendMailTitle(FILE* fp, int count, int sockfd){
    usleep(SLEEPTIME);
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_Mail_Title OK!\n");
                sendEnter(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendPostTo(FILE* fp, int count, int sockfd){
    sendBackwards(sockfd);
    usleep(SLEEPTIME);
    sendChar('s', sockfd);
    usleep(SLEEPTIME);
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_Post_To OK!\n");
                sendEnter(sockfd);
                usleep(SLEEPTIME);
                sendCtrlC(sockfd);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
void sendPost(FILE* fp, int count, int sockfd){
    sendCtrlP(sockfd);
    usleep(SLEEPTIME);
    sendEnter(sockfd);
    usleep(SLEEPTIME);
    char c;
    while ((c = getc(fp)) != EOF) {
        if (c == '<') {
            if ((c = getc(fp)) == '/') {                  // the end tag
                for( ; count >= 0; count--){
                    c = getc(fp);
                }
                printf("\nSEND_Post_Title OK!\n");
                sendEnter(sockfd);
                usleep(SLEEPTIME);
                return;
            }
            else{
                //putchar('<');
                sendChar('<', sockfd);
                ungetc(c, fp);
            }
        }
        else{
            //putchar(c);
            sendChar(c, sockfd);
        }
    }
    printf("ERROR!\n");
    exit(1);
}
