//
// Created by sj on 17/11/19.
//
#include <climits>
#include "simple_bitset.hpp"

simple_bitset::simple_bitset(size_t n) {
    this->n= n, data= std::make_unique<unsigned char[]>(n/UCHAR_WIDTH+7);
    clear_all();
}

bool simple_bitset::test(size_t i) const {
    return data[i/UCHAR_WIDTH] & (1ul<<(i%UCHAR_WIDTH));
}

void simple_bitset::set(size_t i) {
    data[i/UCHAR_WIDTH] |= (1u<<(i%UCHAR_WIDTH));
}

void simple_bitset::clr(size_t i) {
    data[i/UCHAR_WIDTH]&= ~(1u<<(i%UCHAR_WIDTH));
}

void simple_bitset::clear_all() {
    for ( auto i= 0; i < n/UCHAR_WIDTH+7; data[i++]= 0 ) ;
}

double simple_bitset::size_in_bytes() const {
    return sizeof(data) + sizeof(n) + (n/UCHAR_WIDTH+7)*sizeof(unsigned char);
}
