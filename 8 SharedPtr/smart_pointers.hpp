#pragma once
#include <iostream>
#include <memory>  // NOLINT

template <typename T, typename Alloc, typename Deleter>
struct ControlData;

struct BaseControlBlock {
  size_t shared_count;
  size_t weak_count;
  BaseControlBlock(size_t shared_count, size_t weak_count)
          : shared_count(shared_count), weak_count(weak_count) {}
  virtual void deallocate_ptr(void*) = 0;
  virtual void destroy() = 0;
  virtual ~BaseControlBlock() = default;
};

template <typename T, typename Alloc>
struct EasyDel {                                        // NOLINT
  Alloc alloc;                                          // NOLINT
  explicit EasyDel(Alloc alloc) : alloc(alloc) {}       // NOLINT
  void operator()(T* ptr) {                             // NOLINT
    std::allocator_traits<Alloc>::destroy(alloc, ptr);  // NOLINT
  }                                                     // NOLINT
};                                                      // NOLINT

template <typename T, typename Alloc, typename Deleter>
struct ControlBlock : public BaseControlBlock {
  using alloc_data_template = typename std::allocator_traits<
          Alloc>::template rebind_alloc<ControlData<T, Alloc, EasyDel<T, Alloc>>>;
  using alloc_control_block_template = typename std::allocator_traits<
          Alloc>::template rebind_alloc<ControlBlock<T, Alloc, Deleter>>;
  [[no_unique_address]] Deleter deleter;
  [[no_unique_address]] Alloc alloc;

  explicit ControlBlock(Deleter deleter = Deleter(), Alloc alloc = Alloc())
          : BaseControlBlock(1, 0), deleter(deleter), alloc(alloc) {}

  void deallocate_ptr(void* ptr) override {
    deleter(reinterpret_cast<T*>(ptr));
  }

  void destroy() override {
    if (std::is_same<Deleter, EasyDel<T, Alloc>>::value) {
      alloc_data_template alloc_data = alloc;
      std::allocator_traits<alloc_data_template>::deallocate(
              alloc_data,
              reinterpret_cast<ControlData<T, Alloc, EasyDel<T, Alloc>>*>(this), 1);
    } else {
      alloc_control_block_template alloc_control_block = alloc;
      std::allocator_traits<alloc_control_block_template>::deallocate(
              alloc_control_block, this, 1);
    }
  }

  ~ControlBlock() override = default;
};

template <typename T, typename Alloc, typename Deleter>
struct ControlData {
  ControlBlock<T, Alloc, Deleter> cb;
  T object;
};

template <typename T>
class SharedPtr {
  template <typename U>
  friend class SharedPtr;
  template <typename U>
  friend class WeakPtr;

 public:
  void swap(SharedPtr& other) noexcept {
    std::swap(cb_, other.cb_);
    std::swap(ptr_, other.ptr_);
  }

  template <typename U, typename Deleter, typename Alloc>
  SharedPtr(U* ptr, Deleter del, Alloc alloc) : cb_(nullptr), ptr_(ptr) {
    using alloc_control_block_template = typename std::allocator_traits<
            Alloc>::template rebind_alloc<ControlBlock<U, Alloc, Deleter>>;
    alloc_control_block_template alloc_control_block = alloc;
    cb_ = alloc_control_block.allocate(1);
    new (cb_) ControlBlock<U, Alloc, Deleter>(del, alloc);
  }

  SharedPtr() : cb_(nullptr), ptr_(nullptr) {}

  template <typename U>  // NOLINT
  explicit SharedPtr(U* ptr)
          : SharedPtr(ptr, std::default_delete<U>(), std::allocator<U>()) {
  }  // NOLINT

  SharedPtr(const SharedPtr& other) : cb_(other.cb_), ptr_(other.ptr_) {
    if (static_cast<bool>(cb_)) {
      cb_->shared_count++;
    }
  }

  template <typename U>
  SharedPtr(const SharedPtr<U>& other) : cb_(other.cb_), ptr_(other.ptr_) {
    if (static_cast<bool>(cb_)) {
      cb_->shared_count++;
    }
  }

  template <typename U>
  explicit SharedPtr(SharedPtr<U>&& other) : cb_(other.cb_), ptr_(other.ptr_) {
    other.ptr_ = nullptr;
    other.cb_ = nullptr;
  }

  SharedPtr(SharedPtr<T>&& other) noexcept : cb_(other.cb_), ptr_(other.ptr_) {
    other.ptr_ = nullptr;
    other.cb_ = nullptr;
  }

  template <typename U>
  SharedPtr<T>& operator=(U&& other) {
    SharedPtr<T> tmp(std::forward<U>(other));
    SharedPtr<T>::swap(*this, tmp);
    return *this;
  }

  template <typename U, typename Deleter>
  SharedPtr(U* ptr, Deleter del) : SharedPtr(ptr, del, std::allocator<U>()) {}

  ~SharedPtr() {
    if (static_cast<bool>(cb_)) {
      if (cb_->shared_count != 1) {
        cb_->shared_count--;
      } else {
        cb_->deallocate_ptr(reinterpret_cast<void*>(ptr_));
        if (cb_->weak_count != 0) {
          cb_->shared_count--;
        } else {
          cb_->destroy();
        }
      }
    }
  }

