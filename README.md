# Unrolled List

A lightweight C++20 container that implements the [Unrolled Linked List](https://en.wikipedia.org/wiki/Unrolled_linked_list) data structure. Compared with `std::list`, it stores several elements per node, offering better cache locality while still providing constant‑time insertion and removal at both ends.

---

##  Features

- **STL‑compatible API** – the class meets the named requirements for:
  - `Container`, `SequenceContainer` (except `emplace*`, `assign_range`, `prepend_range`, `operator[]`)
  - `ReversibleContainer`
  - `AllocatorAwareContainer`
  - bidirectional iterators
- **Configurable design** – the primary template is
  ```cpp
  template<class T,
           std::size_t NodeMaxSize = 10,
           class Allocator = std::allocator<T>>
  class unrolled_list;
  ```
- **Predictable complexity & strong exception safety** for all modifying operations.
- **Zero dependency** – does not rely on other standard containers.
- **Comprehensive unit tests** built with Google Test (>90 % statement coverage).

---

##  Complexity & Guarantees

| Operation    | Complexity (single / N elements) | Exception guarantee |
| ------------ | -------------------------------- | ------------------- |
| `push_back`  | O(1)                             | strong              |
| `push_front` | O(1)                             | strong              |
| `pop_back`   | O(1)                             | `noexcept`          |
| `pop_front`  | O(1)                             | `noexcept`          |
| `insert`     | O(1) / O(N)                      | strong              |
| `erase`      | O(1) / O(N)                      | `noexcept`          |
| `clear`      | O(N)                             | `noexcept`          |

---


##  Quick Start

```cpp
#include "unrolled_list.h"
#include <iostream>

int main() {
    unrolled_list<int, 8> list{1, 2, 3};   // initializer‑list ctor
    list.push_front(0);                    // 0 1 2 3
    list.insert(++list.begin(), 42);       // 0 42 1 2 3
    list.pop_back();                       // 0 42 1 2

    for (int v : list) std::cout << v << ' ';
}
```

### Iterator Interop

```cpp
std::reverse(list.begin(), list.end());  // works with std algorithms
```

---


##  Contributing

Pull requests and issues are welcome! 
---


