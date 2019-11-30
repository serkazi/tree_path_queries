//
// Created by sj on 17/11/19.
//

#ifndef SPQ_SIMPLE_BITSET_HPP
#define SPQ_SIMPLE_BITSET_HPP
#include <memory>

class simple_bitset {
private:
    std::unique_ptr<unsigned char[]> data;
    size_t n;
public:
    explicit simple_bitset( size_t n ) ;
    virtual ~simple_bitset() = default; //to keep smart pointers happy
    virtual bool test( size_t i ) const ;
    virtual void set( size_t i ) ;
    virtual void clr( size_t i ) ;
    virtual void clear_all() ;
    virtual double size_in_bytes() const ;
};


#endif //SPQ_SIMPLE_BITSET_HPP
