#include <cstdio>

#include <random>
#include <chrono>
#include <vector>

#include "linear.hpp"
#include "cuckoo.hpp"


template <typename THashTable>
void test(const char *name, const char *color) {
    using clock = std::chrono::steady_clock;

    // static std::random_device rd;
    static std::mt19937 randu(0x19260817);

    constexpr int N_INIT = 10000;
    constexpr int N_TIMED_SET = 1000;
    constexpr int N_TEST = 1000;

    std::vector<uint32_t> keys;
    keys.resize(N_INIT * 2);
    for (auto &key : keys) {
        key = randu();
    }

    std::vector<long> set_time;
    set_time.resize(N_TIMED_SET);

    THashTable inst;

    for (int i = 0; i < N_INIT; i++) {
        auto key = keys[i];
        auto value = randu();

        if (i < N_TIMED_SET) {
            auto t_begin = clock::now();
            inst.set(key, value);
            auto t_end = clock::now();
            set_time[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end - t_begin).count();
        } else
            inst.set(key, value);
    }

    printf("ListLinePlot[Legended[Style[{0");
    for (auto t : set_time) {
        printf(",%ld", t);
    }
    printf("},RGBColor[%s]],%s],PlotRange->{{0,1000},{0,400}},ImageSize->Large],\n", color, name);

    auto copy1 = inst;
    auto min = std::numeric_limits<long>::max();
    auto max = std::numeric_limits<long>::min();
    auto sum = 0.0L;

    auto t_begin = clock::now();
    int count = 0;
    for (int i = 0; i < N_TEST; i++) {
        auto key = keys[randu() % keys.size()];

        auto t_begin = clock::now();
        auto ptr = copy1.get(key);
        auto t_end = clock::now();

        if (ptr)
            count++;

        auto used = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end - t_begin).count();
        min = std::min(min, used);
        max = std::max(max, used);
        sum += used;
    }
    auto t_end = clock::now();
    auto used = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end - t_begin).count();
    auto avg = sum / N_TEST;
    auto throughput = N_TEST / (double(used) / 1e9);
    fprintf(stderr,
        "min=%ldns, max=%ldns, avg=%.4Lf, throu.=%.4lf, count=%dns\n",
        min, max, avg, throughput, count
    );
    printf(
        "Legended[Around[%.6Lf,{%.6Lf,%.6Lf}],\"get/%s\"]\n",
        avg, avg - min, max - avg, name
    );
    printf(
        "Legended[%.6lf,\"get/%s\"]\n",
        throughput, name
    );

    auto copy2 = inst;
    min = std::numeric_limits<long>::max();
    max = std::numeric_limits<long>::min();
    sum = 0.0L;
    t_begin = clock::now();
    for (int i = 0; i < N_TEST; i++) {
        auto key1 = keys[randu() % keys.size()];
        auto key2 = randu();
        auto value = randu();

        auto t_begin = clock::now();
        copy2.set(key2, value);
        auto ptr = copy2.get(key1);
        auto t_end = clock::now();

        if (ptr)
            count++;

        auto used = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end - t_begin).count();
        min = std::min(min, used);
        max = std::max(max, used);
        sum += used;
    }
    t_end = clock::now();
    used = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end - t_begin).count();
    avg = sum / N_TEST;
    throughput = N_TEST / (double(used) / 1e9);
    fprintf(stderr,
        "min=%ldns, max=%ldns, avg=%.4Lf, throu.=%.4lf, count=%dns\n",
        min, max, avg, throughput, count
    );
    printf(
        "Legended[Around[%.6Lf,{%.6Lf,%.6Lf}],\"get+set/%s\"]\n",
        avg, avg - min, max - avg, name
    );
    printf(
        "Legended[%.6lf,\"get+set/%s\"]\n",
        throughput, name
    );
}

int main() {
    test<LinearHashTable<>>("linear", "1,0,0,0.5");
    test<CuckooHashTable<>>("cuckoo", "0,1,0,0.5");
    return 0;
}
