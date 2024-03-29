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
#include "wt_hpd_ptr.hpp"
#include "bp_trees.hpp"

namespace experiments {

    enum class IMPLS {
        NV=                 1 << 0,
        NV_LCA=             1 << 1,
        NV_SUCC=            1 << 2,
        TREE_EXT_PTR=       1 << 3,
        WT_HPD_PTR=         1 << 4,
        WT_HPD_UN=          1 << 5,
        WT_HPD_RRR=         1 << 6,
        TREE_EXT_SCT_UN=    1 << 7,
        TREE_EXT_SCT_RRR=   1 << 8
    };

    unsigned operator | ( const IMPLS &a, const IMPLS &b ) {
        return static_cast<unsigned>(a)|static_cast<unsigned>(b);
    }
    unsigned operator | ( unsigned int a, const IMPLS &b ) {
        return static_cast<unsigned>(a)|static_cast<unsigned>(b);
    }
    unsigned operator | ( const IMPLS &a, unsigned b ) {
        return static_cast<unsigned>(a)|static_cast<unsigned>(b);
    }

    // human-readable names
    const std::vector<std::string> hrnames= {"nv","nv_lca","nsrs",
                                  "tree_ext_ptr", "wt_hpd_rrr", "wt_hpd_un","wt_hpd_rrr",
                                  "tree_ext_sct_un","tree_ext_sct_rrr"};

