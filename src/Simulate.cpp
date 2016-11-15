#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>
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
#include "2level.hpp"
#include "Br_Predictor.hpp"
#include "Driver.hpp"
#include "Cpu.hpp"
#include "Cpu_Prefetch_InsOnly.hpp"
#include "Simulate.hpp"

using namespace std;

Simulate::Simulate(const Config &config): config(config)
{
	set_mem_hierarchy(new Mem_Hierarchy(config));
	for(int i = 0; i < config.cpu_configs.size(); i++) {
		if (config.l1i_standalone) {
			Cpu_Prefetch_InsOnly *new_cpu = new Cpu_Prefetch_InsOnly(config, i);
			new_cpu->add_l1i(mem_hierarchy->get_cpu_l1i());
			add_driver(new_cpu);
		} else {
			Cpu *new_cpu = new Cpu(config, i);
			new_cpu->add_l1i_mmu(mem_hierarchy->get_cpu_mmu(INSTRUCTION, i));
			new_cpu->add_l1d_mmu(mem_hierarchy->get_cpu_mmu(DATA, i));
			add_driver(new_cpu);
		}
	}
	log_insanity("Simulate::Simulate exited");
}

void Simulate::run()
{
	log_insanity("Simulate::run entered");
	int num_fail;
	//for (steps = 0; steps < 1000000; steps++) {
	do {
		num_fail = 0;
		for (Driver *driver : drivers) {
			res_t res = driver->step();
			if (res == FAIL)
				num_fail++;
		}
		if (num_fail >= drivers.size()) break;
		mem_hierarchy->step();
		steps += 1;
		if ((steps % 10000000) == 0) {
			cout << steps << " cycles simulated; " << drivers[0]->get_retire_ins_count() << " instructions executed" << endl;
		}
		//log_insanity("here");
	//}
	} while (num_fail < drivers.size());
}

void Simulate::add_driver(Driver *driver)
{
	drivers.push_back(driver);
}


void Simulate::set_mem_hierarchy(Mem_Hierarchy *mem_hierarchy)
{
	this->mem_hierarchy = mem_hierarchy;
}

void Simulate::print_results()
{
	for (Driver *driver : drivers) {
		driver->print_results();
	}
	mem_hierarchy->print_results();
}