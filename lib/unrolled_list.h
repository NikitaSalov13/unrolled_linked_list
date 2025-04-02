#pragma once
#include <memory>
#include <random>
#include <ranges>
#include <iostream>

template<typename T, size_t NodeMaxSize = 10, typename t_allocator = std::allocator<T>>
class unrolled_list {

struct Node {
        size_t node_size;
        T* data;
        Node *next;
        Node *prev;

        Node() : node_size(0), data(nullptr), next(nullptr), prev(nullptr) {}

        Node(const size_t node_size, T *data, Node *next, Node *prev)
            : node_size(node_size)
            , data(data)
            , next(next)
            , prev(prev) {
        }

        Node(const Node &other)
            : node_size(other.node_size)
            , data(other.data)
            , next(other.next)
            , prev(other.prev) {
        }

        Node& operator=(const Node &other) {
            if (this == &other)
                return *this;
            node_size = other.node_size;
            data = other.data;
            next = other.next;
            prev = other.prev;
            return *this;
        }
    };

public:

    template<bool IsConst = false>
    struct ul_iterator;

    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using iterator = ul_iterator<>;
    using allocator_type = t_allocator;
    using const_iterator = ul_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;
    using t_allocator_traits = std::allocator_traits<t_allocator>;
    using node_allocator = typename t_allocator_traits::template rebind_alloc<Node>;
    using node_allocator_traits = std::allocator_traits<node_allocator>;


private:

    Node sentinel_;
    Node* tail_;
    Node* head_;
    size_t size_{};
    bool is_empty_ = false;
    allocator_type t_alloc_;
    node_allocator node_alloc_;

public:

    template<bool IsConst>
    struct ul_iterator {

        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using ref = T&;
        using node_ptr = std::conditional_t<IsConst, const Node*, Node*>;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using iterator_category = std::bidirectional_iterator_tag;

    protected:

        node_ptr current_node;
        size_t current_index;

    public:

        friend class unrolled_list;

        ul_iterator() : current_node(nullptr) , current_index(0) {}
        ul_iterator(const ul_iterator &other)
            : current_node(other.current_node)
              , current_index(other.current_index) {
        }
        ul_iterator(node_ptr node, const size_t index) : current_node(node), current_index(index) {
        }
        ul_iterator &operator=(const ul_iterator &other) = default;

        friend void swap(ul_iterator &it1, ul_iterator &it2) noexcept {
            std::swap(it1.current_node, it2.current_node);
            std::swap(it1.current_index, it2.current_index);
        }

        bool operator==(const ul_iterator& other) const {
            return (current_node == other.current_node && current_index == other.current_index);
        }

        bool operator!=(const ul_iterator& other) const {
            return (current_node != other.current_node || current_index != other.current_index);
        }

        reference operator*() const {
            return current_node->data[current_index];
        }

        pointer operator->() const {
            return &(current_node->data[current_index]);
        }

        ul_iterator& operator++() {
            if (current_index + 1 < current_node->node_size) {
                ++current_index;
                return *this;
            }
            current_node = current_node->next;
            current_index = 0;
            return *this;
        }

        ul_iterator operator+(size_t count) const {
            ul_iterator it = *this;

            while (count > 0) {
                const size_t remaining_in_node = it.current_node->node_size - it.current_index;
                if (count < remaining_in_node) {
                    it.current_index += count;
                    return it;
                }
                count -= remaining_in_node;
                it.current_node = it.current_node->next;
                it.current_index = 0;
            }

            return it;
        }

        ul_iterator operator-(size_t count) const {
            ul_iterator it = *this;

            while (count > 0) {
                if (it.current_index >= count) {
                    it.current_index -= count;
                    return it;
                }
                if (it.current_node->prev == nullptr) {
                    return it;
                }

                count -= (it.current_index + 1);
                it.current_node = it.current_node->prev;
                it.current_index = it.current_node->node_size - 1;
            }

            return it;
        }

        ul_iterator operator++(int) {
            ul_iterator temp = *this;
            ++(*this);
            return temp;
        }

        ul_iterator& operator--() {
            if (current_index > 0) {
                --current_index;
                return *this;
            }
            current_node = current_node->prev;
            current_index = NodeMaxSize - 1;
            return *this;
        }


