//
// Created by sj on 15/11/19.
//
// sizeof(vec) gave 24 on my machine, regardless of the size -- that means it is a header info
#include <vector>
template<typename T>
double size_of_vector_in_bytes( const std::vector<T> &vec ) {
    return sizeof(vec)+vec.size()*sizeof(T);
}

//template<typename T>
//double size_of_unique_ptr( std::unique_ptr<T> ptr ) {
//}