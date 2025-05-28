#include "./RedisLock.h"

void test()
{
    auto redis = sw::redis::Redis("tcp://127.0.0.1:6379");
    std::string lockId = RedisLock::getInstance()->acquireLock(redis, "lunisolar", 10, 5);
    std::cout << "lockId: " << lockId << std::endl;
}

int main()
{
    test();
    for (int i = 0; i < 10; i++)
    {
        std::cout << "hello world" << std::endl;
    }
    return 0;
}