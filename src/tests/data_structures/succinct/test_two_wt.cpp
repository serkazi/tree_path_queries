#include <random>
#include <sys/resource.h>
#include <algorithm>
#include <numeric>
#include <set>
#include "gtest/gtest.h"
#include "raw_processor.hpp"
#include "two_wt.hpp"
#include "pq_types.hpp"

namespace {

    using node_type  = pq_types::node_type;
    using size_type  = pq_types::size_type;
    using value_type = pq_types::value_type;
    const int ITERS = 0x400;  // 1024 random iterations

    // ---- Tree generators ----

    /**
     * Generate a random valid BP string for n nodes (length 2n).
     * At each step, track:
     *   opens_placed: number of '(' placed so far
     *   depth: current nesting depth (unmatched opens)
     * Can place '(' if opens_placed < n and depth+1 <= remaining-1
     *   (need room to close all opens)
     * Can place ')' if depth > 0
     */
    std::string random_bp(int n, std::mt19937 &rng) {
        std::string s;
        s.reserve(2 * n);
        int opens_placed = 0;
        int depth = 0;
        for (int i = 0; i < 2 * n; ++i) {
            int remaining_after = 2 * n - i - 1;
            bool can_open = (opens_placed < n) && (depth + 1 <= remaining_after);
            bool can_close = (depth > 0);
            if (can_open && can_close) {
                if (std::uniform_int_distribution<int>(0, 1)(rng) == 0)
                    s += '(', ++opens_placed, ++depth;
                else
                    s += ')', --depth;
            } else if (can_open) {
                s += '(', ++opens_placed, ++depth;
            } else {
                s += ')', --depth;
            }
        }
        assert(depth == 0);
        assert(opens_placed == n);
        assert(static_cast<int>(s.size()) == 2 * n);
        return s;
    }

    /**
     * Generate a random single-rooted tree BP string for n nodes.
     * Wraps random_bp(n-1) in "()" so the result is always a single root.
     */
    std::string random_tree_bp(int n, std::mt19937 &rng) {
        if (n == 1) return "()";
        return "(" + random_bp(n - 1, rng) + ")";
    }

    /**
     * Generate random weights in [0, sigma) for n nodes.
     */
    std::vector<value_type> random_weights(int n, int sigma, std::mt19937 &rng) {
        std::vector<value_type> w(n);
        std::uniform_int_distribution<value_type> dist(0, sigma - 1);
        for (auto &x : w)
            x = dist(rng);
        return w;
    }

    /**
     * Generate BP string for a path graph (chain) of n nodes.
     * Structure: (((...)))
     */
    std::string chain_bp(int n) {
        std::string s;
        s.reserve(2 * n);
        for (int i = 0; i < n; ++i) s += '(';
        for (int i = 0; i < n; ++i) s += ')';
        return s;
    }

    /**
     * Generate BP string for a star graph: root with n-1 leaf children.
     * Structure: (()()...())
     */
    std::string star_bp(int n) {
        std::string s = "(";
        for (int i = 1; i < n; ++i) s += "()";
        s += ")";
        return s;
    }

    /**
     * Generate BP string for a complete binary tree with n nodes.
     * n should be 2^k - 1 for a perfectly complete tree.
     */
    std::string complete_binary_bp(int n) {
        // Build recursively using a stack to avoid deep recursion
        // We'll build the tree top-down: for each node, add children if budget allows
        std::string s;
        s.reserve(2 * n);
        // Use a queue-like approach: BFS order
        std::vector<int> subtree_sizes;
        // Compute subtree sizes for a complete binary tree
        // Simpler: use recursive generation with explicit stack
        struct Frame { int sz; int phase; };
        std::vector<Frame> stack;
        stack.push_back({n, 0});
        while (!stack.empty()) {
            auto &f = stack.back();
            if (f.sz <= 0) {
                stack.pop_back();
                continue;
            }
            if (f.phase == 0) {
                s += '(';
                f.phase = 1;
                if (f.sz > 1) {
                    int left_sz = (f.sz - 1) / 2;
                    // Push right child first (processed second due to stack)
                    // But we need left first in DFS order, so push right, then handle left
                    int right_sz = f.sz - 1 - left_sz;
                    f.phase = 2;
                    // We need to process left subtree next, then right, then close
                    // Rearrange: current frame will close after children
                    // Push right child
                    stack.push_back({right_sz, 0});
                    // Push left child (on top, processed first)
                    stack.push_back({left_sz, 0});
                } else {
                    // Leaf
                    s += ')';
                    stack.pop_back();
                }
            } else if (f.phase == 1) {
                // Leaf: already added '(' and ')', shouldn't reach here
                s += ')';
                stack.pop_back();
            } else {
                // phase == 2: children done, close this node
                s += ')';
                stack.pop_back();
            }
        }
        return s;
    }

