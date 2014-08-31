#pragma once

#include <vector>
#include <functional>
#include <mutex>

namespace util {
    typedef std::function<void()> Task;

    /**
     * Concurrent container for function objects
     *
     * One thread can add tasks with add, other thread can execute them with run
     */
    class TaskList {
        std::vector<Task> tasks;
        std::mutex mutex;
        
    public:
        void add(Task t);
        void run();
    };
}
