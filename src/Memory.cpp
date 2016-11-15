#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <fstream>
#include "zlib.h"
#include "defines.hpp"
#include "debug.hpp"
#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Mem_Interface.hpp"
#include "Memory.hpp"

using namespace std;

Memory::Memory(const Config &config): Mem_Interface(config)
{
	bus_last_use_step = 0;
}

res_t Memory::step()
{
	log_insanity("Memory::step entered");
	if ((super_in_request_buffer.size() > 0) && (super_in_response_buffer.size() < config.mem_config.mlp)) {
		Response *response = new Response(config, super_in_request_buffer.front());
		super_in_request_buffer.pop_front();
		super_in_response_buffer[steps+config.mem_config.latency] = response;
		log_insanity("Memory - received "+to_hex_string(response->request->da)+" can return on cycle "+to_string(steps+config.mem_config.latency));
	}
	log_insanity("Memory::step exited");
	return SUCCESS;
}

res_t Memory::sub_send_request(Request *request)
{
	if ((bus_last_use_step + config.mem_config.bus_speed) > steps) {
		log_insanity("Memory::sub_send_request - request sent too early, ready at "+to_string(bus_last_use_step+config.mem_config.bus_speed));
		return FAIL;
	} else if(super_in_request_buffer.size() >= config.mem_config.mlp) {
		return FAIL;
	} else {
		super_in_request_buffer.push_back(request);
		bus_last_use_step = steps;
		return SUCCESS;
	}
}

Response *Memory::sub_get_response()
{
	if (super_in_response_buffer.size() == 0) {
		return NULL;
	} else {
		map<unsigned long, Response *>::iterator it = super_in_response_buffer.begin();
		if (it->first <= steps) {
			Response *return_response = it->second;
			super_in_response_buffer.erase(it->first);
			return return_response;
		} else {
			return NULL;
		}
	}
}

res_t Memory::super_send_request(Request *request)
{
	return SUCCESS;
}

Response *Memory::super_get_response()
{
	return NULL;
}

void Memory::print_results()
{

}