//
// Created by xianghui on 2/27/20.
//

#ifndef HW2_GIVEUP_LOGGER_H
#define HW2_GIVEUP_LOGGER_H

#include <iostream>
#include <fstream>

#include "request.h"

class Logger{
private:
    std::ofstream of;
    std::string content;
public:
    Logger(){
        content = "";
        of.open("../proxy.log", std::ios::out|std::ios::app);
        //of.open("../proxy.log", std::ios::out);
    }

    ~Logger(){
        of.close();
    }

    void logRequest(Request & req, std::string clie_ip){
        tm *gmtm = gmtime(&req.request_time);
        char *dt = asctime(gmtm);
        of<<req.request_id<<":"<<" \""<<req.to_logger<<"\" from "<<clie_ip + " @ " + dt;
    }

    void addContent(std::string & str){
        content += str;
    }

    void writeContent(){
        of<<content;
    }
};
#endif //HW2_GIVEUP_LOGGER_H