    template<
            typename node_type,
            typename size_type,
            typename value_type
    >
    std::unique_ptr<path_query_processor<node_type,size_type,value_type>>
    instantiate( const std::string &topology, const std::vector<value_type> &w, uint16_t i ) {

		using wt_hpd_un= wt_hpd<node_type,size_type,value_type,
				bp_trees::bp_gg_fast<node_type,size_type>,
				sdsl::bit_vector,
				sdsl::rank_support_v5<>,
				sdsl::select_support_mcl<1,1>,
				sdsl::select_support_mcl<0,1>
		>;
		using tree_ext_sct_un= tree_ext_sct<
				node_type,size_type,value_type,
				sdsl::bp_support_gg<>,
				2,
				sdsl::bit_vector ,
				sdsl::rank_support_v5<>,
				sdsl::select_support_mcl<1,1>,
				sdsl::select_support_mcl<0,1>
		>;
		using wt_hpd_rrr= wt_hpd<
				node_type,size_type,value_type,
				bp_trees::bp_gg_fast<node_type,size_type>,
				sdsl::rrr_vector<>
		>;
		using tree_ext_sct_rrr= tree_ext_sct<
				node_type,size_type,
				value_type,
				sdsl::bp_support_gg<>,
				2,
				sdsl::rrr_vector<>
		>;

		using nv                 = naive_processor<node_type,size_type,value_type>;
		using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
		using nv_succ            = nsrs<node_type,size_type,value_type>;
		using wt_hp_ptr          = wt_hpd_ptr<node_type,size_type,value_type>;
		using tree_ext_ptr       = ext_ptr<node_type,size_type,value_type>;


        // some shorthands for further use
		/*
        using nv                 = naive_processor<node_type,size_type,value_type>;
        using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
        using nv_succ            = nsrs<node_type,size_type,value_type>;
        using wt_hp_ptr          = wt_hpd_ptr<node_type,size_type,value_type>;
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
		*/

        try {
            switch (i) {
                case static_cast<int>(experiments::IMPLS::NV): {
                    return std::move(std::make_unique<nv>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::NV_LCA): {
                    return std::move(std::make_unique<nv_lca>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::NV_SUCC): {
                    return std::move(std::make_unique<nv_succ>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::WT_HPD_PTR ): {
                    return std::move(std::make_unique<wt_hp_ptr>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::TREE_EXT_PTR): {
                    return std::move(std::make_unique<tree_ext_ptr>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::WT_HPD_UN): {
                    return std::move(std::make_unique<wt_hpd_un>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::WT_HPD_RRR): {
                    return std::move(std::make_unique<wt_hpd_rrr>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::TREE_EXT_SCT_UN): {
                    return std::move(std::make_unique<tree_ext_sct_un>(topology, w));
                }
                case static_cast<int>(experiments::IMPLS::TREE_EXT_SCT_RRR): {
                    return std::move(std::make_unique<tree_ext_sct_rrr>(topology, w));
                }
                default:
                    assert(false);
            }
        } catch ( std::exception &e ) {
            std::cerr << e.what() << std::endl;
            throw e;
        }
        return nullptr;
    }
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

using wt_hpd_un= wt_hpd<node_type,size_type,value_type,
	  	bp_trees::bp_gg_fast<node_type,size_type>,
        sdsl::bit_vector,
		sdsl::rank_support_v5<>,
        sdsl::select_support_mcl<1,1>,
        sdsl::select_support_mcl<0,1>
>;
using tree_ext_sct_un= tree_ext_sct<
        node_type,size_type,value_type,
        sdsl::bp_support_gg<>,
		2,
        sdsl::bit_vector ,
        sdsl::rank_support_v5<>,
        sdsl::select_support_mcl<1,1>,
        sdsl::select_support_mcl<0,1>
>;
using wt_hpd_rrr= wt_hpd<
        node_type,size_type,value_type,
	  	bp_trees::bp_gg_fast<node_type,size_type>,
        sdsl::rrr_vector<>
>;
using tree_ext_sct_rrr= tree_ext_sct<
        node_type,size_type,
		value_type,
        sdsl::bp_support_gg<>,
		2,
        sdsl::rrr_vector<>
>;

using nv                 = naive_processor<node_type,size_type,value_type>;
using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
using nv_sct            = nsrs<node_type,size_type,value_type>;
using wt_hp_ptr          = wt_hpd_ptr<node_type,size_type,value_type>;
using tree_ext_ptr       = ext_ptr<node_type,size_type,value_type>;

/*

    using nv                 = naive_processor<node_type,size_type,value_type>;
    using nv_lca             = naive_processor_lca<node_type,size_type,value_type>;
    using nv_succ            = nsrs<node_type,size_type,value_type>;
    using wt_hp              = wt_hpd_ptr<node_type,size_type,value_type>;
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
*/
    std::string description;
    value_type a,b;
    size_type n;
    std::string topology;
    std::vector<value_type> w;

    uint16_t mask= 0;

public:

    explicit fixed_dataset_manager( std::istream &is, uint16_t mask, const std::string &descr= "N/A" ) {
        this->mask= mask;
        is >> topology;
        w.resize(topology.size()/2);
        for ( auto &x: w )
            is >> x;
        n= topology.size()/2, a= *(std::min_element(begin(w),end(w))), b= *(std::max_element(begin(w),end(w)));
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
        if ( configs.count("median"))
            builder.set_node_range(n).set(path_queries::QUERY_TYPE::MEDIAN,configs["median"]);
        if ( configs.count("counting"))
            builder.set_weight_range(a,b).set(path_queries::QUERY_TYPE::COUNTING,configs["counting"]);
        if ( configs.count("reporting"))
            builder.set(path_queries::QUERY_TYPE::REPORTING,configs["reporting"]);
        if ( configs.count("selection"))
            builder.set(path_queries::QUERY_TYPE::SELECTION,configs["selection"]);
        if ( configs.count("K"))
            builder.set_scaling_param(configs["K"]);
        /**
         * create some temporary file, feed the queries into it
         * TODO: look how Gog uses this id() in his own code
         */
         std::string filename= "query_file_"+std::to_string(sdsl::util::id())+
                 "_on_"+std::to_string(sdsl::util::pid())+".json";
         std::ofstream os(filename);
         builder.build();
         for ( const auto &q: builder ) os << q << '\n';
         os.close();
         nlohmann::json res, per_datastructure;

         for ( int i= 0; i < 9; ++i ) {
             if ( not (mask & (1u<<i)) ) continue ;
             nlohmann::json obj;
             double construction_time= 0.00;
             std::unique_ptr<fixed_processor_manager<node_type,size_type,value_type>> pm;
             // using I.I.L.E. in order to measure construction time
             // we use try-catch, because some datasets may be too large to fit into a specific
             // data structure
             try {
                 pm = [&]() {
                     duration_timer<std::chrono::seconds> dt(construction_time);
                     return std::make_unique<fixed_processor_manager<node_type, size_type, value_type>>(
                             experiments::instantiate<node_type, size_type, value_type>(topology, w, 1 << i));
                 }();
                 obj["constructionTimeInSeconds"] = construction_time;
                 std::ifstream is(filename);
                 obj["breakdownInMicroseconds"] = pm->invoke_with(is);
             } catch ( std::exception &e ) {
                 obj["exceptionThrown"]= std::string(e.what());
             }
             per_datastructure[experiments::hrnames[i]] = obj;
         }
         res= {{"dataset",description},{"config",configs},{"results",per_datastructure}};
         std::remove(filename.c_str());
         return res;
    }
};
#endif //TREE_PATH_QUERIES_FIXED_DATASET_MANAGER_HPP
