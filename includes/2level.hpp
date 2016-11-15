
/* pbred: A branch Predictor class
** when instantiated, it will model
** any branch predictor based on the
** i, j, k, and s inputs
*/
class bpred {

public:
	bpred(int i, int j, int k, int s, int init_val);
	~bpred();

	/* pred: make a prediction based on current state of branch predictor
	** inputs: addr - branch address
	** outputs: int - either 1 or 0 for Taken or Not Taken respectively
	*/
	int pred(long addr);

	/* update: update predictor state based on actual outcome of branch
	** inputs: 	addr - branch address
	**			pred - either 1 or 0 for the predicted branch direction
	**			actual - either 1 or 0 for the actual branch direction
	** outputs: void
	*/
	void update(long addr, int pred, int actual);

	std::string BHSR_string(long addr);
	std::string BHT_set(long addr);
	std::string BHT_index(long addr);
	std::string PHT_set(long addr);
	std::string PHT_pht(long addr);
	std::string PHT_entry(long addr);
	int get_k();

	// statistics
	int predictions;
	int incorrect;
	
private:
	// variables related to BHT table
	int **BHT;
	int BHT_sets;
	int BHT_entries;
	int Hist_len;

	// variables related to PHT table
	int ***PHT;
	int PHT_sets;
	int PHTs_per_set;
	int PHT_entries;
	

	int i;
	int j;
	int k;
	int s;

	// masks and shifts that help define what
	// bit range of address should be used for indexing
	long imask;
	long jmask;
	long kmask;
	long ismask;
	long jsmask;
	int sshift;

};