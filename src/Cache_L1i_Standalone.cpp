#include <cmath>
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
#include "Mem_Interface.hpp"
#include "Cache.hpp"
#include "Cache_L1i_Standalone.hpp"

using namespace std;

Cache_L1i_Standalone::Cache_L1i_Standalone(const Config &config, struct cache_config cache_config, cache_type_t cache_type): Cache(config, cache_config, cache_type)
{
	miss_latency = cache_config.miss_latency;
	last_miss_cycle = 0;
	last_miss_bytes = 0;
}

bool Cache_L1i_Standalone::L2_accept()
{
	if (cache_config.miss_byte_bw != -1) { // stays the default unless there's a bw constraint
		// check if this cycle block has been used previously
		if (steps < (last_miss_cycle + (cache_config.cl_size / cache_config.miss_byte_bw))) {
			// check if this cycle block has already been fully saturated
			if ((last_miss_bytes + cache_config.cl_size) <= cache_config.miss_byte_bw) {
				// if not, add more bytes to this cycle block
				last_miss_bytes += cache_config.cl_size;
				return true;
			} else {
				// if saturated, move to next cycle block, reset cycle byte count
				return false;
			}	
		} else {
			last_miss_bytes = cache_config.cl_size;
			last_miss_cycle = steps;
			return true;
		}
	} else {
		return true;
	}
}

// int Cache_L1i_Standalone::get_miss_completion_cyce(Request *request)
// {
// 	if (super_in_request_completion.find(request->da) != super_in_request_completion.end()) {
// 		completion_cycle = super_in_request_completion[request->da];
// 		map<unsigned long, list<Request *> >::iterator it = super_in_request_buffer.find(completion_cycle);
// 		list<Request *>::iterator it2 = it->second.begin();
// 		while (it2 != it->second.end()) {
// 			if (((*it2)->da == request->da) && ((*it2)->req_obj == "pf")) {
// 				Request::FreeRequest(*it2);
// 				it->second.erase(it2++);
// 			} else {
// 				++it2;
// 			}
// 		}
// 	}
// 	return completion_cycle;
// }

