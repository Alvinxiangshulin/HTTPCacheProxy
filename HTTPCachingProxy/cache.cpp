//
// Created by xianghui on 2/28/20.
//

#include "cache.h"

void Cache::addCacheItem(Request & request, Response & response){
    std::string key = request.to_logger;

    if(mp.count(key) > 0){
        std::map<std::string, CacheItem>::iterator it;
        it = mp.find(key);
        (*it).second = response;
        return;
    }

    if(mp.size() >= CACHE_SIZE){
        std::string remove = q.front();
        q.pop();
        mp.erase(remove);
        printf("BEBUG: cache is full\n");
    }
    CacheItem cacheItem(response);
    q.push(request.to_logger);
    mp.insert(std::pair<std::string, CacheItem>(request.to_logger, cacheItem));
}

cacheAction Cache::receiveRequest(Request & request, std::string & msg){
    msg = std::to_string(request.request_id);
    std::string key = request.to_logger;
    cacheAction act = NOT_IN_CACHE;

    if(mp.count(key) == 0){
        msg += ": not in cache\n";
        act = NOT_IN_CACHE;
        printf("DEBUG: %d is not in cache\n", request.request_id);
    }
    else{
        if(request.field_mp.count("Cache-Control") > 0){
            std::string request_control = request.field_mp["Cache-Control"];
            if(request_control.find("no-cache") != std::string::npos){
                msg += ": cached, but requires re-validation\n";
                Response cached_response = mp[key].getResponse();
                if(cached_response.field_mp.count("ETag") > 0 || cached_response.field_mp.count("Etag") > 0){
                    request.addValidate("If-None-Match", cached_response.field_mp["ETag"]);
                }
                if(cached_response.field_mp.count("Last-Modified") > 0){
                    request.addValidate("If-Modified-Since", cached_response.field_mp["Last-Modified"]);
                }
                act = RE_VALID;
            }
            else{
                bool flag = isCacheStale(request, msg);
                if(flag){
                    msg += ": cached, but requires re-validation\n";
                    act = RE_VALID;
                }
                else {
                    msg += ": in cache, valid\n";
                    act = IN_CACHE;
                }
            }
        }
        else{
            msg += ": in cache, valid\n";
            act = IN_CACHE;
        }
        printf("DEBUG: %d found in cache\n", request.request_id);
    }
    return act;
}

std::string Cache::receiveResponse(Request& request, Response &response) {
    std::string msg = "";
    std::string key = request.to_logger;

        if (response.field_mp.count("Cache-Control")) {
            msg = std::to_string(response.id);
            if (response.field_mp["Cache-Control"].find("no-store") != std::string::npos) {
                msg += ": not cacheable because no-store\n";
                return msg;
            } else if (response.field_mp["Cache-Control"].find("private") != std::string::npos) {
                msg += ": not cacheable because private\n";
                return msg;
            } else if (response.field_mp["Cache-Control"].find("no-cache") != std::string::npos) {
                msg += ": cached, but requires re-validation\n";
                return msg;
            } else if (response.field_mp["Cache-Control"].find("max-age") != std::string::npos) {
                msg = getExpireForResponse(request, response);
                if (mp.count(key) == 0) {
                    addCacheItem(request, response);
                }
                return msg;
            } else {

                addCacheItem(request, response);
                msg += std::to_string(request.request_id) + ": cached, no cache-control found\n";
            }
        } else {
            if (response.field_mp.count("Expires")) {
                msg = getExpireForResponse(request, response);
            }

            addCacheItem(request, response);

            msg += std::to_string(request.request_id) + ": cached, no cache-control found\n";
        }

    return msg;
}

const Response Cache::getCachedResponse(Request & request){
    std::string key = request.to_logger;
    CacheItem ci = mp[key];
    return ci.getResponse();
}