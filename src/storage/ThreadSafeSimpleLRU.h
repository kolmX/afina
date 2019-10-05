#ifndef AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
#define AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H

#include <map>
#include <mutex>
#include <string>

#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

/**
 * # SimpleLRU thread safe version
 *
 *
 */
class ThreadSafeSimplLRU : public SimpleLRU {
public:
    ThreadSafeSimplLRU(size_t max_size = 1024) : SimpleLRU(max_size) {}
    ~ThreadSafeSimplLRU() {}

    // see SimpleLRU.h
    bool Put(const std::string &key, const std::string &value) override {
        {
            std::lock_guard<std::mutex> lock(_access_mutex);
            bool result = SimpleLRU::Put(key, value);
            return result;
        }
    }

    // see SimpleLRU.h
    bool PutIfAbsent(const std::string &key, const std::string &value) override {
        {
            std::lock_guard<std::mutex> lock(_access_mutex);
            bool result = SimpleLRU::PutIfAbsent(key, value);
            return result;
        }
    }

    // see SimpleLRU.h
    bool Set(const std::string &key, const std::string &value) override {
        {
            std::lock_guard<std::mutex> lock(_access_mutex);
            bool result = SimpleLRU::Set(key, value);
            return result;
        }
    }

    // see SimpleLRU.h
    bool Delete(const std::string &key) override {
        {
            std::lock_guard<std::mutex> lock(_access_mutex);
            bool result = SimpleLRU::Delete(key);
            return result;
        }
    }

    // see SimpleLRU.h
    bool Get(const std::string &key, std::string &value) override {
        {
            std::lock_guard<std::mutex> lock(_access_mutex);
            bool result = SimpleLRU::Get(key, value);
            return result;
        }
    }

private:
    std::mutex _access_mutex;
};

} // namespace Backend
} // namespace Afina

#endif // AFINA_STORAGE_THREAD_SAFE_SIMPLE_LRU_H
