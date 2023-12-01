#include <iostream>
#include <memory>

template<size_t N>
class StackStorage {
 public:
  alignas(max_align_t) char storage[N];
  size_t sz_ = 0;
  StackStorage() = default;
  StackStorage(const StackStorage&) = delete;
  ~StackStorage() = default;
  StackStorage& operator=(const StackStorage&) = delete;

  template<typename T>
  T* allocate(size_t count) {
    size_t n = count * sizeof(T);
    sz_ = sz_ + n + alignof(T) - sz_ % alignof(T);
    if (sz_ > N) {
      throw std::bad_alloc();
    }
    return reinterpret_cast<T*>(storage + sz_ - n);
  }
};

template<typename T, size_t N>
class StackAllocator {
 public:
  using value_type = T;
  StackStorage<N>* storage;

  template<typename U, size_t M>

  StackAllocator() = delete;
  ~StackAllocator() = default;
  StackAllocator(StackStorage<N>& storage) : storage(&storage) {}
  template<typename U>
  StackAllocator(const StackAllocator<U, N>& other) : storage(other.storage) {}
  template<typename U>
  StackAllocator& operator=(const StackAllocator<U, N>& other) {
    storage = other.storage;
    return *this;
  }

  template<typename U>
  struct rebind {
    using other = StackAllocator<U, N>;
  };

  T* allocate(size_t count) {
    return reinterpret_cast<T*>(storage->template allocate<T>(count));
  }
  void deallocate(T*, size_t) {}

  bool operator==(const StackAllocator<T, N>& other) const {
    return storage == other.storage;
  }
  bool operator!=(const StackAllocator<T, N>& other) const {
    return !(*this == other);
  }
};

template<typename T, typename Allocator = std::allocator<T>>
class List {
 public:
  struct BaseNode {
   public:
    BaseNode* prev;
    BaseNode* next;
    BaseNode() : prev(this), next(this) {}
  };
  struct Node : BaseNode {
   public:
    T value;
    Node(const T& value) : value(value) {}
    Node(): value() {}
  };

  template<bool is_const>
  class base_iterator;
  using iterator = base_iterator<false>;
  using const_iterator = base_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using allocator_adoptor = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
  using AllocTraits = std::allocator_traits<allocator_adoptor>;

  size_t sz_;
  BaseNode fake_node_;
  iterator begin_;
  iterator end_;
  [[no_unique_address]] allocator_adoptor allocator_;

  void swap(List& other) {
    std::swap(fake_node_, other.fake_node_);
    std::swap(begin_, other.begin_);
    std::swap(fake_node_.prev->next, other.fake_node_.prev->next);
    std::swap(fake_node_.next->prev, other.fake_node_.next->prev);
    std::swap(sz_, other.sz_);
    std::swap(allocator_, other.allocator_);
  }

  void clear() {
    while (sz_ > 0) {
      pop_back();
    }
  }

  template<typename ... Args>
  void fill(size_t size, Args ... args) {
    size_t i = 0;
    try {
      for (; i < size; ++i) {
        push_back(args...);
      }
    } catch (...) {
      clear();
      throw;
    }
  }

  List() : sz_(0), begin_(&fake_node_), end_(&fake_node_) {}
  List(size_t size) : List() {
    fill(size);
  }
  List(size_t size, const T& value) : List() {
    fill(size, value);
  }
  List(const Allocator& allocator) : sz_(0),
                                     begin_(&fake_node_),
                                     end_(&fake_node_),
                                     allocator_(allocator) {}
  List(size_t size, const Allocator& allocator) : List(allocator) {
    fill(size);
  }
  List(size_t size, const T& value, const Allocator& allocator) : List(allocator) {
    fill(size, value);
  }

  List(const List& other) : List(AllocTraits::select_on_container_copy_construction(other.allocator_)) {
    for (auto it = other.begin(); it != other.end(); ++it) {
      try {
        push_back(*it);
      } catch (...) {
        clear();
        throw;
      }
    }
  }
  ~List() {
    clear();
  }
  List& operator=(const List& other) {
    List<T, Allocator>
        tmp(AllocTraits::propagate_on_container_copy_assignment::value ? other.allocator_ : allocator_);
    for (auto it = other.begin(); it != other.end(); ++it) {
      try {
        tmp.push_back(*it);
      } catch (...) {
        tmp.clear();
        throw;
      }
    }
    swap(tmp);
    return *this;
  }

