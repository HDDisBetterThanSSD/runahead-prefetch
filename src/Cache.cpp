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

using namespace std;

Cache::Cache(const Config &config, struct cache_config cache_config, cache_type_t cache_type): Mem_Interface(config), cache_config(cache_config), cache_type(cache_type)
{
	set_shift = (uint32_t) log2(cache_config.cl_size);
	set_mask = (uint64_t) (cache_config.cl_size-1);
	tag_shift = (uint32_t) (log2(cache_config.sets) + set_shift);
	tag_mask = (uint64_t) (pow(2, cache_config.addr_bits - tag_shift) - 1);
	total_requests = 0;
	total_misses = 0;
}

res_t Cache::sub_send_request(Request *request)
{
	if(super_in_request_buffer.size() >= cache_config.in_request_buffer_size) {
		return FAIL;
	} else {
		super_in_request_buffer.push_back(request);
		return SUCCESS;
	}
}

// super memory interface calls this function of its sub memory interface to get responses
Response *Cache::sub_get_response()
{
	if (super_in_response_buffer.size() == 0) {
		return NULL;
	} else {
		map<unsigned long, Response *>::iterator it = super_in_response_buffer.begin();
		log_insanity("sub_get_response - top response "+to_hex_string(it->second->request->da)+" return in cycle "+to_string(it->first));
		if (it->first <= steps) {
			Response *return_response = it->second;
			super_in_response_buffer.erase(it->first);
			return return_response;
		} else {
			return NULL;
		}
	}
}

res_t Cache::super_send_request(Request *request)
{
	if(sub_in_request_buffer.size() >= cache_config.in_request_buffer_size) {
		return FAIL;
	} else {
		sub_in_request_buffer.push_back(request);
		return SUCCESS;
	}
}

Response *Cache::super_get_response()
{
	if (sub_in_response_buffer.size() == 0) {
		return NULL;
	} else {
		map<unsigned long, Response *>::iterator it = sub_in_response_buffer.begin();
		if (it->first <= steps) {
			Response *return_response = it->second;
			sub_in_response_buffer.erase(it->first);
			return return_response;
		} else {
			return NULL;
		}
	}
}

res_t Cache::step()
{
	log_insanity("Cache::step entered");
	//bool cache_accessed = false;
	//act on super responses first
	// Response *super_response = super_interface->super_get_response();
	// if (super_response != NULL) {
	// 	if (response->response_res == FAIL) {
	// 		res_t res = super_interface->super_send_request(super_response->request);
	// 	} else {
	// 		cache_action_t cache_action = act_on_response(response, super_out_request_buffer, sub_in_response_buffer);
	// 		// if (cache_action == CACHE_ACCESS) {
	// 		// 	cache_accessed = true;
	// 		// }
	// 	}
	// }

	//act on sub responses first
	Response *sub_response = sub_interface->sub_get_response();
	if (sub_response != NULL) {
		log_insanity("Cache - resonse to "+to_hex_string(sub_response->request->da)+" received");
		if (sub_response->response_res == FAIL) {
			//error
		} else {
			if ( (sub_response->request->request_type == FETCH) ||
			(sub_response->request->request_type == DEMAND_DATA) ||
			(sub_response->request->request_type == RFO) ||
			(sub_response->request->request_type == WRITE) ) {
				log_insanity("Cache - "+to_hex_string(sub_response->request->da)+" filled, can return in cycle "+to_string(sub_out_request_buffer[sub_response->request]));
				fill(sub_response->request->da, sub_response->mesi_bits);
				unsigned long completion_step = sub_out_request_buffer[sub_response->request];
				sub_out_request_buffer.erase(sub_response->request);
				super_in_response_buffer[completion_step] = sub_response;
			} else {
				//error
			}
		}
	}

	// //response to sub requests first
	// Request *request = sub_in_request_buffer.front();
	// if (request->request_type == WRITE) {
	// 	if (can_write(request->da->da)) {
	// 		write(request->da->da);
	// 	} else {
	// 		Request *request = new Request
	// 	}
	// }

	Request *super_request = super_in_request_buffer.front();
	if (super_request != NULL) {
		total_requests++;
		if (can_read(super_request->da) == SUCCESS) {
			log_insanity("Cache - request "+to_hex_string(super_request->da)+" hit, will return in cycle "+to_string(steps+cache_config.hit_latency));
			read(super_request->da);
			Response *super_response = new Response(config, super_request);
			super_response->response_res = SUCCESS;
			super_response->response_type = FILL;
			super_in_response_buffer[steps+cache_config.hit_latency] = super_response;
			super_in_request_buffer.pop_front();
		} else {
			log_insanity("Cache - request "+to_hex_string(super_request->da)+" missed, can return in cycle "+to_string(steps+cache_config.hit_latency));
			total_misses++;
			if (sub_interface->sub_send_request(super_request) == SUCCESS) {
				sub_out_request_buffer[super_request] = steps+cache_config.hit_latency;
				super_in_request_buffer.pop_front();
			}
		}
	}
	log_insanity("Cache::step exited");
	return SUCCESS;
}

