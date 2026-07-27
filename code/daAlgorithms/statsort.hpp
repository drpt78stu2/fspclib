#ifndef BOOST_ALGORITHM_STATSORT_HPP
#define BOOST_ALGORITHM_STATSORT_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <type_traits>
#include <vector>

namespace boost {
    namespace algorithm {

        namespace detail {

            // ── Insertion sorts ────────────────────────────────────────────────────────

            /// Plain arithmetic insertion sort.
            template <typename T>
            inline void statsort_insertion(T* data, std::size_t n) noexcept
            {
                for (std::size_t i = 1; i < n; ++i) {
                    T key = data[i];
                    std::size_t j = i;
                    while (j > 0 && data[j - 1] > key) {
                        data[j] = data[j - 1];
                        --j;
                    }
                    data[j] = key;
                }
            }

            /// Insertion sort for complex objects ordered by proj(element).
            template <typename T, typename Proj>
            inline void statsort_insertion_proj(T* data, std::size_t n, Proj proj) noexcept
            {
                for (std::size_t i = 1; i < n; ++i) {
                    T key = data[i];
                    auto key_val = proj(key);
                    std::size_t j = i;
                    while (j > 0 && proj(data[j - 1]) > key_val) {
                        data[j] = data[j - 1];
                        --j;
                    }
                    data[j] = key;
                }
            }

            // ── Recursive cores ────────────────────────────────────────────────────────

            /// Recursive core for plain arithmetic types.
            template <typename T>
            void statsort_impl(T* data, std::size_t n,
                               double min, double max,
                               T* scratch)
            {
                static constexpr std::size_t THRESHOLD = 16;
                if (n <= THRESHOLD) { statsort_insertion(data, n); return; }

                const std::size_t m     = static_cast<std::size_t>(std::sqrt(static_cast<double>(n)));
                const double      scale = static_cast<double>(m) / (max - min);

                std::vector<std::size_t> cnt(m, 0);
                for (std::size_t i = 0; i < n; ++i) {
                    std::size_t b = static_cast<std::size_t>((static_cast<double>(data[i]) - min) * scale);
                    if (b >= m) b = m - 1;
                    ++cnt[b];
                }

                std::vector<std::size_t> off(m + 1, 0);
                for (std::size_t i = 0; i < m; ++i)
                    off[i + 1] = off[i] + cnt[i];

                {
                    std::vector<std::size_t> pos(off.begin(), off.begin() + m);
                    for (std::size_t i = 0; i < n; ++i) {
                        std::size_t b = static_cast<std::size_t>((static_cast<double>(data[i]) - min) * scale);
                        if (b >= m) b = m - 1;
                        scratch[pos[b]++] = data[i];
                    }
                }

                {
                    std::size_t nonempty = 0;
                    for (std::size_t i = 0; i < m; ++i) if (cnt[i] > 0) ++nonempty;
                    if (nonempty == 1) {
                        std::sort(scratch, scratch + n);
                        std::copy(scratch, scratch + n, data);
                        return;
                    }
                }

                for (std::size_t b = 0; b < m; ++b) {
                    const std::size_t bstart = off[b];
                    const std::size_t bsize  = cnt[b];
                    if (bsize == 0) continue;
                    const double bmin = min + static_cast<double>(b)     * (max - min) / static_cast<double>(m);
                    const double bmax = min + static_cast<double>(b + 1) * (max - min) / static_cast<double>(m);
                    if (bsize <= THRESHOLD)
                        statsort_insertion(scratch + bstart, bsize);
                    else
                        statsort_impl(scratch + bstart, bsize, bmin, bmax, data + bstart);
                }

                std::copy(scratch, scratch + n, data);
            }
	
