/**
 * here, a binary tree is turned into an ordered tree using
 * "natural correspondence", as shown here
 * https://watermark.silverchair.com/230161.pdf
 * we take a BP sequence of a binary tree
 * and write it as a BP sequence of an ordered tree
 */
#include <cassert>
#include <iostream>
#include <vector>
#include <optional>

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
			graph() {
				V= E= 0, adj.clear();
			}
			graph( const std::string &s ) {
				std::stack<node_type> st{};
				node_type x,y;
				size_type n= s.size()/2, V= E= 0;
				adj.resize(n);
				for ( auto &v: adj )
					v.clear();
				for ( auto ch: s ) {
					case '(': st.push(V++); break ;
					case ')': assert( not st.empty() );
							  add_arcs(st.top(),new_node());
							  break ;
					default: assert( false );
				}
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
			void transform( graph &dst, node_type x, const graph &src ) {
				const auto &adj= src.children(x);
				// we assert this is a binary tree
				assert( adj.size() <= 2 );
				graph lft{},rgt{};
				if ( adj.size() >= 1 ) 
					transform(dst,adj.front(),lft);
				if ( adj.size() >= 2 ) 
					transform(dst,adj.back(),rgt);
				auto root= dst.new_node();
				size_type offset= 0;
				{
					auto left_forest= lft.rest();
					offset= dst.V;
					for ( auto z= 0; z < lft.V; ++z ) {
						for ( auto &t: lft.adj[z] )
							t+= offset;
					}
					for ( auto z= 0; z < lft.size(); ++z ) {
						auto new_z= z+offset;
						for (;new_z >= dst.size(); dst.new_node() ) ;
						dst.adj[new_z]= std::move(lft.adj[z]);
					}
					if ( lft.size() )
						dst.adj[root].push_back(0+offset), offset+= lft.size();
				}
				{
					auto right_forest= rgt.rest();
					for ( auto z= 1; z < rgt.size(); ++z ) {
						auto new_z= z+offset-1;
						for (;new_z >= dst.size(); dst.new_node() ) ;
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

int main() {
	return 0;
}

