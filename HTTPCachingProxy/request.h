#ifndef INC_568_HW2_GIVEUP_REQUEST_H
#define INC_568_HW2_GIVEUP_REQUEST_H

#include <vector>
#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <string>
#include <map>
#include <sstream>
#include <cstring>
#include <iostream>
#include <mutex>

#include "utils.h"
#include "proxyexceptions.h"

class Request{
private:
    std::string header;
    std::string method;
    std::string version;
    std::string host;
    std::string path;
    std::string port;
    std::string to_logger;
    time_t request_time;
    int request_id;

    std::map<std::string, std::string> field_mp;

    friend class Logger;
    friend class Cache;

    std::string getLine(size_t line_start){
        size_t end_idx = header.find("\r\n", line_start);
        if(end_idx == std::string::npos){
            return "";
        }
        return header.substr(line_start, end_idx - line_start);
    }

    void parseHead();

public:

    Request(int client_fd){
        time_t t_now= time(0);
        struct tm * ptm;
        ptm = gmtime(&t_now);
        request_time = mktime(ptm);

        char receive_char[1];
        int n;
        while ((n = recv(client_fd, receive_char, 1, MSG_WAITALL)) > 0) {
            header.push_back(receive_char[0]);
            size_t temp = header.find("\r\n\r\n");
            if (temp != std::string::npos) {
                break;
            }
        }

        if(header.empty()){
            throw MyException("Header is empty");
        }
    }
    
    std::string getBody(int client_fd){
        std::string body;
        char receive_char[1];
        int n;
        int cnt = 0;
        while ((n = recv(client_fd, receive_char, 1, MSG_WAITALL)) > 0) {
            body.push_back(receive_char[0]);
            cnt++;
            if (cnt == std::stoi(field_mp["Content-Length"]) || n == 0) {
                break;
            }
        }
        std::cout<<body;
        return body;
    }
    
    std::string getRequestString(){
        parseHead();
        std::string first_line = method + " " +path + " " + version + "\r\n";
        std::string remain_request = header.substr(header.find("\r\n") + 2);
        return first_line + remain_request;
    }

    void addValidate(const std::string& key, const std::string& val){
        field_mp.insert(std::pair<std::string, std::string>(key, val));
    }

    std::string getHost() const;

    const std::string &getPort() const;

    const std::string &getMethod() const;

    int getRequestId() const;

    void setRequestId(int requestId);

    std::string getLog();

    std::string revise_request_str(){
        std::string first_line = method + " " +path + " " + version + "\r\n";
        std::map<std::string, std::string>::iterator it;
        for(it = field_mp.begin(); it != field_mp.end(); ++it){
            if((*it).first == "ETag"){
                continue;
            }
            if((*it).first == "Last-Modified"){
                continue;
            }
            first_line += (*it).first + ": " + (*it).second + "\r\n";
        }
        first_line += "\r\n";
        return first_line;
    }

    std::string revise_request_str_no_cache(){
        std::string first_line = method + " " +path + " " + version + "\r\n";
        std::map<std::string, std::string>::iterator it;
        for(it = field_mp.begin(); it != field_mp.end(); ++it){
            if((*it).first == "ETag"){
                continue;
            }
            if((*it).first == "Last-Modified"){
                continue;
            }
            if((*it).first == "If-Modified-Since"){
                continue;
            }
            if((*it).first == "If-None-Match"){
                continue;
            }
            first_line += (*it).first + ": " + (*it).second + "\r\n";
        }
        first_line += "\r\n";
        return first_line;
    }

    std::string connectLog(){
        std::string rlt = std::to_string(request_id) + ": Tunnel closed\n";
        return rlt;
    }
};
#endif //INC_568_HW2_GIVEUP_REQUEST_H
