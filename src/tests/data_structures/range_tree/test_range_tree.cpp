//
// Created by sj on 11/11/19.
//
#include "gtest/gtest.h"
#include <algorithm>
#include <numeric>
#include <random>
#include "range_tree.hpp"
#include "simple_bitset.hpp"

namespace {
    TEST(range_trees, small_range_tree_construction) {
        std::vector<std::pair<int, int>> w = {{0, 3},
                                              {1, 5},
                                              {2, 1},
                                              {3, 2},
                                              {4, 4}};
        auto uptr = std::make_unique<range_tree < int, int>>(w);
    }

    TEST(range_trees, large_random_tree_construction_permutations) {
        int n = 1'000'000;
        std::vector<std::pair<int, int>> w(n);
        std::vector<int> v(n);
        iota(begin(v), end(v), 0);
        std::shuffle(begin(v), end(v), std::default_random_engine());
        for (int i = 0; i < v.size(); ++i)
            w[i] = {i, v[i]};
        auto uptr = std::make_unique<range_tree < int, int>>
        (w);
        std::cerr << uptr->size() << std::endl;
    }

// we assume it is sorted by first coordinate, and they are also in rank space
// i.e. essentially w[i].first == i
    std::vector <std::pair<int, int>> brute_force_2dsearch(const std::vector<std::pair<int, int>> &w, int i, int j, int a, int b) {
        std::vector<std::pair<int, int>> res;
        for (; i <= j; ++i)
            if (a <= w[i].second and w[i].second <= b)
                res.push_back(w[i]);
        return res;
    }

    void display(const std::vector<std::pair<int,int>> &a) {
        std::cerr << "{";
        for (const auto &x: a)
            std::cerr << "(" << x.first << "," << x.second << "),";
        std::cerr << "}\n";
    }

    bool equal_vectors(std::vector <std::pair<int, int>> &a, std::vector <std::pair<int, int>> &b) {
        sort(begin(a), end(a)), sort(begin(b), end(b));
        if (a.size() != b.size()) {
            std::cerr << "size a = " << a.size() << ", size b = " << b.size() << std::endl;
            display(a), display(b);
            return false;
        }
        for (int i = 0; i < a.size(); ++i)
            if (a[i] != b[i]) {
                std::cerr << a[i].first << ", " << a[i].second << " != " << b[i].first << ", " << b[i].second << std::endl;
                return false;
            }
        return true;
    }

    TEST(range_trees, search_2d_narrow_weight_range) {
        int n = 10'000;
        std::vector <std::pair<int, int>> w;
        w.resize(n);
        std::vector<int> v;
        v.resize(n);
        iota(begin(v), end(v), 0);
        int sigma = static_cast<int>(log(n) / log(2));

        std::default_random_engine floatgen;
        std::uniform_real_distribution<double> floatdistr(0.0, sigma);

        std::shuffle(begin(v), end(v), std::default_random_engine());
        for (int i = 0; i < n; ++i)
            w[i] = {i, static_cast<int>(floatdistr(floatgen))};
        for (const auto &x: w)
            assert(0 <= x.second and x.second <= sigma);

        auto uptr = std::make_unique<range_tree < int, int>>(w);
        std::cerr << "Tree constructed" << std::endl;

        auto rng = std::make_unique<std::default_random_engine>();
        auto generator = std::make_unique<std::mt19937>();
        auto distribution = std::make_unique<std::uniform_int_distribution<int>>(0, n - 1);
        auto wdistr = std::make_unique<std::uniform_int_distribution<int>>(0, sigma);

        for (int qr = 0; qr < 0x400; ++qr) {
            int a = (*wdistr)(*generator),
                    b = (*wdistr)(*generator);
            int i = (*distribution)(*generator),
                    j = (*distribution)(*generator);
            auto qi = std::min(i, j), qj = std::max(i, j);
            auto qa = std::min(a, b), qb = std::max(a, b);
            // cerr << "["<<qi << "," << qj<<"]x[" <<qa<<","<<qb<<"]\n";
            auto bfres = brute_force_2dsearch(w, qi, qj, qa, qb);
            std::vector <std::pair<int, int>> rt_res;
            (*uptr).range_2d_reporting_query(qi, qj, qa, qb, rt_res);
            ASSERT_EQ(equal_vectors(rt_res, bfres), true);
        }
    }

