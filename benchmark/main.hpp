#ifndef MAIN_HPP
#define MAIN_HPP

#include "common.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <utility>
#include <vector>

#define EXPERIMENT_SIZE 1000
#define BASE_SIZE 2048
#define DOUBLINGS 10

struct ConfigHolder
{
	int64_t experiment_size;
	double relative_experiment_size;
	bool use_relative_size = false;
	int64_t base_size;
	size_t doublings;

	int64_t seed_start;
	size_t seed_count;
};

ConfigHolder CFG;

void
BuildRange(::benchmark::internal::Benchmark * b)
{
	for (int64_t seed = CFG.seed_start;
	     seed < CFG.seed_start + static_cast<int64_t>(CFG.seed_count); ++seed) {
		for (size_t doubling = 0; doubling < CFG.doublings; ++doubling) {
			if (CFG.use_relative_size) {
				b->Args({CFG.base_size << doubling,
				         static_cast<int64_t>(std::round((CFG.base_size << doubling) *
				                                         CFG.relative_experiment_size)),
				         seed});
			} else {
				b->Args({CFG.base_size << doubling, CFG.experiment_size, seed});
			}
		}
	}
}

int
main(int argc, char ** argv)
{
#ifndef __OPTIMIZE__
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	std::cout << "!!                  Warning                   !!\n";
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	std::cout << "Either you compiled this binary without optimization,\nor my "
	             "optimization detection hack does not work for your compiler.\n";
	std::cout << "Doing benchmarks without optimization is not very useful,\nthe "
	             "numbers you derive from it will not be meaningful.\n";
	std::cout << "Please make sure optimization is turned on, and if so,\nsubmit "
	             "a bug report.\n";
	std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
	std::cout << "\n\n";

#endif

	int remaining_argc = argc;
	char ** remaining_argv = reinterpret_cast<char **>(
	    malloc(static_cast<size_t>(argc) * sizeof(char *)));

	CFG.experiment_size = EXPERIMENT_SIZE;
	CFG.base_size = BASE_SIZE;
	CFG.doublings = DOUBLINGS;
	CFG.seed_start = 4;
	CFG.seed_count = 2;

	std::string filter_re = "";

	remaining_argv[0] = argv[0];
	size_t j = 1;
	for (size_t i = 1; i < static_cast<size_t>(argc); ++i) {
		if (strncmp(argv[i], "--papi", strlen("--papi")) == 0) {
			char * tok = strtok(argv[i + 1], ",");

			while (tok != NULL) {
				std::cout << tok << "\n";
				PAPI_MEASUREMENTS.emplace_back(tok);
				tok = strtok(NULL, ",");
			}

			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--doublings", strlen("--doublings")) == 0) {
			CFG.doublings = static_cast<size_t>(atoi(argv[i + 1]));
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--base_size", strlen("--base_size")) == 0) {
			CFG.base_size = static_cast<int64_t>(atoi(argv[i + 1]));
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--seed_start", strlen("--seed_start")) == 0) {
			CFG.seed_start = atoi(argv[i + 1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--seed_count", strlen("--seed_count")) == 0) {
			CFG.seed_count = static_cast<size_t>(atoi(argv[i + 1]));
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--filter", strlen("--filter")) == 0) {
			std::cout << "Setting filter: " << argv[i + 1] << "\n" << std::flush;
			filter_re = std::string(argv[i + 1]);
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--experiment_size",
		                   strlen("--experiment_size")) == 0) {
			CFG.experiment_size = static_cast<int64_t>(atoi(argv[i + 1]));
			i += 1;
			remaining_argc -= 2;
		} else if (strncmp(argv[i], "--relative_experiment_size",
		                   strlen("--relative_experiment_size")) == 0) {
			CFG.relative_experiment_size = static_cast<double>(atof(argv[i + 1]));
			i += 1;
			remaining_argc -= 2;
			CFG.use_relative_size = true;
		} else {
			remaining_argv[j++] = argv[i];
		}
	}

	/*
	 * Register all tests
	 */
	auto plugins = DRAUP_GET_REGISTERED();

	boost::hana::for_each(plugins, [&](auto plugin_cls) {
		using plugin = typename decltype(plugin_cls)::type;

		auto name = plugin::get_name();

		bool skip = false;
		if (filter_re.compare("") != 0) {
			std::regex re(filter_re, std::regex_constants::ECMAScript |
			                             std::regex_constants::icase);
			if (!std::regex_match(name, re)) {
				skip = true;
			}
		}

		if (!skip) {
			auto * bench = new plugin();
			bench->set_name(name);
			/*
			RangeVector ranges;
			ranges.emplace_back(base_size, base_size * (1 << doublings));
			ranges.emplace_back(experiment_size, experiment_size);
			bench->RangeMultiplier(2);
			bench->Ranges(ranges);
			*/
			bench->Apply(BuildRange);
			::benchmark::internal::RegisterBenchmarkInternal(bench);
		}
	});

	PAPI_STATS_WRITTEN = false;

	::benchmark::Initialize(&remaining_argc, remaining_argv);
	if (::benchmark::ReportUnrecognizedArguments(remaining_argc,
	                                             remaining_argv)) {
		return 1;
	}

	::benchmark::RunSpecifiedBenchmarks();
	return 0;
}

#endif
