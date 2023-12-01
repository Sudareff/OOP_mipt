namespace smart_pointers {

template<typename T, typename U>
using base_or_same = std::enable_if_t<std::is_base_of_v<T, U> || std::is_same_v<T, U>>;

struct BaseControlBlock {
  size_t shared_count = 0;
  size_t weak_count = 0;
  BaseControlBlock(size_t shared_count, size_t weak_count) :
      shared_count(shared_count),
      weak_count(weak_count) {}
  virtual ~BaseControlBlock() = default;

  void shared_release() {
    --shared_count;
    if (shared_count == 0) {
      dispose();
    }
    if (shared_count == 0 && weak_count == 0) {
      destroy();
    }
  }

  void weak_release() {
    --weak_count;
    if (shared_count == 0 && weak_count == 0) {
      destroy();
    }
  }

  virtual void destroy() = 0;
  virtual void dispose() = 0;
};

template<typename T, typename Allocator, typename Del>
struct ControlBlockDirect : BaseControlBlock {
  using AllocatorAdoptor =
      typename
      std::allocator_traits<Allocator>::template
      rebind_alloc<ControlBlockDirect>;

  T* ptr;
  AllocatorAdoptor alloc;
  Del del;

  ControlBlockDirect(size_t shared_count, size_t weak_count,
                     T* ptr, const Allocator& alloc, const Del& del) :
      BaseControlBlock(shared_count, weak_count),
      ptr(ptr),
      alloc(alloc),
      del(del) {}

  void destroy() override {
    std::allocator_traits<AllocatorAdoptor>::deallocate(alloc, this, 1);
  }

  void dispose() override {
    del(ptr);
  }
};

template<typename T, typename Allocator>
struct ControlBlockMakeShared : BaseControlBlock {
  using AllocatorAdoptor =
      typename
      std::allocator_traits<Allocator>::template
      rebind_alloc<ControlBlockMakeShared>;
  AllocatorAdoptor alloc;
  T obj;

  ControlBlockMakeShared(size_t shared_count, size_t weak_count,
                         const Allocator& alloc, T&& obj) :
      BaseControlBlock(shared_count, weak_count),
      alloc(alloc),
      obj(std::move(obj)) {}

  ControlBlockMakeShared(size_t shared_count, size_t weak_count,
                         const Allocator& alloc)
      : BaseControlBlock(shared_count, weak_count),
        alloc(alloc) {}

  void destroy() override {
    std::allocator_traits<AllocatorAdoptor>::deallocate(alloc, this, 1);
  }
  void dispose() override {
    auto tmp = typename std::allocator_traits<Allocator>::template rebind_alloc<T>(alloc);
    std::allocator_traits<Allocator>::destroy(tmp, &obj);
  }

  T* get_ptr() {
    return &obj;
  }
};

}  // namespace smart_pointers

template<typename Y>
class WeakPtr;

template<typename T>
class SharedPtr {
 private:
  using BaseControlBlock = smart_pointers::BaseControlBlock;

  template<typename V, typename U>
  using base_or_same = smart_pointers::base_or_same<V, U>;

  template<typename V, typename Allocator, typename Del>
  using ControlBlockDirect = smart_pointers::ControlBlockDirect<V, Allocator, Del>;

  template<typename V, typename Allocator>
  using ControlBlockMakeShared = smart_pointers::ControlBlockMakeShared<V, Allocator>;

  BaseControlBlock* control_block = nullptr;
  T* ptr = nullptr;

  template<typename Y, typename Allocator, typename... Args>
  friend auto allocateShared(const Allocator& alloc, Args&& ... args);

  template<typename Y, typename... Args>
  friend SharedPtr<Y> makeShared(Args&& ... args);

  template<typename Allocator>
  SharedPtr(ControlBlockMakeShared<T, Allocator>* cb) :
      control_block(cb),
      ptr(cb->get_ptr()) {}

  SharedPtr(const WeakPtr<T>& weak_ptr) :
      control_block(weak_ptr.control_block),
      ptr(weak_ptr.ptr) {
    ++(control_block->shared_count);
  }

 public:
  SharedPtr() :
      control_block(nullptr), ptr(nullptr) {}

  template<typename Y, typename Deleter, typename Allocator,
      typename = base_or_same<T, Y>>
  SharedPtr(Y* p, const Deleter& del, const Allocator& alloc) :
      ptr(p) {
    using ControlBlock = ControlBlockDirect<T, Allocator, Deleter>;
    using AllocatorAdoptor =
        typename
        std::allocator_traits<Allocator>::template
        rebind_alloc<ControlBlock>;
    AllocatorAdoptor alloc_adopt(alloc);
    control_block = std::allocator_traits<AllocatorAdoptor>::allocate(alloc_adopt, 1);
    new(control_block) ControlBlockDirect<T, Allocator, Deleter>(1, 0, ptr, alloc, del);
  }

  template<typename Y>
  explicit SharedPtr(Y* p) :
      SharedPtr(p, std::default_delete<T>(), std::allocator<T>()) {}

  template<typename Y, typename Deleter, typename = base_or_same<T, Y>>
  SharedPtr(Y* p, const Deleter& del):
      SharedPtr(p, del, std::allocator<T>()) {}

  SharedPtr(const SharedPtr& other)
      : control_block(other.control_block),
        ptr(other.ptr) {
    if (control_block != nullptr) {
      ++(control_block->shared_count);
    }
  }

