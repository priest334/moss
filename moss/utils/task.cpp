#include "task.h"
#include <iostream>


namespace moss {
	class TaskQueue {
	public:
		TaskQueue()
			: mutex_(std::make_shared<mutex>()),
			cond_(std::make_shared<condition_variable>()),
			tasks_(std::make_shared<deque<shared_ptr<Task>>>()) {
		}

		size_t Push(shared_ptr<Task> task) {
			tasks_->push_back(task);
			return tasks_->size();
		}

		void Signal(bool all) {
			if (all) {
				cond_->notify_all();
			} else {
				cond_->notify_one();
			}
		}

		size_t LockPush(shared_ptr<Task> task) {
			std::lock_guard<mutex> lock(*mutex_);
			return Push(task);
		}
		size_t LockPushSignal(shared_ptr<Task> task) {
			std::lock_guard<mutex> lock(*mutex_);
			size_t size = Push(task);
			Signal(size > 1);
			return size;
		}

		size_t GetTasks(vector<shared_ptr<Task>>& tasks, int batch_size) {
			if (batch_size<0 || batch_size > (int)tasks_->size()) {
				tasks.insert(tasks.end(), tasks_->begin(), tasks_->end());
			} else {
				tasks.insert(tasks.end(), tasks_->begin(), tasks_->begin() + batch_size);
			}
			return tasks.size();
		}

		shared_ptr<Task> GetTask() {
			if (!tasks_->empty()) {
				auto task = tasks_->front();
				tasks_->pop_front();
				return task;
			}
			return nullptr;
		}

		size_t LockAndGetTasks(vector<shared_ptr<Task>>& tasks, int batch_size) {
			std::lock_guard<mutex> lock(*mutex_);
			return GetTasks(tasks, batch_size);
		}

		shared_ptr<Task> LockGetTask() {
			std::lock_guard<mutex> lock(*mutex_);
			return GetTask();
		}

		size_t WaitTasks(vector<shared_ptr<Task>>& tasks, int batch_size, const std::chrono::milliseconds& ms) {
			std::unique_lock<mutex> lock(*mutex_);
			cond_->wait_for(lock, ms);
			return GetTasks(tasks, batch_size);
		}

		shared_ptr<Task> WaitTask(const std::chrono::milliseconds& ms) {
			std::unique_lock<mutex> lock(*mutex_);
			if (tasks_->empty()) {
				cond_->wait_for(lock, ms);
			}
			return GetTask();
		}
	private:
		shared_ptr<mutex> mutex_;
		shared_ptr<condition_variable> cond_;
		shared_ptr<deque<shared_ptr<Task>>> tasks_;
	};

	void TaskRunner::Worker::ThreadProc(shared_ptr<Worker> worker) {
		auto task_runner = worker->GetTaskRunner();
		if (task_runner) {
			task_runner->WorkerLoop(worker);
		}
	}

	TaskRunner::Worker::Worker(shared_ptr<TaskRunner> task_runner)
		: task_runner_(task_runner), busy_(false) {
	}

	TaskRunner::Worker::~Worker() {
	}

	shared_ptr<TaskRunner> TaskRunner::Worker::GetTaskRunner() {
		return task_runner_.lock();
	}

	void TaskRunner::Worker::Start(shared_ptr<thread> t) {
		stopping_ = false;
		thread_ = t;
	}

	bool TaskRunner::Worker::IsBusy() const {
		return busy_;
	}

	void TaskRunner::Worker::SetBusy() {
		busy_ = true;
		busy_start_time_ = steady_clock::now();
	}

	void TaskRunner::Worker::SetIdle() {
		busy_ = false;
	}

	void TaskRunner::Worker::Stop() {
		stopping_ = true;
	}

	bool TaskRunner::Worker::IsStopping() const {
		return stopping_.load();
	}

	void TaskRunner::Worker::Join() {
		thread_->join();
	}

