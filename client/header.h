//
//  header.h
//  client
//
//  Created by liuxt on 15/11/18.
//  Copyright (c) 2015年 liuxt. All rights reserved.
//

#ifndef client_header_h
#define client_header_h
typedef enum STATE {
    STATE_NONE,
    STATE_POST,
    STATE_SEND,
    STATE_MAIL
} STATE;

#define TAGSIZE 20
#define BUFFSIZE 1024
#define SLEEPTIME 50000                                                     // Sleep for 0.05s


void user(FILE* fp, int count, int sockfd);                                 // Enter username
void pass(FILE* fp, int count, int sockfd);                                 // Enter passward
void logout(FILE* fp, int count, int sockfd);                               // Exit
void sendMessage(FILE* fp, int count, int sockfd);
void sendMessageTo(FILE* fp, int count, int sockfd);                        // send Message to USERID
void sendMessageContent(FILE* fp, int count, int sockfd);                   // send Message Content
void sendMailContent(FILE* fp, int count, int sockfd);                      // send Mail content
void sendPostContent(FILE* fp, int count, int sockfd);                      // send Post content
void sendMailTo(FILE* fp, int count, int sockfd);                           // send Mail to USERID
void sendMailTitle(FILE* fp, int count, int sockfd);                        // send Mail title
void sendPostTo(FILE* fp, int count, int sockfd);                           // send Post to BOARDNAME
void sendPost(FILE* fp, int count, int sockfd);                             // send Post TITLE
void sendBackwards(int sockfd);                                             // send 5 BACKWARDS to return to the MENU page!
void sendChar(char c, int sockfd);                                          // send a single char
void checkMessage(int sockfd);                                              // for bonus
void sendEnter(int sockfd);                                                 // send LINEFEED
void sendCtrlC(int sockfd);                                                 
void sendCtrlX(int sockfd);
void sendCtrlP(int sockfd);
void sendCtrlU(int sockfd);
int client(FILE* fp);                                                       // client program
#endif
