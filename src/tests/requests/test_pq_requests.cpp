//
// Created by sj on 21/11/19.
//
#include "gtest/gtest.h"
#include "../../../include/requests/pq_request.hpp"

TEST( to_json, CorrectlySeralizesToJson ) {
    path_queries::counting_query<pq_types::node_type,pq_types::size_type,pq_types::value_type> c(13,17,99,100);
    nlohmann::json gold;
    gold["type"]= "counting", gold["x"]= 13, gold["y"]= 17, gold["a"]= 99, gold["b"]= 100;
    auto res= path_queries::to_json<pq_types::node_type,pq_types::size_type,pq_types::value_type>(c);
    ASSERT_EQ(res,gold);
    gold["type"]= "xyz", gold["x"]= 13, gold["y"]= 17, gold["a"]= 99, gold["b"]= 100;
    ASSERT_NE(res,gold);
}

int main( int argc, char **argv ) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
