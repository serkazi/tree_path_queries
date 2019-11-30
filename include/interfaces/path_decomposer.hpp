//
// Created by sj on 06/06/19.
//
#ifndef SPQ_PATH_DECOMPOSER_HPP
#define SPQ_PATH_DECOMPOSER_HPP
#include "pq_types.hpp"
template<typename node_type= pq_types::node_type, typename size_type= pq_types::size_type>
class path_decomposer {
public:
    virtual size_type get_decomposition_length( node_type x, node_type y ) const = 0;
};
#endif //SPQ_PATH_DECOMPOSER_HPP
