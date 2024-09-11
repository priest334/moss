#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <uv.h>
#include "uv_types.h"


using std::atomic_int64_t;
using std::mutex;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
using std::weak_ptr;
namespace moss {
	class TcpServer;
	class TcpEventHandler;
	class UvWorker;
	class UvConnection;
	class TcpServerImpl
		: public std::enable_shared_from_this<TcpServerImpl> {
		friend class UvWorker;
		friend class UvConnection;
	public:
		TcpServerImpl(shared_ptr<TcpServer> server);
		~TcpServerImpl();
		shared_ptr<uv_loop_t> GetLoop() const;
		int SetupWorkers(int workers);
		void StartWorkers();
		void AcceptWorker();
		shared_ptr<UvWorker> GetWorker(int worker_id);
		int Start(const string& ip, int port, int num_of_workers = 16);
		int Stop();
		shared_ptr<TcpEventHandler> GetIoEventHandler() const;
	protected:
		shared_ptr<TcpServer> GetServer() const;
		shared_ptr<UvWorker> GetWorker() const;
			
	private:
		weak_ptr<TcpServer> server_;
		shared_ptr<uv_loop_t> loop_;
		shared_ptr<uv_tcp_t> listener_;
		shared_ptr<uv_pipe_t> ipc_;
		shared_ptr<mutex> mutex_;
		vector<shared_ptr<UvWorker>> workers_;
		int ready_workers_;
	};
} // namespace moss

