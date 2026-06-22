#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

using clock_type = std::chrono::high_resolution_clock;

template <typename Fn>
double bench_us_per_op(const std::string& name, Fn&& fn, int iterations) {
    const auto start = clock_type::now();
    for (int i = 0; i < iterations; ++i) {
        fn();
    }
    const auto end = clock_type::now();
    const auto total_us =
        std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(end - start).count();
    const double us_per_op = total_us / static_cast<double>(iterations);
    std::cout << std::left << std::setw(32) << name << std::right << std::setw(12)
              << std::fixed << std::setprecision(3) << us_per_op << " us/op ("
              << iterations << " iters)\n";
    return us_per_op;
}

void compare(const std::string& a, double va, const std::string& b, double vb) {
    if (vb == 0.0) {
        std::cout << "  " << a << " vs " << b << " ratio: n/a\n";
        return;
    }
    std::cout << "  " << a << " vs " << b << " ratio: "
              << std::fixed << std::setprecision(3) << (va / vb) << "x\n";
}

} // namespace

int main() {
    constexpr int outer_iters = 200;
    constexpr int item_count = 500;

    std::vector<int> items;
    items.reserve(item_count);
    std::vector<std::pair<std::string, std::string>> key_values;
    key_values.reserve(item_count);
    for (int i = 0; i < item_count; ++i) {
        items.push_back(i);
        key_values.emplace_back("k" + std::to_string(i), "v" + std::to_string(i));
    }
    const std::string mid_key = key_values[static_cast<std::size_t>(item_count / 2)].first;

    std::cout << "== C++ benchmark: vector/unordered_map ==\n\n";

    const auto vector_push = bench_us_per_op("cpp vector push", [&]() {
        std::vector<int> v;
        for (int item : items) {
            v.push_back(item);
        }
    }, outer_iters);

    const auto vector_push_reserve = bench_us_per_op("cpp vector push(reserve)", [&]() {
        std::vector<int> v;
        v.reserve(item_count);
        for (int item : items) {
            v.push_back(item);
        }
    }, outer_iters);

    const auto vector_read = bench_us_per_op("cpp vector at(read)", [&]() {
        std::vector<int> v;
        v.reserve(item_count);
        for (int item : items) {
            v.push_back(item);
        }
        volatile int x = v.at(item_count / 2);
        (void)x;
    }, outer_iters);

    compare("push(reserve)", vector_push_reserve, "push", vector_push);
    std::cout << "\n";

    const auto map_put = bench_us_per_op("cpp unordered_map put", [&]() {
        std::unordered_map<std::string, std::string> m;
        for (const auto& kv : key_values) {
            m.emplace(kv.first, kv.second);
        }
    }, outer_iters);

    const auto map_put_reserve = bench_us_per_op("cpp unordered_map put(reserve)", [&]() {
        std::unordered_map<std::string, std::string> m;
        m.reserve(item_count);
        for (const auto& kv : key_values) {
            m.emplace(kv.first, kv.second);
        }
    }, outer_iters);

    const auto map_get = bench_us_per_op("cpp unordered_map get", [&]() {
        std::unordered_map<std::string, std::string> m;
        m.reserve(item_count);
        for (const auto& kv : key_values) {
            m.emplace(kv.first, kv.second);
        }
        volatile std::size_t n = m.at(mid_key).size();
        (void)n;
    }, outer_iters);

    compare("put(reserve)", map_put_reserve, "put", map_put);
    std::cout << "\n";

    std::cout << "Summary:\n";
    std::cout << "- Use these numbers beside Tcl benchmark output from becnhmarks.tcl.\n";
    std::cout << "- C++ is native; Tcl wrapper and interpreter dispatch add overhead.\n";

    (void)vector_read;
    (void)map_get;
    return 0;
}
