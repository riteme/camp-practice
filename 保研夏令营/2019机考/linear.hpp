#pragma once

#include <cstdint>
#include <cassert>

#include <tuple>
#include <vector>


template <
    typename TKey = std::uint32_t,
    typename TValue = std::uint32_t
>
class LinearHashTable {
public:
    static constexpr size_t INIT_TABLE_SIZE = 8;

    LinearHashTable() : m_internal(INIT_TABLE_SIZE) {}
    LinearHashTable(const LinearHashTable &) = default;

    void set(const TKey &key, const TValue &value) {
        if (need_expand())
            expand();

        m_internal.set(key, value);
    }

    auto get(const TKey &key) const -> const TValue * {
        return m_internal.get(key);
    }

    void remove(const TKey &key) {
        return m_internal.remove(key);
    }

    auto capacity() const -> size_t {
        return m_internal.capacity();
    }

    auto size() const -> size_t {
        return m_internal.size;
    }

private:
    struct Internal {
        enum State : char {
            UNUSED, USED, TOMBSTONE
        };

        Internal(size_t capacity) : size(0) {
            state.resize(capacity, UNUSED);
            keys.resize(capacity);
            values.resize(capacity);
        }

        Internal(const Internal &) = default;
        Internal &operator=(Internal &&) = default;

        size_t size;
        std::vector<State> state;
        std::vector<TKey> keys;
        std::vector<TValue> values;

        auto capacity() const -> size_t {
            return state.size();
        }

        auto h(const TKey &key) const -> size_t {
            return static_cast<size_t>(key) % capacity();
        }

        auto next(size_t i) const -> size_t {
            return i + 1 < capacity() ? i + 1 : 0;
        }

        auto prev(size_t i) const -> size_t {
            return i == 0 ? capacity() - 1 : i - 1;
        }

        auto search(const TKey &key) const -> std::tuple<size_t, bool> {
            auto i = h(key);

            size_t count = 0;
            while (count < size &&
                ((state[i] == USED && keys[i] != key) || state[i] == TOMBSTONE)) {
                i = next(i);
                count++;
            }

            if (state[i] == USED && keys[i] == key)
                return std::make_tuple(i, true);

            assert(size < capacity());

            i = h(key);
            while (state[i] == USED) {
                i = next(i);
            }

            return std::make_tuple(i, false);
        }

        void set(const TKey &key, const TValue &value) {
            assert(size < capacity());

            bool found;
            size_t i;
            std::tie(i, found) = search(key);

            if (!found) {
                size++;
                state[i] = USED;
                keys[i] = key;
            }

            values[i] = value;
        }

        auto get(const TKey &key) const -> const TValue * {
            bool found;
            size_t i;
            std::tie(i, found) = search(key);
            return found ? &values[i] : nullptr;
        }

        void remove(const TKey &key) {
            bool found;
            size_t i;
            std::tie(i, found) = search(key);

            if (found) {
                size--;
                state[i] = TOMBSTONE;
            }
        }
    };

    Internal m_internal;

    bool need_expand() const {
        return size() * 2 > capacity();
    }

    void expand() {
        Internal replacement(2 * capacity());

        for (size_t i = 0; i < capacity(); i++) {
            if (m_internal.state[i] == Internal::USED)
                replacement.set(m_internal.keys[i], m_internal.values[i]);
        }

        m_internal = std::move(replacement);
    }
};