    // ---- Multiset comparison for report queries ----

    bool equal_multisets(
            const std::vector<std::pair<value_type, size_type>> &a,
            const std::vector<std::pair<value_type, size_type>> &b) {
        std::multiset<size_type> s, t;
        std::for_each(a.begin(), a.end(), [&s](const auto &z) {
            s.insert(z.second);
        });
        std::for_each(b.begin(), b.end(), [&t](const auto &z) {
            t.insert(z.second);
        });
        return s == t;
    }

    // ---- Helper: run all query types against oracle ----

    void run_all_queries(
            const std::string &s,
            const std::vector<value_type> &w,
            int iters = ITERS) {

        auto n = static_cast<node_type>(s.size() / 2);
        auto processor = std::make_unique<two_wt<node_type, size_type, value_type>>(s, w);
        auto oracle = std::make_unique<raw_processor<node_type, size_type, value_type>>(s, w);

        std::mt19937 rng(42);
        std::uniform_int_distribution<node_type> node_dist(0, n - 1);

        std::vector<value_type> sorted_w(w.begin(), w.begin() + n);
        std::sort(sorted_w.begin(), sorted_w.end());

        std::uniform_int_distribution<size_type> widx_dist(0, n - 1);
        std::uniform_int_distribution<size_type> qntl_dist(1, 100);

        // --- Counting ---
        for (int it = 0; it < iters; ++it) {
            auto x = node_dist(rng), y = node_dist(rng);
            auto vl = sorted_w[widx_dist(rng)], vr = sorted_w[widx_dist(rng)];
            if (vl > vr) std::swap(vl, vr);
            auto ans1 = processor->count(x, y, vl, vr);
            auto ans2 = oracle->count(x, y, vl, vr);
            ASSERT_EQ(ans1, ans2) << "count mismatch: x=" << x << " y=" << y
                                  << " a=" << vl << " b=" << vr;
        }

        // --- Reporting ---
        for (int it = 0; it < iters; ++it) {
            auto x = node_dist(rng), y = node_dist(rng);
            auto vl = sorted_w[widx_dist(rng)], vr = sorted_w[widx_dist(rng)];
            if (vl > vr) std::swap(vl, vr);
            std::vector<std::pair<value_type, size_type>> answer1, answer2;
            processor->report(x, y, vl, vr, answer1);
            oracle->report(x, y, vl, vr, answer2);
            ASSERT_TRUE(equal_multisets(answer1, answer2))
                << "report mismatch: x=" << x << " y=" << y
                << " a=" << vl << " b=" << vr;
        }

        // --- Median ---
        for (int it = 0; it < iters; ++it) {
            auto x = node_dist(rng), y = node_dist(rng);
            auto ans1 = processor->query(x, y);
            auto ans2 = oracle->query(x, y);
            ASSERT_EQ(ans1, ans2) << "median mismatch: x=" << x << " y=" << y;
        }

        // --- Selection ---
        for (int it = 0; it < iters; ++it) {
            auto x = node_dist(rng), y = node_dist(rng);
            auto qntl = qntl_dist(rng);
            auto ans1 = processor->selection(x, y, qntl);
            auto ans2 = oracle->selection(x, y, qntl);
            ASSERT_EQ(ans1, ans2) << "selection mismatch: x=" << x << " y=" << y
                                  << " qntl=" << qntl;
        }
    }

