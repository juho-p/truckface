#pragma once

#include <cassert>
#include <atomic>

namespace util {
    namespace threaded {
        enum Status_ {
            Idle, Running, Stopping
        };
        typedef std::atomic<Status_> Status;

        template<class C>
        void stop(C* obj) {
            if (obj->thread_status != Idle) {
                auto expected = Running;
                obj->thread_status.compare_exchange_weak(expected, Stopping);
                obj->thread.join();
                assert(obj->thread_status == Idle);
            }
        }

        template<class C>
        bool pre_run(C* obj) {
            auto status = obj->thread_status.load();
            if (status == Running) {
                // thread already running, do nothing
                return false;
            } else if (status == Stopping) {
                // wait for thread to stop before starting it again
                stop(obj);
            }

            // start the thread
            assert(obj->thread_status == Idle);
            obj->thread_status.store(Running);

            return true;
        }

        template<class C>
        void post_run(C* obj) {
            auto expected = Stopping;
            bool ok = obj->thread_status.compare_exchange_weak(expected, Idle);
            assert(ok);
        }

        template<class C>
        bool is_running(C* obj) {
            return obj->thread_status == Running;
        }
    }
}
