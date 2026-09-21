#ifndef TWO_WT_HPP
#define TWO_WT_HPP

#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <vector>
#include <memory>
#include <tuple>
#include "sdsl/bit_vectors.hpp"
#include "sdsl/bp_support_sada.hpp"
#include "sdsl/wt_int.hpp"
#include "sdsl/int_vector.hpp"
#include "sdsl/construct.hpp"
#include "pq_types.hpp"
#include "succinct_tree.hpp"
#include "path_query_processor.hpp"

/**
 * @brief Path queries via two wavelet trees (preorder + exit-order).
 *
 * Uses 2n lg σ + 2n + o(n) bits total:
 *   - BP bitvector (2n bits) with BP support for tree navigation
 *   - WT over weights in preorder (n lg σ bits)
 *   - WT over weights in exit order (n lg σ bits)
 *
 * Key identity: for any node v, the set of proper ancestors of v
 * equals the set difference (preorder prefix of v) \ (exit-order prefix of v).
 * This enables O(lg σ) path counting via anc_count on both endpoints and LCA.
 */
template <
    typename node_type = pq_types::node_type,
    typename size_type = pq_types::size_type,
    typename value_type = pq_types::value_type
>
class two_wt :
    public path_query_processor<node_type, size_type, value_type>,
    public succinct_tree<node_type, size_type> {

private:
    // BP bitvector and support — must be declared before m_bp (lifetime dependency)
    std::unique_ptr<sdsl::bit_vector> m_bv;
    sdsl::bp_support_sada<> m_bp;

    // Wavelet trees over weights
    std::unique_ptr<sdsl::wt_int<>> wt_pre;   // weights in preorder
    std::unique_ptr<sdsl::wt_int<>> wt_exit;  // weights in exit order

    // Raw weights for weight_of() lookups
    std::vector<value_type> m_weights;

    // Number of nodes
    size_type m_n;

    // ---- BP helpers ----

    // Position of the opening parenthesis of node x (0-indexed preorder)
    size_type node2position(node_type x) const {
        return m_bp.select(x + 1);
    }

    // Preorder number of the node whose opening paren is at position i
    node_type position2node(size_type i) const {
        return static_cast<node_type>(m_bp.rank(i) - 1);
    }

    // (opening, closing) parenthesis positions for node x
    std::pair<size_type, size_type> interval(node_type x) const {
        auto i = node2position(x);
        return {i, m_bp.find_close(i)};
    }

    /**
     * @brief Exit rank of node x (0-indexed).
     *
     * The exit rank of a node is its position in the order that nodes
     * are "exited" during a DFS (i.e., the order of closing parentheses).
     *
     * close(v) = find_close(select(v+1))
     * exit_rank(v) = close(v) - rank(close(v))
     *
     * rank(i) counts opening parens in [0..i], so (i+1 - rank(i)) counts
     * closing parens in [0..i], and exit_rank = that - 1.
     */
    size_type exit_rank(node_type v) const {
        auto open_pos = m_bp.select(v + 1);
        auto close_pos = m_bp.find_close(open_pos);
        return close_pos - m_bp.rank(close_pos);
    }

    /**
     * @brief Subtree size of node v (including v itself).
     * Computed from the BP interval: (close - open + 1) / 2.
     */
    size_type subtree_size(node_type v) const {
        auto open_pos = m_bp.select(v + 1);
        auto close_pos = m_bp.find_close(open_pos);
        return (close_pos - open_pos + 1) / 2;
    }

    // ---- Wavelet tree range counting helpers ----

    /**
     * @brief Count values <= c among positions [0..i) in wavelet tree wt.
     *
     * Uses lex_smaller_count(i, c) which returns (rank_c, count_less_than_c).
     * count_leq = rank_c + count_less_than_c.
     */
    static size_type count_leq(const sdsl::wt_int<> &wt, size_type i, value_type c) {
        if (i == 0) return 0;
        auto res = wt.lex_smaller_count(i, c);
        return std::get<0>(res) + std::get<1>(res);
    }

    /**
     * @brief Count values in [a,b] among positions [0..i) in wavelet tree wt.
     */
    static size_type range_rank(const sdsl::wt_int<> &wt, size_type i, value_type a, value_type b) {
        if (i == 0) return 0;
        auto hi = count_leq(wt, i, b);
        auto lo = (a > 0) ? count_leq(wt, i, a - 1) : static_cast<size_type>(0);
        return hi - lo;
    }

    /**
     * @brief Count of ancestors of v (inclusive) with weight in [a,b].
     *
     * ancestors(v) = {u : u is a proper ancestor of v or u == v}
     *
     * Key identity: let S = subtree_size(v). Then
     *   preorder_prefix(v + S) = ancestors(v) ∪ left-non-ancestors ∪ subtree(v)
     *   exit_prefix(exit_rank(v)) = left-non-ancestors ∪ proper-descendants(v)
     * Subtracting:
     *   anc_count(v, a, b) = range_rank(wt_pre, v + S, a, b)
     *                      - range_rank(wt_exit, exit_rank(v), a, b)
     */
    size_type anc_count(node_type v, value_type a, value_type b) const {
        auto ss = subtree_size(v);
        return range_rank(*wt_pre, static_cast<size_type>(v) + ss, a, b)
             - range_rank(*wt_exit, exit_rank(v), a, b);
    }

    /**
     * @brief Count on the path x--y (given precomputed LCA z) with weight in [a,b].
     */
    size_type count_on_path(node_type x, node_type y, node_type z,
                            value_type a, value_type b) const {
        size_type cx = anc_count(x, a, b);
        size_type cy = anc_count(y, a, b);
        size_type cz = anc_count(z, a, b);
        size_type z_in = (a <= m_weights[z] && m_weights[z] <= b) ? 1 : 0;
        return cx + cy - 2 * cz + z_in;
    }

    /**
     * @brief Binary search over weight space for the k-th smallest on a path.
     * O(lg^2 sigma) — binary search over [lo..hi], each step calls count_on_path.
     */
    value_type _select(node_type x, node_type y, node_type z,
                       size_type k, value_type lo, value_type hi) const {
        while (lo < hi) {
            value_type mid = lo + (hi - lo) / 2;
            size_type left_count = count_on_path(x, y, z, lo, mid);
            if (k < left_count) {
                hi = mid;
            } else {
                k -= left_count;
                lo = mid + 1;
            }
        }
        return lo;
    }

public:

    /**
     * @brief Construct two_wt from a BP string and weight vector.
     * @param s  Balanced parenthesis string, e.g. "(((()))()(()))"
     * @param w  Weight vector, one entry per node in DFS preorder
     */
    two_wt(const std::string &s, const std::vector<value_type> &w) {
        m_n = s.size() / 2;
        assert(s.size() % 2 == 0);
        assert(w.size() >= m_n);

        // 1. Build BP bitvector from string
        m_bv = std::make_unique<sdsl::bit_vector>(s.size(), 0);
        for (size_t i = 0; i < s.size(); ++i) {
            if (s[i] == '(')
                (*m_bv)[i] = 1;
        }
        m_bp = sdsl::bp_support_sada<>(m_bv.get());

        // 2. Store raw weights, compute sigma
        m_weights.assign(w.begin(), w.begin() + m_n);
        this->m_sigma = *std::max_element(m_weights.begin(), m_weights.end()) + 1;

        // 3. Build wt_pre: weights in preorder
        {
            sdsl::int_vector<> iv(m_n);
            for (size_type i = 0; i < m_n; ++i)
                iv[i] = m_weights[i];
            wt_pre = std::make_unique<sdsl::wt_int<>>();
            construct_im(*wt_pre, iv);
        }

        // 4. Build wt_exit: weights in exit order
        {
            sdsl::int_vector<> ev(m_n);
            for (size_type v = 0; v < m_n; ++v) {
                auto er = exit_rank(static_cast<node_type>(v));
                ev[er] = m_weights[v];
            }
            wt_exit = std::make_unique<sdsl::wt_int<>>();
            construct_im(*wt_exit, ev);
        }
    }

    virtual ~two_wt() = default;

    // ============================================================
    // path_query_processor interface
    // ============================================================

    /**
     * @brief Count nodes with weight in [a,b] on the path x--y.
     */
    size_type count(node_type x, node_type y,
                    value_type a, value_type b) const override {
        auto z = lca(x, y);
        return count_on_path(x, y, z, a, b);
    }

    /**
     * @brief Path median: return the median weight on the path x--y.
     */
    value_type query(node_type x, node_type y) const override {
        auto z = lca(x, y);
        size_type path_len = depth(x) + depth(y) - 2 * depth(z) + 1;
        return _select(x, y, z, path_len / 2, 0, this->m_sigma - 1);
    }

    /**
     * @brief Path selection: return the qntl-th quantile weight on x--y.
     */
    value_type selection(node_type x, node_type y, size_type qntl) const override {
        auto z = lca(x, y);
        size_type path_len = depth(x) + depth(y) - 2 * depth(z) + 1;
        return _select(x, y, z, this->qntl2rnk(path_len, qntl), 0, this->m_sigma - 1);
    }

    /**
     * @brief Report all nodes on path x--y with weight in [a,b].
     * Walks the path explicitly via parent(). O(path_length).
     */
    void report(node_type x, node_type y,
                value_type a, value_type b,
                std::vector<std::pair<value_type, size_type>> &res) const override {
        auto z = lca(x, y);
        // Walk x -> z (exclusive of z)
        for (auto cur = x; cur != z; ) {
            if (a <= m_weights[cur] && m_weights[cur] <= b)
                res.emplace_back(cur, m_weights[cur]);
            cur = parent(cur).value();
        }
        // Walk y -> z (exclusive of z)
        for (auto cur = y; cur != z; ) {
            if (a <= m_weights[cur] && m_weights[cur] <= b)
                res.emplace_back(cur, m_weights[cur]);
            cur = parent(cur).value();
        }
        // Check z itself
        if (a <= m_weights[z] && m_weights[z] <= b)
            res.emplace_back(z, m_weights[z]);
    }

    value_type weight_of(node_type x) const override {
        return m_weights[x];
    }

    value_type weight(node_type x) const override {
        return m_weights[x];
    }

    // ============================================================
    // succinct_tree interface (delegated to m_bp)
    // ============================================================

    size_type size() const override {
        return m_n;
    }

    std::optional<node_type> parent(node_type x) const override {
        auto pos = node2position(x);
        if (pos == 0)
            return std::nullopt;
        return position2node(m_bp.enclose(pos));
    }

    std::optional<node_type> ancestor(node_type x, size_type i) const override {
        if (i == 0) return x;
        auto d = depth(x);
        if (i > d) return std::nullopt;
        // Walk up i levels
        auto cur = x;
        for (size_type step = 0; step < i; ++step)
            cur = parent(cur).value();
        return cur;
    }

    std::vector<node_type> children(node_type x) const override {
        std::vector<node_type> res;
        auto ix = interval(x);
        for (auto i = ix.first; i + 1 < m_bp.size() && (*m_bv)[i + 1];
             i = m_bp.find_close(i + 1))
            res.push_back(position2node(i + 1));
        return res;
    }

    node_type lca(node_type x, node_type y) const override {
        if (is_ancestor(x, y)) return x;
        if (is_ancestor(y, x)) return y;
        auto ix = interval(x);
        auto iy = interval(y);
        assert(ix.second < iy.first || iy.second < ix.first);
        if (ix.second < iy.first)
            return position2node(m_bp.double_enclose(ix.first, iy.first));
        return position2node(m_bp.double_enclose(iy.first, ix.first));
    }

    size_type depth(node_type x) const override {
        auto pos = node2position(x);
        if (pos == 0) return 0;
        return static_cast<size_type>(m_bp.excess(pos) - 1);
    }

    bool is_ancestor(node_type p, node_type x) const override {
        auto ip = interval(p);
        auto ix = interval(x);
        return ip.first <= ix.first && ix.second <= ip.second;
    }

    bool is_leaf(node_type x) const override {
        auto i = node2position(x);
        return m_bp.find_close(i) == i + 1;
    }
};

#endif // TWO_WT_HPP