	steady_clock::duration TaskRunner::Worker::BusyTime() const {
		if (!busy_) {
			return steady_clock::duration(0);
		}
		return steady_clock::now() - busy_start_time_;
	}

	void TaskRunner::Monitor::ThreadProc(shared_ptr<Monitor> monitor) {
		auto task_runner = monitor->GetTaskRunner();
		if (task_runner) {
			task_runner->MonitorLoop(monitor);
		}
	}

	TaskRunner::Monitor::Monitor(shared_ptr<TaskRunner> task_runner)
		: task_runner_(task_runner) {
	}

	shared_ptr<TaskRunner> TaskRunner::Monitor::GetTaskRunner() {
		return task_runner_.lock();
	}

	void TaskRunner::Monitor::Start(shared_ptr<thread> t) {
		thread_ = t;
	}

	void TaskRunner::Monitor::Stop() {
		thread_->join();
	}

	void TaskRunner::WorkerLoop(shared_ptr<Worker> worker) {
		while (!stopped_ && !worker->IsStopping()) {
			auto task = tasks_->WaitTask(std::chrono::milliseconds(1000));
			if (task) {
				worker->SetBusy();
				task->Run();
				worker->SetIdle();
			}
		}
		if (stopping_) {
			vector<shared_ptr<Task>> tasks;
			tasks_->LockAndGetTasks(tasks, -1);
			for (auto& task : tasks) {
				task->Run();
			}
		}
	}

	void TaskRunner::MonitorLoop(shared_ptr<Monitor> monitor) {
		bool idle = true;
		steady_clock::time_point idle_start_time = steady_clock::now();
		while (!stopped_) {
			size_t busy_workers = 0;
			for (auto& worker : workers_) {
				busy_workers += worker->IsBusy();
			}
			if (busy_workers == workers_.size()) {
				idle = false;
				StartWorker();
			} else if (busy_workers < min_workers_ && !idle) {
				idle = true;
				idle_start_time = steady_clock::now();
			}
			if (idle && steady_clock::now() - idle_start_time > std::chrono::seconds(5)) {
				size_t threads_to_stop = workers_.size() - min_workers_;
				for (auto it = workers_.begin(); threads_to_stop > 0 && it != workers_.end();) {
					auto& worker = *it;
					if (!worker->IsBusy()) {
						worker->Stop();
						tasks_->Signal(true);
						worker->Join();
						threads_to_stop--;
						it = workers_.erase(it);
					} else {
						++it;
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	void TaskRunner::StartWorker() {
		if (workers_.size() > max_workers_)
			return;
		auto t = std::make_shared<Worker>(shared_from_this());
		workers_.emplace_back(t);
		t->Start(std::make_shared<std::thread>(&Worker::ThreadProc, t));
	}


	TaskRunner::TaskRunner(size_t max_workers/* = 32*/)
		: min_workers_(0), 
		max_workers_(max_workers), 
		stopped_(true), 
		stopping_(false),
		tasks_(std::make_shared<TaskQueue>()) {
	}

	TaskRunner::~TaskRunner() {
		Stop();
	}

	void TaskRunner::Start(size_t size, bool no_monitor/* = false*/) {
		min_workers_ = size;
		stopped_ = false;
		stopping_ = false;
		for (size_t i = 0; i < min_workers_; i++) {
			StartWorker();
		}
		if (!no_monitor) {
			monitor_ = std::make_shared<Monitor>(shared_from_this());
			monitor_->Start(std::make_shared<thread>(&Monitor::ThreadProc, monitor_));
		}
	}

	size_t TaskRunner::Push(shared_ptr<Task> task) {
		if (stopped_ || stopping_)
			return -1;
		return tasks_->LockPushSignal(task);
	}

	void TaskRunner::Stop(bool gracefully/* = false*/) {
		if (stopped_)
			return;
		stopped_ = true;
		stopping_ = gracefully;
		if (monitor_) {
			monitor_->Stop();
		}
		for (auto& worker : workers_) {
			worker->Stop();
		}
	}
} // namespace moss

