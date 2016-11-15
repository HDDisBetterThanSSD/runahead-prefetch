
class Br_Predictor
{
public:
	Br_Predictor(const Config &config);
	res_t predict(Ins *ins);
	void print_results(void);
private:
	const Config &config;

	bpred *branch_predictor;

	res_t predict_target(Ins *ins);
	res_t predict_dir(Ins *ins);
	res_t push_call(void);
	res_t predict_ret(void);

	std::unordered_map<uint64_t, uint64_t> btb_tag_store;
	std::unordered_map<uint64_t, uint64_t> btb_data_store;
	uint32_t btb_set_shift;
	uint64_t btb_set_mask;
	uint32_t btb_tag_shift;
	uint64_t btb_tag_mask;

	uint32_t call_stack;

	unsigned long total_exact_references;

	unsigned long total_taken_branches;
	unsigned long total_not_taken_branches;
	unsigned long total_jumps;
	unsigned long total_calls;
	unsigned long total_returns;
	unsigned long total_branch_mispredictions;
	unsigned long total_branch_dir_mispredictions;
	unsigned long total_branch_target_mispredictions;
	unsigned long total_jump_mispredictions;
	unsigned long total_call_mispredictions;
	unsigned long total_return_mispredictions;
};