// cache_action_t Cache::act_on_response(
// 	Response *response,
// 	const unordered_set<Request *> &out_request_buffer,
// 	const map<unsigned long, Response *> &in_response_buffer)
// {
// 	if (response == NULL) {
// 		return NOTHING;
// 	}
// 	if (response->response_res == FAIL) {

// 	}
// 	Request *parent_request = out_request_buffer[response->request];
// 	if ()
// 	if (parent_request->request_type == )
// }

uint64_t Cache::get_set_idx(uint64_t addr)
{
	return ((addr >> set_shift) & set_mask);
}

uint64_t Cache::get_tag(uint64_t addr)
{
	return ((addr >> tag_shift) & tag_mask);
}

res_t Cache::fill(uint64_t addr, mesi_t mesi_bits)
{
	uint64_t set_idx = get_set_idx(addr);
	struct cache_index &set = tag_store[set_idx];
	if (set.replacement_order.size() >= cache_config.ways) {
		uint64_t wb_tag = set.replacement_order.front();
		if (set.ways[wb_tag].dirty_bit == true) {
			//TODO: deal with writeback
		}
		// Request *request = new Request(config, addr);
		// request->request_type = WRITE;
		// request->tag = 0;
		// write_request_buffer.push_back(request);
		set.replacement_order.pop_front();
		set.ways.erase(wb_tag);
	}
	uint64_t tag = get_tag(addr);
	struct cache_way new_way = {
			.dirty_bit = false,
			.mesi_bits = mesi_bits
		};
	set.ways[tag] = new_way;
	set.replacement_order.push_back(tag);
	return SUCCESS;
}

res_t Cache::read(uint64_t addr)
{
	// Can Read
	uint64_t set_idx = get_set_idx(addr);
	if (tag_store.find(set_idx) == tag_store.end()) {
		return FAIL;
	}
	uint64_t tag = get_tag(addr);
	struct cache_index &set = tag_store[set_idx];
	if (set.ways.find(tag) == set.ways.end()) {
		return FAIL;
	}

	//do the read
	set.replacement_order.remove(tag);
	set.replacement_order.push_back(tag);
	return SUCCESS;
}

res_t Cache::can_read(uint64_t addr)
{
	uint64_t set_idx = get_set_idx(addr);
	if (tag_store.find(set_idx) == tag_store.end()) {
		return FAIL;
	}
	uint64_t tag = get_tag(addr);
	struct cache_index &set = tag_store[set_idx];
	if (set.ways.find(tag) == set.ways.end()) {
		return FAIL;
	}
	return SUCCESS;
}

res_t Cache::write(uint64_t addr)
{
	//Can Write
	uint64_t set_idx = get_set_idx(addr);
	if (tag_store.find(set_idx) == tag_store.end()) {
		return FAIL;
	}
	uint64_t tag = get_tag(addr);
	struct cache_index &set = tag_store[set_idx];
	if (set.ways.find(tag) == set.ways.end()) {
		return FAIL;
	}
	struct cache_way &way = set.ways[tag];
	if ((way.mesi_bits != MODIFIED) || (way.mesi_bits != EXCLUSIVE)) {
		return FAIL;
	}

	//Do the writing
	way.mesi_bits = MODIFIED;
	way.dirty_bit = true;
	return SUCCESS;
}

res_t Cache::can_write(uint64_t addr)
{
	uint64_t set_idx = get_set_idx(addr);
	if (tag_store.find(set_idx) == tag_store.end()) {
		return FAIL;
	}
	uint64_t tag = get_tag(addr);
	struct cache_index &set = tag_store[set_idx];
	if (set.ways.find(tag) == set.ways.end()) {
		return FAIL;
	}
	struct cache_way &way = set.ways[tag];
	if ((way.mesi_bits != MODIFIED) || (way.mesi_bits != EXCLUSIVE)) {
		return FAIL;
	}
	return SUCCESS;
}


// if exclusive respond independently
// if inclusive pass up and change exclusive
res_t Cache::invalidate(bool writeback)
{
	return SUCCESS;
}

void Cache::print_results()
{
	log_info("Total Misses: "+to_string(total_misses));
	log_info("Total Requests: "+to_string(total_requests));
}