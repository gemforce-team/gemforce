#ifndef CMDLINE_OPTIONS_H_
#define CMDLINE_OPTIONS_H_

#include <filesystem>
#include <string>
#include <sstream>
#include <vector>
#include <getopt.h>

using namespace std::string_literals;

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
	} skills = {0, 0};

	// amps
	struct {
		int number_per_gem;
		double average_gems_seen;
	} amps = {0, 1};

	// tuning
	struct {
		double combine_growth;
		int spec_limit;
		double max_ag_cost_ratio;
		int final_eq_grade;
	} tuning = {0, 0, 1, 30};

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

	std::string help_text;
	unsigned int num_tables_ = 0;
	bool has_lenc_ = false;

public:
	cmdline_options()
	{
		this->add_option({"help",      no_argument, NULL, 'h'}, "show this help message and exit");
		this->add_option({"log-quiet", no_argument, NULL, 'q'}, "log less, skip unneded computation");
		this->add_option({"verbose",   no_argument, NULL, 'v'}, "log a lot more");
	}

	void has_printing()
	{
		this->add_option({"print-parens",    no_argument, NULL, 'p'}, "print parens representations");
		this->add_option({"print-tree",      no_argument, NULL, 't'}, "print tree representations");
		this->add_option({"print-equations", no_argument, NULL, 'e'}, "print equations representations");
		this->add_option({"print-table",     no_argument, NULL, 'c'}, "print table of values");
	}

	void has_extra_search()
	{
		this->add_option({"upto",    no_argument, NULL, 'u'}, "return best result up to bound");
		this->add_option({"add-red", no_argument, NULL, 'r'}, "add a single \"red\" gem to the result");
	}

	void has_amps()
	{
		this->add_option({"skill-amps",     required_argument, NULL, 'A'}, "value of the amp skill");
		this->add_option({"amps-per-gem",   required_argument, NULL, 'Q'}, "number of amps per gem");
		this->add_option({"avg-gems-seen",  required_argument, NULL, 'G'}, "average gems seens by each amp");
		this->add_option({"max-cost-ratio", required_argument, NULL, 'R'}, "max cost ratio amp/gem");
	}

	void has_nonpures()
	{
		this->add_option({"skill-TC", required_argument, NULL, 'T'}, "value of the TC skill");
	}

	void has_combine_growth()
	{
		this->add_option({"combine-growth", required_argument, NULL, 'g'}, "value of the combine growth");
	}

	void has_spec_limit()
	{
		this->add_option({"limit-spec", required_argument, NULL, 'l'}, "max spec size");
	}

	void has_final_eq_grade()
	{
		this->add_option({"final-eq-grade", required_argument, NULL, 'F'}, "final equivalent grade");
	}

	void set_num_tables(int num_tables)
	{
		this->num_tables_ = num_tables;
		this->add_option({"table-file", required_argument, NULL, 'f'}, "table[,tableA][,tableC]");
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
			case 'R':
				this->tuning.max_ag_cost_ratio = atof(optarg);
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
					printf("Too many table names, expected %u\n", this->num_tables_);
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

		return true;
	}

	void print_help() const
	{
		// can (and should) be improved
		printf("len\t\tlength of result\n");
		if (this->has_lenc_) printf("lenc\t\tlength of combine\n");
		printf("%s\n", this->help_text.c_str());
	}

	void table_selection(unsigned int num, const std::string& default_name)
	{
		std::string& filename = this->tables[num];

		if (!filename.empty())
			;
		else if (std::filesystem::exists(default_name)) {
			filename = default_name;
		}
		else {
			std::string other_path = "gem_tables/" + default_name;
			if (std::filesystem::exists(other_path))
				filename = other_path;
			else
				// we know it'll fail, but at least the error message will make sense
				filename = default_name;
		}

		// print table list at last iteration, when all are known
		if (this->output.debug && num == this->num_tables_ - 1) {
			printf("Selected table(s):");
			for (auto& s : this->tables)
				printf(" %s", s.c_str());
			printf("\n");
		}
	}

private:

	void add_option(struct option&& option, const std::string& help_text)
	{
		this->getopt.short_options += (char)option.val;
		this->getopt.short_options += std::string(option.has_arg, ':');

		this->getopt.long_options.push_back(option);
		this->help_text += "-"s + (char)option.val + " --" + option.name + (option.has_arg ? " X" : "") +
						   " \t" + help_text + '\n';
	}
};

#endif /* CMDLINE_OPTIONS_H_ */