    // ---- Test cases ----

    TEST(TwoWtTest, SingleNode) {
        std::string s = "()";
        std::vector<value_type> w = {5};
        auto proc = std::make_unique<two_wt<node_type, size_type, value_type>>(s, w);
        auto oracle = std::make_unique<raw_processor<node_type, size_type, value_type>>(s, w);

        // weight_of
        ASSERT_EQ(proc->weight_of(0), 5u);

        // count: node 0 to itself, range including 5
        ASSERT_EQ(proc->count(0, 0, 0, 10), 1u);
        ASSERT_EQ(proc->count(0, 0, 6, 10), 0u);

        // median
        ASSERT_EQ(proc->query(0, 0), 5u);

        // selection
        ASSERT_EQ(proc->selection(0, 0, 50), 5u);

        // report
        std::vector<std::pair<value_type, size_type>> res;
        proc->report(0, 0, 0, 10, res);
        ASSERT_EQ(res.size(), 1u);
    }

    TEST(TwoWtTest, SmallTree7) {
        // Tree: (((()))()(()))
        // Nodes (preorder): 0=root, 1, 2, 3, 4, 5, 6
        //        0
        //      / | \
        //     1  4   5
        //    /       |
        //   2        6
        //   |
        //   3
        std::string s = "(((()))()(()))";
        std::vector<value_type> w = {10, 20, 30, 40, 50, 60, 70};

        run_all_queries(s, w, 256);
    }

    TEST(TwoWtTest, PathGraph) {
        // Chain of 100 nodes
        int n = 100;
        std::string s = chain_bp(n);
        std::mt19937 rng(123);
        auto w = random_weights(n, 1000, rng);
        run_all_queries(s, w);
    }

    TEST(TwoWtTest, StarGraph) {
        // Root with 99 leaf children
        int n = 100;
        std::string s = star_bp(n);
        std::mt19937 rng(456);
        auto w = random_weights(n, 1000, rng);
        run_all_queries(s, w);
    }

    TEST(TwoWtTest, AllEqualWeights) {
        int n = 200;
        std::mt19937 rng(789);
        std::string s = random_tree_bp(n, rng);
        std::vector<value_type> w(n, 42);  // all same weight
        run_all_queries(s, w);
    }

    TEST(TwoWtTest, BinaryWeights) {
        int n = 300;
        std::mt19937 rng(101);
        std::string s = random_tree_bp(n, rng);
        auto w = random_weights(n, 2, rng);  // sigma = 2
        run_all_queries(s, w);
    }

    TEST(TwoWtTest, CompleteBinaryTree) {
        int n = 127;  // 2^7 - 1
        std::string s = complete_binary_bp(n);
        std::mt19937 rng(202);
        auto w = random_weights(n, 500, rng);
        run_all_queries(s, w);
    }

    TEST(TwoWtTest, RandomSmall) {
        int n = 500;
        std::mt19937 rng(303);
        std::string s = random_tree_bp(n, rng);
        auto w = random_weights(n, 100, rng);
        run_all_queries(s, w);
    }

    TEST(TwoWtTest, RandomMedium) {
        int n = 5000;
        std::mt19937 rng(404);
        std::string s = random_tree_bp(n, rng);
        auto w = random_weights(n, 1000, rng);
        run_all_queries(s, w);
    }

    TEST(TwoWtTest, WeightOfCorrect) {
        int n = 200;
        std::mt19937 rng(505);
        std::string s = random_tree_bp(n, rng);
        auto w = random_weights(n, 500, rng);
        auto proc = std::make_unique<two_wt<node_type, size_type, value_type>>(s, w);
        auto oracle = std::make_unique<raw_processor<node_type, size_type, value_type>>(s, w);
        for (node_type v = 0; v < static_cast<node_type>(n); ++v) {
            ASSERT_EQ(proc->weight_of(v), oracle->weight_of(v))
                << "weight_of mismatch at node " << v;
            ASSERT_EQ(proc->weight(v), oracle->weight(v))
                << "weight mismatch at node " << v;
        }
    }

