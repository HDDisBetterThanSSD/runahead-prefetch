#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
#include <string>
#include "2level.hpp"

using namespace std;

#define T 1 // Taken
#define N 0 // Not Taken

// debug variable
// used to toggle debug print statements during run-time
bool d = false;

string to_binary(int n) {
	string str;
	do {
		str = to_string(n&0x1) + str;
		n >>= 1;
	} while (n != 0);
	return str;
}

	// Constructor
bpred::bpred(int i, int j, int k, int s, int init_val) {
	// keep instance copy of i, j, k, and s variables for reference
	this->i = i;
	this->j = j;
	this->k = k;
	this->s = s;

	// initialize mask values

	// condition ((j < s) && (i >= s)) makes sure Sa_ predictors are
	// only made when an _as predictor is impossible and there are enough
	// BHT per address bits to differentiate for the number of sets
	imask = ((j < s) && (i >= s)) ? ~((~0) << (i-s)) : ~((~0) << i);
	// condition (j >= s) makes sure _as predictors are made
	// only when there are enough PHT per address bits to differentiate
	// for the number of sets
	jmask = (j >= s) ? ~((~0) << (j-s)) : ~((~0) << j);
	kmask = ~((~0) << k);
	ismask = ((j < s) && (i >= s)) ? (~((~0) << s)) << 10 : 0x0;
	jsmask = (j >= s) ? (~((~0) << s)) << 10 : 0x0;
	sshift = 10;

	// initialize branch predictor parameters
	BHT_sets = ((j < s) && (i >= s)) ? pow(2.0, s) : 1;
	BHT_entries = ((j < s) && (i >= s)) ? pow(2.0, i-s) : pow(2.0, i);
	PHT_sets = (j >= s) ? pow(2.0, s) : 1;
	PHTs_per_set = (j >= s) ? pow(2.0, j-s) : pow(2.0, j);
	PHT_entries = pow(2.0, k);

	Hist_len = k; // BHSR history lengths are determined solely by k input

	// initialize 2d BHT array ([set][entry])
	BHT = new int*[BHT_sets];
	for (int n = 0; n < BHT_sets; n++) {
		BHT[n] = new int[BHT_entries];
		for (int m = 0; m < BHT_entries; m++) {
			BHT[n][m] = 0;
		}
	}

	// initialize 3d PHT array ([set][pht][entry])
	PHT = new int**[PHT_sets];
	for (int n = 0; n < PHT_sets; n++) {
		PHT[n] = new int*[PHTs_per_set];
		for (int m = 0; m < PHTs_per_set; m++) {
			PHT[n][m] = new int[PHT_entries];
			for (int l = 0; l < PHT_entries; l++) {
				PHT[n][m][l] = init_val;
			}
		}
	}

	// clear branch predictor statistics
	predictions = 0;
	incorrect = 0;

	// Report parameters of the branch predictor generated
	// to verify correct branch predictor was made
	cout << "BHT sets: " << BHT_sets << endl;
	cout << "BHSRs per set: " << BHT_entries << endl;
	cout << "BHSR history bits: " << Hist_len << endl;
	cout << "PHT sets: " << PHT_sets << endl;
	cout << "PHTs per set: " << PHTs_per_set << endl;
	cout << "entries per PHTs: " << PHT_entries << endl;
	cout << endl;
}

// Destructor
bpred::~bpred() {
	for (int a = 0; a < BHT_sets; a++) {
		delete[] BHT[a];
	}
	delete[] BHT;
	for (int a = 0; a < PHT_sets; a++) {
		for (int b = 0; b < PHTs_per_set; b++) {
			delete[] PHT[a][b];
		}
		delete[] PHT[a];
	}
	delete[] PHT;
}

/* pred: make a prediction based on current state of branch predictor
** inputs: addr - branch address
** outputs: int - either 1 or 0 for Taken or Not Taken respectively
*/
int bpred::pred(long addr) {
	predictions++; // record statistics on how many times the branch predictor made a prediction
	return (PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask] >> 1);
}

/* update: update predictor state based on actual outcome of branch
** inputs: 	addr - branch address
**			pred - either 1 or 0 for the predicted branch direction
**			actual - either 1 or 0 for the actual branch direction
** outputs: void
*/
void bpred::update(long addr, int pred, int actual) {
	if (pred != actual) {
		incorrect++;
	}
	//int jm = (addr & jmask);// + ((addr & j2mask) >> s);
	if (actual == N) {
		if (PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask] > 0)
			PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask] =
				(PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask] - 1) & 0x3;
	} else if (actual == T) {
		if (PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask] < 3)
			PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask] =
				(PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask] + 1) & 0x3;
	}
	BHT[(addr & ismask) >> sshift][addr & imask] = (BHT[(addr & ismask) >> sshift][addr & imask] << 1) | (actual & 0x1);
}

