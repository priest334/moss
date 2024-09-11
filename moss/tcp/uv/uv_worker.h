#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <uv.h>
#include "uv_types.h"


using std::atomic_int64_t;
using std::mutex;
using std::shared_ptr;
using std::string;
using std::thread;
using std::unordered_map;
using std::vector;
using std::weak_ptr;
namespace moss {
	class TcpEventHandler;
	class TcpServerImpl;
	class UvConnection;
	class UvWorker
		: public std::enable_shared_from_this<UvWorker> {
		friend class TcpServerImpl;
		using WriteJobs = std::unordered_map<int64_t, int64_t>;
	public:
		UvWorker(worker_id_t id, shared_ptr<TcpServerImpl> server);
		worker_id_t Id() const;
		shared_ptr<TcpServerImpl> GetServer() const;
		shared_ptr<TcpEventHandler> GetIoEventHandler() const;
		shared_ptr<UvWorker> SharedFromPodPointer() const;
		shared_ptr<uv_loop_t> GetLoop() const;
		shared_ptr<uv_tcp_t> Listener() const;
		shared_ptr<uv_tcp_t> Listener();
		shared_ptr<uv_pipe_t> Pipe() const;
		shared_ptr<string> Scratch() const;
		shared_ptr<string> Scratch(size_t size);
		void Setup();
		void Start();
		void Listen();
		void Stop();
		void Run();

		shared_ptr<UvConnection> CreateConnection(shared_ptr<uv_tcp_t> handle);
		shared_ptr<UvConnection> GetConnection(int64_t id) const;
		void CloseConnection(int64_t id);
		void Accept();
		void Write(int64_t connection_id);
		void Write();
	private:
		worker_id_t id_;
		weak_ptr<TcpServerImpl> server_;
		shared_ptr<uv_loop_t> loop_;
		shared_ptr<uv_async_t> async_;
		shared_ptr<thread> thread_;
		shared_ptr<mutex> mutex_;
		shared_ptr<WriteJobs> jobs_;
		shared_ptr<uv_tcp_t> listener_;
		shared_ptr<uv_sem_t> semaphore_;
		shared_ptr<uv_pipe_t> pipe_;
		shared_ptr<string> scratch_;
		shared_ptr<mutex> connections_mutex_;
		atomic_int64_t connection_id_;
		unordered_map<int64_t, shared_ptr<UvConnection>> connections_;
	};
} // namespace moss