res_t Cache_L1i_Standalone::step()
{
	log_insanity("Cache_L1i_Standalone::step entered");
	
	//fill on ready L2 requests
	for(int i = 0; i < super_in_miss_buffer.size(); i++) {
		map<unsigned long, list<Request *> >::iterator it = super_in_miss_buffer.begin();
		if (it->first <= steps) {
			list<Request *> requests = it->second;
			for (Request *request : requests) {
				log_insanity("sub_send_request - FILLED request "+to_hex_string(request->da)+" from "+request->req_obj);
				fill(request->da, EXCLUSIVE);
				read(request->da);
				super_in_request_completion.erase(request->da);
				if (request->req_obj == "pf") {
					Request::FreeRequest(request);
				}
			}
			super_in_miss_buffer.erase(it->first);

		} else {
			break;
		}
	}

	// handle fetch requests first
	// std::list<Request *> f_request_buffer = super_in_request_buffer["f"];
	log_insanity("Cache_L1i_Standalone::step f requests first");
	int i = 0;
	for (; (cache_config.hit_request_bw == -1) || (i < cache_config.hit_request_bw); i++) {
		Request *request = super_in_request_buffer["f"].front();
		if (request == NULL) {
			break;
		}
		uint64_t completion_cycle = steps;
		if (can_read(request->da) == FAIL) {
			if (super_in_request_completion.find(request->da) != super_in_request_completion.end()) {
				total_requests["f"] += 1;
				total_misses["f"] += 1;
				completion_cycle = super_in_request_completion[request->da];
			} else if (L2_accept()) {
				total_requests["f"] += 1;
				total_misses["f"] += 1;
				completion_cycle = steps + miss_latency;
				super_in_miss_buffer[steps + miss_latency].push_back(request);
				super_in_request_completion[request->da] = completion_cycle;
			} else {
				break;
			}
			log_insanity("sub_send_request - MISS received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(completion_cycle));
		} else {
			total_requests["f"] += 1;
			read(request->da);
			log_insanity("sub_send_request - HIT received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(steps));
		}
		super_in_request_buffer["f"].pop_front();
		Response *response = Response::NewResponse(config, request);
		response->response_res = SUCCESS;
		response->response_type = DO_NOTHING;
		response->response_time = completion_cycle;
		super_in_response_buffer[request->req_obj].push_back(response);
	}
	// now handle prefetch requests
	// f_request_buffer = super_in_request_buffer["pf"];
	while (1) {
		Request *request = super_in_request_buffer["pf"].front();
		if (request == NULL) {
			break;
		}
		if (super_in_request_completion.find(request->da) != super_in_request_completion.end()) {
			total_requests["pf"] += 1;
			super_in_request_buffer["pf"].pop_front();
			Request::FreeRequest(request);
			continue;
		} else if (can_read(request->da) == SUCCESS) {
			if ((cache_config.hit_request_bw == -1) || (i < cache_config.hit_request_bw)) {
				total_requests["pf"] += 1;
				super_in_request_buffer["pf"].pop_front();
				Request::FreeRequest(request);
				i++;
				log_insanity("sub_send_request - HIT received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(steps));
				continue;
			}
			log_insanity("sub_send_request - HIT received request "+to_hex_string(request->da)+" from "+request->req_obj+" false miss to do lack of L1i bw");
		}
		if (L2_accept()){
			total_requests["pf"] += 1;
			total_misses["pf"] += 1;
			super_in_miss_buffer[steps + miss_latency].push_back(request);
			super_in_request_completion[request->da] = steps + miss_latency;
			super_in_request_buffer["pf"].pop_front();
			log_insanity("sub_send_request - MISS received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(steps + miss_latency));
		} else {
			break;
		}
	}

	
	log_insanity("Cache_L1i_Standalone::step exited");
	return SUCCESS;
}

// res_t Cache_L1i_Standalone::step()
// {
// 	log_insanity("Cache_L1i_Standalone::step entered");
// 	for(int i = 0; i < super_in_request_buffer.size(); i++) {
// 		map<unsigned long, list<Request *> >::iterator it = super_in_request_buffer.begin();
// 		if (it->first <= steps) {
// 			list<Request *> requests = it->second;
// 			for (Request *request : requests) {
// 				if (super_in_request_completion.find(request->da) != super_in_request_completion.end()) {
// 					super_in_request_completion.erase(request->da);
// 				}
// 				if (can_read(request->da) == FAIL) {
// 					fill(request->da, EXCLUSIVE);
// 				}
// 				read(request->da);

// 				Response *response = Response::NewResponse(config, request);
// 				response->response_res = SUCCESS;
// 				response->response_type = DO_NOTHING;
// 				super_in_response_buffer[request->req_obj].push_back(response);
// 			}
// 			super_in_request_buffer.erase(it->first);
// 		} else {
// 			break;
// 		}
// 	}
// 	log_insanity("Cache_L1i_Standalone::step exited");
// 	return SUCCESS;
// }

void Cache_L1i_Standalone::add_miss_latency(int miss_latency)
{
	this->miss_latency = miss_latency;
}

res_t Cache_L1i_Standalone::sub_send_request(Request *request)
{
	if (request->req_obj == "pf") {
		if (super_in_request_completion.find(request->da) != super_in_request_completion.end()) {
			log_insanity("sub_send_request - ignored request "+to_hex_string(request->da)+" from "+request->req_obj+"due to previous request.");
			return SUCCESS;
		}
	}
	
	super_in_request_buffer[request->req_obj].push_back(request);

	if (super_in_request_buffer["pf"].size() > cache_config.in_request_buffer_size) {
		super_in_request_buffer["pf"].pop_front();
		log_insanity("sub_send_request - prefetch buffer overflow");
	}
	log_insanity("sub_send_request - received request "+to_hex_string(request->da)+" from "+request->req_obj);
	return SUCCESS;
	// if (can_read(request->da) == FAIL) {
	// 	super_in_request_completion[request->da] = steps + miss_latency;
	// }
	// super_in_request_completion[request->da] = completion_cycle;
}

// res_t Cache_L1i_Standalone::sub_send_request(Request *request)
// {
// 	// if(super_in_request_buffer.size() >= cache_config.in_request_buffer_size) {
// 	// 	return FAIL;
// 	// } else {
// 		total_requests[request->req_obj] += 1;
// 		int completion_cycle;

// 		if (can_read(request->da) == SUCCESS) {
// 			completion_cycle = steps;
// 			if (cache_config.hit_request_bw != -1) {
// 				while (super_in_request_buffer[completion_cycle].size() >= cache_config.hit_request_bw) {
// 					completion_cycle += 1;
// 				}
// 			}
// 			log_insanity("sub_send_request - HIT received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(steps));
// 			super_in_request_buffer[completion_cycle].push_back(request);
// 		} else {
// 			if (super_in_request_completion.find(request->da) != super_in_request_completion.end()) {
// 				completion_cycle = super_in_request_completion[request->da];
// 				map<unsigned long, list<Request *> >::iterator it = super_in_request_buffer.find(completion_cycle);
// 				list<Request *>::iterator it2 = it->second.begin();
// 				while (it2 != it->second.end()) {
// 					if (((*it2)->da == request->da) && ((*it2)->req_obj == "pf")) {
// 						Request::FreeRequest(*it2);
// 						it->second.erase(it2++);
// 					} else {
// 						++it2;
// 					}
// 				}
// 			} else {
// 				completion_cycle = steps + miss_latency; //by default new misses have set miss latency
// 				if (cache_config.miss_byte_bw != -1) { // stays the default unless there's a bw constraint
// 					// check if this cycle block has been used previously
// 					if (completion_cycle < (last_miss_cycle + (cache_config.cl_size / cache_config.miss_byte_bw))) {
// 						// check if this cycle block has already been fully saturated
// 						if ((last_miss_bytes + cache_config.cl_size) <= cache_config.miss_byte_bw) {
// 							// if not, add more bytes to this cycle block
// 							last_miss_bytes += cache_config.cl_size;
// 						} else {
// 							// if saturated, move to next cycle block, reset cycle byte count
// 							completion_cycle = last_miss_cycle + (cache_config.cl_size / cache_config.miss_byte_bw);
// 							last_miss_bytes = cache_config.cl_size;
// 						}	
// 					} else {
// 						last_miss_bytes = cache_config.cl_size;
// 					}
// 					last_miss_cycle = completion_cycle;
// 				}
// 				// if (cache_config.hit_request_bw != -1) {
// 				// 	while (super_in_request_buffer[completion_cycle].size() >= cache_config.hit_request_bw) {
// 				// 		completion_cycle += 1;
// 				// 	}
// 				// }
// 				super_in_request_completion[request->da] = completion_cycle;
// 			}
// 			log_insanity("sub_send_request - MISS received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(completion_cycle));
// 			super_in_request_buffer[completion_cycle].push_back(request);
// 			total_misses[request->req_obj] += 1;
// 		}

// 		return SUCCESS;

// 		// if (can_read(request->da) == SUCCESS) {
// 		// 	log_insanity("sub_send_request - HIT received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(steps));
// 		// 	super_in_request_buffer[completion_cycle].push_back(request);
// 		// } else {
// 		// 	if (super_in_request_completion.find(request->da) != super_in_request_completion.end()) {
// 		// 		super_in_request_buffer[super_in_request_completion[request->da]].push_back(request);
// 		// 		log_insanity("sub_send_request - MISS received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(super_in_request_completion[request->da]));
// 		// 	} else {
// 		// 		log_insanity("sub_send_request - MISS received request "+to_hex_string(request->da)+" from "+request->req_obj+" return in cycle "+to_string(steps + miss_latency));
// 		// 		super_in_request_buffer[steps + miss_latency].push_back(request);
// 		// 		super_in_request_completion[request->da] = steps + miss_latency;
// 		// 	}
// 		// 	total_misses[request->req_obj] += 1;
// 		// }
// 		// return SUCCESS;
// 	// }
// }

Response *Cache_L1i_Standalone::sub_get_response(string req_obj)
{
	if (super_in_response_buffer[req_obj].size() == 0) {
		return NULL;
	} else {
		Response *response = super_in_response_buffer[req_obj].front();
		if (response->response_time > steps) {
			return NULL;
		}
		super_in_response_buffer[req_obj].pop_front();
		log_insanity("sub_get_response - top response for "+req_obj+" "+to_hex_string(response->request->da)+" return in cycle "+to_string(steps));
		return response;
	}
}

void Cache_L1i_Standalone::print_results()
{
	for (map<string, unsigned long>::iterator it = total_misses.begin(); it != total_misses.end(); it++) {
		log_info("Total "+it->first+" Misses,"+to_string(it->second));
	}
	for (map<string, unsigned long>::iterator it = total_requests.begin(); it != total_requests.end(); it++) {
		log_info("Total "+it->first+" Requests,"+to_string(it->second));
	}

}