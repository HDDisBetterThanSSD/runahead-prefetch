#include <stdint.h>
#include <iostream>
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
#include "2level.hpp"
#include "Br_Predictor.hpp"
#include "Driver.hpp"
#include "Cpu.hpp"
#include "Cpu_Prefetch_InsOnly.hpp"
#include "Simulate.hpp"

using namespace std;

unsigned long steps = 0;

int main(int argc, char *argv[])
{
	Config config;

	//cout << hex;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-c") == 0) {
			ifstream config_file(argv[i+1]);
			config.parse_config(config_file);
		}
	}
	config.parse_args(argc, argv);

	set_debug_level(config.debug_level);
	set_out_file(config.outfile);
	set_log_file(config.logfile);

	config.print_config();

	Simulate simulation(config);
	simulation.run();

	simulation.print_results();

	return 0;
}