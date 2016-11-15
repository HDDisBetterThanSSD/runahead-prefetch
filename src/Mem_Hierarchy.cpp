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
#include "Mmu.hpp"
#include "Memory.hpp"
#include "Mem_Hierarchy.hpp"

using namespace std;

Mem_Hierarchy::Mem_Hierarchy(const Config &config): config(config)
{
	// if (config.mem_hier_config.l1i_on) {
	// 	vector<Cpu *>;
	// 	for(int i = 0;)
	// }
	
	// default setup
	if (config.l1i_standalone) {
		cache_l1i_standalone = new Cache_L1i_Standalone(config, config.l1i_configs[0], L1i);
		caches["l1i"].push_back(cache_l1i_standalone);
		memory = NULL;
	} else {
		mmus[INSTRUCTION].push_back(new Mmu(config, INSTRUCTION));
		caches["l1i"].push_back(new Cache(config, config.l1i_configs[0], L1i));
		memory = new Memory(config);

		mmus[INSTRUCTION][0]->add_sub_interface(caches["l1i"][0]);
		caches["l1i"][0]->add_super_interface(mmus[INSTRUCTION][0]);
		caches["l1i"][0]->add_sub_interface(memory);
		memory->add_super_interface(caches["l1i"][0]);
	}
}

Mmu *Mem_Hierarchy::get_cpu_mmu(mmu_type_t mmu_type, int cpu_num)
{
	if (mmus.find(mmu_type) == mmus.end()) {
		return NULL;
	}
	if (mmus[mmu_type].size() < cpu_num) {
		return NULL;
	}
	return mmus[mmu_type][cpu_num];
}

Cache_L1i_Standalone *Mem_Hierarchy::get_cpu_l1i()
{
	return cache_l1i_standalone;
}

res_t Mem_Hierarchy::step()
{
	log_insanity("Mem_Hierarchy::step entered");
	for (map<mmu_type_t, vector<Mmu *> >::iterator it = mmus.begin(); it != mmus.end(); it++) {
		for (Mmu *mmu : it->second) {
			mmu->step();
		}
	}
	for (map<string, vector<Mem_Interface *> >::iterator it = caches.begin(); it != caches.end(); it++) {
		for (Mem_Interface *mem_interface : it->second) {
			mem_interface->step();
		}
	}
	//log_insanity("Here1");
	if (memory != NULL) {
		//log_insanity("Here1");
		memory->step();
	}
	log_insanity("Mem_Hierarchy::step exited");
	return SUCCESS;
}

void Mem_Hierarchy::print_results()
{
	for (map<string, vector<Mem_Interface *> >::iterator it = caches.begin(); it != caches.end(); it++) {
		for (Mem_Interface *mem_interface : it->second) {
			mem_interface->print_results();
		}
	}
}