#include "routes.h"

#include "route.h"


namespace moss {
	namespace http {
		int Routes::Install(shared_ptr<Route> route) {
			string path = route->Path();
			if (path.empty())
				return -1;
			if (path.at(0) == '~') {
				pattern_routes_.push_back(route);
				return 0;
			}
			auto it = normal_routes_.find(route->Path());
			if (it != normal_routes_.end()) {
				it->second->Append(route);
			} else {
				normal_routes_.insert(std::make_pair(route->Path(), route));
			}
			return 0;
		}

		shared_ptr<Route> Routes::Find(const string& method, const string& path, unordered_map<string, string>& args) {
			auto it = normal_routes_.find(path);
			if (it != normal_routes_.end()) {
				if (it->second->MatchMethod(method)) {
					return it->second;
				}
				auto route = it->second->Match(method);
				if (route) {
					return route;
				}
			}
			for (auto it = pattern_routes_.begin(); it != pattern_routes_.end(); ++it) {
				string route_path = (*it)->Path();
				string pattern = route_path.substr(1);
				if ((*it)->Match(method, pattern, path, args)) {
					return *it;
				}
			}
			return nullptr;
		}
	} // namespace http
} // namespace moss

