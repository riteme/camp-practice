#include <iostream>

#if defined(CUCKOO)
#include "cuckoo.hpp"
#else
#include "linear.hpp"
#endif


int main() {
    using std::cin;
    using std::cout;
    std::ios::sync_with_stdio(false);

#if defined(CUCKOO)
    CuckooHashTable table;
#else
    LinearHashTable table;
#endif

    std::string command;
    std::uint32_t key, value;
    while (cin >> command) {
        if (command == "Set") {
            cin >> key >> value;
            table.set(key, value);
        } else if (command == "Get") {
            cin >> key;
            auto ptr = table.get(key);
            if (ptr)
                cout << *ptr << "\n";
            else
                cout << "null\n";
        } else if (command == "Del") {
            cin >> key;
            table.remove(key);
        }
    }

    return 0;
}