            template <typename T, typename Proj>
            void statsort_impl_proj(T* data, std::size_t n,
                                    double min, double max,
                                    T* scratch,
                                    Proj proj)
            {
                static constexpr std::size_t THRESHOLD = 16;
                if (n <= THRESHOLD) { statsort_insertion_proj(data, n, proj); return; }

                const std::size_t m     = static_cast<std::size_t>(std::sqrt(static_cast<double>(n)));
                const double      scale = static_cast<double>(m) / (max - min);

                // Pass 1: count
                std::vector<std::size_t> cnt(m, 0);
                for (std::size_t i = 0; i < n; ++i) {
                    std::size_t b = static_cast<std::size_t>((static_cast<double>(proj(data[i])) - min) * scale);
                    if (b >= m) b = m - 1;
                    ++cnt[b];
                }

                // Pass 2: prefix sums
                std::vector<std::size_t> off(m + 1, 0);
                for (std::size_t i = 0; i < m; ++i)
                    off[i + 1] = off[i] + cnt[i];

                // Pass 3: scatter
                {
                    std::vector<std::size_t> pos(off.begin(), off.begin() + m);
                    for (std::size_t i = 0; i < n; ++i) {
                        std::size_t b = static_cast<std::size_t>((static_cast<double>(proj(data[i])) - min) * scale);
                        if (b >= m) b = m - 1;
                        scratch[pos[b]++] = data[i];
                    }
                }

                // Guard: same degenerate-bucket protection as in statsort_impl — if all
                // elements land in one bucket, fall back to std::sort to break the cycle.
                {
                    std::size_t nonempty = 0;
                    for (std::size_t i = 0; i < m; ++i) if (cnt[i] > 0) ++nonempty;
                    if (nonempty == 1) {
                        std::sort(scratch, scratch + n,
                                  [&](const T& a, const T& b){ return proj(a) < proj(b); });
                        std::copy(scratch, scratch + n, data);
                        return;
                    }
                }

                // Pass 4: recurse per bucket
                for (std::size_t b = 0; b < m; ++b) {
                    const std::size_t bstart = off[b];
                    const std::size_t bsize  = cnt[b];
                    if (bsize == 0) continue;
                    if (bsize <= THRESHOLD)
                    {
                        statsort_insertion_proj(scratch + bstart, bsize, proj);
					}
                    else
                    {
                    		const double bmin = min + static_cast<double>(b)     * (max - min) / static_cast<double>(m);
                    		const double bmax = min + static_cast<double>(b + 1) * (max - min) / static_cast<double>(m);
							statsort_impl_proj(scratch + bstart, bsize, bmin, bmax, data + bstart, proj);
					}
                }

                // Pass 5: copy back
                std::copy(scratch, scratch + n, data);
            }

        } // namespace detail


        // -----------------------------------------------------------------------------
        // Public API
        // -----------------------------------------------------------------------------

        template <
        typename Container,
        typename T = typename Container::value_type,
        typename   = std::enable_if_t<std::is_arithmetic_v<T>>
        >
        void statsort(Container& c)
        {
            const std::size_t n = c.size();
            if (n <= 1) return;

            T* data = c.data();
            T min_val = *std::min_element(data, data + n);
            T max_val = *std::max_element(data, data + n);
            if (min_val >= max_val) return;

            const double mind = static_cast<double>(min_val);
            const double maxd = static_cast<double>(max_val)
            + 0.0001 * (static_cast<double>(max_val) - mind);

            std::vector<T> scratch(n);
            detail::statsort_impl(data, n, mind, maxd, scratch.data());
        }
        
        template <
        typename Container,
        typename Proj,
        typename T    = typename Container::value_type,
        typename Key  = std::invoke_result_t<Proj, const T&>,
        typename      = std::enable_if_t<std::is_arithmetic_v<Key>>
        >
        void statsort(Container& c, Proj proj)
        {
            const std::size_t n = c.size();
            if (n <= 1) return;

            T* data = c.data();

            auto key_cmp = [&](const T& a, const T& b) {
                return std::invoke(proj, a) < std::invoke(proj, b);
            };
            Key min_key = std::invoke(proj, *std::min_element(data, data + n, key_cmp));
            Key max_key = std::invoke(proj, *std::max_element(data, data + n, key_cmp));

            if (min_key >= max_key) return; // all keys equal

            const double mind = static_cast<double>(min_key);
            const double maxd = static_cast<double>(max_key)
            + 0.0001 * (static_cast<double>(max_key) - mind);

            // Wrap in a lambda so std::invoke handles both lambdas and member pointers
            auto proj_fn = [&](const T& elem) -> Key {
                return std::invoke(proj, elem);
            };

            std::vector<T> scratch(n);
            detail::statsort_impl_proj(data, n, mind, maxd, scratch.data(), proj_fn);
        }

        template <
        typename RandomIt,
        typename T = typename std::iterator_traits<RandomIt>::value_type,
        typename   = std::enable_if_t<std::is_arithmetic_v<T>>
        >
        void statsort(RandomIt first, RandomIt last)
        {
            const std::size_t n = static_cast<std::size_t>(std::distance(first, last));
            if (n <= 1) return;

            std::vector<T> tmp(first, last);
            statsort(tmp);
            std::copy(tmp.begin(), tmp.end(), first);
        }

        template <
        typename RandomIt,
        typename Proj,
        typename T   = typename std::iterator_traits<RandomIt>::value_type,
        typename Key = std::invoke_result_t<Proj, const T&>,
        typename     = std::enable_if_t<std::is_arithmetic_v<Key>>
        >
        void statsort(RandomIt first, RandomIt last, Proj proj)
        {
            const std::size_t n = static_cast<std::size_t>(std::distance(first, last));
            if (n <= 1) return;

            std::vector<T> tmp(first, last);
            statsort(tmp, proj);
            std::copy(tmp.begin(), tmp.end(), first);
        }


    } // namespace algorithm
} // namespace boost

#endif // BOOST_ALGORITHM_STATSORT_HPP
