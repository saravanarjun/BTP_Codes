#include "benchmark_runner.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string mode = "--benchmark";
    if (argc > 1) {
        mode = argv[1];
    }

    std::string staticResCsv = "benchmarks/maximal_cliques/results/static/static_results.csv";
    std::string staticSumCsv = "benchmarks/maximal_cliques/results/static/static_summary.csv";
    std::string dynInsCsv    = "benchmarks/maximal_cliques/results/dynamic/insertion/dynamic_insertion_results.csv";
    std::string dynDelCsv    = "benchmarks/maximal_cliques/results/dynamic/deletion/dynamic_deletion_results.csv";
    std::string dynMixCsv    = "benchmarks/maximal_cliques/results/dynamic/mixed/dynamic_mixed_results.csv";
    std::string dynSumCsv    = "benchmarks/maximal_cliques/results/dynamic/dynamic_summary.csv";
    std::string verifCsv     = "benchmarks/maximal_cliques/results/verification/verification_results.csv";

    if (mode == "--verify") {
        bool ok = BenchmarkRunner::runVerificationSuite(verifCsv);
        return ok ? 0 : 1;
    } else if (mode == "--smoke") {
        std::cout << "[*] Running in SMOKE test mode (fast execution)...\n";
        bool ok = BenchmarkRunner::runVerificationSuite(verifCsv);
        if (!ok) {
            std::cerr << "[-] Verification failed in smoke mode!\n";
            return 1;
        }
        BenchmarkRunner::runStaticBenchmarks(staticResCsv, staticSumCsv, true);
        BenchmarkRunner::runDynamicBenchmarks(dynInsCsv, dynDelCsv, dynMixCsv, dynSumCsv, true);
        std::cout << "[+] Smoke tests completed successfully!\n";
        return 0;
    } else if (mode == "--static-only") {
        BenchmarkRunner::runStaticBenchmarks(staticResCsv, staticSumCsv, false);
        return 0;
    } else if (mode == "--dynamic-only") {
        BenchmarkRunner::runDynamicBenchmarks(dynInsCsv, dynDelCsv, dynMixCsv, dynSumCsv, false);
        return 0;
    } else if (mode == "--benchmark" || mode == "--all") {
        bool ok = BenchmarkRunner::runVerificationSuite(verifCsv);
        if (!ok) {
            std::cerr << "[-] Verification failed prior to benchmarking! Halting.\n";
            return 1;
        }
        BenchmarkRunner::runStaticBenchmarks(staticResCsv, staticSumCsv, false);
        BenchmarkRunner::runDynamicBenchmarks(dynInsCsv, dynDelCsv, dynMixCsv, dynSumCsv, false);
        std::cout << "\n[+] Complete static and dynamic benchmarking suite finished successfully!\n";
        return 0;
    } else {
        std::cout << "Usage: clique_benchmark [--verify | --benchmark | --smoke | --static-only | --dynamic-only]\n";
        return 1;
    }
}
