#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include "zlib.h"
#include "debug.hpp"
#include "defines.hpp"
#include "Config.hpp"
#include "Ins.hpp"

using namespace std;

Ins::Ins(const Config& config, uint64_t ia): config(config)
{
	this->ia = ia;
	control_type = NO_BRANCH;
}

vector<Ins *> Ins::free_ins_list;

Ins *Ins::NewIns(const Config& config, uint64_t ia)
{
	if (free_ins_list.size() == 0) {
		return new Ins(config, ia);
	} else {
		Ins *ins = free_ins_list.back();
		free_ins_list.pop_back();
		ins->ia = ia;
		ins->data_accesses.clear();
		ins->control_type = NO_BRANCH;
		ins->br_dir = NOT_TAKEN;
		ins->ta = 0;
		return ins;
	}
}

void Ins::FreeIns(Ins *ins) {
	free_ins_list.push_back(ins);
}

void Ins::set_control_type(control_type_t control_type)
{
	this->control_type = control_type;
}

void Ins::add_load_addr(uint64_t da)
{
	//struct data_access *data_access = new struct data_access(.data_type = READ, .da = da);
	struct data_access data_access = {.data_type = LOAD, .da = da};
	this->data_accesses.push_back(data_access);
}

void Ins::add_store_addr(uint64_t da)
{
	//struct data_access *data_access = new struct data_access(.data_type = WRITE, .da = da);
	struct  data_access data_access = {.data_type = STORE, .da = da};
	this->data_accesses.push_back(data_access);
}

void Ins::add_branch_dir(bool dir)
{
	this->br_dir = dir;
}

void Ins::add_branch_target(uint64_t ta)
{
	this->ta = ta;
}

string Ins::to_string()
{
	return this->ins_str;
}