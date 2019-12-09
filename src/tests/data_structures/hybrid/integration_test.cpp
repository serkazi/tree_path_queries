#include <random>
#include <fstream>
#include "gtest/gtest.h"
#include "raw_processor.hpp"
#include "hybrid_processor.hpp"
#include "pq_types.hpp"

namespace {

    using node_type= pq_types::node_type;
    using size_type= pq_types::size_type;
    using value_type= pq_types::value_type;
    const int ITERS= 0x400;

    const std::string root_dir= "/users/grad/kazi/CLionProjects/tree_path_queries/data/testdata/";
    const std::string paths[]= {
            std::string("degenerate_tree_equal_weights.txt"),
            std::string("log_weights.txt"),
            std::string("sqrt_weights.txt"),
            std::string("linear_small_weights.txt"),
            std::string("linear_weights.txt"),
            std::string("us.rd.d.dfs.dimacs.puu")
    };

    using holder= std::unique_ptr<hybrid_processor<node_type,size_type,value_type>>;

    class hybrid_processor_test: public testing::Test {
    protected:
        size_type n;
        std::vector<value_type> w;
        std::default_random_engine gen_nodes, gen_weights, gen_qntl;
        std::unique_ptr<std::uniform_int_distribution<node_type>> nodes_distr;
        std::unique_ptr<std::uniform_int_distribution<size_type>> qntl_distr;
        std::unique_ptr<std::uniform_int_distribution<value_type>> weights_distr;
        holder processor= nullptr;
        std::unique_ptr<path_query_processor<node_type,size_type,value_type>> raw= nullptr;

        void SetUp() override {
            std::ifstream is(root_dir+paths[2]);
            std::string s; is >> s;
            w.resize(s.size()/2);
            for ( auto &x: w )
                is >> x;
            n= s.size()/2;
            processor=
                    std::make_unique<hybrid_processor<node_type,size_type,value_type>>(s,w);
            raw= std::make_unique<raw_processor<node_type,size_type,value_type>>(s,w);
            nodes_distr= std::make_unique<std::uniform_int_distribution<node_type>>(0,n-1);
            qntl_distr= std::make_unique<std::uniform_int_distribution<size_type>>(1,100);
            value_type lower= *(min_element(begin(w),end(w))), upper= *(max_element(begin(w),end(w)));
            weights_distr= std::make_unique<std::uniform_int_distribution<value_type>>(lower,upper);
        }
        void TearDown() override {};
    };

    TEST_F(hybrid_processor_test,counting_correct) {
        auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
        auto weights_dice= std::bind(*(this->weights_distr),this->gen_weights);
        for ( auto it= 0; it < ITERS; ++it ) {
            assert( this->w.size() >= this->n );
            auto x= nodes_dice(), y= nodes_dice();
            auto vl= weights_dice(), vr= weights_dice();
            if ( vl > vr ) std::swap(vl,vr);
            auto ans1= this->processor->count(x,y,static_cast<value_type>(vl),static_cast<value_type>(vr));
            auto ans2= this->raw->count((node_type)x,(node_type)y,vl,vr);
            ASSERT_EQ(ans1,ans2);
        }
    }

    TEST_F( hybrid_processor_test, median_is_correct ) {
        auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
        for ( auto it= 0; it < ITERS; ++it ) {
            auto x= nodes_dice(), y= nodes_dice();
            assert( this->w.size() >= this->n );
            auto ans1= this->processor->query(x,y);
            auto ans2= this->raw->query((node_type)x,(node_type)y);
            ASSERT_EQ(ans1,ans2);
        }
    };

    TEST_F( hybrid_processor_test, selection_is_correct ) {
        auto nodes_dice= std::bind(*(this->nodes_distr),this->gen_nodes);
        auto qntl_dice= std::bind(*(this->qntl_distr),this->gen_qntl);
        for ( auto it= 0; it < ITERS; ++it ) {
            auto x= nodes_dice(), y= nodes_dice();
            assert( this->w.size() >= this->n );
            auto qntl= qntl_dice();
            auto ans1= this->processor->selection(x,y,qntl);
            auto ans2= this->raw->selection((node_type)x,(node_type)y,qntl);
            ASSERT_EQ(ans1,ans2);
        }
    };
    //INSTANTIATE_TEST_CASE_P(testSuiteP,wthpd_test,testing::ValuesyIn(paths));
    //INSTANTIATE_TEST_CASE_P(testSuiteP,wthpd_test,testing::Values(paths[0],paths[1]));
}

int main( int argc, char **argv ) {
    testing::InitGoogleTest(&argc,argv);
    return ::RUN_ALL_TESTS();
}
