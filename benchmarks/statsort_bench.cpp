#include <daAlgorithms/statsort.hpp>
#include <boost/sort/spreadsort/spreadsort.hpp>
#include <boost/sort/sort.hpp>

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

void basic_examples()
{
    std::cout << "=== Basic usage ===\n\n";

    std::vector<double> prices = {9.99, 1.49, 5.00, 3.75, 8.20, 0.50};
    fspclib::algorithm::statsort(prices);
    std::cout << "Sorted prices: ";
    for (double p : prices) std::cout << p << " ";
    std::cout << "\n";

    std::vector<int> scores = {88, 42, 95, 17, 63, 71, 55};
    fspclib::algorithm::statsort(scores);
    std::cout << "Sorted scores: ";
    for (int s : scores) std::cout << s << " ";
    std::cout << "\n";

    std::vector<float> temps = {36.6f, 38.1f, 37.0f, 35.9f, 39.2f};
    fspclib::algorithm::statsort(temps.begin(), temps.end());
    std::cout << "Sorted temps:  ";
    for (float t : temps) std::cout << t << " ";
    std::cout << "\n\n";
}

using Clock = std::chrono::high_resolution_clock;
using Ms    = std::chrono::duration<double, std::milli>;

template <typename SortFn>
double time_ms(SortFn fn, const std::vector<double>& base, int runs = 5)
{
    double total = 0;
    for (int r = 0; r < runs; ++r) {
        auto v = base;
        auto t0 = Clock::now();
        fn(v);
        total += Ms(Clock::now() - t0).count();
    }
    return total / runs;
}

void benchmark()
{
    std::cout << "=== Performance comparison ===\n\n";
    std::cout << std::left  << std::setw(3) << "| Distribution | N"
              << std::right << std::setw(12) << "| std::sort"
              << std::setw(12) << "| statsort"
              << std::setw(12) << "| spreadsort"
              << std::setw(12) << "| pdqsort |\n"
              <<  "|---|---|---|---|---|---|\n";

    auto print_row = [&](const std::string& label,
                         const std::vector<double>& base) {
        double t_std             = time_ms([](auto& v){ std::sort(v.begin(), v.end()); }, base);
        double t_stat            = time_ms([](auto& v){ fspclib::algorithm::statsort(v); }, base);
        double t_spreadsort      = time_ms([](auto& v){ boost::sort::spreadsort::spreadsort(v.begin(), v.end()); }, base);
        double t_pdqsort         = time_ms([](auto& v){ boost::sort::pdqsort(v.begin(), v.end()); }, base);

        double t_min = std::min(t_std, std::min(t_stat, std::min(t_spreadsort, t_pdqsort)));

        std::cout << std::left  << std::setw(2) << "| " << label << " | "
                  << std::right << std::fixed << std::setprecision(2)
                  << ((t_std == t_min) ? "**" : "") << std::setw(3) << t_std << ((t_std == t_min) ? "**" : "") << "  |"
                  << ((t_stat == t_min) ? "**" : "") << std::setw(3) << t_stat << ((t_stat == t_min) ? "**" : "") << "  |"
                  << ((t_spreadsort == t_min) ? "**" : "") << std::setw(3) << t_spreadsort << ((t_spreadsort == t_min) ? "**" : "") << "  |"
                  << ((t_pdqsort == t_min) ? "**" : "") << std::setw(3) << t_pdqsort << ((t_pdqsort == t_min) ? "**" : "") << "  |\n";
    };

    // requires 16 GB or more of RAM memory
    for (std::size_t n :
        {1000000UL, 5000000UL, 10000000UL, 50000000UL, 100000000UL, 250000000UL, 500000000UL}
    ) {
        std::mt19937 rng(427);
        std::string ns = std::to_string(n);
        auto t0 = Clock::now();
	    

        { std::uniform_real_distribution<double> d(0,1e6);
          std::vector<double> v(n); std::generate(v.begin(),v.end(),[&]{return d(rng);});
          auto t0 = Clock::now();
          print_row("Uniform|   " + ns, v); }
    	std::cout.flush();

        { std::normal_distribution<double> d(500000,100000);
          std::vector<double> v(n); std::generate(v.begin(),v.end(),[&]{return d(rng);});
		  auto t0 = Clock::now();
          //std::cout << "Clock " << t0 << "\n";
          print_row("Gaussian|  " + ns, v); }
        std::cout.flush();

        { std::exponential_distribution<double> d(0.00001);
          std::vector<double> v(n); std::generate(v.begin(),v.end(),[&]{return d(rng);});
		  auto t0 = Clock::now();
          //std::cout << "Clock " << t0 << "\n";
          print_row("Exponential|" + ns, v); }

        std::cout << "|---|---|---|---|---|---|\n";
        std::cout.flush();
	}
	std::cout << "\n";
    std::cout << "\n";
    std::cout.flush();
}


int main()
{
    basic_examples();
    benchmark();
    return 0;
}

