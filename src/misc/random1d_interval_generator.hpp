//
// Created by sj on 08/04/19.
//
#ifndef SPQ_RANDOM1D_INTERVAL_GENERATOR_HPP
#define SPQ_RANDOM1D_INTERVAL_GENERATOR_HPP
#include <cstdlib>
#include <memory>
#include <random>
#include <cassert>

#define DERANDOMIZE 1

template<typename T>
class random1d_interval_generator {
private:
    T _a,_b;
#if DERANDOMIZE
    std::unique_ptr<std::mt19937> generator= std::make_unique<std::mt19937>();
#else
    std::unique_ptr<std::mt19937> generator= std::make_unique<std::mt19937>(std::random_device{}());
#endif
    std::unique_ptr<std::uniform_real_distribution<double>> distribution;
    T scale( double tau, T a, T b ) {
        return std::min(b,static_cast<T>(floor(a + tau*(b-a)+1e-6)));
    }
    double next_real() {
        return (*distribution)(*generator);
    }

public:

    random1d_interval_generator( T a, T b ) {
        _a= a, _b= b;
        distribution= std::make_unique<std::uniform_real_distribution<double>>(0,1);
    }
    /**
     *
     * @param n
     * @param K
     * @return n intervals \f$ [l,r] \subseq [a,b] \f$, such that
     * \f$ l \f$ is chosen u.a.r. from \f$[a,b]\f$, whereas
     * \f$r\f$ is chosen at distance at most \f$ (b-l)/K \f$ from \f$l\f$
     */
    std::vector<std::pair<T,T>> operator()( const size_t n, const size_t K= 1 ) {
        std::vector<std::pair<T,T>> res(n);
        assert( K >= 1 );
        for ( auto i= 0; i < n; ++i ) {
            auto x= scale(next_real(),_a,_b);
            assert( _a <= x and x <= _b );
            auto len= std::max(static_cast<T>(1),static_cast<T>((_b-x)/K));
            assert( len >= 1 );
            auto y= x+scale(next_real(),1,len)-1;
            assert( y <= _b );
            assert( y >= x and (y-x+1) <= len );
            res[i]= {x,std::min(y,_b)};
            assert( res[i].first <= res[i].second );
            assert( _a <= res[i].first and res[i].first <= _b );
            assert( _a <= res[i].second and res[i].second <= _b );
        }
        return res;
    }
    virtual ~random1d_interval_generator() = default;
};
#endif //SPQ_RANDOM1D_INTERVAL_GENERATOR_HPP
