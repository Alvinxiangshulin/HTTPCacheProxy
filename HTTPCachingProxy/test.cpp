//
// Created by xianghui on 2/23/20.
//
#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include <string>
#include <iomanip>
#include <sstream>

int main(){
    std::string date = "Fri, 28 Feb 2020 14:00:01 GMT";
    std::string gmt = date.substr(0, date.length() - 4);
    struct std::tm tm;
    std::istringstream ss(gmt);
    ss>>std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");

    time_t t = mktime(&tm);
    time_t t_now = time(0);

    struct tm * ptm;
    ptm = gmtime(&t_now);
    t_now = mktime(ptm);
    printf("%s\n",asctime(&tm));
    printf("%s\n",asctime(ptm));
    printf("compare %d\n", t < t_now);
    return 0;
}