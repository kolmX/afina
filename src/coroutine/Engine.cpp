#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx) {
    char top;
    ctx.Low = &top;
    ctx.Hight = StackBottom;
    size_t size = ctx.Hight - ctx.Low;

    if (std::get<1>(ctx.Stack) < size) {
        delete[] std::get<0>(ctx.Stack);
        std::get<0>(ctx.Stack) = new char[size];
        std::get<1>(ctx.Stack) = size;
    }
    memcpy(std::get<0>(ctx.Stack), ctx.Low, size);
}

void Engine::Restore(context &ctx) {

    char stack_ptr;
    if (&stack_ptr >= ctx.Low) {
        Restore(ctx);
    }

    memcpy(ctx.Low, std::get<0>(ctx.Stack), std::get<1>(ctx.Stack));
    longjmp(ctx.Environment, 1);
}

void Engine::yield() {
    context *next_routine = alive;

    if (next_routine == cur_routine && next_routine) {
        next_routine = next_routine->next;
    }
    if (!next_routine) {
        return;
    }

    sched(next_routine);
}

void Engine::sched(void *routine_) {
    if (cur_routine != nullptr) {
        // update Enviroment
        if (setjmp(cur_routine->Environment) > 0) {
            return;
        }
        Store(*cur_routine);
    }
    cur_routine = static_cast<context *>(routine_);
    Restore(*cur_routine);
}

} // namespace Coroutine
} // namespace Afina
