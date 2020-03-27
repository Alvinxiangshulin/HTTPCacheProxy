//
// Created by xianghui on 2/23/20.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <string>
#include <vector>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "proxyexceptions.h"

void send_message(const char * buffer, size_t len, int fd){
    while(len > 0){
        ssize_t status = send(fd, buffer, len, 0);
        if(status < 0){
            throw MyException("send error");
        }
        len -= status;
        buffer += status;
    }
}

//int pipe_communicate(int source_fd, int destination_fd){
//    std::vector<char> buf(BUFSIZ);
//    int status;
//    size_t recv_len = recv(source_fd, buf.data(), BUFSIZ, 0);
//    if(recv_len == 0){
//        return 0;
//    }
//    if(recv_len < 0){
//        return -1;
//    }
//    else{
//        status = send(destination_fd, buf.data(), recv_len, 0);
//        if(status < 0){
//            return -1;
//        }
//    }
//    return 1;
//}

int pipe_communicate(int source_fd, int destination_fd){
    std::vector<char> buf(BUFSIZ);
    int status;
    size_t recv_len = recv(source_fd, buf.data(), BUFSIZ, 0);
    if(recv_len == 0){
        return 0;
    }
    if(recv_len < 0){
        return -1;
    }
    status = send(destination_fd, buf.data(), recv_len, 0);
    if(status < 0){
        return -1;
    }
    return status;
}

std::string getClientIp(int cfd) {
    struct sockaddr_in clie_addr;
    socklen_t clie_addr_len = sizeof(clie_addr);
    getpeername(cfd, reinterpret_cast<sockaddr *>(&clie_addr), &clie_addr_len);
    char clie_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clie_addr.sin_addr.s_addr, clie_ip, INET_ADDRSTRLEN);
    return std::string(clie_ip);
}