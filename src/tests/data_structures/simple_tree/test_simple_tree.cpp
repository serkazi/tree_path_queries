#include "simple_tree.hpp"
#include <string>
#include <memory>
#include "gtest/gtest.h"

template class tree<int,int,int>;
/**
 * A simple tree with structure as below, and few sanity checks
 *       0
 *      / \
 *     /  /\
 *         |
 */
namespace Test {
    TEST(simple_tree, constructs_and_destroys) {
        std::stringstream str(std::string("((())(()(())))\n1 2 3 4 5 6 7"));
        std::unique_ptr<tree<int, int, int>> t = std::make_unique<tree<int, int, int>>(str);
        ASSERT_EQ(t->size(),7);
        ASSERT_EQ(t->parent(5),3);
        auto sz_before= t->size_in_bytes();
        t->shed_redudancy();
        auto sz_after= t->size_in_bytes();
        ASSERT_LE(sz_after,sz_before);
        ASSERT_EQ(t->depth(6),3);
        t= nullptr;
    }
}

int main( int argc, char **argv ) {
    testing::InitGoogleTest(&argc,argv);
    return ::RUN_ALL_TESTS();
}