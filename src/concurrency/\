#include "afina/concurrency/Executor.h"

#include <cassert>

namespace Afina {
namespace Concurrency {
Executor::Executor(const uint32_t low_watermark, const uint32_t high_watermark,
    const uint32_t max_queue_size, const  uint32_t idle_time){

        this->low_watermark = low_watermark;
        this->high_watermark = high_watermark;
        this->max_queue_size = max_queue_size;
        this->idle_time = idle_time;
        this->num_idle = low_watermark;

        state.store(State::kRun);

        for (size_t i = 0; i < low_watermark; i++){
            threads.emplace_back(std::thread(&Executor::perform, this, ThreadType::low));
        }
}

void Executor::Stop(const bool await){

    state.store(State::kStopping);
    //unblock all thread if task queue is empty
    empty_tasks.notify_all();

    if (await){
        for (std::thread &thread:threads){
            assert(thread.joinable());
            thread.join();
        }
        state.store(State::kStopped);
    }
}

bool Executor::have_tasks(){
    std::lock_guard<std::mutex> lk(state_mutex);
    return tasks.empty();
}

void Executor::exec_func(){


    std::unique_lock<std::mutex> lk(state_mutex);
    if (tasks.empty()){
        return;
    }

    auto func = tasks.front();
    tasks.pop_front();
    lk.unlock();

    num_idle--;
    func();
    num_idle++;
    return;

}

void Executor::perform(const ThreadType type){

    while (state.load() == State::kRun){
        if (type == ThreadType::low){
            std::unique_lock<std::mutex> lk(cond_mutex);
            while ( have_tasks()){
                empty_tasks.wait(lk);
            }
        }

        else if (type == ThreadType::high){
            std::unique_lock<std::mutex> lk(cond_mutex);
            while (have_tasks()){
                if (empty_tasks.wait_for(lk, milliseconds(idle_time)) == std::cv_status::timeout){
                    num_idle--;
                    return;
                }
            }
        }

        if (state.load() == State::kStopping){
            break;
        }
        exec_func();
    }

    while (state.load() == State::kStopping && have_tasks()){
        exec_func();
    }
    return;
}


}
} // namespace Afina
