#include "timer.h"

namespace moss {
	namespace {
		volatile int timer_id_seq = 0;
		std::once_flag default_timer_loop_initialized;
		shared_ptr<TimerLoop> default_timer_loop_ = nullptr;
		void InitializeDefaultTimerLoop() {
			std::call_once(default_timer_loop_initialized, []() {
				default_timer_loop_ = std::make_shared<TimerLoop>();
				default_timer_loop_->Start();
			});
		}
	}

	Timer::Timer(const steady_clock::duration& elapse, int repeats/* = -1*/)
		: timer_id_(++timer_id_seq),
		repeats_(repeats),
		elapse_(elapse) {
		AttachTimerLoop(nullptr);
	}

	Timer::Timer(shared_ptr<TimerLoop> loop, const steady_clock::duration& elapse, int repeats/* = -1*/)
		: timer_id_(++timer_id_seq),
		repeats_(repeats),
		elapse_(elapse) {
		AttachTimerLoop(loop);
	}

	Timer::~Timer() {
	}

	int Timer::AttachTimerLoop(shared_ptr<TimerLoop> timer_loop) {
		auto loop = loop_.lock();
		if (loop) {
			return -1;
		}
		if (!timer_loop) {
			InitializeDefaultTimerLoop();
			loop_ = default_timer_loop_;
		} else {
			loop_ = timer_loop;
		}
		return 0;
	}

	int Timer::Id() const {
		return timer_id_;
	}

	int Timer::Start(shared_ptr<Task> task) {
		timestamp_ = steady_clock::now();
		task_ = task;
		auto loop = loop_.lock();
		if (!loop || 0 == repeats_) {
			return -1;
		}
		return loop->StartTimer(shared_from_this());
	}

	void Timer::Run() {
		task_->Run();
	}


	void TimerLoop::ThreadProc(shared_ptr<TimerLoop> loop) {
		loop->MainLoop();
	}

	void TimerLoop::MainLoop() {
		stopped_ = false;
		while (!stopped_) {
			auto timers = GetTimers();
			auto now = steady_clock::now();
			for (auto it : timers) {
				auto timer = it.second;
				auto elapse = now - timer->timestamp_;
				if (elapse < timer->elapse_)
					continue;
				timer->Run();
				timer->timestamp_ = now;
				if (timer->repeats_ > 0 && --timer->repeats_ == 0) {
					CloseTimer(timer);
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	TimerLoop::Timers TimerLoop::GetTimers() {
		Timers timers;
		std::unique_lock<std::mutex> lock(*mutex_);
		if (timers_) {
			while (!stopped_ && timers_->empty()) {
				cond_->wait_for(lock, std::chrono::milliseconds(1));
			}
			timers = *timers_;
			lock.unlock();
		}
		return timers;
	}

	TimerLoop::TimerLoop()
		: stopped_(true),
		cond_(std::make_shared<std::condition_variable>()),
		mutex_(std::make_shared<std::mutex>()),
		timers_(std::make_shared<Timers>()) {
	}

	TimerLoop::~TimerLoop() {
		Stop();
	}

	int TimerLoop::Start() {
		stopped_ = false;
		thread_ = std::make_shared<std::thread>(&TimerLoop::ThreadProc, shared_from_this());
		return 0;
	}

	void TimerLoop::Stop() {
		stopped_ = true;
		if (thread_) {
			thread_->join();
			thread_.reset();
		}
	}

	int TimerLoop::StartTimer(std::shared_ptr<Timer> timer) {
		if (!timers_)
			return -1;
		std::lock_guard<std::mutex> lock(*mutex_);
		timers_->insert(std::make_pair(timer->Id(), timer));
		cond_->notify_one();
		return timer->Id();
	}

	int TimerLoop::CloseTimer(std::shared_ptr<Timer> timer) {
		int timer_id = -1;
		if (!timers_)
			return timer_id;
		std::lock_guard<std::mutex> lock(*mutex_);
		auto it = timers_->find(timer->Id());
		if (it != timers_->end()) {
			timer_id = it->second->Id();
			timers_->erase(it);
		}
		return timer_id;
	}
} // namespace moss

