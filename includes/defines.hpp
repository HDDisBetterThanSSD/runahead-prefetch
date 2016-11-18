typedef enum replacement_policy {
	LRU
} replacement_policy_t;


struct cpu_config {
	bool back_end;
	bool st_fwd;
	int pf_pipe_width;
	int fe_pipe_depth;
	int fe_pipe_width;
	int ls_buffer_size;
	int ld_buffer_size;
	int st_buffer_size;
	int f_buffer_size;
	int iq_buffer_size;
	int pd_pipe_width;
	int d_pipe_width;
};

typedef enum mesi {
	MODIFIED,
	EXCLUSIVE,
	SHARED,
	INVALID
} mesi_t;

struct cache_config {
	int sets;
	int ways;
	int cl_size;
	replacement_policy_t replacement_policy;
	int hit_latency;
	int hit_request_bw;
	int miss_latency;
	int miss_byte_bw;
	int addr_bits;
	int in_request_buffer_size;
	int in_response_buffer_size;
	int read_request_buffer_size;
	int write_request_buffer_size;
};

struct mem_config {
	int bus_speed;
	int bandwidth;
	int latency;
	int mlp;
};

struct mem_hier_config {
	bool l1i_on;
	bool l1d_on;
	bool l2_on;
	bool l3_on;
	bool l2_private;
	bool l3_private;
};

struct tlb_config {
	int sets;
	int ways;
	int miss_latency;
};

struct page_table_config {
	int pages;
};

typedef enum res {
	SUCCESS,
	FAIL
} res_t;

extern unsigned long steps;

typedef enum prefetch_type {
	N1L,
	N2L,
	BR_PREDICTOR
} prefetch_type_t;

struct prefetch_config {
	bool prefetch;
	prefetch_type_t prefetch_type;
};

typedef enum br_pred_type {
	NONE,
	PERFECT,
	TWO_LEVEL,
	EXACT
} br_pred_type_t;

struct two_level_config {
	int i;
	int j;
	int k;
	int s;
	int init_val;
};

struct btb_config {
	int sets;
	int addr_bits;
};

struct call_stack_config {
	int size;
};

struct br_predictor_config {
	bool br_predictor;
	br_pred_type_t br_pred_type;
	int exact_references_per_miss;
	struct two_level_config two_level_config;
	struct btb_config btb_config;
	struct call_stack_config call_stack_config;
};
