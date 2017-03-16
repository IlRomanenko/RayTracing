//
// Created by ilya on 11.03.17.
//
#pragma once

#include <thread>
#include <condition_variable>
#include <future>
#include <vector>
#include <queue>
#include <functional>

using namespace std;

template<class Value>
class ThreadPool {
    bool isShutdowned = false, shutdownWorker = false;
    mutex queue_mutex;
    vector<thread> workers;
    queue<packaged_task<Value()> > task_queue;
    condition_variable task_queue_not_empty;

    void TryExecuteTask() {
        packaged_task<Value()> task;
        {
            unique_lock<mutex> lock(queue_mutex);
            if (task_queue.empty()) {
                return;
            }
            task = move(task_queue.front());
            task_queue.pop();
        }
        task();
    }

    void workerFunction() {
        for (;;) {
            packaged_task<Value()> task;
            {
                unique_lock<mutex> lock(queue_mutex);
                task_queue_not_empty.wait(lock, [&]() { return !task_queue.empty() || isShutdowned || shutdownWorker; });

                if (shutdownWorker) {
                    shutdownWorker = false;
                    return;
                }

                if (task_queue.empty() && isShutdowned) {
                    return;
                }
                task = move(task_queue.front());
                task_queue.pop();
            }
            task();
        }
    }

public:
    explicit ThreadPool(size_t num_workers = 0) {
        for (size_t i = 0; i < num_workers; i++) {
            workers.emplace_back([&]() { workerFunction(); });
        }
    }

    void setWorkersNumber(size_t num_workers) {
        while (workers.size() != num_workers) {
            while (workers.size() < num_workers) {
                workers.emplace_back([&]() { workerFunction(); });
            }
        }
    }

    future<Value> submit(function<Value()> task) {

        unique_lock<mutex> lock(queue_mutex);
        packaged_task<Value()> pckg_task(task);
        future<Value> result = pckg_task.get_future();

        if (isShutdowned) {
            throw exception();
        }
        task_queue.push(std::move(pckg_task));
        task_queue_not_empty.notify_one();
        return result;
    }

    void wait(future<Value> &&future) {
        while (future.wait_for(chrono::milliseconds(0)) != future_status::ready) {
            TryExecuteTask();
        }
    }

    void shutdown() {
        {
            unique_lock<mutex> lock(queue_mutex);
            if (isShutdowned)
                return;
            isShutdowned = true;
        }
        task_queue_not_empty.notify_all();
        for (auto &worker : workers) {
            worker.join();
        }
    }
};