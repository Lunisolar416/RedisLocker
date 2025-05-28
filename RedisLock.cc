#include "RedisLock.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <sw/redis++/command_options.h>
#include <sw/redis++/redis++.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <thread>
#include <chrono>
std::shared_ptr<RedisLock> RedisLock::instance = nullptr;
std::once_flag RedisLock::initFlag;

static std::string generate_uuid()
{
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
}

std::shared_ptr<RedisLock> RedisLock::getInstance()
{

    std::call_once(initFlag, []() { instance = std::shared_ptr<RedisLock>(new RedisLock()); });
    return instance;
}

std::string RedisLock::acquireLock(sw::redis::Redis& redis, const std::string& lockname, int lockTimeOut,
                                   int acquireTimeOut)
{
    std::string identifier = generate_uuid();
    std::string lockKey = "lock:" + lockname;

    auto endTime = std::chrono::steady_clock::now() + std::chrono::seconds(acquireTimeOut);
    while (std::chrono::steady_clock::now() < endTime)
    {
        bool ok = redis.set(lockKey, identifier, std::chrono::seconds(lockTimeOut), sw::redis::UpdateType::NOT_EXIST);
        if (ok)
        {
            return identifier;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return "";
}
bool RedisLock::releaseLock(sw::redis::Redis& redis, const std::string& lockname, const std::string& identifier)
{
    std::string lockKey = "lock:" + lockname;
    const char* luaScript = "if redis.call('get', KEYS[1]) == ARGV[1] then \
                                return redis.call('del', KEYS[1]) \
                             else \
                                return 0 \
                             end";
    try
    {
        auto result = redis.eval<long long>(luaScript, {lockKey}, {identifier});
        return result == 1;
    }
    catch (const sw::redis::Error& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
}