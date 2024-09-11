#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include "task.h"


using namespace std::chrono;
namespace moss {
	class TimerLoop;
	class Timer
		: public std::enable_shared_from_this<Timer> {
		friend class TimerLoop;
	public:
		Timer(const steady_clock::duration& elapse, int repeats = -1);
		Timer(shared_ptr<TimerLoop> loop, const steady_clock::duration& elapse, int repeats = -1);
		virtual ~Timer();
		int Id() const;
		int AttachTimerLoop(shared_ptr<TimerLoop> timer_loop);
		virtual int Start(shared_ptr<Task> task);
		virtual void Run();
	private:
		int timer_id_;
		int repeats_;
		std::weak_ptr<TimerLoop> loop_;
		steady_clock::duration elapse_;
		steady_clock::time_point timestamp_;
		shared_ptr<Task> task_;
	};

	class TimerLoop
		: public std::enable_shared_from_this<TimerLoop> {
		using Timers = std::unordered_map<int64_t, std::shared_ptr<Timer>>;
		std::atomic_bool stopped_;
		std::shared_ptr<std::condition_variable> cond_;
		std::shared_ptr<std::mutex> mutex_;
		std::shared_ptr<Timers> timers_;
		std::shared_ptr<std::thread> thread_;
		static void ThreadProc(shared_ptr<TimerLoop> loop);
		void MainLoop();
		Timers GetTimers();
	public:
		TimerLoop();
		~TimerLoop();
		int Start();
		void Stop();
		int StartTimer(std::shared_ptr<Timer> timer);
		int CloseTimer(std::shared_ptr<Timer> timer);
	};
} // namespace moss

