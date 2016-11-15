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
#include "Driver.hpp"
#include "Cpu.hpp"

using namespace std;

Cpu::Cpu(const Config &config, int cpu_num): Driver(config, cpu_num)
{
	log_insanity("Cpu::Cpu entered");
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

	gzFile infile = gzopen(config.trace_files[cpu_num].c_str(), "r");
	trace = new Trace(config, infile);
	log_insanity("Cpu::Cpu exited");
}

res_t Cpu::step()
{
	// Ins *ins = trace->next_ins();
	// if (ins == NULL)
	// 	return FAIL;
	// log_insanity("specal: "+to_hex_string(ins->ia));
	// return SUCCESS;
	log_insanity("Cpu::step entered");

	if (config.cpu_configs[cpu_num].back_end == false) {
		vector<Ins *>fe_pipe_end = fe_pipe.front();
		fe_pipe.pop_front();
		for (int i = 0; i < fe_pipe_end.size(); i++) {
			delete fe_pipe_end[i];
			retire_ins_count++;
		}
	}
	bool be_stall = false;

	if (!be_stall) {
		if (steps == 0) {
			fe_pipe.push_back(vector<Ins*>());
			for(int i = 0; i < fe_pipe_width; i++) {
				Ins *ins = trace->next_ins();
				log_insanity("CPU - fetched "+to_hex_string(ins->ia));
				if (ins == NULL) return FAIL;
				fetch_block.push_back(ins);
				if (ins->control_type != NO_BRANCH) break;
			}
			struct data_access *fa = new data_access();
			fa->data_type = LOAD;
			fa->da = fetch_block.front()->ia;
			fa->tag = 0;
			if (l1i_mmu->send_request(fa) == FAIL) {
				return SUCCESS;
			}
		} else {
			struct data_access *rda = l1i_mmu->get_response();
			if (rda == NULL) {
				fe_pipe.push_back(vector<Ins*>());
			} else if (rda->da != fetch_block.front()->ia) {
				log_insanity("Cpu::step - fetch return address "+to_hex_string(rda->da)+" does not match request address "+to_hex_string(fetch_block.front()->ia));
				fe_pipe.push_back(vector<Ins*>());
			} else {
				fe_pipe.push_back(fetch_block);
				fetch_block.clear();
				for(int i = 0; i < fe_pipe_width; i++) {
					Ins *ins = trace->next_ins();
					log_insanity("CPU - fetched "+to_hex_string(ins->ia));
					if (ins == NULL) return FAIL;
					fetch_block.push_back(ins);
					if (ins->control_type != NO_BRANCH) break;
				}
				struct data_access *fa = new data_access();
				fa->data_type = LOAD;
				fa->da = fetch_block.front()->ia;
				fa->tag = 0;
				if (l1i_mmu->send_request(fa) == FAIL) {
					return SUCCESS;
				}
			}
		}
	}
	log_insanity("CPU::step exited");
	return SUCCESS;

}

res_t Cpu::add_l1i_mmu(Mmu *l1i_mmu)
{
	this->l1i_mmu = l1i_mmu;
	return SUCCESS;
}

res_t Cpu::add_l1d_mmu(Mmu *l1d_mmu)
{
	this->l1d_mmu = l1d_mmu;
	return SUCCESS;
}
