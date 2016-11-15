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
#include "Response.hpp"

using namespace std;

Response::Response(const Config &config, Request *request): config(config), request(request) { }

Response::~Response()
{
	Request::FreeRequest(request);
}

vector<Response *> Response::free_response_list;

Response *Response::NewResponse(const Config &config, Request *request)
{
	if (free_response_list.size() == 0) {
		return new Response(config, request);
	} else {
		Response *response = free_response_list.back();
		free_response_list.pop_back();
		response->request = request;
		return response;
	}
}

void Response::FreeResponse(Response *response)
{
	Request::FreeRequest(response->request);
	free_response_list.push_back(response);
}