#include <sstream>
#include "simple_tree.hpp"
#include "heavy_path_decomp.hpp"
#include "gtest/gtest.h"
#include <string>

template class heavy_path_decomp<int,int,int>;

namespace {
    TEST(hpd_tests,correctly_builds_chain) {
            std::istringstream str("((()())(()(()(()()))()))\n0 1 2 3 4 5 6 7 8 9 10 11");
            heavy_path_decomp<int, int, int> h(str);
            std::string geld("0 4 6 8 9 10 7 11 5 1 2 3 \n0 5 5 6 0 4 0 2 0 0 1 3 \n");
            std::ostringstream ost;
            ost << h;
            ASSERT_EQ(geld, ost.str());
    }
}

int main() {
    testing::InitGoogleTest();
    return ::RUN_ALL_TESTS();
}

