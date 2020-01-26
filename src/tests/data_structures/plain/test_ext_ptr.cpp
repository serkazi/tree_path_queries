#include <random>
#include <sys/resource.h>
#include <regex>
#include <fstream>
#include "gtest/gtest.h"
#include "raw_processor.hpp"
#include "ext_ptr.hpp"
#include "pq_types.hpp"
#include <functional>

namespace {

    using node_type= pq_types::node_type;
    using size_type= pq_types::size_type;
    using value_type= pq_types::value_type;
    const int ITERS= 0x400;

    const std::string root_dir= "/users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/";
    const std::string paths[]= {
            /*
            std::string("degenerate_tree_equal_weights.txt"),
            std::string("log_weights.txt"),
            std::string("sqrt_weights.txt"),
            std::string("linear_small_weights.txt"),
            std::string("linear_weights.txt")
            std::string("us.rd.d.dfs.dimacs.puu")*/
            std::string("eu.d.mst.dimacs.puu"),
            std::string("eu.emst.dem.puu"),
            std::string("eu.mst.osm.puu"),
            std::string("mars.emst.dem.puu"),
            std::string("us.rd.d.dfs.dimacs.puu"),
            std::string("rnd.100mln.sqrt.puu")
    };

    using holder= std::shared_ptr<ext_ptr<node_type,size_type,value_type>>;

    bool equal_multisets(
            const std::vector<std::pair<value_type,size_type>> &a,
            const std::vector<std::pair<value_type,size_type>> &b )
    {
        std::multiset<size_type> s,t;
        std::for_each(a.begin(),a.end(),[&s]( const auto &z ) {
            s.insert(z.second);
        });
        std::for_each(b.begin(),b.end(),[&t]( const auto &z ) {
            t.insert(z.second);
        });
        if ( s != t ) {
            for ( auto &it: s )
                std::cerr << it << ", ";
            std::cerr << std::endl;
            for ( auto &it: t )
                std::cerr << it << ", ";
            std::cerr << std::endl;
        }
        return s == t;
    }

    class ext_ptr_test: public testing::TestWithParam<std::string> {
    public:

        using key_type= std::string;
        key_type pathname;

        static std::map<key_type,holder> processors;
        static std::map<key_type,std::shared_ptr<path_query_processor<node_type, size_type, value_type>>> raws;
        static std::map<key_type,size_type> n;
        static std::map<key_type,std::vector<value_type>> w;
        static std::map<key_type,std::vector<value_type>> sorted_weights;
        static std::map<key_type,std::default_random_engine> gen_nodes, gen_weights, gen_qntl;
        static std::map<key_type,std::shared_ptr<std::uniform_int_distribution<node_type>>> nodes_distr;
        static std::map<key_type,std::shared_ptr<std::uniform_int_distribution<size_type>>> qntl_distr;
        static std::map<key_type,std::shared_ptr<std::uniform_int_distribution<value_type>>> weights_distr;

        void SetUp() override {
            if ( not processors.count(GetParam()) ) {
                pathname= GetParam();
                auto key= GetParam();
                std::ifstream is(GetParam());
                std::string s;
                is >> s;
                w[key].resize(s.size() / 2);
                for (auto &x: w[key])
                    is >> x;
                sorted_weights[key]= w[key], std::sort(sorted_weights[key].begin(),sorted_weights[key].end());
                n[key] = s.size() / 2;
                processors[key] =
                        std::make_shared<ext_ptr<node_type, size_type, value_type>>(s, w[key]);
                raws[key]= std::make_shared<raw_processor<node_type, size_type, value_type>>(s, w[key]);
                nodes_distr[key] = std::make_shared<std::uniform_int_distribution<node_type>>(0, n[key]-1);
                qntl_distr[key] = std::make_shared<std::uniform_int_distribution<size_type>>(1, 100);
                value_type lower= *(min_element(begin(w[key]), end(w[key]))), upper= *(max_element(begin(w[key]),end(w[key])));
                weights_distr[key]= std::make_shared<std::uniform_int_distribution<value_type>>(0, n[key]-1);
            }
        }
        void TearDown() override {
            auto key= pathname;
            if ( processors.count(key) ) {
                processors[key].reset(), raws[key].reset();
                nodes_distr[key]->reset(), qntl_distr[key]->reset(), weights_distr[key]->reset();
                processors.erase(key);
            }
        }
    };

    std::map<std::string,holder> ext_ptr_test::processors;
    std::map<std::string,std::shared_ptr<path_query_processor<node_type, size_type, value_type>>> ext_ptr_test::raws;
    std::map<std::string,size_type> ext_ptr_test::n;
    std::map<std::string,std::vector<value_type>> ext_ptr_test::w;
    std::map<std::string,std::vector<value_type>> ext_ptr_test::sorted_weights;
    std::map<std::string,std::default_random_engine> ext_ptr_test::gen_nodes, ext_ptr_test::gen_weights, ext_ptr_test::gen_qntl;
    std::map<std::string,std::shared_ptr<std::uniform_int_distribution<node_type>>> ext_ptr_test::nodes_distr;
    std::map<std::string,std::shared_ptr<std::uniform_int_distribution<size_type>>> ext_ptr_test::qntl_distr;
    std::map<std::string,std::shared_ptr<std::uniform_int_distribution<value_type>>> ext_ptr_test::weights_distr;

