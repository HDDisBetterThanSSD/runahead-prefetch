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
#include "Request.hpp"
#include "Response.hpp"
#include "Mem_Interface.hpp"

using namespace std;

Mem_Interface::Mem_Interface(const Config &config): config(config) { }

void Mem_Interface::add_sub_interface(Mem_Interface *sub_interface)
{
	this->sub_interface = sub_interface;
}

void Mem_Interface::add_super_interface(Mem_Interface *super_interface)
{
	this->super_interface = super_interface;
}

