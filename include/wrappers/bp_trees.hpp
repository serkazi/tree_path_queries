#include "bp_tree_sada.hpp"
#include "bp_tree_gg.hpp"
#include "sdsl/select_support_scan.hpp"
#include "sdsl/rank_support_scan.hpp"

namespace bp_trees {
		// fast structures use the default template parameters -- fast rank/select
		template<typename node_type,typename size_type>
		using bp_sada_fast= bp_tree_sada<node_type,size_type>;
		template<typename node_type,typename size_type>
		using bp_gg_fast= bp_tree_gg<node_type,size_type>;

		// small structures use constant-space rank/select
		template<typename node_type,typename size_type>
		using bp_sada_small= bp_tree_sada<node_type,size_type,256,32,sdsl::rank_support_scan<>,sdsl::select_support_scan<>>;
		template<typename node_type,typename size_type>
		using bp_gg_small= bp_tree_gg<node_type,size_type,sdsl::nearest_neighbour_dictionary<30>,sdsl::rank_support_scan<>,sdsl::select_support_scan<>,840>;
};
