#include <stdint.h>
#include <string>
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
#include "Trace.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Mem_Interface.hpp"
#include "Cache.hpp"
#include "Cache_L1i_Standalone.hpp"
#include "Mmu.hpp"
#include "Memory.hpp"
#include "Mem_Hierarchy.hpp"
#include "Driver.hpp"
#include "Cpu.hpp"
#include "Simulate.hpp"

using namespace std;

Config::Config()
{
	outfile = "mp_tc_sim_res.txt";
	logfile = "mp_tc_sim.log";
	l1i_standalone = true;
	struct cpu_config cpu_config = {
		.back_end = false,
		.st_fwd = false,
		.pf_pipe_width = 32,
		.fe_pipe_depth = 4,
		.fe_pipe_width = 16,
		.ls_buffer_size = 16,
		.ld_buffer_size = 16,
		.st_buffer_size = 16,
		.f_buffer_size = 16,
	};
	cpu_configs.push_back(cpu_config);
	mem_hier_config = {
		.l1i_on = true,
		.l1d_on = false,
		.l2_on = false,
		.l3_on = false,
		.l2_private = true,
		.l3_private = false
	};
	struct cache_config l1i_config = {
		.sets = 64,
		.ways = 8,
		.cl_size = 64,
		.replacement_policy = LRU,
		.hit_latency = 1,
		.hit_request_bw = -1,
		.miss_latency = 8,
		.miss_byte_bw = -1,
		.addr_bits = 48,
		.in_request_buffer_size = 16,
		.in_response_buffer_size = 16,
		.read_request_buffer_size = 16,
		.write_request_buffer_size = 16
	};
	l1i_configs.push_back(l1i_config);
	struct cache_config l1d_config = {
		.sets = 128,
		.ways = 4,
		.cl_size = 64,
		.replacement_policy = LRU,
		.hit_latency = 1,
		.addr_bits = 48,
		.in_request_buffer_size = 16,
		.in_response_buffer_size = 16,
		.read_request_buffer_size = 16,
		.write_request_buffer_size = 16
	};
	l1d_configs.push_back(l1d_config);
	struct cache_config l2_config = {
		.sets = 128,
		.ways = 4,
		.cl_size = 64,
		.replacement_policy = LRU,
		.hit_latency = 1,
		.addr_bits = 48,
		.in_request_buffer_size = 16,
		.in_response_buffer_size = 16,
		.read_request_buffer_size = 16,
		.write_request_buffer_size = 16
	};
	l2_configs.push_back(l2_config);
	mem_config = {
		.bus_speed = 8,
		.bandwidth = 8,
		.latency = 20,
		.mlp = 10
	};
	tlb_config = {
		.sets = 1,
		.ways = 32,
		.miss_latency = 100
	};
	page_table_config = {
		.pages = 16777216
	};
	prefetch_config = {
		.prefetch = true,
		.prefetch_type = BR_PREDICTOR
	};
	br_predictor_config = {
		.br_predictor = true,
		.br_pred_type = EXACT,
		.exact_references_per_miss = 128,
		.two_level_config = {
			.i = 0,
			.j = 8, 
			.k = 6,
			.s = 0,
			.init_val = 1
		},
		.btb_config = {
			.sets = 2048,
			.addr_bits = 48
		},
		.call_stack_config = {
			.size = 64
		}
	};
	debug_level = 0;
}

void Config::parse_config(ifstream &config_file)
{

}

void Config::parse_args(int argc, char *argv[])
{
	for (int i = 0; i < argc; i++) {
		if (strncmp(argv[i], "-c", 2) == 0) {
			i += 1;
		} else if (strncmp(argv[i], "-t", 2) == 0) {
			update_config(string(argv[i]), string(argv[i+1]));
		} else if (strncmp(argv[i], "-outfile", 8) == 0) {
			update_config(string(argv[i]), string(argv[i+1]));
		} else if (strncmp(argv[i], "-logfile", 8) == 0) {
			update_config(string(argv[i]), string(argv[i+1]));
		}
	}
}

