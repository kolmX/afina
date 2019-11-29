#include <afina/concurrency/Executor.h>

#include <cassert>

namespace Afina {
namespace Concurrency {

Executor::Executor(const uint32_t low_watermark, const uint32_t high_watermark, const uint32_t max_queue_size,
                   const uint32_t idle_time) {

    //--------------------One thread is here -----------------------------------
    this->low_watermark = low_watermark;
    this->high_watermark = high_watermark;
    this->max_queue_size = max_queue_size;
    this->idle_time = idle_time;
    this->num_idle = low_watermark;
    num_threads = this->low_watermark;
    state = State::kRun;
    //--------------------------------------------------------------------------

    for (size_t i = 0; i < low_watermark; i++) {
        std::thread t = std::thread(&Executor::perform, this);
        t.detach();
    }
}

void Executor::Stop(const bool await) {

    std::unique_lock<std::mutex> lk(_mutex);
    if (state == State::kStopped) {
        return;
    }

    state = State::kStopping;
    if (!num_threads) {
        state = State::kStopped;
        return;
    }
    // unblock all thread
    empty_condition.notify_all();

    if (await) {

        while (state != State::kStopped) {
            empty_threads.wait(lk);
        }
    }
}

void Executor::perform() {

    for (;;) {

        std::unique_lock<std::mutex> lk(_mutex);
        bool isTimeout = false;
        if (num_threads < high_watermark) {
            while (tasks.empty() && state == State::kRun) {
                if (empty_condition.wait_for(lk, milliseconds(idle_time)) == std::cv_status::timeout) {
                    isTimeout = true;
                    break;
                }
            }
        }

        if ((tasks.empty() && state == State::kStopping)) {
            break
        }
        if (isTimeout) {
            if (num_threads > low_watermark) {
                break;
            }
            continue;
        }
        auto func = tasks.front();
        tasks.pop_front();
        lk.unlock();

        num_idle--;
        try {
            func();
        } catch (...) {
            std::terminate();
        }

        num_idle++;
    }

    {
        std::lock_guard<std::mutex> lk(_mutex);
        num_idle--;
        num_threads--;
        if (!num_threads) {
            state = State::kStopped;
            empty_threads.notify_all();
        }

        return;
    }
}

Executor::~Executor() { Stop(true); }

} // namespace Concurrency
} // namespace Afina
