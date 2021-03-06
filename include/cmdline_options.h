#ifndef ARGPARSER_ARGPARSER_H_
#define ARGPARSER_ARGPARSER_H_

#include <string>
#include <sstream>
#include <vector>
#include <getopt.h>

class cmdline_options
{
public:
	// result gem representations
	struct {
		unsigned parens:1;
		unsigned tree:1;
		unsigned equations:1;
		unsigned table:1;
	} print = {0, 0, 0, 0};

	// output verbosity
	struct {
		unsigned quiet:1;
		unsigned info:1;
		unsigned debug:1;
	} output = {0, 0, 0};

	// skills
	struct {
		int TC;
		int amps;
	} skills= {0, 0};

	// amps
	struct {
		int number_per_gem;
		double average_gems_seen;
	} amps = {0, 1};

	// tuning
	struct {
		double combine_growth;
		int spec_limit;
		int final_eq_grade;
	} tuning = {0, 0, 30};

	// gem tables paths
	std::vector<std::string> tables;

	// stuff to do
	struct {
		// extra search
		unsigned upto:1;
		unsigned chain:1;

		// gem sizes (positional args)
		int len;
		int lenc;

		int pool_zero;
	} target = {0, 0, 0, 16, 1};

private:

	struct {
		std::string short_options;
		std::vector<struct option> long_options;
	} getopt;

	uint num_tables_ = 0;
	bool has_lenc_ = false;

public:
	cmdline_options()
	{
		this->add_option({"help",      no_argument, NULL, 'h'});
		this->add_option({"log-quiet", no_argument, NULL, 'q'});
		this->add_option({"verbose",   no_argument, NULL, 'v'});
	}

	void has_printing()
	{
		this->add_option({"print-parens",    no_argument, NULL, 'p'});
		this->add_option({"print-tree",      no_argument, NULL, 't'});
		this->add_option({"print-equations", no_argument, NULL, 'e'});
		this->add_option({"print-table",     no_argument, NULL, 'c'});
	}

	void has_extra_search()
	{
		this->add_option({"upto",    no_argument, NULL, 'u'});
		this->add_option({"add-red", no_argument, NULL, 'r'});
	}

	void has_amps()
	{
		this->add_option({"skill-amps",    required_argument, NULL, 'A'});
		this->add_option({"amps-per-gem",  required_argument, NULL, 'Q'});
		this->add_option({"avg-gems-seen", required_argument, NULL, 'G'});
	}

	void has_nonpures()
	{
		this->add_option({"skill-TC", required_argument, NULL, 'T'});
	}

	void has_combine_growth()
	{
		this->add_option({"combine-growth", required_argument, NULL, 'g'});
	}

	void has_spec_limit()
	{
		this->add_option({"limit-spec", required_argument, NULL, 'l'});
	}

	void has_final_eq_grade()
	{
		this->add_option({"final-eq-grade", required_argument, NULL, 'F'});
	}

	void set_num_tables(int num_tables)
	{
		this->num_tables_ = num_tables;
		this->add_option({"table-file", required_argument, NULL, 'f'});
	}

	void has_lenc()
	{
		this->has_lenc_ = true;
	}

	inline bool read_cmdline_opt(int opt, const char* optarg)
	{
		switch(opt) {
			case 'h':
				this->print_help();
				return false;
			case 'q':
					this->output.quiet = true;
				break;
			case 'v':
				if (this->output.info)
					this->output.debug = true;
				else
					this->output.info = true;
				break;

			case 'p':
				this->print.parens = true;
				break;
			case 't':
				this->print.tree = true;
				break;
			case 'e':
				this->print.equations = true;
				break;
			case 'c':
				this->print.table = true;
				break;

			case 'u':
				this->target.upto = true;
				break;
			case 'r':
				this->target.chain = true;
				break;

			case 'A':
				this->skills.amps = atoi(optarg);
				break;
			case 'Q':
				this->amps.number_per_gem = atof(optarg);
				break;
			case 'G':
				this->amps.average_gems_seen = atof(optarg);
				break;

			case 'T':
				this->skills.TC = atoi(optarg);
				break;

			case 'g':
				this->tuning.combine_growth = atof(optarg);
				break;
			case 'l':
				this->tuning.spec_limit = atoi(optarg);
				break;
			case 'F':
				this->tuning.final_eq_grade = atoi(optarg);
				break;

			case 'f': {
				std::stringstream ss(optarg);
				std::string s;
				while (std::getline(ss, s, ',')) {
					this->tables.push_back(s);
				}
				if (this->tables.size() > this->num_tables_) {
					printf("Too many table names, expected %d\n", this->num_tables_);
					return false;
				}
				break;
			}

			case '?':
				return false;
			default:
				break;
		}
		return true;
	}

	// I assume defaults have been set before calling this
	bool parse_args(int argc, char*const* argv)
	{
		int opt;
		while ((opt = getopt_long(argc, argv, this->getopt.short_options.c_str(), this->getopt.long_options.data(), NULL)) != -1) {
			if (!this->read_cmdline_opt(opt, optarg))
				return false;
		}

		if (optind == argc) {
			printf("No length specified\n");
			return false;
		}
		if (optind + 1 == argc) {
			this->target.len = atoi(argv[optind]);
			char* p = argv[optind];
			while (*p != '\0')
				p++;
			if (*(p - 1) == 'c')
				this->target.pool_zero = 1;
		}
		else if (optind + 2 == argc && has_lenc_) {
			this->target.len = atoi(argv[optind]);
			this->target.lenc = atoi(argv[optind + 1]);
		}
		else {
			printf("Too many arguments:\n");
			while (argv[optind] != NULL) {
				printf("%s ", argv[optind]);
				optind++;
			}
			printf("\n");
			return false;
		}

		if (this->target.len == 0 || (has_lenc_ && this->target.lenc == 0)) {
			printf("Improper gem number\n");
			return false;
		}

		// fill up to desired size with empty strings
		this->tables.resize(this->num_tables_);

		if (this->output.debug && this->num_tables_ > 0) {
			printf("Selected table(s):");
			for (auto& s : this->tables)
				printf(" %s", s.c_str());
			printf("\n");
		}

		return true;
	}

	void print_help() const
	{
		// can (and should) be improved
		printf("%s\n", this->getopt.short_options.c_str());
	}

	void table_selection(int num, std::string default_name)
	{
		std::string& filename = this->tables[num];

		if (!filename.empty())
			return;
		else if (file_exists(default_name)) {
			filename = default_name;
		}
		else {
			std::string other_path = "gem_tables/" + default_name;
			if (file_exists(other_path))
				filename = other_path;
			else
				// we know it'll fail, but at least the error message will make sense
				filename = default_name;
		}
	}

private:
	static inline bool file_exists (const std::string& name)
	{
		FILE* file;
		if ((file = fopen(name.c_str(), "r"))) {
			fclose(file);
			return true;
		}
		else
			return false;
	}

	void add_option(struct option&& option)
	{
		this->getopt.short_options += (char)option.val;
		this->getopt.short_options += std::string(option.has_arg, ':');

		this->getopt.long_options.push_back(option);
	}

};

#endif /* ARGPARSER_ARGPARSER_H_ */