res_t Config::update_config(const string &config, const string &value)
{
	if (config == "-t") {
		trace_files.push_back(value);
		return SUCCESS;
	} else if (config == "-outfile") {
		outfile = value;
		return SUCCESS;
	} else if (config == "-logfile") {
		logfile = value;
		return SUCCESS;
	}
	return FAIL;
}

void Config::print_config()
{
	for (int i = 0; i < trace_files.size(); i++) {
		log_info("-trace_files["+to_string(i)+"],"+trace_files[i]);
	}
	log_info("-outfile,"+outfile);
	log_info("-l1i_standalone,"+to_string(l1i_standalone));
	for (int i = 0; i < cpu_configs.size(); i++) {
		log_info("-cpu_configs["+to_string(i)+"].pf_pipe_width,"+to_string(cpu_configs[i].pf_pipe_width));
		log_info("-cpu_configs["+to_string(i)+"].fe_pipe_width,"+to_string(cpu_configs[i].fe_pipe_width));
		log_info("-cpu_configs["+to_string(i)+"].f_buffer_size,"+to_string(cpu_configs[i].f_buffer_size));
	}
	for (int i = 0; i < l1i_configs.size(); i++) {
		log_info("-l1i_configs["+to_string(i)+"].sets,"+to_string(l1i_configs[i].sets));
		log_info("-l1i_configs["+to_string(i)+"].ways,"+to_string(l1i_configs[i].ways));
		log_info("-l1i_configs["+to_string(i)+"].cl_size,"+to_string(l1i_configs[i].cl_size));
		log_info("-l1i_configs["+to_string(i)+"].replacement_policy,"+to_string(l1i_configs[i].replacement_policy));
		log_info("-l1i_configs["+to_string(i)+"].hit_latency,"+to_string(l1i_configs[i].hit_latency));
		log_info("-l1i_configs["+to_string(i)+"].hit_request_bw,"+to_string(l1i_configs[i].hit_request_bw));
		log_info("-l1i_configs["+to_string(i)+"].miss_latency,"+to_string(l1i_configs[i].miss_latency));
		log_info("-l1i_configs["+to_string(i)+"].miss_byte_bw,"+to_string(l1i_configs[i].miss_byte_bw));
		log_info("-l1i_configs["+to_string(i)+"].addr_bits,"+to_string(l1i_configs[i].addr_bits));
		log_info("-l1i_configs["+to_string(i)+"].in_request_buffer_size,"+to_string(l1i_configs[i].in_request_buffer_size));
		log_info("-l1i_configs["+to_string(i)+"].in_response_buffer_size,"+to_string(l1i_configs[i].in_response_buffer_size));
	}
	log_info("-prefetch_config.prefetch,"+to_string(prefetch_config.prefetch));
	log_info("-prefetch_config.prefetch_type,"+to_string(prefetch_config.prefetch_type));
	log_info("-br_predictor_config.br_predictor,"+to_string(br_predictor_config.br_predictor));
	log_info("-br_predictor_config.br_pred_type,"+to_string(br_predictor_config.br_pred_type));
	log_info("-br_predictor_config.exact_references_per_miss,"+to_string(br_predictor_config.exact_references_per_miss));
	log_info("-br_predictor_config.two_level_config.i,"+to_string(br_predictor_config.two_level_config.i));
	log_info("-br_predictor_config.two_level_config.j,"+to_string(br_predictor_config.two_level_config.j));
	log_info("-br_predictor_config.two_level_config.k,"+to_string(br_predictor_config.two_level_config.k));
	log_info("-br_predictor_config.two_level_config.s,"+to_string(br_predictor_config.two_level_config.s));
	log_info("-br_predictor_config.two_level_config.init_val,"+to_string(br_predictor_config.two_level_config.init_val));
	log_info("-br_predictor_config.btb_config.sets,"+to_string(br_predictor_config.btb_config.sets));
	log_info("-br_predictor_config.btb_config.addr_bits,"+to_string(br_predictor_config.btb_config.addr_bits));
	log_info("-br_predictor_config.call_stack_config.size,"+to_string(br_predictor_config.call_stack_config.size));
	log_info("-debug_level,"+to_string(debug_level));
}