/**
 * here, a binary tree is turned into an ordered tree using
 * "natural correspondence", as shown here
 * https://watermark.silverchair.com/230161.pdf
 * we take a BP sequence of a binary tree
 * and write it as a BP sequence of an ordered tree
 */
#include <iostream>
#include <fstream>
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

namespace gentree {
	using node_type= std::int64_t;
	using size_type= std::int64_t;
	/**
	 */
	class graph {  
		private:
			std::vector<std::vector<node_type>> adj;
			size_type V,E;
			void _serialize( std::ostream &os, node_type x ) const {
				os << '(';
				for ( auto z: adj[x] ) 
					_serialize(os,z);
				os << ')';
			}
			void init( const std::string &s ) {
				std::stack<node_type> st{};
				node_type x,y;
				size_type n= s.size()/2, V= E= 0;
				adj.resize(n);
				for ( auto &v: adj )
					v.clear();
				for ( auto ch: s ) {
					switch ( ch ) {
						case '(': if ( not st.empty() )
								  	add_arcs(st.top(),x= new_node());
								  st.push(x); 
								  break ;
						case ')': assert( not st.empty() );
								  st.pop();
						default: assert( false );
					}
				}
				assert( st.empty() );
			}
		public:
			node_type new_node() {
				auto res= V++;
				for (;res >= adj.size(); adj.push_back(std::vector<node_type>{}) ) ;
				return res;
			}
			void add_arcs( node_type from, node_type to ) {
				for ( ;from >= adj.size(); adj.push_back(std::vector<node_type>{}) ) ;
				adj[from].push_back(to);
			}
			graph() { V= E= 0, adj.clear(); }
			graph( const std::string &s ) { init(s); }
			graph( std::istream &is ) {
				std::string s;
				is >> s, init(s);
			}
			size_type size() const {
				return V;
			}
			const std::vector<node_type> &children( node_type x ) const {
				return adj[x];
			}
			std::vector<node_type> rest() {
				return V==0?std::vector<node_type>{}:adj[0];
			}
			void add_node( node_type x ) {
				for (;x >= size(); new_node() ) ;
			}
			void transform( node_type x, const graph &src ) {
				auto &dst= *this;
				const auto &adj= src.children(x);
				// we assert this is a binary tree
				assert( adj.size() <= 2 );
				graph lft{},rgt{};
				if ( adj.size() >= 1 ) 
					transform(adj.front(),lft);
				if ( adj.size() >= 2 ) 
					transform(adj.back(),rgt);
				auto root= dst.new_node();
				size_type offset= dst.V;
				{
					auto left_forest= lft.rest();
					for ( auto z= 0; z < lft.V; ++z ) {
						for ( auto &t: lft.adj[z] )
							t+= offset;
					}
					for ( auto z= 0; z < lft.size(); ++z ) {
						auto new_z= z+offset;
						dst.add_node(new_z);
						dst.adj[new_z]= std::move(lft.adj[z]);
					}
					if ( lft.size() )
						dst.adj[root].push_back(0+offset);
					offset+= lft.size();
				}
				{
					auto right_forest= rgt.rest();
					for ( auto z= 0; z < rgt.size(); ++z ) {
						for ( auto &t: rgt.adj[z] )
							--t, t+= offset;
					}
					for ( auto z= 1; z < rgt.size(); ++z ) {
						auto new_z= (z-1)+offset;
						dst.add_node(new_z);
						dst.adj[new_z]= std::move(rgt.adj[z]);
					} 
					if ( rgt.size() ) {
						for ( auto &t: rgt.adj.front() )
							t+= offset-1;
						dst.adj[root].insert(dst.adj[root].end(),rgt.adj.front().begin(),rgt.adj.front().end());
					}
				}
			}
			void serialize( std::ostream &os ) const {
				_serialize(os,0);
			}
	};
};

DEFINE_string(output_file,"","output file path");
DEFINE_string(input_file,"","input file path");

int main( int argc, char **argv ) {
	//gflags::SetUsageMessage();
	gflags::ParseCommandLineFlags(&argc,&argv,true);
	std::unique_ptr<gentree::graph> ptr, qtr;

	if ( FLAGS_input_file != "" ) {
		std::ifstream is(FLAGS_input_file);
   		ptr= std::make_unique<gentree::graph>(is);
		is.close();
	}
	else {
   		ptr= std::make_unique<gentree::graph>(std::cin);
	}
   	qtr= std::make_unique<gentree::graph>();
	qtr->transform(0,*ptr);
	if ( FLAGS_output_file != "" ) {
		std::ofstream os(FLAGS_output_file);
		qtr->serialize(os);
		os.close();
	}
	else {
		qtr->serialize(std::cout);
	}
	return 0;
}

