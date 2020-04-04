

#include "cacheitem.h"

time_t CacheItem::getCacheTime() const {
    return cache_time;
}

const Response &CacheItem::getResponse() const {
    return response;
}
