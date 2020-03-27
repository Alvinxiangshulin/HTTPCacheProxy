//
// Created by xianghui on 2/27/20.
//

#ifndef HW2_GIVEUP_PROXYEXCEPTIONS_H
#define HW2_GIVEUP_PROXYEXCEPTIONS_H

#include <exception>
#include <string>

class URLErrorException: public std::exception{

public:
    const char * what () const throw () override
    {
        return "URL error";
    }
};

class ConnectionErrorException: public std::exception{

public:
    const char * what () const throw () override
    {
        return "connection error or closed";
    }
};

class CacheErrorException: public std::exception{

public:
    const char * what () const throw () override
    {
        return "receive 304 from web, but no such item in cache\n";
    }
};

class CorruptRequestException: public std::exception{

public:
    const char * what () const throw () override
    {
        return "Illegal request\n";
    }
};

class MyException : public std::exception
{
    std::string error;
public:
    MyException(const std::string &err): error(err){}
    virtual const char * what () const noexcept override
    {
        return error.c_str();
    }
};
#endif //HW2_GIVEUP_PROXYEXCEPTIONS_H
