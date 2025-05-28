#ifndef REDISLOCK_H
#define REDISLOCK_H

#include <memory>
#include <string>
#include <sw/redis++/redis++.h>
#include <sw/redis++/redis.h>
#include <iostream>
class RedisLock
{
  public:
    static std::shared_ptr<RedisLock> getInstance();
    std::string acquireLock(sw::redis::Redis& redis, const std::string& lockname, int lockTimeOut, int acquireTimeOut);
    bool releaseLock(sw::redis::Redis& redis, const std::string& lockname, const std::string& identifier);

  private:
    static std::shared_ptr<RedisLock> instance;
    static std::once_flag initFlag;
    RedisLock() = default;
};

#endif // REDISLOCK_H