    TEST(TwoWtTest, TreeNavCorrect) {
        // Build a naive oracle from the BP string: parse parent/depth arrays
        int n = 200;
        std::mt19937 rng(606);
        std::string s = random_tree_bp(n, rng);
        auto w = random_weights(n, 500, rng);
        auto proc = std::make_unique<two_wt<node_type, size_type, value_type>>(s, w);

        // Parse BP string to get parent[] and depth[] arrays
        std::vector<int> par(n, -1);
        std::vector<size_type> dep(n, 0);
        std::vector<bool> leaf(n, true);
        {
            std::vector<int> stk;
            int node_id = 0;
            for (size_t i = 0; i < s.size(); ++i) {
                if (s[i] == '(') {
                    int v = node_id++;
                    if (!stk.empty()) {
                        par[v] = stk.back();
                        leaf[stk.back()] = false;
                    }
                    dep[v] = stk.size();
                    stk.push_back(v);
                } else {
                    stk.pop_back();
                }
            }
        }

        ASSERT_EQ(proc->size(), static_cast<size_type>(n));

        for (node_type v = 0; v < static_cast<node_type>(n); ++v) {
            // parent
            if (v == 0) {
                ASSERT_EQ(proc->parent(v), std::nullopt) << "root should have no parent";
            } else {
                ASSERT_TRUE(proc->parent(v).has_value()) << "node " << v << " should have parent";
                ASSERT_EQ(proc->parent(v).value(), static_cast<node_type>(par[v]))
                    << "parent mismatch at node " << v;
            }

            // depth
            ASSERT_EQ(proc->depth(v), dep[v])
                << "depth mismatch at node " << v;

            // is_leaf
            ASSERT_EQ(proc->is_leaf(v), leaf[v])
                << "is_leaf mismatch at node " << v;
        }

        // Naive LCA via walking up to root
        auto naive_lca = [&](node_type x, node_type y) -> node_type {
            // Collect ancestors of x
            std::set<node_type> anc_x;
            for (auto u = static_cast<int>(x); u >= 0; u = par[u])
                anc_x.insert(static_cast<node_type>(u));
            // Walk y up until we hit an ancestor of x
            for (auto u = static_cast<int>(y); u >= 0; u = par[u])
                if (anc_x.count(static_cast<node_type>(u)))
                    return static_cast<node_type>(u);
            return 0; // root
        };

        auto naive_is_ancestor = [&](node_type p, node_type x) -> bool {
            for (auto u = static_cast<int>(x); u >= 0; u = par[u])
                if (static_cast<node_type>(u) == p) return true;
            return false;
        };

        // lca and is_ancestor: random pairs
        std::uniform_int_distribution<node_type> node_dist(0, n - 1);
        for (int it = 0; it < ITERS; ++it) {
            auto x = node_dist(rng), y = node_dist(rng);
            ASSERT_EQ(proc->lca(x, y), naive_lca(x, y))
                << "lca mismatch: x=" << x << " y=" << y;
            ASSERT_EQ(proc->is_ancestor(x, y), naive_is_ancestor(x, y))
                << "is_ancestor mismatch: x=" << x << " y=" << y;
        }
    }

    TEST(TwoWtTest, ConstructorDestructorSanity) {
        std::string s("(((()))()(()))");
        std::vector<value_type> w = {1, 2, 3, 4, 5, 6, 7};
        auto processor = std::make_unique<two_wt<node_type, size_type, value_type>>(s, w);
        processor = nullptr;  // should not crash
    }
}

int main(int argc, char **argv) {
    const rlim_t kStackSize = 20 * 1024ll * 1024ll * 1024ll;
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0) {
                std::cerr << "setrlimit returned result = " << result << std::endl;
            }
        }
    }
    testing::InitGoogleTest(&argc, argv);
    return ::RUN_ALL_TESTS();
}
