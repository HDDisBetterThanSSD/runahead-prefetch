
typedef enum cache_type {
	L1i,
	L1d,
	L2,
	L3
} cache_type_t;

typedef enum cache_action {
	NOTHING,
	NO_CACHE_ACCESS,
	CACHE_ACCESS
} cache_action_t;

struct cache_index {
	std::list<uint64_t> replacement_order;
	std::unordered_map<uint64_t, struct cache_way> ways;
};

struct cache_way {
	bool dirty_bit;
	mesi_t mesi_bits;
};

class Cache: public Mem_Interface
{
public:
	Cache(const Config& config, struct cache_config cache_config, cache_type_t cache_type);
	res_t sub_send_request(Request *request);
	Response *sub_get_response(void);
	res_t super_send_request(Request *request);
	Response *super_get_response(void);
	res_t step(void);
	void print_results(void);
protected:
	cache_type_t cache_type;
	std::unordered_map<uint64_t, struct cache_index>tag_store;
	struct cache_config cache_config;

	uint32_t set_shift;
	uint64_t set_mask;
	uint64_t get_set_idx(uint64_t addr);
	uint32_t tag_mask;
	uint64_t tag_shift;
	uint64_t get_tag(uint64_t addr);

	res_t fill(uint64_t addr, mesi_t mesi_bits);
	res_t read(uint64_t addr);
	res_t can_read(uint64_t addr);
	res_t write(uint64_t addr);
	res_t can_write(uint64_t addr);
	res_t invalidate(bool writeback);

	//statistics
	uint64_t total_misses;
	uint64_t total_requests;
};