  template<typename Y, typename = base_or_same<T, Y>>
  SharedPtr(const SharedPtr<Y>& other)
      : control_block(other.control_block),
        ptr(other.ptr) {
    if (control_block != nullptr) {
      ++(control_block->shared_count);
    }
  }

  template<typename Y, typename = base_or_same<T, Y>>
  SharedPtr(SharedPtr<Y>&& other)
      : control_block(other.control_block),
        ptr(other.ptr) {
    other.control_block = nullptr;
    other.ptr = nullptr;
  }

  template<typename Y, typename = base_or_same<T, Y>>
  SharedPtr& operator=(SharedPtr<Y>& other) {
    auto tmp = SharedPtr<T>(other);
    swap(tmp);
    return *this;
  }

  template<typename Y>
  SharedPtr& operator=(SharedPtr<Y>&& other) {
    SharedPtr<T>(std::move(other)).swap(*this);
    return *this;
  }

  ~SharedPtr() {
    if (control_block != nullptr) {
      control_block->shared_release();
    }
  }

  void swap(SharedPtr& other) {
    std::swap(control_block, other.control_block);
    std::swap(ptr, other.ptr);
  }

  T& operator*() { return *ptr; }
  const T& operator*() const { return *ptr; }

  T* operator->() { return ptr; }
  const T* operator->() const { return ptr; }

  T* get() { return ptr; }
  const T* get() const { return ptr; }

  void reset() { SharedPtr().swap(*this); }

  template<typename Y>
  void reset(Y* p) { SharedPtr<Y>(p).swap(*this); }

  size_t use_count() const { return control_block->shared_count; }

  template<typename Y>
  friend
  class SharedPtr;

  template<typename Y>
  friend
  class WeakPtr;
};

template<typename T, typename Allocator, typename... Args>
auto allocateShared(const Allocator& alloc, Args&& ... args) {
  using ControlBlockMakeShared = smart_pointers::ControlBlockMakeShared<T, Allocator>;
  using AllocatorAdoptor =
      typename std::allocator_traits<Allocator>::template
      rebind_alloc<ControlBlockMakeShared>;
  AllocatorAdoptor cb_alloc(alloc);
  auto tmp_ptr = std::allocator_traits<AllocatorAdoptor>::allocate(cb_alloc, 1);
  std::allocator_traits<AllocatorAdoptor>::construct(cb_alloc, tmp_ptr, 1, 0, alloc, std::forward<Args>(args)...);
  return SharedPtr<T>(tmp_ptr);
}

template<typename T, typename... Args>
SharedPtr<T> makeShared(Args&& ... args) {
  return allocateShared<T>(std::allocator<T>(),
                           std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr {
 private:

  using BaseControlBlock = smart_pointers::BaseControlBlock;

  template<typename V, typename U>
  using base_or_same = smart_pointers::base_or_same<V, U>;

  template<typename V, typename Allocator, typename Del>
  using ControlBlockDirect = smart_pointers::ControlBlockDirect<V, Allocator, Del>;

  template<typename V, typename Allocator>
  using ControlBlockMakeShared = smart_pointers::ControlBlockMakeShared<V, Allocator>;

  BaseControlBlock* control_block = nullptr;
  T* ptr = nullptr;

 public:
  WeakPtr() : control_block(nullptr), ptr(nullptr) {}

  template<typename Y, typename = base_or_same<T, Y>>
  WeakPtr(const SharedPtr<Y>& shared_ptr)
      : control_block(shared_ptr.control_block),
        ptr(shared_ptr.ptr) {
    ++(control_block->weak_count);
  }

  WeakPtr(const WeakPtr& other)
      : control_block(other.control_block),
        ptr(other.ptr) {
    ++(control_block->weak_count);
  }

  template<typename Y, typename = base_or_same<T, Y>>
  WeakPtr(const WeakPtr<Y>& other)
      : control_block(other.control_block),
        ptr(other.ptr) {
    ++(control_block->weak_count);
  }

  template<typename Y, typename = base_or_same<T, Y>>
  WeakPtr(WeakPtr<Y>&& other)
      : control_block(other.control_block),
        ptr(other.ptr) {
    other.control_block = nullptr;
    other.ptr = nullptr;
  }

  WeakPtr& operator=(WeakPtr other) {
    swap(other);
    return *this;
  }

  WeakPtr& operator=(WeakPtr&& other) {
    WeakPtr<T>(std::move(other)).swap(*this);
    return *this;
  }

  template<typename Y, typename = base_or_same<T, Y>>
  WeakPtr& operator=(SharedPtr<Y>& shared_ptr) {
    WeakPtr<T>(shared_ptr).swap(*this);
    return *this;
  }

  ~WeakPtr() {
    if (control_block != nullptr) {
      control_block->weak_release();
    }
  }

  void swap(WeakPtr& other) {
    std::swap(control_block, other.control_block);
    std::swap(ptr, other.ptr);
  }

  bool expired() const { return control_block != nullptr && control_block->shared_count == 0; }

  SharedPtr<T> lock() const {
    return expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
  }

  size_t use_count() const { return control_block->shared_count; }

  template<typename Y>
  friend
  class WeakPtr;

  template<typename Y>
  friend
  class SharedPtr;
};
