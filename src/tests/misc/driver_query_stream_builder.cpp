#include <iostream>
#include "query_stream_builder.hpp"

/**
 * @details tests interactively I/O of requests
 * @return
 */
int main() {
    int i,j,k;
    char a[0x20];
    while ( true ) {
        query_stream_builder builder;
        while ( 1 == scanf("%s",a) ) {
            switch ( 0[a] ) {
                case 'c': scanf("%d",&i); builder.set(path_queries::QUERY_TYPE::COUNTING, i); break ;
                case 'r': scanf("%d",&i); builder.set(path_queries::QUERY_TYPE::REPORTING, i); break ;
                case 'm': scanf("%d",&i); builder.set(path_queries::QUERY_TYPE::MEDIAN, i); break ;
                case 's': scanf("%d",&i); builder.set(path_queries::QUERY_TYPE::SELECTION, i); break ;
                case 'q': goto nx;
            }
            continue ;
            nx: break ;
        }
        builder.set_node_range(7).set_scaling_param(2).set_weight_range(1,100);
        builder.build();
        for ( auto pr: builder ) {
            std::cout << pr << std::endl;
        }
    }
    return 0;
}

