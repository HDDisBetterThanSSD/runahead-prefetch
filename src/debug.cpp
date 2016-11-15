#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "defines.hpp"
#include "debug.hpp"

using namespace std;

static int level = 0;

ofstream outfile;
ofstream logfile;

void set_out_file(string str)
{
	outfile.open(str, ios::app);
}

void set_log_file(string str)
{
	logfile.open(str, ios::app);
}

void set_debug_level(int d_level)
{
	level = d_level;
}

void log_info(string str)
{
	outfile << steps << ": " << str << endl;
	logfile << steps << ": " << str << endl;
}

void log_debug(string str)
{
	if (level > 0)
		cout << str << endl;
}

void log_details(string str)
{
	if (level > 1)
		cout << str << endl;
}

void log_insanity(string str)
{
	if (level > 2) {
		logfile << steps << ": " << str << endl;
	}
}

string to_hex_string(uint64_t num)
{
	stringstream str;
	str << hex << num << dec;
	string res;
	str >> res;
	return res;
}