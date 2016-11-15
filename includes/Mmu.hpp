
typedef enum mmu_type {
	INSTRUCTION,
	DATA
} mmu_type_t;

class Mmu: public Mem_Interface
{
public:
	Mmu(const Config& config, mmu_type_t mmu_type);
	res_t send_request(struct data_access *da);
	struct data_access *get_response(void);
	res_t sub_send_request(Request *request);
	Response *sub_get_response(void);
	res_t super_send_request(Request *request);
	Response *super_get_response(void);
	res_t step(void);
	void print_results(void);
private:
	mmu_type_t mmu_type;
	std::unordered_map<Request *, struct data_access *> cache_out_request_buffer;
};