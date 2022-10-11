#include <sys/types.h>
#include <sys/socket.h>

#include "eventLoop.h"
//int myListen(int port) {

//    int sockfd, connfd; socklen_t len;
//    struct sockaddr_in servaddr, cli;

//    // socket create and verification
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sockfd == -1) {
//        printf("socket creation failed...\n");
//        exit(0);
//    }
//    else
//        printf("Socket successfully created..\n");
//    bzero(&servaddr, sizeof(servaddr));

//    // assign IP, PORT
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//    servaddr.sin_port = htons(6602);

//    // Binding newly created socket to given IP and verification
//    if ((bind(sockfd, ( struct sockaddr*)&servaddr, sizeof(servaddr))) != 0) {
//        perror("bind: ");
//        printf("socket bind failed...\n");

//        exit(0);
//    }
//    else
//        fprintf(stderr, "Socket successfully binded..\n");

//    // Now server is ready to listen and verification
//    if ((myListen(sockfd, 5)) != 0) {
//        fprintf(stderr, "Listen failed...\n");
//        exit(0);
//    }
//    else
//        fprintf(stderr, "Server listening..\n");
//    len = sizeof(cli);

//    // Accept the data packet from client and verification
//    connfd = accept(sockfd, (sockaddr*)&cli, &len);
//    if (connfd < 0) {
//        fprintf(stderr, "server acccept failed...\n");
//        exit(0);
//    }
//    else
//        fprintf(stderr, "server acccept the client...\n");


//    fprintf(stderr, "accepted");

//}
