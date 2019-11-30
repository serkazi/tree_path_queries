//
// Created by sj on 16/12/18.
//
#include "gtest/gtest.h"
#include "succinct_tree.hpp"
#include "simple_tree.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <memory>
#include <string>
#include <algorithm>
#include <functional>
#include <bp_tree_sada.hpp>
#include <gtest/gtest-typed-test.h>
#include <gtest-typed-test.h>

namespace {
    const int ITERS = 0x400;

    const std::string paths[] = {
            std::string("degenerate_tree_equal_weights.txt"),
            std::string("log_weights.txt"),
            std::string("sqrt_weights.txt"),
            std::string("linear_small_weights.txt"),
            std::string("linear_weights.txt"),
            std::string("us.rd.d.dfs.dimacs.puu")
    };


    template<class T>
    class SuccinctTreeImplementation : public testing::Test {
    public:
        std::string filename = paths[1];
        std::unique_ptr<tree<int,int,int>> raw;
        std::unique_ptr<T> scnt_tree;
        int n;
        std::default_random_engine gen_nodes;
        std::string backbone;
        std::unique_ptr<std::uniform_int_distribution<int>> nodes_distr;
        std::vector<int> weights;
        SuccinctTreeImplementation() {
            std::ifstream in(this->filename);
            //std::cerr << "Reading file: " << this->filename << "\n";
            in >> backbone;
            n = backbone.size() / 2;
            in.close();
            weights.resize(n);
            raw = std::make_unique<tree<int,int,int>>(backbone, weights);
            scnt_tree = std::make_unique<T>(backbone);
            nodes_distr = std::make_unique<std::uniform_int_distribution<int>>(0,n-1);
        }
        virtual ~SuccinctTreeImplementation() = default;
    };

#if GTEST_HAS_TYPED_TEST_P

    using testing::Types;

    TYPED_TEST_SUITE_P(SuccinctTreeImplementation);

    TYPED_TEST_P( SuccinctTreeImplementation, SupportsLCACorrectly ) {
        auto nodes_dice = std::bind(*(this->nodes_distr), this->gen_nodes);
        for (int it = 0; it < ITERS; ++it) {
            int x = nodes_dice(), y = nodes_dice();
            int ans1 = this->scnt_tree->lca(static_cast<int>(x), static_cast<int>(y));
            int ans2 = this->raw->lca((int) x, (int) y);
            ASSERT_EQ(ans1, ans2);
        }
    }

    TYPED_TEST_P(SuccinctTreeImplementation, HasCorrectChildrenList) {
        auto nodes_dice = std::bind(*(this->nodes_distr), this->gen_nodes);
        for (auto it = 0; it < ITERS; ++it) {
            auto x = nodes_dice();
            auto ans1 = this->scnt_tree->children(static_cast<int>(x));
            auto ans2 = this->raw->kinder((int) x);
            ASSERT_EQ(ans1.size(), ans2.size());
            std::sort(ans1.begin(), ans1.end());
            std::sort(ans2.begin(), ans2.end());
            for (auto l = 0; l < ans1.size(); ++l)
                ASSERT_EQ(ans1[l], ans2[l]);
            ASSERT_EQ(ans1, ans2);
        }
    }

    TYPED_TEST_P(SuccinctTreeImplementation, HasCorrectDepth) {
        auto nodes_dice = std::bind(*(this->nodes_distr), this->gen_nodes);
        for (auto it = 0; it < ITERS; ++it) {
            auto x = nodes_dice();
            auto ans1 = this->scnt_tree->depth(static_cast<int>(x));
            auto ans2 = this->raw->depth((int) x);
            ASSERT_EQ(ans1, ans2);
        }
    }

    TYPED_TEST_P(SuccinctTreeImplementation, HasCorrectBPS) {
        std::ostringstream ost;
        ost << *(this->scnt_tree);
        ASSERT_EQ(this->backbone, ost.str());
    }

    REGISTER_TYPED_TEST_CASE_P(SuccinctTreeImplementation,SupportsLCACorrectly,HasCorrectDepth,HasCorrectChildrenList,HasCorrectBPS);

//typedef Types<tree_extraction_mh_flat<>,tree_extraction_flat<>,bp_tree_g<>,bp_tree_gg<>,bp_tree_sada<>> impls;
    //using bptreesada= bp_tree_sada<int,int>;
    //typedef ::testing::Types<bptreesada> impls;
    // INSTANTIATE_TYPED_TEST_SUITE_P( impl_test, SuccinctTreeImplementation, impls );
    //INSTANTIATE_TYPED_TEST_CASE_P( impl_test, SuccinctTreeImplementation, impls );

#endif  // GTEST_HAS_TYPED_TEST_P
}

int main( int argc, char **argv ) {
	testing::InitGoogleTest(&argc,argv);
	return RUN_ALL_TESTS();
}