  const allocator_adoptor& get_allocator() const {
    return allocator_;
  }

  size_t size() const {
    return sz_;
  }

  template<typename... Args>
  iterator insert(const_iterator iter, const Args& ... args) {
    BaseNode* ptr = AllocTraits::allocate(allocator_, 1);
    try {
      AllocTraits::construct(allocator_, static_cast<Node*>(ptr), args...);
    } catch (...) {
      AllocTraits::deallocate(allocator_, static_cast<Node*>(ptr), 1);
      throw;
    }
    BaseNode* prev_node = (iter.node)->prev;
    BaseNode* next_node = static_cast<BaseNode*>(iter.node);
    ptr->prev = prev_node;
    ptr->next = next_node;
    prev_node->next = ptr;
    next_node->prev = ptr;
    if (iter == begin_) {
      begin_ = iterator(ptr);
    }
    ++sz_;
    return iterator(ptr);
  }

  iterator erase(const_iterator iter) {
    BaseNode* ptr = iter.node;
    BaseNode* prev_node = ptr->prev;
    BaseNode* next_node = ptr->next;
    prev_node->next = next_node;
    next_node->prev = prev_node;
    AllocTraits::destroy(allocator_, static_cast<Node*>(ptr));
    AllocTraits::deallocate(allocator_, static_cast<Node*>(ptr), 1);
    if (iter == begin_) {
      begin_ = iterator(next_node);
    }
    --sz_;
    return iterator(next_node);
  }

  void push_back(const T& value) {
    insert(end_, value);
  }
  void push_back() {
    insert(end_);
  }
  void push_front(const T& value) {
    insert(begin_, value);
  }
  void pop_back() {
    erase(std::prev(end_));
  }
  void pop_front() {
    erase(begin_);
  }

  iterator begin() {
    return begin_;
  }
  const_iterator begin() const {
    return cbegin();
  }
  const_iterator cbegin() const {
    return static_cast<const_iterator>(begin_);
  }
  iterator end() {
    return end_;
  }
  const_iterator end() const {
    return cend();
  }
  const_iterator cend() const {
    return static_cast<const_iterator>(end_);
  }

  reverse_iterator rbegin() {
    return reverse_iterator(end_);
  }
  const_reverse_iterator rbegin() const {
    return crbegin();
  }
  const_reverse_iterator crbegin() const {
    return static_cast<const_reverse_iterator >(end_);
  }
  reverse_iterator rend() {
    return reverse_iterator(begin_);
  }
  const_reverse_iterator rend() const {
    return crend();
  }
  const_reverse_iterator crend() const {
    return static_cast<const_reverse_iterator >(begin_);
  }
};

template<typename T, typename Allocator>
template<bool is_const>
class List<T, Allocator>::base_iterator {
 public:
  BaseNode* node;
  base_iterator() = default;
  base_iterator(BaseNode* node) : node(node) {}

  using value_type = T;
  using pointer = typename std::conditional<is_const, const T*, T*>::type;
  using reference = typename std::conditional<is_const, const T&, T&>::type;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;

  reference operator*() const {
    return reinterpret_cast<Node*>(node)->value;
  }
  pointer operator->() const {
    return &(reinterpret_cast<Node*>(node)->value);
  }

  operator base_iterator<true>() const {
    return base_iterator<true>(node);
  }

  base_iterator operator++(int) {
    base_iterator copy = *this;
    ++(*this);
    return copy;
  }
  base_iterator operator--(int) {
    base_iterator copy = *this;
    --(*this);
    return copy;
  }
  base_iterator& operator++() {
    node = node->next;
    return *this;
  }
  base_iterator& operator--() {
    node = node->prev;
    return *this;
  }
  bool operator==(const base_iterator& other) const {
    return node == other.node;
  }
  bool operator!=(const base_iterator& other) const {
    return !(*this == other);
  }
};
