//
// Created by kazi on 2019-11-26.
//

#include "gtest/gtest.h"
#include "ff_bitvector.hpp"
#include <memory>

namespace {
    TEST(ff_bitvector, access_is_correct) {
        sdsl::bit_vector bv(5);
        bv[0]= 1; bv[3]= 1;
        auto ff= std::make_unique<ff_bitvector<>>(&bv);
        for ( int i= 0; i < 5; ++i )
            ASSERT_EQ((*ff)[i],bv[i]);
    }
}

int main( int argc, char **argv ) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}