    TEST( range_trees, search_2d_large_test ) {
        int n = 10'000;
        std::vector <std::pair<int, int>> w;
        w.resize(n);
        std::vector<int> v;
        v.resize(n);
        iota(begin(v), end(v), 0);
        std::shuffle(begin(v), end(v), std::default_random_engine());
        for (int i = 0; i < n; ++i)
            w[i] = {i, v[i]};
        auto uptr = std::make_unique<range_tree < int, int>>(w);
        std::cerr << "Tree constructed" << std::endl;

        auto rng = std::make_unique<std::default_random_engine>();
        auto generator = std::make_unique<std::mt19937>();
        auto distribution = std::make_unique<std::uniform_int_distribution<int>>(0, n - 1);
        auto wdistr = std::make_unique<std::uniform_int_distribution<int>>(0, n - 1);

        for (int qr = 0; qr < 0x400; ++qr) {
            int a = (*wdistr)(*generator),
                    b = (*wdistr)(*generator);
            int i = (*distribution)(*generator),
                    j = (*distribution)(*generator);
            auto qi = std::min(i, j), qj = std::max(i, j);
            auto qa = std::min(a, b), qb = std::max(a, b);
            // cerr << "["<<qi << "," << qj<<"]x[" <<qa<<","<<qb<<"]\n";
            auto bfres = brute_force_2dsearch(w, qi, qj, qa, qb);
            std::vector<std::pair<int, int>> rt_res;
            (*uptr).range_2d_reporting_query(qi, qj, qa, qb, rt_res);
            ASSERT_EQ(equal_vectors(rt_res, bfres), true);
        }
    }

    TEST(range_trees,counting_2d_large_test) {
        int n = 10'000;
        std::vector <std::pair<int, int>> w;
        w.resize(n);
        std::vector<int> v;
        v.resize(n);
        iota(begin(v), end(v), 0);
        std::shuffle(begin(v), end(v), std::default_random_engine());
        for (int i = 0; i < n; ++i)
            w[i] = {i, v[i]};
        auto uptr = std::make_unique<range_tree < int, int>>(w);
        std::cerr << "Tree constructed" << std::endl;

        auto rng = std::make_unique<std::default_random_engine>();
        auto generator = std::make_unique<std::mt19937>();
        auto distribution = std::make_unique<std::uniform_int_distribution<int>>(0, n - 1);
        auto wdistr = std::make_unique<std::uniform_int_distribution<int>>(0, n - 1);

        for (int qr = 0; qr < 0x400; ++qr) {
            int a = (*wdistr)(*generator),
                    b = (*wdistr)(*generator);
            int i = (*distribution)(*generator),
                    j = (*distribution)(*generator);
            auto qi = std::min(i, j), qj = std::max(i, j);
            auto qa = std::min(a, b), qb = std::max(a, b);
            // cerr << "["<<qi << "," << qj<<"]x[" <<qa<<","<<qb<<"]\n";
            auto bfres = brute_force_2dsearch(w, qi, qj, qa, qb);
            std::vector <std::pair<int, int>> rt_res;
            auto cnt = (*uptr).range_2d_counting_query(qi, qj, qa, qb);
            ASSERT_EQ(bfres.size(), cnt);
        }
    }

    TEST(range_trees,counting_2d_empty_answers) {
        int n = 10'000;
        std::vector <std::pair<int, int>> w;
        w.resize(n);
        std::vector<int> v;
        v.resize(n);
        iota(begin(v), end(v), 0);
        std::shuffle(begin(v), end(v), std::default_random_engine());
        for (int i = 0; i < n; ++i)
            w[i] = {i, n + v[i]};

        auto uptr = std::make_unique<range_tree < int, int>>(w);
        std::cerr << "Tree constructed" << std::endl;

        auto rng = std::make_unique<std::default_random_engine>();
        auto generator = std::make_unique<std::mt19937>();
        auto distribution = std::make_unique<std::uniform_int_distribution<int>>(0, n - 1);
        auto wdistr = std::make_unique<std::uniform_int_distribution<int>>(0, n - 1);

        for (int qr = 0; qr < 0x400; ++qr) {
            int a = (*wdistr)(*generator),
                    b = (*wdistr)(*generator);
            int i = (*distribution)(*generator),
                    j = (*distribution)(*generator);
            auto qi = std::min(i, j), qj = std::max(i, j);
            auto qa = std::min(a, b), qb = std::max(a, b);
            // cerr << "["<<qi << "," << qj<<"]x[" <<qa<<","<<qb<<"]\n";
            auto bfres = brute_force_2dsearch(w, qi, qj, qa, qb);
            std::vector <std::pair<int, int>> rt_res;
            auto cnt = (*uptr).range_2d_counting_query(qi, qj, qa, qb);
            ASSERT_EQ(bfres.size(), cnt);
        }
    }
}

int main() {
    testing::InitGoogleTest();
    return ::RUN_ALL_TESTS();
}

