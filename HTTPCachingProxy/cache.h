//
// Created by xianghui on 2/28/20.
//

#ifndef HW2_GIVEUP_CACHE_H
#define HW2_GIVEUP_CACHE_H

#include <map>
#include <queue>
#include <iomanip>

#include "cacheitem.h"
#include "request.h"
#include "logger.h"

#define CACHE_SIZE 50

enum cacheAction{
    IN_CACHE,
    RE_VALID,
    NOT_IN_CACHE
};

class Cache{
private:
    std::map<std::string, CacheItem> mp;
    std::queue<std::string> q;

    int getMaxAge(Request & request){
        int req_max_age = INT32_MAX;
        int res_max_age = INT32_MAX;
        std::string key = request.to_logger;

        if(request.field_mp.count("Cache-Control") > 0) {
            std::string request_control = request.field_mp["Cache-Control"];
            if (request_control.find("max-age") != std::string::npos) {
                size_t pos = request_control.find("max-age");
                pos = pos + 8;
                req_max_age = std::stoi(request_control.substr(pos));
            }
        }

        CacheItem ci = mp[key];
        Response response = ci.getResponse();
        if(response.field_mp.count("Cache-Control") > 0) {
            std::string response_control = response.field_mp["Cache-Control"];
            if (response_control.find("max-age") != std::string::npos) {
                size_t pos = response_control.find("max-age");
                pos = pos + 8;
                res_max_age = std::stoi(response_control.substr(pos));
            }
        }

        int max_age = std::min(req_max_age, res_max_age);
        return max_age;
    }

    int getStaleTime(Request & request){
        int max_stale = 0;
        std::string key = request.to_logger;
        if(request.field_mp.count("Cache-Control") > 0){
            std::string request_control = request.field_mp["Cache-Control"];
            if(request_control.find("max-stale") != std::string::npos){
                size_t pos = request_control.find("max-stale");
                pos = pos + 9;
                max_stale = std::stoi(request_control.substr(pos));
            }
        }
        return max_stale;
    }

    int getMinFresh(Request & request){
        int min_fresh = 0;
        std::string key = request.to_logger;
        if(request.field_mp.count("Cache-Control") > 0){
            std::string request_control = request.field_mp["Cache-Control"];
            if(request_control.find("min-fresh") != std::string::npos){
                size_t pos = request_control.find("max-age");
                pos = pos + 9;
                min_fresh = std::stoi(request_control.substr(pos));
            }

        }
        return min_fresh;
    }

    time_t getExpireTime(int max_age, int max_stale, int min_fresh, Response & response){
        std::string date = response.field_mp["Date"];
        std::string gmt = date.substr(0, date.length() - 4);
        struct std::tm tm;
        std::istringstream ss(gmt);
        ss>>std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");

        time_t t = mktime(&tm);

        time_t expire_time = t + max_age + max_stale - min_fresh;

        if(response.field_mp.count("Expires") > 0){
            std::string expire_str = response.field_mp["Expires"];

            std::string gmt_expire = expire_str.substr(0, date.length() - 4);
            struct std::tm tm_expire;
            std::istringstream ss_expire(gmt_expire);
            ss>>std::get_time(&tm_expire, "%a, %d %b %Y %H:%M:%S");

            time_t expire = mktime(&tm_expire);
            return std::min(expire, expire_time);
        }
        return expire_time;
    }

    bool isCacheStale(Request & request, std::string & message){
        int max_age = getMaxAge(request);
        int max_stale = getStaleTime(request);
        int min_fresh = getMinFresh(request);
        time_t request_time = request.request_time;
        //for test
       // max_age = 500;

        std::string key = request.to_logger;
        CacheItem ci = mp[key];
        Response response = ci.getResponse();
        if(response.field_mp.count("Cache-Control") > 0){
            if(max_age == 0){
                if(response.field_mp.count("ETag") > 0 || response.field_mp.count("Etag") > 0){
                    request.addValidate("If-None-Match", response.field_mp["ETag"]);
                }
                if(response.field_mp.count("Last-Modified") > 0){
                    request.addValidate("If-Modified-Since", response.field_mp["Last-Modified"]);
                }
                return true;
            }

            time_t expireTime = getExpireTime(max_age, max_stale, min_fresh, response);
            tm *gmtm = gmtime(&expireTime);
            char *dt = asctime(gmtm);

            bool flag = (expireTime <= request_time);

            if(flag){
                if(response.field_mp.count("ETag") > 0 || response.field_mp.count("Etag") > 0){
                    request.addValidate("If-None-Match", response.field_mp["ETag"]);
                }
                if(response.field_mp.count("Last-Modified") > 0){
                    request.addValidate("If-Modified-Since", response.field_mp["Last-Modified"]);
                }
                return true;
            }
            return flag;
        }
        else{
            //int max_age = getMaxAge(request);
            //time_t request_time = request.request_time;
            if(max_age == 0){
                if(response.field_mp.count("ETag") > 0 || response.field_mp.count("Etag") > 0){
                    request.addValidate("If-None-Match", response.field_mp["ETag"]);
                }
                if(response.field_mp.count("Last-Modified") > 0){
                    request.addValidate("If-Modified-Since", response.field_mp["Last-Modified"]);
                }
                return true;
            }
            else{
                time_t expireTime = getExpireTime(max_age, max_stale, min_fresh, response);
                tm *gmtm = gmtime(&expireTime);
                char *dt = asctime(gmtm);
                message = request.request_id + ": cached, expires at " + std::string(dt);

                bool flag = (expireTime <= request_time);

                if(flag){
                    if(response.field_mp.count("ETag") > 0 || response.field_mp.count("Etag") > 0){
                        request.addValidate("If-None-Match", response.field_mp["ETag"]);
                    }
                    if(response.field_mp.count("Last-Modified") > 0){
                        request.addValidate("If-Modified-Since", response.field_mp["Last-Modified"]);
                    }
                    return true;
                }
                return flag;
            }
            return false;
        }
    }

    std::string getExpireForResponse(Request & request, Response & response){
        std::string msg = "";
        if(response.field_mp.count("Cache-Control") > 0) {
            std::string response_control = response.field_mp["Cache-Control"];
            if (response_control.find("max-age") != std::string::npos) {
                size_t pos = response_control.find("max-age");
                pos = pos + 8;
                int res_max_age = std::stoi(response_control.substr(pos));
                time_t expireTime = getExpireTime(res_max_age, 0, 0, response);
                tm *gmtm = gmtime(&expireTime);
                char *dt = asctime(gmtm);
                msg = std::to_string(request.request_id) + ": cached, expires at " + std::string(dt);
            }
        }
        return msg;
    }

public:
    Cache(){}

    void addCacheItem(Request & request, Response & response);

    cacheAction receiveRequest(Request & request, std::string & msg);

    std::string receiveResponse(Request& request, Response &response);

    const Response getCachedResponse(Request & request);
};
#endif //HW2_GIVEUP_CACHE_H
