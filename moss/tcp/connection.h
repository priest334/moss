#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
using std::weak_ptr;
using std::mutex;
namespace moss {
	using connection_id_t = int64_t;
	class Connection {
	public:
		MOSS_EXPORT Connection(int64_t id);
		MOSS_EXPORT virtual ~Connection();
		MOSS_EXPORT int64_t Id() const;
		MOSS_EXPORT void SetUserContext(shared_ptr<void> user_context);
		MOSS_EXPORT shared_ptr<void> UserContext();
		MOSS_EXPORT shared_ptr<void> UserContext() const;
		MOSS_EXPORT virtual int Write(shared_ptr<string> wrbuf) = 0;
		MOSS_EXPORT virtual int Close() = 0;
		MOSS_EXPORT virtual string Ip() const = 0;
	private:
		int64_t id_;
		weak_ptr<void> user_context_;
	};
} // namespace moss