        ul_iterator operator--(int) {
            ul_iterator temp = *this;
            --(*this);
            return temp;
        }

        ~ul_iterator() = default;
    };

    unrolled_list() : sentinel_(), tail_(nullptr), head_(nullptr), size_(0), is_empty_(true) {
    }
    explicit unrolled_list(const allocator_type& alloc)
    : tail_(nullptr), head_(nullptr), size_(0), is_empty_(true), t_alloc_(alloc), node_alloc_(alloc) {}

    unrolled_list(const unrolled_list &other) {
        for (auto& i : other) {
            push_back(i);
        }
    }

    unrolled_list(const unrolled_list& other, const allocator_type& alloc)
    : tail_(nullptr), head_(nullptr), size_(0), is_empty_(true), t_alloc_(alloc), node_alloc_(alloc) {
        for (const auto& elem : other) {
            push_back(elem);
        }
    }
    unrolled_list(const size_t count, const T& value) {
        if (count == 0) {
            sentinel_ = Node();
            is_empty_ = true;
            head_ = nullptr;
            tail_ = nullptr;
            size_ = 0;

            return;
        }
        const size_t rem = count % NodeMaxSize;
        size_t node_count = count / NodeMaxSize;
        size_t ul_size = node_count * NodeMaxSize;

        if (rem) {
            ++node_count;
            ul_size += rem;
        }

        size_ = ul_size;

        Node* prev_node = nullptr;

        for (size_t i = 0; i < node_count; ++i) {
            Node* new_node = node_allocator_traits::allocate(node_alloc_, 1);
            node_allocator_traits::construct(node_alloc_, new_node, 0, nullptr, nullptr, prev_node);
            new_node->data = t_allocator_traits::allocate(t_alloc_, NodeMaxSize);

            if (i == 0) {
                head_ = new_node;
            }

            if (i == node_count - 1 && rem != 0) {
                for (size_t j = 0; j < rem; j++) {
                    t_allocator_traits::construct(t_alloc_, new_node->data + j, value);
                    ++new_node->node_size;
                }
                prev_node->next = new_node;
                tail_ = new_node;

                return;
            }
            for (size_t j = 0; j < NodeMaxSize; j++) {
                t_allocator_traits::construct(t_alloc_, new_node->data + j, value);
                ++new_node->node_size;

            }

            if (i == node_count - 1) {
                tail_ = new_node;
            }

            if (prev_node) {
                prev_node->next = new_node;
            }
            prev_node = new_node;
        }
    }

    template<typename InputIterator, typename = std::enable_if_t<!std::is_integral_v<InputIterator>>>
    unrolled_list(InputIterator first, InputIterator last) {

        if (first == last) {
            is_empty_ = true;
            head_ = nullptr;
            tail_ = nullptr;
            size_ = 0;

            return;
        }
        is_empty_ = false;

        size_t ul_size = 0;
        Node* prev_node = nullptr;
        bool is_first_it = true;
        while (first != last) {
            Node* new_node = node_allocator_traits::allocate(node_alloc_, 1);
            node_allocator_traits::construct(node_alloc_, new_node, 0, nullptr, nullptr, prev_node);

            if (is_first_it) {
                head_ = new_node;
                is_first_it = false;
            }

            new_node->data = t_allocator_traits::allocate(t_alloc_, NodeMaxSize);
            for (int i = 0; i < NodeMaxSize; ++i) {
                t_allocator_traits::construct(t_alloc_, new_node->data + i, *first);
                ++ul_size;
                ++new_node->node_size;
                ++first;
                if (first == last) {
                    if (prev_node) {
                        prev_node->next = new_node;
                    }
                    tail_ = new_node;
                    break;
                }

            }
            if (prev_node) {
                prev_node->next = new_node;
            }
            prev_node = new_node;
        }


        size_ = ul_size;
    }

