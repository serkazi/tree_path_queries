//
// Created by sj on 18/11/19.
//
#include "gtest/gtest.h"
#include "experimental_run.hpp"
#include <sstream>
#include <nlohmann/json.hpp>

TEST(obj_test,CorrectlyReadsFromStream) {
    nlohmann::json obj;
    obj["name"]= "xyz", obj["space"]= 12.53, obj["time"]= 0.07, obj["type"]= 2;
    obj["hash"]= "pqr";
    std::stringstream str;
    str << obj;
    experimental_run e;
    str >> e;
    ASSERT_EQ(e.ds_name, obj["name"]);
    ASSERT_EQ(e.total_space, obj["space"]);
    ASSERT_EQ(e.total_time, obj["time"]);
    ASSERT_EQ(e.type, static_cast<path_queries::QUERY_TYPE>(obj["type"]));
    ASSERT_EQ(e.sha512hash, obj["hash"]);
}

TEST(obj_test,CorrectlyOutputsToStream) {
    experimental_run_builder builder;
    auto e=
            builder.set_hash("pqr")
                .set_name("xyz")
                .set_total_space(12.53)
                .set_total_time(0.07)
                .set_type(path_queries::QUERY_TYPE::MEDIAN)
                .build();

    std::stringstream str;
    str << e;
    nlohmann::json obj;
    str >> obj;

    ASSERT_EQ(e.ds_name, obj["name"]);
    ASSERT_EQ(e.total_space, obj["space"]);
    ASSERT_EQ(e.total_time, obj["time"]);
    ASSERT_EQ(e.type, static_cast<path_queries::QUERY_TYPE>(obj["type"]));
    ASSERT_EQ(e.sha512hash, obj["hash"]);
}

TEST(obj_test,DoesNotBuildPartialObject) {
    experimental_run_builder builder;
            builder.set_hash("pqr")
                    .set_name("xyz")
                    .set_total_space(12.53)
                    /*.set_total_time(0.07)*/
                    .set_type(path_queries::QUERY_TYPE::MEDIAN);
    ASSERT_FALSE(builder.object_is_sane());
}

TEST(obj_test,QueryTypeCastsWork) {
    experimental_run_builder builder;
    auto e=
            builder.set_hash("pqr")
                    .set_name("xyz")
                    .set_total_space(12.53)
                    .set_total_time(0.07)
                    .set_type(path_queries::QUERY_TYPE::SELECTION)
                    .build();
    std::stringstream str;
    str << e;
    nlohmann::json obj;
    str >> obj;
    obj["type"]= 3;

    ASSERT_EQ(e.ds_name, obj["name"]);
    ASSERT_EQ(e.total_space, obj["space"]);
    ASSERT_EQ(e.total_time, obj["time"]);
    ASSERT_NE(e.type, static_cast<path_queries::QUERY_TYPE>(obj["type"]));
    ASSERT_EQ(e.sha512hash, obj["hash"]);
}

int main( int argc, char **argv ) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
