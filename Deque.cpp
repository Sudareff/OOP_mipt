#include <iostream>
#include <iterator>
#include <type_traits>

template<typename T>
class Deque {
 private:

  T** data;
  size_t begin_pos;
  size_t end_pos;
  size_t block_count;
  static const int block_sz = 128;

  void clear(size_t start, size_t end) {
    for (size_t j = start; j < end; ++j) {
      (data[j / block_sz] + (j % block_sz))->~T();
    }
    for (size_t k = 0; k < block_count; ++k) {
      delete[] reinterpret_cast<char*>(data[k]);
    }
    delete[] data;
  }

  void allocate() {
    data = new T* [block_count];
    for (size_t i = 0; i < block_count; ++i) {
      data[i] = reinterpret_cast<T*>(new char[block_sz * sizeof(T)]);
    }
  }

  void relocate() {
    T** new_data = new T* [3 * block_count];
    for (size_t i = 0; i < block_count; ++i) {
      new_data[block_count + i] = data[i];
      new_data[i] = reinterpret_cast<T*>(new char[block_sz * sizeof(T)]);
      new_data[2 * block_count + i] = reinterpret_cast<T*>(new char[block_sz * sizeof(T)]);
    }
    delete[] data;
    data = new_data;
    begin_pos += block_count * block_sz;
    end_pos += block_count * block_sz;
    block_count *= 3;
  }

  void swap(Deque& other) {
    std::swap(data, other.data);
    std::swap(begin_pos, other.begin_pos);
    std::swap(end_pos, other.end_pos);
    std::swap(block_count, other.block_count);
  }

 public:

  size_t size() const { return end_pos - begin_pos; }

  Deque() : data(new T* [1]), begin_pos(1), end_pos(1), block_count(1) {
    data[0] = reinterpret_cast<T*>(new char[block_sz * sizeof(T)]);
  }

  Deque(size_t n, const T& value) : begin_pos(1), end_pos(n + 1) {
    n += 2;
    block_count = ((n + block_sz) / block_sz - static_cast<size_t>(n % block_sz == 0));
    allocate();
    size_t i = begin_pos;
    try {
      while (i <= end_pos - begin_pos) {
        new(data[i / block_sz] + (i % block_sz)) T(value);
        ++i;
      }
    } catch (...) {
      clear(begin_pos, i);
      throw;
    }
  }

  Deque(size_t n) : Deque(n, T()) {}

  Deque(const Deque& other) : begin_pos(other.begin_pos), end_pos(other.end_pos), block_count(other.block_count) {
    allocate();
    size_t i = begin_pos;
    try {
      while (i < end_pos) {
        new(data[i / block_sz] + (i % block_sz)) T(other[i - begin_pos]);
        ++i;
      }
    } catch (...) {
      clear(begin_pos, i);
      throw;
    }
  }

  ~Deque() {
    clear(begin_pos, end_pos);
  }

  Deque& operator=(Deque other) {
    swap(other);
    return *this;
  }

  T& operator[](size_t n) {
    n += begin_pos;
    return data[n / block_sz][n % block_sz];
  };

  const T& operator[](size_t n) const {
    n += begin_pos;
    return data[n / block_sz][n % block_sz];
  };

  T& at(size_t n) {
    if (n < end_pos - begin_pos) {
      n += begin_pos;
      return data[n / block_sz][n % block_sz];
    } else {
      throw std::out_of_range("");
    }
  };

  const T& at(size_t n) const {
    if (n < end_pos - begin_pos) {
      n += begin_pos;
      return data[n / block_sz][n % block_sz];
    } else {
      throw std::out_of_range("");
    }
  };

  void push_back(const T& value) {
    new(data[end_pos / block_sz] + (end_pos % block_sz)) T(value);
    ++end_pos;
    if (end_pos == block_sz * block_count) {
      relocate();
    }
  }

  void push_front(const T& value) {
    new(data[(begin_pos - 1) / block_sz] + ((begin_pos - 1) % block_sz)) T(value);
    --begin_pos;
    if (begin_pos == 0) {
      relocate();
    }
  }

  void pop_back() {
    --end_pos;
    (data[end_pos / block_sz] + (end_pos % block_sz))->~T();
  }

  void pop_front() {
    (data[begin_pos / block_sz] + (begin_pos % block_sz))->~T();
    ++begin_pos;
  }

  template<bool is_const>
  class base_iterator {
   public:

    using value_type = typename std::conditional<is_const, const T, T>::type;
    using reference = typename std::conditional<is_const, const T&, T&>::type;
    using pointer = typename std::conditional<is_const, const T*, T*>::type;
    using block_pointer = typename std::conditional<is_const, const T**, T**>::type;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = int;

