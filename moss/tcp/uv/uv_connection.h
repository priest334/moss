#pragma once

#include <deque>
#include <vector>
#include <uv.h>
#include "../connection.h"


using std::deque;
using std::vector;
namespace moss {
	class TcpEventHandler;
	class UvWorker;
	class TcpServerImpl;
	class UvConnection
		: public Connection,
		public std::enable_shared_from_this<UvConnection> {
		using WriteQueue = deque<shared_ptr<string>>;
		friend class UvWorker;
	public:
		UvConnection(int64_t id, shared_ptr<uv_tcp_t> handle, shared_ptr<UvWorker> worker);
		~UvConnection();
		shared_ptr<UvWorker> GetWorker() const;
		shared_ptr<TcpEventHandler> GetIoEventHandler() const;
		shared_ptr<UvConnection> SharedFromPodPointer() const;
		shared_ptr<TcpServerImpl> GetUvTcpServer() const;
		uv_tcp_t* Handle();
		shared_ptr<string> ReadBuffer();
		shared_ptr<string> WriteBuffer();
		void WriteFinished();
		void Cleanup();
		void Start();

		int Write(shared_ptr<string> wrbuf) override;
		int Close() override;
		string Ip() const override;
	private:
		int Write();
		string GetIp() const;
		weak_ptr<UvWorker> worker_;
		shared_ptr<uv_tcp_t> handle_;
		shared_ptr<string> ip_;
		shared_ptr<string> rdbuf_;
		shared_ptr<mutex> mutex_;
		shared_ptr<WriteQueue> wq_;
		shared_ptr<WriteQueue> wqs_;
	};
} // namespace moss


