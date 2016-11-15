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
#include "Trace.hpp"

using namespace std;

Trace::Trace(const Config &config, gzFile infile): config(config)
{
	char char_line[500];
	this->infile = infile;
	gzgets(infile, char_line, 500);
	gzgets(infile, char_line, 500);
	temp_ins = parse_ins();
}

Ins *Trace::next_ins(void)
{
	Ins *return_ins = temp_ins;
	temp_ins = parse_ins();
	if ((temp_ins == NULL) || (return_ins == NULL)) return NULL;
	return_ins->add_branch_target(temp_ins->ia);

	return return_ins;
}

Ins *Trace::parse_ins(void)
{
	char char_line[500];
	uint64_t ia;
	char c_qual[10];

	if (gzgets(infile, char_line, 500) == NULL) return NULL;
	string string_line(char_line);
	string_line.erase(remove(string_line.begin(), string_line.end(), '\n'), string_line.end());
	//log_insanity(char_line);
	istringstream line(string_line);

	line >> hex >> ia;

	Ins *ins = Ins::NewIns(config, ia);

	ins->ins_str = string_line;

	//line.getline(c_qual, 10, ' ');
	//string qual(c_qual);
	string qual;
	while (line >> qual) {
		if (qual == "T") {
			ins->set_control_type(BRANCH);
			ins->add_branch_dir(TAKEN);
		} else if (qual == "N") {
			ins->set_control_type(BRANCH);
			ins->add_branch_dir(NOT_TAKEN);
		} else if (qual == "J") {
			ins->set_control_type(JUMP);
			ins->add_branch_dir(TAKEN);
		} else if (qual == "A") { //Call
			ins->set_control_type(CALL);
			ins->add_branch_dir(TAKEN);
		} else if (qual == "E") { //Return
			ins->set_control_type(RET);
			ins->add_branch_dir(TAKEN);
		} else if (qual == "R" || qual == "R2") {
			uint64_t ra;
			line >> ra;
			ins->add_load_addr(ra);
		} else if (qual == "W") {
			uint64_t wa;
			line >> wa;
			ins->add_store_addr(wa);
		}
		//log_insanity("here "+qual);
	}

	return ins;
}