   template<typename InputIterator, typename = std::enable_if_t<!std::is_integral_v<InputIterator>>>
unrolled_list(InputIterator first, InputIterator last, const allocator_type alloc) : t_alloc_(alloc) {
    if (first == last) {
        is_empty_ = true;
        head_ = nullptr;
        tail_ = nullptr;
        size_ = 0;
        return;
    }

    is_empty_ = false;

    std::vector<Node*> allocated_nodes;
    size_t ul_size = 0;
    Node* prev_node = nullptr;
    bool is_first_it = true;
    size_t sucussfull_construct = 0;

    try {

        while (first != last) {
            Node* new_node = node_allocator_traits::allocate(node_alloc_, 1);
            allocated_nodes.push_back(new_node);
            node_allocator_traits::construct(node_alloc_, new_node, 0, nullptr, nullptr, prev_node);

            if (is_first_it) {
                head_ = new_node;
                is_first_it = false;
            }

            new_node->data = t_allocator_traits::allocate(t_alloc_, NodeMaxSize);


            for (int i = 0; i < NodeMaxSize; ++i) {
                t_allocator_traits::construct(t_alloc_, new_node->data + i, *first);
                sucussfull_construct++;
                ++ul_size;
                ++new_node->node_size;
                ++first;
                if (first == last) {
                    if (prev_node) {
                        prev_node->next = new_node;
                    }
                    tail_ = new_node;
                    break;
                }
            }


            if (prev_node) {
                prev_node->next = new_node;
            }

            prev_node = new_node;
        }


        size_ = ul_size;

    } catch (...) {
        const size_t full_destroyed_nodes = sucussfull_construct / NodeMaxSize;
        const size_t rem = sucussfull_construct % NodeMaxSize;
        size_t it = 0;
        while (it != full_destroyed_nodes) {
            Node* node = allocated_nodes[it];
            for (int i = 0; i < NodeMaxSize; ++i) {
                t_allocator_traits::destroy(t_alloc_, node->data + i);
            }
            t_allocator_traits::deallocate(t_alloc_, node->data, NodeMaxSize);

            node_allocator_traits::destroy(node_alloc_, node);
            node_allocator_traits::deallocate(node_alloc_, node, 1);
            ++it;
        }
        if (rem != 0) {
            Node* node = allocated_nodes[it];
            for (int i = 0; i < rem; ++i) {
                t_allocator_traits::destroy(t_alloc_, node->data + i);
            }
            t_allocator_traits::deallocate(t_alloc_, node->data, NodeMaxSize);

            node_allocator_traits::destroy(node_alloc_, node);
            node_allocator_traits::deallocate(node_alloc_, node, 1);
        }
        throw;
    }
}





    unrolled_list(std::initializer_list<T> list) : unrolled_list(list.begin(), list.end()) {}

    ~unrolled_list() {
        Node* current_node = head_;
        while (current_node) {
            Node* temp = current_node->next;
            for (int i = 0; i < current_node->node_size; ++i) {
                t_allocator_traits::destroy(t_alloc_, current_node->data + i);
            }
            t_allocator_traits::deallocate(t_alloc_, current_node->data, NodeMaxSize);
            node_allocator_traits::destroy(node_alloc_, current_node);
            node_allocator_traits::deallocate(node_alloc_, current_node, 1);
            current_node = temp;
        }
    }

    unrolled_list &operator=(const unrolled_list &other) {
        if (this != &other) {
            t_alloc_ = other.t_alloc_;
            node_alloc_ = other.node_alloc_;
            unrolled_list temp(other.cbegin(), other.cend());
            swap(temp);
        }
        return *this;
    }

    Node* allocate_node() {
        return node_allocator_traits::allocate(node_alloc_, 1);
    }

    T* allocate_t() {
        return t_allocator_traits::allocate(t_alloc_, NodeMaxSize);
    }

    void construct_node(Node* place, const size_t& node_size, T* data, Node* next, Node* prev) {
        node_allocator_traits::construct(node_alloc_, place, node_size, data, next, prev);
    }

    void construct_t(T* place, const T& value) {
        t_allocator_traits::construct(t_alloc_, place, value);
    }

    void destroy_t(T* place) {
        t_allocator_traits::destroy(t_alloc_, place);
    }

    void destroy_node(Node* place) {
        node_allocator_traits::destroy(node_alloc_, place);
    }

    void delete_node(Node* current_node) {
        for (int i = 0; i < NodeMaxSize; ++i) {
            t_allocator_traits::destroy(t_alloc_, current_node->data + i);
        }
        t_allocator_traits::deallocate(t_alloc_, current_node->data, NodeMaxSize);
        node_allocator_traits::destroy(node_alloc_, current_node);
        node_allocator_traits::deallocate(node_alloc_, current_node, 1);
    }


