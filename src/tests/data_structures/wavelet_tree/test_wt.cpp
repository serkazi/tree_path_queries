#include "gtest/gtest.h"
#include <algorithm>
#include <numeric>
#include <random>
#include "wavelet_tree.hpp"
#include <iostream>
#include <string>

namespace {
    int kth(const std::vector<int> &w, int i, int j, int k) {
        std::vector<int> s(begin(w) + i, begin(w) + j + 1);
        std::sort(begin(s), end(s));
        return s[k];
    }

    int kthmultiple(const std::vector<int> &w, const std::vector<std::pair<int, int>> &vec, int k) {
        std::vector<int> s;
        for (const auto &pr: vec) {
            for (int i = pr.first; i <= pr.second; ++i)
                s.push_back(w[i]);
        }
        sort(begin(s), end(s));
        return s[k];
    }

    TEST(wavelet_tree_test, small_random_test) {
        std::vector<int> w(100, 0);
        auto generator = std::make_unique<std::mt19937>();
        auto wdistr = std::make_unique<std::uniform_int_distribution<int>>(0, 50);
        for (auto &x: w)
            x = (*wdistr)(*generator);
        std::cerr << "w.size() = " << w.size() << std::endl;
        auto wt = std::make_unique<wavelet_tree<int, int>>(w);

        for (int l = 0; l < w.size(); ++l)
            for (int r = l; r < w.size(); ++r) {
                auto k = std::uniform_int_distribution<int>(0, r - l)(*generator);
                ASSERT_EQ(wt->range_quantile(l, r, k), kth(w, l, r, k));
            }
    }

    TEST(wavelet_tree_test, random_test_with_multiple_segments) {
        const int n = 10'000;

        std::vector<int> w;
        w.resize(n);
        auto generator = std::make_unique<std::mt19937>();
        auto wdistr = std::make_unique<std::uniform_int_distribution<int>>(0, n / 2);
        for (auto &x: w)
            x = (*wdistr)(*generator);
        std::cerr << "w.size() = " << w.size() << std::endl;
        auto wt = std::make_unique<wavelet_tree<int, int>>(w);

        //wavelet_tree<size_type, value_type>::range_quantile( vector<pair<size_type,size_type>> &vec, size_type k ) const {

        for (auto it = 0; it < 7; ++it) {
            std::vector<int> in, out;
            in.resize(n), iota(begin(in), end(in), 0);
            std::sample(in.begin(), in.end(), std::back_inserter(out), 26, std::mt19937{std::random_device{}()});
            std::vector<std::pair<int, int>> vec;
            // picking up some intervals along [0,n-1]
            int len = 0;
            for (int l = 0; l + 1 < out.size(); l += 2)
                vec.emplace_back(out[l], out[l + 1]), len += out[l + 1] - out[l] + 1;
            auto k = std::uniform_int_distribution<int>(0, len - 1)(*generator);
            ASSERT_EQ(wt->range_quantile(vec, k), kthmultiple(w, vec, k));
        }
    }

    TEST(wavelet_tree_test, random_test_with_trivial_segments) {
        const int n = 10'000;

        std::vector<int> w;
        w.resize(n);
        auto generator = std::make_unique<std::mt19937>();
        auto wdistr = std::make_unique<std::uniform_int_distribution<int>>(0, n / 2);
        for (auto &x: w)
            x = (*wdistr)(*generator);
        std::cerr << "w.size() = " << w.size() << std::endl;
        auto wt = std::make_unique<wavelet_tree<int, int>>(w);

        //wavelet_tree<size_type, value_type>::range_quantile( vector<pair<size_type,size_type>> &vec, size_type k ) const {

        for (auto it = 0; it < 7; ++it) {
            std::vector<int> in, out;
            in.resize(n), iota(begin(in), end(in), 0);
            std::sample(in.begin(), in.end(), std::back_inserter(out), 26, std::mt19937{std::random_device{}()});
            std::vector<std::pair<int, int>> vec;
            // picking up some intervals along [0,n-1]
            int len = 0;
            for (int l = 0; l < out.size(); ++l)
                vec.emplace_back(out[l], out[l]), ++len;
            auto k = std::uniform_int_distribution<int>(0, len - 1)(*generator);
            ASSERT_EQ(wt->range_quantile(vec, k), kthmultiple(w, vec, k));
        }
    }
}

int main() {
    testing::InitGoogleTest();
    return ::RUN_ALL_TESTS();
}
