

class Trace
{
public:
	Trace(const Config &config, gzFile infile);
	Ins *next_ins(void);
private:
	Ins *parse_ins(void);
	gzFile infile;
	const Config &config;
	Ins *temp_ins;
};