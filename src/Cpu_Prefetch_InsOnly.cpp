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
#include "Ins.hpp"
#include "Trace.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Mem_Interface.hpp"
#include "Mmu.hpp"
#include "Cache.hpp"
#include "Cache_L1i_Standalone.hpp"
#include "2level.hpp"
#include "Br_Predictor.hpp"
#include "Driver.hpp"
#include "Cpu_Prefetch_InsOnly.hpp"

using namespace std;

Cpu_Prefetch_InsOnly::Cpu_Prefetch_InsOnly(const Config &config, int cpu_num): Driver(config, cpu_num)
{
	log_insanity("Cpu_Prefetch_InsOnly::Cpu_Prefetch_InsOnly entered");
	fe_pipe_depth = config.cpu_configs[cpu_num].fe_pipe_depth;
	fe_pipe_width = config.cpu_configs[cpu_num].fe_pipe_width;
	if (config.cpu_configs[cpu_num].back_end) {
		ls_buffer_size = config.cpu_configs[cpu_num].ls_buffer_size;
		ld_buffer_size = config.cpu_configs[cpu_num].ld_buffer_size;
		st_buffer_size = config.cpu_configs[cpu_num].st_buffer_size;
	} else {
		ls_buffer_size = 0;
		ld_buffer_size = 0;
		st_buffer_size = 0;
	}
	for (int i = 0; i < fe_pipe_depth; i++) {
		fe_pipe.push_back(vector<Ins *>());
	}

	retire_ins_count = 0;
	total_stalls = 0;
	ifetch_stalls = 0;

	gzFile infile = gzopen(config.trace_files[cpu_num].c_str(), "r");
	trace = new Trace(config, infile);
	log_insanity("Cpu_Prefetch_InsOnly::Cpu_Prefetch_InsOnly exited");

	pf_pipe_width = config.cpu_configs[cpu_num].pf_pipe_width;
	prev_prefetch_cl = 0;
	prev_fetch_cb = 0;
	fetch_per_block = 0;
	br_predictor = new Br_Predictor(config);
	f_buffer_size = config.cpu_configs[cpu_num].f_buffer_size;

	cl_mask = ~(config.l1i_configs[0].cl_size - 1);
	log_insanity("cl_mask "+to_hex_string(cl_mask));
}



