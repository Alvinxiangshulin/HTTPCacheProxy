//
//  response.cpp
//  HTTPCahcingProxy
//
//  Created by Shulin Xiang on 2/23/20.
//  Copyright © 2020 Shulin Xiang. All rights reserved.
//

//
// Created by xianghui on 2/23/20.
//

#include "response.h"

Response::Response(const Response &rhs) {
    if(this != &rhs) {
        body = rhs.body;
        field_mp = rhs.field_mp;
        header_str = rhs.header_str;
        status_code = rhs.status_code;
    }
}

void Response::setId(int id) {
    Response::id = id;
}

const std::string &Response::getStatusCode() const {
    return status_code;
}

std::string Response::getLog(Request & request){
    size_t first_line = header_str.find("\r\n");
    std::string toLog = header_str.substr(0, first_line);
    std::string rlt = std::to_string(request.getRequestId()) + ": Received \"" + toLog + "\" from " + request.getHost();
    return rlt;
}

std::string Response::getLogResponding(Request & request){
    size_t first_line = header_str.find("\r\n");
    std::string toLog = header_str.substr(0, first_line);
    std::string rlt = std::to_string(request.getRequestId()) + ": Responding \"" + toLog + "\"";
    return rlt;
}