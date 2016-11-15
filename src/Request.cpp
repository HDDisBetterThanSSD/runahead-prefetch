#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <fstream>
#include "zlib.h"
#include "debug.hpp"
#include "defines.hpp"
#include "Config.hpp"
#include "Request.hpp"

using namespace std;

Request::Request(const Config &config, string req_obj, uint64_t da): config(config), req_obj(req_obj), da(da) { }

vector<Request *> Request::free_request_list;

Request *Request::NewRequest(const Config& config, std::string req_obj, uint64_t da)
{
	if (free_request_list.size() == 0) {
		return new Request(config, req_obj, da);
	} else {
		Request *request = free_request_list.back();
		free_request_list.pop_back();
		request->da = da;
		request->req_obj = req_obj;
		request->request_type = FETCH;
		request->request_size = 0;
		return request;
	}
}

void Request::FreeRequest(Request *request)
{
	free_request_list.push_back(request);
}