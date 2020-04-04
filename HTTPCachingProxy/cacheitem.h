

#ifndef HW2_GIVEUP_CACHEITEM_H
#define HW2_GIVEUP_CACHEITEM_H

#include <ctime>

#include "response.h"

class CacheItem{
private:
    time_t cache_time;
    Response response;
public:

    CacheItem(){}

    CacheItem(Response new_response): response(new_response){
        cache_time = time(0);
    }

    time_t getCacheTime() const;

    const Response &getResponse() const;


};
#endif //HW2_GIVEUP_CACHEITEM_H