    Node* create_node_after(Node* current_node, const size_t size) {
        Node* prev_next = current_node->next;
        current_node->next = allocate_node();
        if (prev_next) {
            prev_next->prev = current_node->next;
        }
        construct_node(current_node->next, size, nullptr, prev_next, current_node);
        Node* new_node = current_node->next;
        new_node->data = allocate_t();

        if (tail_ == current_node) {
            tail_ = new_node;
        }

        return new_node;
    }


    void push_back(const T& value) {
        Node* allocated_node = nullptr;
        Node* prev_tail = tail_;
        try {
            ++size_;
            if (tail_ == nullptr && head_ == nullptr) {
                Node* new_node = allocate_node();
                allocated_node = new_node;
                construct_node(new_node, 1, nullptr, nullptr, nullptr);
                new_node->data = allocate_t();
                construct_t(new_node->data, value);
                tail_ = new_node;
                head_ = new_node;
                is_empty_ = false;
                return;
            }
            if (tail_->node_size != NodeMaxSize) {
                construct_t(tail_->data + tail_->node_size, value);
                ++tail_->node_size;
                return;
            }
            Node* new_node = create_node_after(tail_, 1);
            construct_t(new_node->data,value);
        } catch (...) {
            if (allocated_node) {
                allocated_node->prev->next = nullptr;
                delete_node(allocated_node);
            }
            tail_ = prev_tail;
            throw;
        }
    }
    void push_front(const T& value) {
        Node* allocated_node = nullptr;
        Node* prev_head = head_;
        try {
            ++size_;
            if (tail_ == nullptr && head_ == nullptr) {

                Node* new_node = allocate_node();
                construct_node(new_node, 1, nullptr, nullptr, nullptr);
                new_node->data = allocate_t();
                construct_t(new_node->data, value);
                tail_ = new_node;
                head_ = new_node;
                return;

            }

            if (head_->node_size != NodeMaxSize) {

                for (size_t i = head_->node_size; i > 0; --i) {
                    head_->data[i] = head_->data[i - 1];
                }
                head_->data[0] = value;
                ++head_->node_size;
                return;

            }

            Node* new_node = allocate_node();
            construct_node(new_node, 1, nullptr, head_, nullptr);
            new_node->data = allocate_t();
            construct_t(new_node->data, value);

            head_->prev = new_node;
            head_ = new_node;
        } catch (...) {
            if (allocated_node) {
                allocated_node->next->prev = nullptr;
                delete_node(allocated_node);
            }
            head_ = prev_head;
            throw;
        }
    }


    void pop_back() noexcept {
        if (is_empty_) {
            return;
        }

        --size_;
        if (size_ == 0) {
            is_empty_ = true;
        }

        destroy_t(tail_->data + (tail_->node_size - 1));
        --tail_->node_size;

        check_node_empty(tail_);
    }



    void pop_front() noexcept {
        if (size_ == 0) {
            return;
        }
        --size_;
        if (size_ == 0) {
            is_empty_ = true;
        }
        ul_iterator<> it = begin();
        while (it.current_index != head_->node_size - 1) {
            *it = *(it + 1);
            ++it;
        }
        destroy_t(head_->data + (head_->node_size - 1));
        --head_->node_size;
        check_node_empty(head_);
    }


    ul_iterator<> insert(const_iterator position, const T& value) {
        ++size_;

        ul_iterator<> pos = position;

        if (pos == end()) {
            push_back(value);
            return ul_iterator<>(tail_, tail_->node_size - 1);
        }

        size_t& current_node_size = pos.current_node->node_size;
        Node* current_node = pos.current_node;
        T* new_t_position = pos.current_node->data + current_node_size;

        if (current_node_size != NodeMaxSize) {
            construct_t(new_t_position, *(pos.current_node->data + (current_node_size - 1)));
            if (pos.current_index == current_node_size - 1) {
                *pos = value;
            } else {
                ul_iterator<> it(current_node, current_node_size - 1);
                while (it != pos) {
                    *it = *(it-1);
                    --it;
                }
                *it = value;
            }
            ++current_node_size;
            ul_iterator<> result(current_node, current_node_size - 1);
            return result;
        }
        Node* new_node = create_node_after(current_node, 1);
        construct_t(new_node->data, current_node->data[NodeMaxSize - 1]);
        if (pos.current_index == current_node_size - 1) {
            construct_t(new_node->data, current_node->data[NodeMaxSize - 1]);
            current_node->data[NodeMaxSize - 1] = value;
        } else {
            ul_iterator<> it(current_node, current_node_size - 1);
            while (it != pos) {
                *it = *(it-1);
                --it;
            }
            *it = value;
        }

        ul_iterator<> result(current_node, current_node_size - 1);
        return result;
    }

