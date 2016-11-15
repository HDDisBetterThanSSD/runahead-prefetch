#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <fstream>
#include "zlib.h"
#include "defines.hpp"
#include "debug.hpp"
#include "Config.hpp"
#include "Ins.hpp"
#include "Trace.hpp"
#include "2level.hpp"
#include "Br_Predictor.hpp"

using namespace std;

Br_Predictor::Br_Predictor(const Config &config): config(config)
{
	btb_set_shift = (uint32_t) 1;
	btb_set_mask = (uint64_t) (config.br_predictor_config.btb_config.sets-1);
	btb_tag_shift = (uint32_t) (log2(config.br_predictor_config.btb_config.sets) + btb_set_shift);
	btb_tag_mask = (uint64_t) (pow(2, config.br_predictor_config.btb_config.sets - btb_tag_shift) - 1);

	total_exact_references = 0;

	total_taken_branches = 0;
	total_not_taken_branches = 0;
	total_jumps = 0;
	total_calls = 0;
	total_returns = 0;
	total_branch_mispredictions = 0;
	total_branch_dir_mispredictions = 0;
	total_branch_target_mispredictions = 0;
	total_jump_mispredictions = 0;
	total_call_mispredictions = 0;
	total_return_mispredictions = 0;

	branch_predictor = new bpred(config.br_predictor_config.two_level_config.i,
		config.br_predictor_config.two_level_config.j,
		config.br_predictor_config.two_level_config.k,
		config.br_predictor_config.two_level_config.s,
		config.br_predictor_config.two_level_config.init_val);
}

res_t Br_Predictor::predict(Ins *ins)
{
	if (config.br_predictor_config.br_predictor) {
		if (config.br_predictor_config.br_pred_type == TWO_LEVEL) {
			if (ins->control_type == JUMP) {
				total_jumps += 1;
				return predict_target(ins);
			} else if (ins->control_type == CALL) {
				total_calls += 1;
				push_call();
				return predict_target(ins);
			} else if (ins->control_type == RET) {
				total_returns += 1;
				return predict_ret();
			} else if (ins->control_type == BRANCH) {
				if (predict_dir(ins) == SUCCESS) {
					if (ins->br_dir == NOT_TAKEN) {
						total_not_taken_branches += 1;
						return SUCCESS;
					} else {
						total_taken_branches += 1;
						res_t predicted = predict_target(ins);
						if (predicted == FAIL) {
							total_branch_mispredictions += 1;
						}
						return predicted;
					}
				} else {
					total_branch_mispredictions += 1;
					if (ins->br_dir == NOT_TAKEN) {
						total_not_taken_branches += 1;
						return SUCCESS;
					} else {
						total_taken_branches += 1;
						predict_target(ins);
						return FAIL;
					}
				}
			}
		} else if (config.br_predictor_config.br_pred_type == PERFECT) {
			return SUCCESS;
		} else if (config.br_predictor_config.br_pred_type == EXACT) {
			total_exact_references += 1;
			if (total_exact_references >= config.br_predictor_config.exact_references_per_miss) {
				total_exact_references = 0;
				return FAIL;
			} else {
				return SUCCESS;
			}
		}
	} else {
		if (ins->br_dir == NOT_TAKEN) {
			return SUCCESS;
		} else {
			return FAIL;
		}
	}
}

res_t Br_Predictor::predict_target(Ins *ins)
{
	uint64_t btb_set_idx = (ins->ia >> btb_set_shift) & btb_set_mask;
	if (btb_tag_store.find(btb_set_idx) != btb_tag_store.end()) {
		uint64_t btb_tag = btb_tag_store[btb_set_idx];
		uint64_t ins_btb_tag = (ins->ia >> btb_tag_shift) & btb_tag_mask;
		if (ins_btb_tag == btb_tag) {
			uint64_t btb_target = btb_data_store[btb_set_idx];
			if (ins->ta == btb_target) {
				return SUCCESS;
			}
		}
	}

	uint64_t ins_btb_tag = (ins->ia >> btb_tag_shift) & btb_tag_mask;
	btb_tag_store[btb_set_idx] = ins_btb_tag;
	btb_data_store[btb_set_idx] = ins->ta;

	if (ins->control_type == BRANCH) {
		total_branch_target_mispredictions += 1;
	} else if (ins->control_type == JUMP) {
		total_jump_mispredictions += 1;
	} else if (ins->control_type == CALL) {
		total_call_mispredictions += 1;
	}

	return FAIL;
}

res_t Br_Predictor::predict_dir(Ins *ins)
{
	int actual_dir;
	int pred_dir = branch_predictor->pred(ins->ia);
	if (ins->br_dir == TAKEN) {
		actual_dir = 1;
	} else {
		actual_dir = 0;
	}
	branch_predictor->update(ins->ia, pred_dir, actual_dir);
	if (pred_dir != actual_dir) {
		total_branch_dir_mispredictions += 1;
		return FAIL;
	} else {
		return SUCCESS;
	}

	// if (ins->br_dir == TAKEN) {
	// 	total_branch_dir_mispredictions += 1;
	// 	return FAIL;
	// } else {
	// 	return SUCCESS;
	// }
}

res_t Br_Predictor::push_call(void)
{
	if (call_stack < config.br_predictor_config.call_stack_config.size) {
		call_stack += 1;
		return SUCCESS;
	} else {
		return FAIL;
	}
}

res_t Br_Predictor::predict_ret(void)
{
	if (call_stack <= 0) {
		total_return_mispredictions += 1;
		return FAIL;
	} else {
		call_stack -= 1;
		return SUCCESS;
	}
}

void Br_Predictor::print_results()
{
	log_info("Total Jumps,"+to_string(total_jumps));
	log_info("Total Mispredicted Jumps,"+to_string(total_jump_mispredictions));
	log_info("Total Calls,"+to_string(total_calls));
	log_info("Total Mispredicted Calls,"+to_string(total_call_mispredictions));
	log_info("Total Returns,"+to_string(total_returns));
	log_info("Total Mispredicted Returns,"+to_string(total_return_mispredictions));
	log_info("Total Taken Branches,"+to_string(total_taken_branches));
	log_info("Total Not Taken Branches,"+to_string(total_not_taken_branches));
	log_info("Total Mispredicted Branches,"+to_string(total_branch_mispredictions));
	log_info("Total Mispredicted Branch Directions,"+to_string(total_branch_dir_mispredictions));
	log_info("Total Mispredicted Branch Targets,"+to_string(total_branch_target_mispredictions));
}