    TEST_P(ext_ptr_test,all_queries_correct) {
        auto ndistr= this->nodes_distr[pathname];
        auto genn= this->gen_nodes[pathname];
        auto wdistr= this->weights_distr[pathname];
        auto gw= this->gen_weights[pathname];

        {
            auto nodes_dice = std::bind(*(ndistr), genn);
            auto weights_dice = std::bind(*(wdistr), gw);
            for (auto it = 0; it < ITERS; ++it) {
                assert(this->w[pathname].size() >= this->n[pathname]);
                auto x = nodes_dice(), y = nodes_dice();
                auto vl = sorted_weights[pathname][weights_dice()], vr = sorted_weights[pathname][weights_dice()];
                if (vl > vr) std::swap(vl, vr);
                std::vector<std::pair<value_type, size_type>> answer1, answer2;
                this->processors[pathname]->report(x, y, static_cast<value_type>(vl), static_cast<value_type>(vr),
                                                   answer1);
                this->raws[pathname]->report((node_type) x, (node_type) y, vl, vr, answer2);
                ASSERT_EQ(equal_multisets(answer1, answer2), true);
            }
        }

        {
            auto nodes_dice = std::bind(*(ndistr), genn);
            auto weights_dice = std::bind(*(wdistr), gw);
            for (auto it = 0; it < ITERS; ++it) {
                assert(this->w[pathname].size() >= this->n[pathname]);
                auto x = nodes_dice(), y = nodes_dice();
                auto vl = sorted_weights[pathname][weights_dice()], vr = sorted_weights[pathname][weights_dice()];
                if (vl > vr) std::swap(vl, vr);
                auto ans1 = this->processors[pathname]->count(x, y, static_cast<value_type>(vl),
                                                              static_cast<value_type>(vr));
                auto ans2 = this->raws[pathname]->count((node_type) x, (node_type) y, vl, vr);
                ASSERT_EQ(ans1, ans2);
            }
        }

        {
            auto nodes_dice= std::bind(*(this->nodes_distr[pathname]),this->gen_nodes[pathname]);
            for ( auto it= 0; it < ITERS; ++it ) {
                auto x= nodes_dice(), y= nodes_dice();
                assert( this->w[pathname].size() >= this->n[pathname] );
                auto ans1= this->processors[pathname]->query(x,y);
                auto ans2= this->raws[pathname]->query((node_type)x,(node_type)y);
                ASSERT_EQ(ans1,ans2);
            }
        }

        {
            auto nodes_dice= std::bind(*(this->nodes_distr[pathname]),this->gen_nodes[pathname]);
            auto qntl_dice= std::bind(*(this->qntl_distr[pathname]),this->gen_qntl[pathname]);
            for ( auto it= 0; it < ITERS; ++it ) {
                auto x= nodes_dice(), y= nodes_dice();
                assert( this->w[pathname].size() >= this->n[pathname] );
                auto qntl= qntl_dice();
                auto ans1= this->processors[pathname]->selection(x,y,qntl);
                auto ans2= this->raws[pathname]->selection((node_type)x,(node_type)y,qntl);
                ASSERT_EQ(ans1,ans2);
            }
        }
    }

    //INSTANTIATE_TEST_CASE_P(testSuiteP,wthpd_test,testing::ValuesyIn(paths));
}

INSTANTIATE_TEST_SUITE_P(testSuiteP,
                         ext_ptr_test,
                         testing::Values(
                                 root_dir+paths[0],
                                 root_dir+paths[1],
                                 root_dir+paths[2],
                                 root_dir+paths[3],
                                 root_dir+paths[4],
                                 root_dir+paths[5]
                         ),
                         [](const testing::TestParamInfo<ext_ptr_test::ParamType>& info) {
                             std::string name= info.param;
                             std::regex r("([^/]+)$");
                             std::smatch m;
                             std::regex_search(name,m,r);
                             auto x= m.str(1);
                             return std::accumulate(x.begin(),x.end(),std::string(""),[](std::string acc, char ch) {
                                 if ( ch == '_' or ch == '.' )
                                     return acc;
                                 return acc+ch;
                             });
                         });

int main( int argc, char **argv ) {
    const rlim_t kStackSize = 20 * 1024ll * 1024ll * 1024ll;
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if ( result != 0 ) {
                std::cerr << "setrlimit returned result = " << result << std::endl;
            }
        }
    }
    testing::InitGoogleTest(&argc,argv);
    return ::RUN_ALL_TESTS();
}