    ul_iterator<> insert(iterator position, const T& value) {
        ++size_;

        ul_iterator<> pos = position;
        Node* allocated_node;
        Node* prev_value_node = position.current_node;
        Node* prev_next_value_node = position.current_node->next;
        size_t current_size = size_;
        Node* current_tail = tail_;
        Node* current_head = head_;

        try {
            if (pos == end()) {
                push_back(value);
                return ul_iterator<>(tail_, tail_->node_size - 1);
            }

            size_t& current_node_size = pos.current_node->node_size;
            Node* current_node = pos.current_node;
            T* new_t_position = pos.current_node->data + current_node_size;

            if (current_node_size != NodeMaxSize) {
                construct_t(new_t_position, *(pos.current_node->data + (current_node_size - 1)));
                if (pos.current_index == current_node_size - 1) {
                    *pos = value;
                } else {
                    ul_iterator<> it(current_node, current_node_size - 1);
                    while (it != pos) {
                        *it = *(it-1);
                        --it;
                    }
                    *it = value;
                }
                ++current_node_size;
                ul_iterator<> result(current_node, current_node_size - 1);
                return result;
            }
            Node* new_node = create_node_after(current_node, 1);
            allocated_node = new_node;
            construct_t(new_node->data, current_node->data[NodeMaxSize - 1]);
            if (pos.current_index == current_node_size - 1) {
                construct_t(new_node->data, current_node->data[NodeMaxSize - 1]);
                current_node->data[NodeMaxSize - 1] = value;
            } else {
                ul_iterator<> it(current_node, current_node_size - 1);
                while (it != pos) {
                    *it = *(it-1);
                    --it;
                }
                *it = value;
            }

            ul_iterator<> result(current_node, current_node_size - 1);
            return result;
        } catch (...) {
            position.current_node = prev_value_node;
            position.current_node->next = prev_next_value_node;
            size_ = current_size;
            head_ = current_head;
            tail_ = current_tail;

            throw;
        }
    }

    ul_iterator<> insert(const_iterator position, const size_t count, const T& value) {
        ul_iterator<> pos = position;
        for (int i = 0; i < count; ++i) {
            insert(pos, value);
        }
        return pos;
    }

    template<typename InputIterator, typename = std::enable_if_t<!std::is_integral_v<InputIterator>>>
    ul_iterator<> insert(const_iterator position, InputIterator first, InputIterator last) {
        ul_iterator<> pos = position;
        --last;
        while (last != first) {
            insert(pos, *last);
            --last;
        }
        insert(pos, *first);
        return pos;
    }

    ul_iterator<> insert(const_iterator position, std::initializer_list<T> list) {
        ul_iterator<> pos = position;
        insert(pos, list.begin(), list.end());
        return pos;
    }

    void check_node_empty(Node* current_node) {
        if (current_node->node_size == 0) {
            if (current_node == head_ && current_node == tail_) {
                head_ = nullptr;
                tail_ = nullptr;
            }

            if (current_node == tail_) {
                if (current_node->prev) {
                    current_node->prev->next = current_node->next;
                }
                tail_ = current_node->prev;
            }
            else if (current_node == head_) {
                if (current_node->next) {
                    current_node->next->prev = current_node->prev;
                }
                head_ = current_node->next;
            }
            else {
                if (current_node->prev) {
                    current_node->prev->next = current_node->next;
                }
                if (current_node->next) {
                    current_node->next->prev = current_node->prev;
                }
            }
            delete_node(current_node);
        }
    }