/* Below are simple print methods to access private variables for debugging purposes only */

string bpred::BHSR_string(long addr) {
	string str = to_binary(BHT[(addr & ismask) >> sshift][addr & imask] & kmask);
	return str;
}
string bpred::BHT_set(long addr) {
	return to_string((addr & ismask) >> sshift);
}
string bpred::BHT_index(long addr) {
	return to_string(addr & imask);
}
string bpred::PHT_set(long addr) {
	return to_string((addr & jsmask) >> sshift);
}
string bpred::PHT_pht(long addr) {
	int jm = (addr & jmask);// + ((addr & j2mask) >> s);
	return to_string(jm);
}
string bpred::PHT_entry(long addr) {
	int jm = (addr & jmask);// + ((addr & j2mask) >> s);
	string str = to_binary(PHT[(addr & jsmask) >> sshift][addr & jmask][BHT[(addr & ismask) >> sshift][addr & imask] & kmask]);
	return str;
}
int bpred::get_k() {
	return this->k;
}

/* simulate: runs the trace based simulation
** inputs: 	bp - reference to current branch predictor object
** 			file - reference to input file stream object to read trace
** no outputs
*/
// void simulate(bpred &bp, ifstream &file) {
// 	string line;
// 	long addr;
// 	int dir;

// 	// debug header
// 	// Address - branch address
// 	// Actual - actual branch direction
// 	// Pred - predicted branch direction
// 	// bhts - BHT set
// 	// bhti - BHT entry ptr
// 	// bhte - BHT entry
// 	// phts - PHT set
// 	// phtp - PHT pht (per address column)
// 	// phte - PHT entry
// 	(d) ? cout << "Address\t\tActual\tPred\t\tbhts\tbhti\tbhte\tphts\tphtp\tphte" << endl : (1);

// 	while (getline(file, line)) {
// 		size_t pos = 0;
// 		try {
// 			addr = stol(line.c_str(), &pos);
// 		} catch (...) {
// 			addr = stol(line.c_str(), &pos, 16);
// 		}
// 		line = line.substr(pos);
// 		dir = stol(line.c_str(), &pos);
// 		int pred = bp.pred(addr);
// 		bp.update(addr, pred, dir);

// 		// print all statistics defined in debug header
// 		(d) ? cout << addr << "\t" << dir << "\t" << pred << "\t" << ((dir == pred) ? "C" : "W") << "\t" << bp.BHT_set(addr) << "\t" << bp.BHT_index(addr) << "\t"
// 		<< bp.BHSR_string(addr) << "\t" << bp.PHT_set(addr) << "\t" << bp.PHT_pht(addr) << "\t" << bp.PHT_entry(addr) << endl : (1);
// 	}
// }

// int main(int argc, char *argv[]) {

// 	if (argc < 6) { // checks for 4 arguments <trace_file> <i> <j> <k> <s>
// 		cerr << "Incorrect number of arguments. Needs 4, not " << argc << endl;
// 		return 1;
// 	}

// 	if (argc > 6) { // optional 5th argument "-d" used for debug mode. must be last
// 		if (string(argv[6]) == "-d") {
// 			d = true;
// 		}
// 	}

// 	// prints filename and inputs arguments for doublechecking
// 	cout << argv[0] << endl;
// 	cout << "<file>: " << argv[1] << endl;
// 	cout << "<i>: " << argv[2] << endl;
// 	cout << "<j>: " << argv[3] << endl;
// 	cout << "<k>: " << argv[4] << endl;
// 	cout << "<s>: " << argv[5] << endl;
// 	cout << endl;

// 	// initialize branch predictor
// 	bpred bp(stoi(argv[2]), stoi(argv[3]), stoi(argv[4]), stoi(argv[5]), 1);
// 	// open input trace file
// 	ifstream file(argv[1]);
	
// 	// run simulation
// 	simulate(bp, file);

// 	//print results
// 	(d) ? cout << endl : (1);
// 	cout << bp.predictions << " predictions, " << bp.incorrect << " incorrect." << endl;
// 	cout << "Accuracy = " << bp.predictions - bp.incorrect << " / " << bp.predictions << " = " 
// 		<< (float)(bp.predictions - bp.incorrect)/(float)bp.predictions * 100 << '%' << endl;

// 	return 0;
// }