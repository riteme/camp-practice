#pragma once

#include <cstdio>
#include <cstdint>

#include <vector>


template <
    typename TKey = std::uint32_t,
    typename TValue = std::uint32_t
>
class CuckooHashTable {
public:
    struct Slot {
        int side;
        size_t index;
    };

    static constexpr size_t INIT_TABLE_SIZE = 8;

    CuckooHashTable() : m_internal(INIT_TABLE_SIZE) {}

    CuckooHashTable(const CuckooHashTable &) = default;

    auto size() const -> size_t {
        return m_internal.size;
    }

    auto capacity() const -> size_t {
        return m_internal.capacity();
    }

    void set(const TKey &key, const TValue &value) {
        while (!m_internal.set(key, value)) {
            expand();
        }
    }

    auto get(const TKey &key) const -> const TValue * {
        return m_internal.get(key);
    }

    void remove(const TKey &key) {
        m_internal.remove(key);
    }

private:
    friend class CuckooVisualization;

    struct Table {
        size_t capacity;
        std::vector<bool> used;
        std::vector<TKey> keys;
        mutable std::vector<TValue> values;
        std::vector<char> count;

        Table(size_t _capacity) : capacity(_capacity) {
            used.resize(capacity * 2);
            keys.resize(capacity * 2);
            values.resize(capacity * 2);
            count.resize(capacity * 2);
        }

        Table(const Table &) = default;
        Table &operator=(Table &&) = default;

        auto index(const Slot &slot) const -> size_t {
            return (slot.side ? capacity : 0) + slot.index;
        }
    };

    struct Internal {
        size_t size;
        Table table;

        Internal(size_t capacity) : size(0), table(capacity) {}

        Internal(const Internal &) = default;
        Internal &operator=(Internal &&) = default;

        auto capacity() const -> size_t {
            return table.capacity;
        }

        auto h(int side, const TKey &key) const -> size_t {
            // it seems that both hash functions are too weak for cuckoo hashing.
            if (side)
                return static_cast<size_t>(key) / capacity() % capacity();
            else
                return static_cast<size_t>(key) % capacity();
        }

        bool push(
            int side,
            const TKey &key,
            const TValue &value,
            std::vector<Slot> *trace = nullptr
        ) {
            auto slot = Slot{side, h(side, key)};
            auto i = table.index(slot);

            if (trace)
                trace->push_back(slot);

            if (table.count[i] >= 2)
                return false;

            bool ok = true;
            table.count[i]++;

            if (table.used[i] && table.keys[i] != key) {
                auto victim_key = table.keys[i];
                auto victim_value = table.values[i];
                table.keys[i] = key;
                table.values[i] = value;

                ok = push(side ^ 1, victim_key, victim_value, trace);

                // rollback on failure.
                if (!ok) {
                    table.keys[i] = victim_key;
                    table.values[i] = victim_value;
                }
            } else {
                if (!table.used[i]) {
                    size++;
                    table.used[i] = true;
                    table.keys[i] = key;
                }

                table.values[i] = value;
            }

            table.count[i]--;
            return ok;
        }

        bool set(const TKey &key, const TValue &value) {
            auto ptr = get(key);

            if (ptr) {
                *ptr = value;
                return true;
            } else
                return push(0, key, value);
        }

        auto get(const TKey &key) const -> TValue * {
            for (int side = 0; side < 2; side++) {
                auto i = table.index({side, h(side, key)});
                if (table.used[i] && table.keys[i] == key)
                    return &table.values[i];
            }

            return nullptr;
        }

        void remove(const TKey &key) {
            for (int side = 0; side < 2; side++) {
                auto i = table.index({side, h(side, key)});
                if (table.used[i] && table.keys[i] == key) {
                    size--;
                    table.used[i] = false;
                    break;
                }
            }
        }
    };

    Internal m_internal;

    void expand() {
        // fprintf(stderr, "capacity()=%zu\n", capacity());
        for (size_t new_capacity = 2 * capacity(); ; new_capacity *= 2) {
            // fprintf(stderr, "new=%zu\n", new_capacity);
            Internal replacement(new_capacity);

            bool ok = true;
            for (size_t i = 0; ok && i < 2 * capacity(); i++) {
                if (m_internal.table.used[i]) {
                    auto key = m_internal.table.keys[i];
                    auto value = m_internal.table.values[i];
                    ok &= replacement.push(0, key, value);
                }
            }

            if (ok) {
                m_internal = std::move(replacement);
                break;
            }
        }
    }
};
