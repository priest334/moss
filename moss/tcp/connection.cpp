#include "connection.h"

#include <thread>
#include <ctime>


namespace moss {
	Connection::Connection(int64_t id)
		: id_(id) {
	}

	Connection::~Connection() {
	}

	int64_t Connection::Id() const {
		return id_;
	}

	void Connection::SetUserContext(shared_ptr<void> user_context) {
		user_context_ = user_context;
	}

	shared_ptr<void> Connection::UserContext() {
		return user_context_.lock();
	}

	shared_ptr<void> Connection::UserContext() const {
		return user_context_.lock();
	}
} // namespace moss

