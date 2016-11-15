
typedef enum data_type {
	LOAD,
	STORE
} data_type_t;

typedef enum control_type {
	NO_BRANCH,
	BRANCH,
	JUMP,
	CALL,
	RET
} control_type_t;

#define TAKEN true
#define NOT_TAKEN false

struct data_access {
	data_type_t data_type;
	uint64_t da;
	uint32_t tag;
};

class Ins
{
public:
	Ins(const Config& config, uint64_t ia);

	static Ins *NewIns(const Config& config, uint64_t ia);
	static void FreeIns(Ins *ins);

	void set_control_type(control_type_t control_type);
	void add_load_addr(uint64_t da);
	void add_store_addr(uint64_t da);
	void add_branch_dir(bool dir);
	void add_branch_target(uint64_t ta);
	std::string to_string(void);
	uint64_t ia;
	std::vector<struct data_access> data_accesses;
	control_type_t control_type;
	bool br_dir;
	uint64_t ta;
	std::string ins_str;
private:
	static std::vector<Ins *> free_ins_list;

	const Config &config;
};