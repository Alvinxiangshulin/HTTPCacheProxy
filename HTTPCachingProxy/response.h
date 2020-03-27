//
// Created by xianghui on 2/23/20.
//

#ifndef INC_568_HW2_GIVEUP_RESPONSE_H
#define INC_568_HW2_GIVEUP_RESPONSE_H

#include <vector>
#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <string>
#include <map>
#include <sstream>
#include <cstring>
#include <iostream>

#include "utils.h"
#include "request.h"

class Response{
private:
    std::vector<char> body;
    std::map<std::string, std::string> field_mp;
    std::string header_str;
    std::string status_code;
    int id;
    friend class Cache;

    std::string getLine(size_t line_start){
        size_t end_idx = header_str.find("\r\n", line_start);
        if(end_idx == std::string::npos){
            return NULL;
        }
        return header_str.substr(line_start, end_idx - line_start);
    }

    void parseHead(){
        std::string temp;
        std::stringstream ss(header_str);
        ss>>temp;
        ss>>status_code;

        size_t line_idx = header_str.find("\r\n") + 2;
        std::string line = getLine(line_idx);

        while (!line.empty()){
            size_t colon = header_str.find(":", line_idx);
            std::string key = header_str.substr(line_idx, colon - line_idx);
            line_idx = header_str.find("\r\n", colon);
            std::string value = header_str.substr(colon + 2, line_idx - colon - 2);
            field_mp.insert(std::pair<std::string, std::string>(key, value));
            line_idx += 2;
            line = getLine(line_idx);
        }
    }

public:
    Response (){}

    Response(int web_fd) {
        char receive_char[1];
        int n;
        while ((n = recv(web_fd, receive_char, 1, MSG_WAITALL)) > 0) {
            header_str.push_back(receive_char[0]);
            size_t temp = header_str.find("\r\n\r\n");
            if (temp != std::string::npos) {
                break;
            }
        }

        //std::cout<<"DEBUG: receive from web: \n"<<header_str<<std::endl;
        parseHead();
    }

    Response(const Response & rhs);

    void getBody(int web_fd){
        if(field_mp.count("Content-Length")) {
            std::stringstream ss(field_mp["Content-Length"]);
            int length;
            int status = 0;
            ss >> length;
            //printf("DEBUG: content length: %d\n", length);
            if (length > 0){
                char body_cstr[length];
                memset(body_cstr, '0', length);
                status = recv(web_fd, body_cstr, length, MSG_WAITALL);
                if(status == -1){
                    throw MyException("Error in recv body in response");
                }
                for (int i = 0; i < length; i++) {
                    body.push_back(body_cstr[i]);
                }
            }
        }
        else if(field_mp.count("Transfer-Encoding") > 0){
            if(field_mp["Transfer-Encoding"] == "chunked"){
                std::string text;
                char receive_char[1];
                int n;
                while((n = recv(web_fd,receive_char,1,MSG_WAITALL))>0){
                    text.push_back(receive_char[0]);
                    if (text.find("\r\n\r\n") != std::string::npos) {
                        break;
                    }
                }
                for (int i = 0; i < text.length(); i++) {
                    body.push_back(text[i]);
                }
            }
        }
        else{
            throw MyException("no content-length");
        }
    }

    void sendClient(int client_fd, int web_fd){
        try{
            getBody(web_fd);
        }
        catch (MyException & e){
            std::cout<<e.what()<<std::endl;
        }

        send_message(header_str.c_str(), header_str.size(), client_fd);
        send_message(body.data(), body.size(), client_fd);
    }

    void sendCacheContent(int client_fd){
        send_message(header_str.c_str(), header_str.size(), client_fd);
        send_message(body.data(), body.size(), client_fd);
    }

    void setId(int id);

    const std::string &getStatusCode() const;

    std::string getLog(Request & request);

    std::string getLogResponding(Request & request);
};
#endif //INC_568_HW2_GIVEUP_RESPONSE_H
