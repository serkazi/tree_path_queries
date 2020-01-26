/**
 *
 */
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <cstdint>
#include <optional>
#include <cassert>
#include <algorithm>
#include <stack>
#include <vector>
#include "gflags/gflags.h"

class gen_rand_topology {
private:
	std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution= std::uniform_real_distribution<double>(0.00,1.00);
	double next_double() { return distribution(generator); }
	std::vector<size_t> rand_subset( size_t N, size_t n ) {
		std::vector<size_t> res(n);
		for ( size_t m= 0, t= 0; m < n; ++t )
			if ( (N-t)*next_double() < n-m )
				res[m++]= t;
		return res;
	}
	static bool is_balanced( const std::string &s ) {
		std::int64_t balance= 0;
		for ( auto ch: s ) 
			if ( (balance+= (ch=='('?1:-1)) < 0 )
				return false ;
		return balance == 0;
	}
	std::string explicit_stack_phi( const std::string &w ) {
		size_t n= w.size()/2, cur= 0;
		std::string sb; sb.resize(2*n);
		std::stack<std::optional<std::pair<size_t,size_t>>> post_action;
		std::stack<size_t> ls, rs;
		std::stack<bool> status;
#define enc(x,y,tf,opt) { ls.push(x),rs.push(y),status.push(tf),post_action.push(opt); }
		enc(0,2*n-1,false,std::nullopt);
		while ( not ls.empty() ) {
			auto left= ls.top(), right= rs.top();
			ls.pop(), rs.pop();
			std::int64_t partial_sum= 0,i,j,k,r= 0;
			bool done= status.top(); status.pop();
			auto action= post_action.top(); post_action.pop();
			if ( done ) { 
				if ( action ) {
					sb[cur++]= ')';
					for ( auto k= action.value().first+1; k <= action.value().second-2; ++k )
						sb[cur++]= (w[k]=='('?')':'(');
				}
				continue ;
			}
			enc(left,right,true,action);
			for ( i= left; i <= right and r == 0; ++i )
				if ( (partial_sum+= (w[i]=='('?1:-1)) == 0 )
					r= i+1;
			if ( left > right ) continue ;
			assert( r > 0 );
			if ( w[left] == '(' ) {
				for ( i= left; i < r; ++i )
					sb[cur++]= w[i];
				enc(r,right,false,std::nullopt);
				continue ;
			}
			assert( w[left] == ')' );
			assert( w[r-1] == '(' );
			sb[cur++]= '(';
			enc(r,right,false,std::make_pair(left,r));
		}
#undef enc
		return sb;
	}
	std::string random_bps( size_t n ) {
		auto L= rand_subset(2*n,n);
		std::string x{};
		x.resize(2*n);
		assert( L.size() == n );
		size_t i,k;
		for ( i= 0, k= 0; k < n; x[i++]= '(', ++k ) 
			for ( ;i < 2*n and i < L[k]; x[i++]= ')' ) ;
		assert( k == n );
		for ( ;i < 2*n; x[i++]= ')' ) ;
		return explicit_stack_phi(x);
	}
public:
	std::string operator() ( size_t n ) {
		std::string res{}; res.push_back('(');
		res+= random_bps(n-1), res.push_back(')');
		assert( this->is_balanced(res) );
		return res;
	}
};

DEFINE_uint64(n,1ull,"n the tree size to generate");
DEFINE_uint64(a,0,"a: the left bound of the [a,b] range of weights");
DEFINE_uint64(b,0,"b: the right bound of the [a,b] range of weights");
// DEFINE_string(dataset_path,"","full path to the dataset");
// DEFINE_string(query_type,"median","type of query: median, counting, reporting");
// DEFINE_string(data_structure,"nv","data structure: one of 'nv', 'nv_lca', 'nv_sct', 'ext_ptr', 'whp_ptr', 'ext_sct_un', 'ext_sct_rrr', 'whp_un', 'whp_rrr'");
// DEFINE_string(output_format,"json","format of output: csv, json");
// DEFINE_string(output_file,"","output file path");

int main( int argc, char **argv ) {

	gflags::ParseCommandLineFlags(&argc,&argv,true);
	std::unique_ptr<gen_rand_topology> ptr= std::make_unique<gen_rand_topology>();
	auto res= (*ptr)(FLAGS_n);
	std::cout << res << std::endl;
	if ( FLAGS_a < FLAGS_b ) {
		std::default_random_engine generator;
	    std::uniform_int_distribution<std::uint64_t> distribution(FLAGS_a,FLAGS_b);
		for ( auto i= 0; i < FLAGS_n; ++i ) 
			std::cout << distribution(generator) << " ";
		std::cout << std::endl;
	}

	return 0;
}

