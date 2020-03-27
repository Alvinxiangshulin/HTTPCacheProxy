#include "request.h"
#include "proxyexceptions.h"

std::string Request::getHost() const {
    size_t colon = host.find(":");
    if(colon != std::string::npos){
        std::string result = host.substr(0, colon);
        return result;
    }
    return host;
}

const std::string &Request::getPort() const {
    return port;
}

const std::string &Request::getMethod() const {
    return method;
}

int Request::getRequestId() const {
    return request_id;
}

void Request::parseHead(){

    std::stringstream ss(header);
    std::string temp;
    ss>>method;
    if(method != "GET" && method != "POST" && method != "CONNECT"){
        throw CorruptRequestException();
    }

    std::string full_path;  //"http://www.baidu.com:335/abc.txt"
    ss>>full_path;


    ss>>version;
    if(version != "HTTP/1.1"){
        throw CorruptRequestException();
    }
    to_logger = method + " " + full_path + " " + version;

    size_t line_idx = header.find("\r\n") + 2;
    std::string line = getLine(line_idx);

    while (!line.empty()){
        size_t colon = header.find(":", line_idx);
        std::string key = header.substr(line_idx, colon - line_idx);
        line_idx = header.find("\r\n", colon);
        std::string value = header.substr(colon + 2, line_idx - colon - 2);
        field_mp.insert(std::pair<std::string, std::string>(key, value));
        line_idx += 2;
        line = getLine(line_idx);
    }

    host = field_mp["Host"];

    const char * url_start = strstr(full_path.c_str(), host.c_str());
    const char * path_c = url_start + strlen(host.c_str());
    path = (std::string) path_c;  // "/abc.txt"
    if(path.empty()){
        path = "/";
    }
    size_t colon_idx = host.find(':');
    if(colon_idx == std::string::npos){
        if(method == "CONNECT"){
            port = "443";
        }
        else{
            port = "80";
        }
    }
    else{
        port = host.substr(colon_idx + 1);
    }
}

void Request::setRequestId(int requestId) {
    request_id = requestId;
}

std::string Request::getLog(){
    std::string rlt = std::to_string(request_id) + ": Requesting \"" + method + " " +path + " " + version +"\" from " + host;
    return rlt;
}