    ul_iterator<> erase(const_iterator position) noexcept {
        --size_;
        ul_iterator<> pos = position;
        size_t& current_node_size = pos.current_node->node_size;
        Node* current_node = pos.current_node;

        if (pos.current_index == current_node_size - 1) {
            destroy_t(&(*pos));
            --current_node_size;
            if (current_node == tail_) {
                return end();
            }
            ul_iterator<> result(current_node->next, 0);
            check_node_empty(current_node);
            return result;
        }
        ul_iterator<> it(pos);
        while (it.current_index != current_node_size - 1) {
            *it = *(it + 1);
            ++it;
        }
        destroy_t(current_node->data + (current_node_size-1));
        --current_node_size;
        return pos;
    }

    template<typename InputIterator>
    ul_iterator<> erase(InputIterator first, InputIterator last) {
        size_t count = 0;
        ul_iterator<> result(first);
        while (first != last) {
            ++first;
            ++count;
        }
        for (int i = 0; i < count; ++i) {
            result = erase(result);
        }
        return result;
    }

    void clear() noexcept {
        Node* current_node = head_;
        while(current_node) {
            Node* temp = current_node;
            current_node = current_node->next;
            delete_node(temp);
        }
        is_empty_ = true;
        size_ = 0;
        head_ = tail_ = nullptr;
    }

    template <typename InputIterator, typename = std::enable_if_t<!std::is_integral_v<InputIterator>>>
    void assign(InputIterator first, InputIterator last) {
        ul_iterator<> it = begin();
        while (first != last) {
            *it = *first;
            ++it;
            ++first;
        }
    }

    void assign(const size_t count, const T& value) {
        ul_iterator<> it = begin();
        for (int i = 0; i < count; ++i) {
            *it = value;
            ++it;
        }
    }

    void assign(std::initializer_list<T> list) {
        assign(list.begin(), list.end());
    }


    ul_iterator<> begin() {
        if (size_ == 0) {
            return ul_iterator(&sentinel_,0);
        }
        return iterator(head_,0);
    }

    ul_iterator<true> begin() const {
        if (size_ == 0) {
            return ul_iterator<true>(&sentinel_, 0);
        }
        return ul_iterator<true>(head_,0);
    }

    ul_iterator<true> cbegin()  {
        if (size_ == 0) {
            return ul_iterator(&sentinel_,0);
        }
        return ul_iterator<true>(head_,0);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    ul_iterator<true> cbegin() const {
        if (size_ == 0) {
            return ul_iterator<true>(&sentinel_, 0);
        }
        return ul_iterator<true>(head_,0);
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(cend());
    }

    ul_iterator<> end() {
        if (size_ == 0) {
            return ul_iterator(&sentinel_,0);
        }
        return ul_iterator(tail_->next, 0);
    }

    ul_iterator<true> end() const {
        if (size_ == 0) {
            return ul_iterator<true>(&sentinel_, 0);
        }
        return ul_iterator<true>(tail_->next, 0);
    }

    ul_iterator<true> cend() {
        if (size_ == 0) {
            return ul_iterator(&sentinel_,0);
        }
        return ul_iterator<true>(tail_->next, 0);
    }
    ul_iterator<true> cend() const {
        if (size_ == 0) {
            return ul_iterator<true>(&sentinel_, 0);
        }
        return ul_iterator<true>(tail_->next, 0);
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(cbegin());
    }

    T& front() {
        return *begin();
    }
    const T& front() const {
        return *begin();
    }

    T& back() {
        return tail_->data[tail_->node_size-1];
    }

    const T& back() const {
        return tail_->data[tail_->node_size-1];
    }

    size_t size() {
        return size_;
    }

    size_t size() const {
        return size_;
    }

    size_t max_size() {
        return node_allocator_traits::max_size(node_alloc_) * NodeMaxSize;
    }

    allocator_type get_allocator() const noexcept {
        return t_alloc_;
    }

    bool operator==(const unrolled_list& other) const {
        if (size_ != other.size_) {
            return false;
        }
        return std::equal(begin(), end(), other.begin());
    }

    bool operator!=(const unrolled_list& other) {
        return !(*this == other);
    }

    void swap(unrolled_list& other) {

        std::swap(tail_, other.tail_);
        std::swap(head_, other.head_);
        std::swap(size_, other.size_);
    }

    bool empty() const {
        return is_empty_;
    }

};
