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
#include "Ins.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Mem_Interface.hpp"
#include "Mmu.hpp"

using namespace std;

Mmu::Mmu(const Config &config, mmu_type_t mmu_type): Mem_Interface(config)
{
	this->mmu_type = mmu_type;
}

res_t Mmu::send_request(struct data_access *data_access)
{
	
	Request *request = new Request(config, "mmu", data_access->da);
	if (mmu_type == INSTRUCTION) {
		request->request_type = FETCH;
	} else if (data_access->data_type == LOAD) {
		request->request_type = DEMAND_DATA;
	} else if (data_access->data_type == STORE) {
		request->request_type = RFO;
	}
	res_t res = sub_interface->sub_send_request(request);
	if (res == FAIL) {
		delete request;
	} else {
		cache_out_request_buffer[request] = data_access;
	}
	return res;
}

struct data_access *Mmu::get_response()
{
	Response *response = sub_interface->sub_get_response();
	if (response == NULL) {
		return NULL;
	}
	log_insanity("Mmu::get_response - got response for "+to_hex_string(response->request->da));
	struct data_access *data_access = cache_out_request_buffer[response->request];
	cache_out_request_buffer.erase(response->request);
	delete response;
	return data_access;
}

res_t Mmu::step()
{
	return SUCCESS;
}

res_t Mmu::sub_send_request(Request *request)
{
	return SUCCESS;
}

Response *Mmu::sub_get_response()
{
	return NULL;
}

res_t Mmu::super_send_request(Request *request)
{
	return SUCCESS;
}

Response *Mmu::super_get_response()
{
	return NULL;
}

void Mmu::print_results()
{

}