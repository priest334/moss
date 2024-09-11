#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>


using namespace std::chrono;
using std::condition_variable;
using std::deque;
using std::shared_ptr;
using std::mutex;
using std::thread;
using std::vector;
using std::weak_ptr;

namespace moss {
	class Task {
	public:
		virtual void Run() = 0;
	};

	class TaskQueue;
	class TaskRunner
		: public std::enable_shared_from_this<TaskRunner> {
		class Worker {
			friend class TaskRunner;
			static void ThreadProc(shared_ptr<Worker> worker);
		public:
			Worker(shared_ptr<TaskRunner> task_runner);
			~Worker();
			shared_ptr<TaskRunner> GetTaskRunner();
			void Start(shared_ptr<thread> t);
			bool IsBusy() const;
			void SetBusy();
			void SetIdle();
			void Stop();
			bool IsStopping() const;
			void Join();
			steady_clock::duration BusyTime() const;
		private:
			weak_ptr<TaskRunner> task_runner_;
			std::atomic_bool busy_;
			std::atomic_bool stopping_;
			shared_ptr<thread> thread_;
			steady_clock::time_point busy_start_time_;
		};
		class Monitor {
			friend class TaskRunner;
			static void ThreadProc(shared_ptr<Monitor> monitor);
		public:
			Monitor(shared_ptr<TaskRunner> task_runner);
			shared_ptr<TaskRunner> GetTaskRunner();
			void Start(shared_ptr<thread> t);
			void Stop();
		private:
			weak_ptr<TaskRunner> task_runner_;
			shared_ptr<thread> thread_;
		};
		friend class Worker;
		friend class Monitor;
		void WorkerLoop(shared_ptr<Worker> worker);
		void MonitorLoop(shared_ptr<Monitor> monitor);
		void StartWorker();
	public:
		TaskRunner(size_t max_workers = 32);
		~TaskRunner();
		void Start(size_t size, bool no_monitor = false);
		void Stop(bool gracefully = false);
		size_t Push(shared_ptr<Task> task);
	private:
		size_t min_workers_;
		size_t max_workers_;
		std::atomic_bool stopped_;
		std::atomic_bool stopping_;
		vector<shared_ptr<Worker>> workers_;
		shared_ptr<Monitor> monitor_;
		shared_ptr<TaskQueue> tasks_;
	};
} // namespace moss

