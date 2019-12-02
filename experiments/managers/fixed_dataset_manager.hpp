#ifndef TREE_PATH_QUERIES_FIXED_DATASET_MANAGER_HPP
#define TREE_PATH_QUERIES_FIXED_DATASET_MANAGER_HPP
#include <algorithm>
#include <memory>
#include <tuple>
#include "naive_processor.hpp"
#include "naive_processor_lca.hpp"
#include "pq_types.hpp"
#include "ext_ptr.hpp"
#include "query_stream_builder.hpp"
#include "wt_hpd.hpp"
#include "sdsl/util.hpp"
#include "fixed_processor_manager.hpp"
#include "nsrs.hpp"
#include "tree_ext_sct.hpp"
#include "hybrid_processor.hpp"

namespace experiments {
    enum class IMPLS {
        NV=                 1 << 0,
        NV_LCA=             1 << 1,
        NV_SUCC=            1 << 2,
        HYBRID=             1 << 3,
        TREE_EXT_PTR=       1 << 4,
        WT_HPD_UN=          1 << 5,
        WT_HPD_RRR=         1 << 6,
        TREE_EXT_SCT_UN=    1 << 7,
        TREE_EXT_SCT_RRR=   1 << 8
    };
    const std::string hrnames[]= {"nv","nvlca","nvsucc","hybrid","treeextptr","wthpdun","wthpdrrr",
                                  "treeextsctun","treeextsctrrr"};

}

/**
 * @details the rational is that for a fixed set of queries,
 * all the relevant implementations should be run
 * @tparam node_type
 * @tparam size_type
 * @tparam value_type
 */
template<typename node_type,typename size_type,typename value_type>
class fixed_dataset_manager {

private:

    template<typename P>
    using processor_holder= std::unique_ptr<fixed_processor_manager<node_type,size_type,value_type,P>>;

    using nv                 = naive_processor<node_type,size_type,value_type>;
    using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
    using nv_succ            = nsrs<node_type,size_type,value_type>;
    using hybrid             = hybrid_processor<node_type,size_type,value_type>;
    using tree_ext_ptr       = ext_ptr<node_type,size_type,value_type>;
    using wt_hpd_uncompressed= wt_hpd<
                                        node_type,size_type,value_type,
                                        bp_tree_sada<node_type,size_type>,
                                        sdsl::bit_vector,sdsl::rank_support_v5<>,
                                        sdsl::select_support_mcl<1,1>,
                                        sdsl::select_support_mcl<0,1>
                                    >;
    using wt_hpd_rrr         = wt_hpd<
                                        node_type,size_type,value_type,
                                        bp_tree_sada<node_type,size_type>,
                                        sdsl::rrr_vector<>
                                     >;
    using tree_ext_sct_un    = tree_ext_sct<
                                        node_type,size_type,value_type,
                                        sdsl::bp_support_sada<>,2,
                                        sdsl::bit_vector ,
                                        sdsl::rank_support_v5<>,
                                        sdsl::select_support_mcl<1,1>,
                                        sdsl::select_support_mcl<0,1>
                                        >;
    using tree_ext_sct_rrr  = tree_ext_sct<
                                        node_type,size_type,value_type,
                                        sdsl::bp_support_sada<>,2,
                                        sdsl::rrr_vector<>
                                        >;
    std::tuple<
                processor_holder<nv>,processor_holder<nv_lca>,processor_holder<nv_succ>,
                processor_holder<hybrid>,processor_holder<tree_ext_ptr>,
                processor_holder<wt_hpd_uncompressed>,processor_holder<wt_hpd_rrr>,
                processor_holder<tree_ext_sct_un>,processor_holder<tree_ext_sct_rrr>
              >
              processor_managers;

    std::string description;
    value_type a,b;
    size_type n;
    std::string topology;
    std::vector<value_type> w;

    uint16_t mask= 0;

public:

    explicit fixed_dataset_manager( std::istream &is, uint16_t mask, const std::string &descr= "N/A" ) {
        this->mask= mask;
        std::string s; is >> topology;
        w.resize(topology.size()/2);
        for ( auto &x: w )
            is >> x;
        n= s.size()/2, a= *(std::min_element(begin(w),end(w))), b= *(std::max_element(begin(w),end(w)));
        description= descr;
    }

    /**
     * @details the same set of queries should be run on all the processors
     * @param configs, a JSON object saying how many of each sort of queries to perform
     * for counting and reporting, it also contains the "K" parameter
     * @return
     */
    nlohmann::json run_config( nlohmann::json configs ) {
        query_stream_builder<node_type,size_type,value_type> builder;
        builder.set_node_range(n).set(path_queries::QUERY_TYPE::MEDIAN,configs["median"]);
        builder.set_weight_range(a,b).set(path_queries::QUERY_TYPE::COUNTING,configs["counting"]);
        builder.set(path_queries::QUERY_TYPE::REPORTING,configs["reporting"]);
        builder.set(path_queries::QUERY_TYPE::SELECTION,configs["selection"]);
        builder.set_scaling_param(configs["K"]);
        /**
         * create some temporary file, feed the queries into it
         * TODO: look how Gog uses this id() in his own code
         */
         std::string filename= "query_file_"+std::to_string(sdsl::util::id())+
                 "_on_"+std::to_string(sdsl::util::pid())+".json";
         std::ofstream os(filename);
         builder.build();
         for ( const auto &q: builder )
             os << q << '\n';
         os.close();
         nlohmann::json res, per_datastructure;
         for ( int i= 0; i < 9; ++i ) {
             if ( not (mask & (1u<<i)) ) continue ;
             switch ( 1 << i ) {
                 case static_cast<int>(experiments::IMPLS::NV):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<nv>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::NV_LCA):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<nv_lca>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::NV_SUCC):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<nv_succ>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::HYBRID):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<hybrid>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::TREE_EXT_PTR):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<tree_ext_ptr>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::WT_HPD_UN):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<wt_hpd_uncompressed>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::WT_HPD_RRR):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<wt_hpd_rrr>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::TREE_EXT_SCT_UN):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<tree_ext_sct_un>>(topology,w);
                     break ;
                 case static_cast<int>(experiments::IMPLS::TREE_EXT_SCT_RRR):
                     std::get<i>(processor_managers)= std::make_unique<processor_holder<tree_ext_sct_rrr>>(topology,w);
                     break ;
             }
             std::ifstream is(filename);
             per_datastructure[experiments::hrnames[i]]= std::get<i>(processor_managers)->invoke_with(is);
         }
         res["dataset"]= description;
         res["config"]= configs;
         res["results"]= per_datastructure;
         return res;
    }
};
#endif //TREE_PATH_QUERIES_FIXED_DATASET_MANAGER_HPP
