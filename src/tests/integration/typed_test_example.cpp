//
// Created by kazi on 2019-11-27.
//

#include "gtest/gtest.h"
#include <memory>

template<class K, class V>
class Bar {
public:
    virtual ~Bar()= default;
};

namespace {
template<typename T>
class MyFooTest: public testing::Test {
    std::unique_ptr<T> x;
    std::unique_ptr<Bar<int,char>> bar;
public:
    MyFooTest() {
        x= std::make_unique<T>();
        bar= std::make_unique<Bar<int,char>>();
    }
    ~MyFooTest() override { x= nullptr; };
};

template class MyFooTest<bool>;
template class MyFooTest<int>;

#if GTEST_HAS_TYPED_TEST_P

using testing::Types;

TYPED_TEST_SUITE_P(MyFooTest);

TYPED_TEST_P( MyFooTest, ConstructorDestructor) {
    ASSERT_EQ(42,42);
}

REGISTER_TYPED_TEST_SUITE_P(MyFooTest,ConstructorDestructor);

typedef ::testing::Types<MyFooTest<int>,MyFooTest<bool>> impls;
INSTANTIATE_TYPED_TEST_SUITE_P( impl_test, MyFooTest, impls);

#endif  // GTEST_HAS_TYPED_TEST_P
}

int main( int argc, char **argv ) {
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}


