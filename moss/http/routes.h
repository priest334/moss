#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
namespace moss {
	namespace http {
		class Route;
		class Routes {
			using NormalRoutes = unordered_map<string, shared_ptr<Route>>;
			using PatternRoutes = vector<shared_ptr<Route>>;
		public:
			int Install(shared_ptr<Route> route);
			shared_ptr<Route> Find(const string& method, const string& path, unordered_map<string, string>& args);
		private:
			NormalRoutes normal_routes_;
			PatternRoutes pattern_routes_;
		};
	} // namespace http
} // namespace moss