  [[nodiscard]] size_t use_count() const {
    if (static_cast<bool>(cb_)) {
      return cb_->shared_count;
    }
    return 0;
  }

  void reset() {
    SharedPtr<T> tmp;
    SharedPtr<T>::swap(*this, tmp);
  }

  template <typename U>
  void reset(U* other) {
    if (other) {
      SharedPtr<T> tmp(other);
      SharedPtr<T>::swap(*this, tmp);
    } else {
      reset();
    }
  }

  T* get() const { return ptr_; }

  T& operator*() const { return *ptr_; }

  T* operator->() const { return ptr_; }

  template <typename U, typename... Args>
  friend SharedPtr<U> makeShared(Args&&... args);  // NOLINT

  template <typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> allocateShared(const Alloc&, Args&&...);  // NOLINT
 private:
  struct PtrConstruct {};
  struct ControlConstruct {};
  BaseControlBlock* cb_{};
  T* ptr_;

  template <typename Alloc, typename... Args>
  SharedPtr(typename SharedPtr<T>::PtrConstruct, Alloc alloc,  // NOLINT
            Args&&... args) {                                  // NOLINT
    typename std::allocator_traits<Alloc>::template rebind_alloc<
            ControlData<T, Alloc, EasyDel<T, Alloc>>>
            alloc_data = alloc;
    auto* data_ptr = alloc_data.allocate(1);
    cb_ = &(data_ptr->cb);
    ptr_ = &(data_ptr->object);
    std::allocator_traits<Alloc>::construct(alloc, ptr_,
                                            std::forward<Args>(args)...);
    new (cb_) ControlBlock<T, Alloc, EasyDel<T, Alloc>>(
            EasyDel<T, Alloc>(alloc), alloc);
  }

  SharedPtr(T* ptr, BaseControlBlock* cobl,
            typename SharedPtr<T>::ControlConstruct)  // NOLINT
          : cb_(cobl), ptr_(ptr) {                        // NOLINT
    cb_->shared_count++;
  }

  void swap(SharedPtr<T>& first, SharedPtr<T>& second) noexcept {
    std::swap(first.cb_, second.cb_);
    std::swap(first.ptr_, second.ptr_);
  }
};

template <typename T, typename... Args>
SharedPtr<T> makeShared(Args&&... args) {  // NOLINT
  return SharedPtr<T>(typename SharedPtr<T>::PtrConstruct(),
                      std::allocator<T>(),
                      std::forward<Args>(args)...);  // NOLINT
}

template <typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc& alloc, Args&&... args) {  // NOLINT
  return SharedPtr<T>(typename SharedPtr<T>::PtrConstruct(), alloc,
                      std::forward<Args>(args)...);  // NOLINT
}

template <typename T>
class WeakPtr {
  template <typename U>
  friend class WeakPtr;
  template <typename U>
  friend class SharedPtr;

 private:
  BaseControlBlock* cb_;
  T* ptr_;

  void swap(WeakPtr<T>& first, WeakPtr<T>& second) {
    std::swap(first.cb_, second.cb_);
    std::swap(first.ptr_, second.ptr_);
  }

 public:
  WeakPtr() : cb_(nullptr), ptr_(nullptr) {}

  WeakPtr(const WeakPtr<T>& other) : cb_(other.cb_), ptr_(other.ptr_) {
    if (static_cast<bool>(cb_)) {
      cb_->weak_count++;
    }
  }

  template <typename U>
  WeakPtr(const WeakPtr<U>& other) : cb_(other.cb_), ptr_(other.ptr_) {
    if (static_cast<bool>(cb_)) {
      cb_->weak_count++;
    }
  }

  template <typename U>
  WeakPtr(WeakPtr<U>&& other) : cb_(other.cb_), ptr_(other.ptr_) {
    other.ptr_ = nullptr;
    other.cb_ = nullptr;
  }

  WeakPtr(WeakPtr<T>&& other) : cb_(other.cb_), ptr_(other.ptr_) {
    other.ptr_ = nullptr;
    other.cb_ = nullptr;
  }

  template <typename U>
  WeakPtr(const SharedPtr<U>& other) : cb_(other.cb_), ptr_(other.ptr_) {
    cb_->weak_count++;
  }

  ~WeakPtr() {
    if (static_cast<bool>(cb_)) {
      cb_->weak_count--;
      if (cb_->shared_count + cb_->weak_count == 0) {
        cb_->destroy();
      }
    }
  }

  template <typename U>
  WeakPtr<T>& operator=(const SharedPtr<U>& other) {
    WeakPtr<T> tmp(other);
    WeakPtr<T>::swap(*this, tmp);
    return *this;
  }

  template <typename Other>
  WeakPtr<T>& operator=(Other&& other) {
    WeakPtr<T> tmp(std::forward<Other>(other));
    WeakPtr<T>::swap(*this, tmp);
    return *this;
  }

  [[nodiscard]] size_t use_count() const {
    if (static_cast<bool>(cb_)) {
      return cb_->shared_count;
    }
    return 0;
  }

  T& operator*() const { return *ptr_; }

  T* operator->() const { return ptr_; }

  SharedPtr<T> lock() const {
    if (expired()) {
      return SharedPtr<T>();
    }
    return SharedPtr<T>(ptr_, cb_, typename SharedPtr<T>::ControlConstruct());
  }

  [[nodiscard]] bool expired() const { return cb_->shared_count == 0; }
};
