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
     * @brief Lockstep WT descent for the k-th smallest on a path.
     * O(lg sigma) — descends both wavelet trees level-by-level,
     * computing the path-left count at each level from 6 prefix ranges.
     *
     * At each level, we maintain prefix ranges in wt_pre and wt_exit
     * for nodes x, y, z. The left-count on the path is:
     *   anc_left(v) = size(left(rp_v)) - size(left(re_v))
     *   path_left = anc_left(x) + anc_left(y) - 2*anc_left(z) + z_goes_left
     */
    value_type _select(node_type x, node_type y, node_type z,
                       size_type k) const {
        using wt_node = typename sdsl::wt_int<>::node_type;
        using range_t = sdsl::range_type;

        // Compute prefix endpoints for the 6 ranges.
        // wt_pre range [0, v+S-1]: ancestors(v) ∪ subtree(v) ∪ left-non-ancestors
        // wt_exit range [0, er_v-1]: left-non-ancestors ∪ proper-descendants(v)
        size_type ep_x = static_cast<size_type>(x) + subtree_size(x);
        size_type ep_y = static_cast<size_type>(y) + subtree_size(y);
        size_type ep_z = static_cast<size_type>(z) + subtree_size(z);
        size_type ee_x = exit_rank(x);
        size_type ee_y = exit_rank(y);
        size_type ee_z = exit_rank(z);

        bool dbg = (m_n == 7 && x == 6 && y == 5);
        if (dbg) {
            std::cerr << "[DBG] _select x=" << x << " y=" << y << " z=" << z << " k=" << k << "\n";
            std::cerr << "[DBG] ep_x=" << ep_x << " ep_y=" << ep_y << " ep_z=" << ep_z << "\n";
            std::cerr << "[DBG] ee_x=" << ee_x << " ee_y=" << ee_y << " ee_z=" << ee_z << "\n";
            std::cerr << "[DBG] weights: z=" << m_weights[z] << " x=" << m_weights[x] << " y=" << m_weights[y] << "\n";
            // Also print all weights and preorder/exit-order
            std::cerr << "[DBG] All weights (preorder):";
            for (size_type i = 0; i < m_n; ++i) std::cerr << " " << m_weights[i];
            std::cerr << "\n";
            std::cerr << "[DBG] subtree_size: x=" << subtree_size(x) << " y=" << subtree_size(y) << " z=" << subtree_size(z) << "\n";
        }

        // Build inclusive range [0, ep-1]; if ep==0, produce empty range.
        auto make_range = [](size_type ep) -> range_t {
            if (ep == 0) return range_t{1, 0};
            return range_t{0, ep - 1};
        };

        range_t rp_x = make_range(ep_x), rp_y = make_range(ep_y), rp_z = make_range(ep_z);
        range_t re_x = make_range(ee_x), re_y = make_range(ee_y), re_z = make_range(ee_z);

        wt_node vp = wt_pre->root();
        wt_node ve = wt_exit->root();
        uint32_t max_lev = wt_pre->max_level;

        if (dbg) std::cerr << "[DBG] max_lev=" << max_lev << "\n";

        for (uint32_t lev = 0; lev < max_lev; ++lev) {
            // Expand WT nodes to get left/right child nodes
            auto cp = wt_pre->expand(vp);   // [left_child, right_child]
            auto ce = wt_exit->expand(ve);

            // Expand a prefix range, guarding against empty ranges.
            // sdsl::expand computes r[1]-r[0]+1 which underflows for empty ranges.
            // We use size()==0 as the emptiness check rather than sdsl::empty(),
            // because expand() can produce {0, UINT64_MAX} (wrapping) which
            // sdsl::empty() incorrectly considers non-empty.
            auto range_is_empty = [](const range_t& r) -> bool {
                return sdsl::size(r) == 0;
            };
            auto safe_expand = [&range_is_empty](const sdsl::wt_int<>& wt,
                                  const wt_node& v, const range_t& r)
                -> std::array<range_t, 2> {
                if (range_is_empty(r)) return {range_t{1, 0}, range_t{1, 0}};
                auto result = wt.expand(v, r);
                // Guard outputs too: expand can produce wrapped ranges
                if (range_is_empty(result[0])) result[0] = range_t{1, 0};
                if (range_is_empty(result[1])) result[1] = range_t{1, 0};
                return result;
            };

            auto exp_rp_x = safe_expand(*wt_pre, vp, rp_x);
            auto exp_rp_y = safe_expand(*wt_pre, vp, rp_y);
            auto exp_rp_z = safe_expand(*wt_pre, vp, rp_z);
            auto exp_re_x = safe_expand(*wt_exit, ve, re_x);
            auto exp_re_y = safe_expand(*wt_exit, ve, re_y);
            auto exp_re_z = safe_expand(*wt_exit, ve, re_z);

            auto sz = [&range_is_empty](const range_t& r) -> size_type {
                return range_is_empty(r) ? 0 : sdsl::size(r);
            };

            size_type anc_left_x = sz(exp_rp_x[0]) - sz(exp_re_x[0]);
            size_type anc_left_y = sz(exp_rp_y[0]) - sz(exp_re_y[0]);
            size_type anc_left_z = sz(exp_rp_z[0]) - sz(exp_re_z[0]);
            bool z_left = ((m_weights[z] >> (max_lev - 1 - lev)) & 1) == 0;
            size_type path_left = anc_left_x + anc_left_y
                                - 2 * anc_left_z + (z_left ? 1 : 0);

            if (dbg) {
                std::cerr << "[DBG] lev=" << lev << " k=" << k
                          << " anc_left: x=" << anc_left_x << " y=" << anc_left_y << " z=" << anc_left_z
                          << " z_left=" << z_left << " path_left=" << path_left
                          << " => " << (k < path_left ? "LEFT" : "RIGHT") << "\n";
                std::cerr << "[DBG]   rp_x=[" << rp_x[0] << "," << rp_x[1] << "] rp_y=[" << rp_y[0] << "," << rp_y[1] << "] rp_z=[" << rp_z[0] << "," << rp_z[1] << "]\n";
                std::cerr << "[DBG]   re_x=[" << re_x[0] << "," << re_x[1] << "] re_y=[" << re_y[0] << "," << re_y[1] << "] re_z=[" << re_z[0] << "," << re_z[1] << "]\n";
                std::cerr << "[DBG]   sz lp: rpx=" << sz(exp_rp_x[0]) << " rpy=" << sz(exp_rp_y[0]) << " rpz=" << sz(exp_rp_z[0]) << "\n";
                std::cerr << "[DBG]   sz le: rex=" << sz(exp_re_x[0]) << " rey=" << sz(exp_re_y[0]) << " rez=" << sz(exp_re_z[0]) << "\n";
            }

            if (k < path_left) {
                // Go left
                vp = cp[0]; ve = ce[0];
                rp_x = exp_rp_x[0]; rp_y = exp_rp_y[0]; rp_z = exp_rp_z[0];
                re_x = exp_re_x[0]; re_y = exp_re_y[0]; re_z = exp_re_z[0];
            } else {
                // Go right
                k -= path_left;
                vp = cp[1]; ve = ce[1];
                rp_x = exp_rp_x[1]; rp_y = exp_rp_y[1]; rp_z = exp_rp_z[1];
                re_x = exp_re_x[1]; re_y = exp_re_y[1]; re_z = exp_re_z[1];
            }
        }

        return wt_pre->sym(vp);
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
        return _select(x, y, z, path_len / 2);
    }

    /**
     * @brief Path selection: return the qntl-th quantile weight on x--y.
     */
    value_type selection(node_type x, node_type y, size_type qntl) const override {
        auto z = lca(x, y);
        size_type path_len = depth(x) + depth(y) - 2 * depth(z) + 1;
        return _select(x, y, z, this->qntl2rnk(path_len, qntl));
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
