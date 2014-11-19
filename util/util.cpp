/* Implementation code for various utilities */

#include "file.hpp"
#include "task_list.hpp"

#include <fstream>
#include <sstream>
#include <thread>

namespace util {
    string read_file(const char* filename) {
        ifstream ifs(filename, ios::in | ios::binary | ios::ate);

        if (!ifs.good()) {
            throw runtime_error(string("Could not open file ") + filename);
        }

        ifstream::pos_type fileSize = ifs.tellg();
        ifs.seekg(0, ios::beg);

        vector<char> bytes(fileSize);
        ifs.read(&bytes[0], fileSize);

        return string{bytes.begin(), bytes.end()};
    }

    void TaskList::add(Task t) {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.push_back(t);
    }

    void TaskList::run() {
        std::vector<Task> run_tasks;
        std::unique_lock<std::mutex> lock(mutex);
        run_tasks.swap(tasks);
        lock.unlock();
        for (auto task : run_tasks) {
            task();
        }
    }
}