res_t Cpu_Prefetch_InsOnly::step()
{
	log_insanity("Cpu_Prefetch_InsOnly::step entered");
	bool trace_over = false;

	//stall if prev fetch failed.
	while (fetch_per_block > 0) {
		Response *f_response = l1i->sub_get_response("f");
		if (f_response == NULL) {
			break;
		} else if (f_response->response_res == FAIL) {
			log_insanity("CPU - fetch failed");
		} else {
			fetch_per_block -= 1;
			log_insanity("CPU - fetch succeeded "+to_hex_string(f_response->request->da)+" fetches left: "+to_string(fetch_per_block));
		}
		delete f_response;
	}

	if (fetch_per_block > 0) {
		ifetch_stalls += 1;
	}

	//fetch if not stalled
	if ((fetch_block.size() == 0) || (fetch_per_block <= 0)) {
		fetch_per_block = 0;

		retire_ins_count += fetch_block.size();
		if (retire_ins_count >= 350000000) {
			return FAIL;
		}
		for (Ins *fetch_block_ins : fetch_block) {
			Ins::FreeIns(fetch_block_ins);
		}
		fetch_block.clear();
		log_insanity("CPU - "+to_string(retire_ins_count)+" instructions retired");
		bool start_f_loop = true;
		uint64_t f_block_start = 0;
		while (1) {
			while ((fetch_buffer.size() > 0) && (fetch_buffer.front().size() == 0)) {
				fetch_buffer.pop_front();
			}
			if (fetch_buffer.size() == 0) {
				break;
			}
			Ins *ins = fetch_buffer.front().front();
			log_insanity("CPU - fetched "+ins->to_string());
			fetch_buffer.front().pop_front();
			fetch_block.push_back(ins);

			//uint64_t ins_cla = ins->ia & ~(15);
			uint64_t ins_cla = ins->ia & cl_mask;
			if (ins_cla != prev_fetch_cb) {
				Request *pf_request = Request::NewRequest(config, "f", (ins->ia & cl_mask));
				pf_request->request_type = FETCH;
				l1i->sub_send_request(pf_request);

				fetch_per_block += 1;
				prev_fetch_cb = ins_cla;
			}
			if (start_f_loop) {
				f_block_start = ins->ia;
				start_f_loop = false;
			}
			if ( !((ins->control_type == NO_BRANCH) || (ins->control_type == BRANCH && ins->br_dir == NOT_TAKEN)) ) {
				break;
			}
			if ((ins->ia - f_block_start) > fe_pipe_width) {
				break;
			}
		}
		log_insanity("CPU - fetch block created with "+to_string(fetch_per_block)+" outstanding requests");
	} else {
		total_stalls += 1;
	}

	// remove unused pf responses
	Response *pf_response;
	pf_response = l1i->sub_get_response("pf");
	while (pf_response != NULL) {
		Response::FreeResponse(pf_response);
		pf_response = l1i->sub_get_response("pf");
	}

	// create prefetch block and prefetch
	if ((last_misp == false) || ((fetch_buffer.size() == 0) && (fetch_block.size() == 0))) {
		last_misp = false;
		if (fetch_buffer.size() < f_buffer_size) {
			bool start_pf_loop = true;
			uint64_t pf_block_start = 0;
			list<Ins *> prefetch_block;
			while (1) {
				Ins *ins = trace->next_ins();
				if (ins == NULL) {
					trace_over = true;
					break;
				}
				log_insanity("CPU - prefetched "+ins->to_string());
				uint64_t ins_cla = ins->ia & cl_mask;
				if ((config.prefetch_config.prefetch) && (ins_cla != prev_prefetch_cl)) {
					Request *pf_request = Request::NewRequest(config, "pf", ins_cla);
					pf_request->request_type = FETCH;
					l1i->sub_send_request(pf_request);

					prev_prefetch_cl = ins_cla;
				}
				prefetch_block.push_back(ins);
				if (start_pf_loop) {
					pf_block_start = ins->ia;
					log_insanity("ins->ia "+to_hex_string(ins->ia)+" pf_block start "+to_hex_string(pf_block_start));
					start_pf_loop = false;
				}
				if (ins->control_type != NO_BRANCH) {
					res_t predict_res = br_predictor->predict(ins);
					if (predict_res == SUCCESS) {
						if (ins->br_dir == TAKEN) {
							log_insanity("CPU - predicted taken control instruction "+ins->to_string());
							if (config.prefetch_config.prefetch && (config.prefetch_config.prefetch_type == BR_PREDICTOR) && ((ins->ta & cl_mask) != prev_prefetch_cl)) {
								Request *pf_request = Request::NewRequest(config, "pf", (ins->ta & cl_mask));
								pf_request->request_type = FETCH;
								l1i->sub_send_request(pf_request);
							}

							prev_prefetch_cl = (ins->ta & cl_mask);
						}
					} else {
						last_misp = true;
					}
					if ( !((ins->control_type == BRANCH) && (ins->br_dir == NOT_TAKEN)) || (predict_res == FAIL) ) {
						log_insanity("CPU - mispredicted not-taken or taken control instruction "+ins->to_string()+"terminate prefetch block");
						break;
					}
				}
				if ((ins->ia - pf_block_start) > pf_pipe_width) {
					break;
				}
			}
			if (prefetch_block.front() != NULL) {
				log_insanity("ins->ia "+to_hex_string(prefetch_block.front()->ia)+" pf_block finished "+to_hex_string(pf_block_start));
			}
			fetch_buffer.push_back(prefetch_block);
		}
	}

	
	log_insanity("Cpu_Prefetch_InsOnly::step exited");
	if ((trace_over) && (fetch_buffer.size() == 0) && (fetch_block.size() == 0)) {
		return FAIL;
	} else {
		return SUCCESS;
	}
}

res_t Cpu_Prefetch_InsOnly::add_l1i(Cache_L1i_Standalone *l1i)
{
	this->l1i = l1i;
	return SUCCESS;
}

void Cpu_Prefetch_InsOnly::print_results()
{
	log_info("Total Instructions,"+to_string(retire_ins_count));
	log_info("Total Stalls,"+to_string(total_stalls));
	log_info("Instruction Fetch Stalls,"+to_string(ifetch_stalls));
	br_predictor->print_results();
}