   private:

    block_pointer block_p;
    pointer block;
    int pos;

   public:

    base_iterator() = default;

    base_iterator(const base_iterator& other) : block_p(other.block_p), block(other.block), pos(other.pos) {}

    base_iterator(block_pointer d, pointer p, int i)
        : block_p(d), block(p), pos(i) {}

    ~base_iterator() = default;

    base_iterator& operator=(const base_iterator& other) = default;

    operator base_iterator<true>() {
      return {const_cast<const T**>(block_p), const_cast<const T*>(block), pos};
    }

    reference operator*() const { return block[pos]; }

    pointer operator->() const { return block + pos; }

    base_iterator& operator++() {
      ++pos;
      if (pos == block_sz) {
        pos = 0;
        ++block_p;
        block = *block_p;
      }
      return *this;
    }

    base_iterator& operator--() {
      --pos;
      if (pos < 0) {
        pos = block_sz - 1;
        --block_p;
        block = *block_p;
      }
      return *this;
    }

    base_iterator operator++(int) {
      base_iterator cp = *this;
      ++(*this);
      return cp;
    }

    base_iterator operator--(int) {
      base_iterator cp = *this;
      --(*this);
      return cp;
    }

    base_iterator& operator+=(int n) {
      int tmp = pos + n;
      if (tmp % block_sz < 0) {
        pos = block_sz + tmp % block_sz;
        block_p += tmp / block_sz - 1;
      } else {
        block_p += tmp / block_sz;
        pos = tmp % block_sz;
      }
      block = *block_p;
      return *this;
    }

    base_iterator& operator-=(int n) {
      (*this) += -n;
      return *this;
    }

    base_iterator operator+(int n) const {
      base_iterator cp = *this;
      cp += n;
      return cp;
    }

    base_iterator operator-(int n) const {
      return (*this) + (-n);
    }

    int operator-(const base_iterator& other) const {
      return (block_p - other.block_p) * block_sz + pos - other.pos;
    }

    bool operator==(const base_iterator& other) const {
      return (*this - other == 0);
    }
    bool operator!=(const base_iterator& other) const {
      return (*this - other != 0);
    }
    bool operator<(const base_iterator& other) const {
      return (*this - other < 0);
    }
    bool operator<=(const base_iterator& other) const {
      return (*this - other <= 0);
    }
    bool operator>(const base_iterator& other) const {
      return (*this - other > 0);
    }
    bool operator>=(const base_iterator& other) const {
      return (*this - other >= 0);
    }
  };

  using const_iterator = base_iterator<true>;
  using iterator = base_iterator<false>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() {
    return iterator(data + begin_pos / block_sz, data[begin_pos / block_sz], begin_pos % block_sz);
  }
  const_iterator cbegin() const {
    return iterator(data + begin_pos / block_sz, data[begin_pos / block_sz], begin_pos % block_sz);
  }
  const_iterator begin() const {
    return cbegin();
  }
  iterator end() {
    return iterator(data + end_pos / block_sz, data[end_pos / block_sz], end_pos % block_sz);
  }
  const_iterator cend() const {
    return iterator(data + end_pos / block_sz, data[end_pos / block_sz], end_pos % block_sz);
  }
  const_iterator end() const {
    return cend();
  }
  reverse_iterator rbegin() {
    return reverse_iterator(iterator(data + end_pos / block_sz, data[end_pos / block_sz], end_pos % block_sz));
  }
  const_reverse_iterator crbegin() const {
    return reverse_iterator(iterator(data + end_pos / block_sz, data[end_pos / block_sz], end_pos % block_sz));
  }
  const_reverse_iterator rbegin() const {
    return crbegin();
  }
  reverse_iterator rend() {
    return reverse_iterator(iterator(data + begin_pos / block_sz, data[begin_pos / block_sz], begin_pos % block_sz));
  }
  const_reverse_iterator crend() const {
    return reverse_iterator(iterator(data + begin_pos / block_sz, data[begin_pos / block_sz], begin_pos % block_sz));
  }
  const_reverse_iterator rend() const {
    return crend();
  }

  iterator insert(const_iterator pos, const T& value) {
    int diff = pos - begin();
    push_back(value);
    iterator it = end() - 1;
    for (; it > begin() + diff; --it) {
      std::swap(*it, *(it - 1));
    }
    return it;
  }

  iterator erase(const_iterator pos) {
    int diff = pos - begin();
    iterator it = begin() + diff;
    for (; it < end() - 1; ++it) {
      std::swap(*it, *(it + 1));
    }
    pop_back();
    return begin() + diff;
  }
};
