#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>

template <size_t N>
class StackStorage {
 public:
  StackStorage() = default;
  StackStorage(const StackStorage&) = delete;
  ~StackStorage() = default;
  void operator=(const StackStorage&) = delete;
  char buffer[N];
  int shift = 0;

 private:
  int four_byte_2_;
  int four_byte_3_;
};

template <typename T, size_t N>
class StackAllocator {
 public:
  using value_type = T;

  StackAllocator() = default;

  template <typename U>
  StackAllocator(const StackAllocator<U, N>& other) : storage(other.storage) {}

  StackAllocator(StackStorage<N>& storage) : storage(&storage) {}

  ~StackAllocator() = default;

  template <typename U>
  StackAllocator<value_type, N>& operator=(const StackAllocator<U, N>& other) {
    storage = other.storage;
    return *this;
  }

  value_type* allocate(size_t n) {
    if (storage == nullptr) {
      throw std::bad_alloc();
    }
    int shift = (sizeof(value_type) - storage->shift % sizeof(value_type)) %
                sizeof(value_type);
    if (n * sizeof(value_type) + storage->shift + shift > N) {
      throw std::bad_alloc();
    }
    storage->shift += n * sizeof(value_type) + shift;
    return reinterpret_cast<value_type*>(
            &storage->buffer[storage->shift - n * sizeof(value_type) - shift] +
            shift);
  }

  void deallocate(value_type* ptr, size_t size) {
    std::ignore = ptr;
    std::ignore = size;
  }

  // BEGIN-NOLINT
  template <typename U>
  struct rebind {  //  NOLINT
    using other = StackAllocator<U, N>;
  };
  // END-NOLINT
  StackStorage<N>* storage = nullptr;
};

template <typename T, typename Alloc = std::allocator<T>>
class List {
 public:
  using value_type = T;
  using AllocatorType = Alloc;  // NOLINT
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::allocator_traits<AllocatorType>::pointer;
  using const_pointer =
          typename std::allocator_traits<AllocatorType>::const_pointer;
  template <bool IsConst>
  class common_iterator;
  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 private:
  /*  struct BaseNode;
  struct Node: public BaseNode {
   public:
    value_type data;
    Node(Node* prev, Node* next, const_reference data): BaseNode(prev, next),
  data(data) {} template <typename... Args> Node(Node* prev, Node* next,
  Args&&... args): BaseNode(prev, next), data(value_type(args...)) {}
  };

  struct BaseNode {
   public:
    Node* next;
    Node* prev;
    BaseNode() : next(reinterpret_cast<Node*>(this)),
  prev(reinterpret_cast<Node*>(this)) {}; BaseNode(Node* prev, Node* next):
  prev(prev), next(next) {}
  };*/

  struct Node {
    Node* next;
    Node* prev;
    T* data = nullptr;
    Node() = default;
    Node(Node* next, Node* prev, T* data)
            : next(next), prev(prev), data(data) {}
    Node(Node* next, Node* prev, const T& data)
            : next(next), prev(prev), data(new T(data)) {}
    template <typename... Args>
    Node(Node* next, Node* prev, Args&... args)
            : next(next), prev(prev), data(new T(args...)) {}
    ~Node() = default;
  };

  using node_alloc = typename std::allocator_traits<
          AllocatorType>::template rebind_alloc<Node>;
  using node_alloc_traits = typename std::allocator_traits<
          AllocatorType>::template rebind_traits<Node>;

  node_alloc alloc;  // NOLINT
  size_t size_;
  Node base_node_{&base_node_, &base_node_, nullptr};
  Node* front_ = &base_node_;
  Node* back_ = &base_node_;
  void swap(List& list);

 public:
  List();
  List(size_t n);
  List(size_t n, const_reference value);
  List(AllocatorType& alloc);
  List(size_t n, AllocatorType& alloc);
  List(size_t n, const_reference value, AllocatorType& alloc);
  List(const List& list);
  List(const List& list, AllocatorType&& alloc);
  ~List();
  List<T, AllocatorType>& operator=(const List<T, AllocatorType>& list);
  AllocatorType get_allocator();
  reference front();
  reference back();
  size_t size() const;
  void clear();
  void push_back(const_reference value);
  // void push_back(T&&);
  void pop_back();
  void push_front(const_reference value);
  // void push_front(T&&);
  void pop_front();
  void insert(const_iterator pos, const_reference value);
  // void insert(const_iterator, T&&);
  template <typename... Args>
  void insert(const_iterator pos, Args&... args);
  void erase(const_iterator pos);

  iterator begin() { return iterator(front_->next); }
  const_iterator begin() const { return const_iterator(front_->next); }
  const_iterator cbegin() const { return const_iterator(front_->next); }
  iterator end() { return iterator(back_); }
  const_iterator end() const { return const_iterator(back_); }
  const_iterator cend() const { return const_iterator(back_); }
  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
  const_reverse_iterator rbegin() const {
    return std::make_reverse_iterator(cend());
  }
  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }
  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
  const_reverse_iterator rend() const {
    return std::make_reverse_iterator(cbegin());
  }
  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }
};

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::swap(List& list) {
  std::swap(alloc, list.alloc);
  std::swap(size_, list.size_);
  std::swap(base_node_, list.base_node_);
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List() : alloc(AllocatorType()), size_(0) {}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List(size_t n) : alloc(AllocatorType()), size_(0) {
  try {
    while (n != 0) {
      List<T, AllocatorType>::insert(begin());
      n--;
    }
  } catch (...) {
    List<T, AllocatorType>::clear();
    throw;
  }
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List(size_t n, const T& value)
        : alloc(AllocatorType()), size_(0) {
  try {
    while (n > 0) {
      List<T, AllocatorType>::insert(begin(), value);
      n--;
    }
  } catch (...) {
    List<T, AllocatorType>::clear();
    throw;
  }
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List(AllocatorType& alloc)
        : alloc(node_alloc(alloc)), size_(0) {}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List(size_t n, AllocatorType& alloc)
        : alloc(node_alloc(alloc)), size_(0) {
  try {
    while (n > 0) {
      List<T, AllocatorType>::insert(begin());
      n--;
    }
  } catch (...) {
    List<T, AllocatorType>::clear();
    throw;
  }
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List(size_t n, const T& value, AllocatorType& alloc)
        : alloc(node_alloc(alloc)), size_(0) {
  try {
    while (n > 0) {
      List<T, AllocatorType>::insert(begin(), value);
      n--;
    }
  } catch (...) {
    List<T, AllocatorType>::clear();
    throw;
  }
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List(const List& list)
        : alloc(std::allocator_traits<
        AllocatorType>::select_on_container_copy_construction(list.alloc)),
          size_(0) {
  try {
    for (auto& value : list) {
      List<T, AllocatorType>::insert(end(), value);
    }
  } catch (...) {
    List<T, AllocatorType>::clear();
    throw;
  }
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::List(const List& list, AllocatorType&& alloc)
        : alloc(alloc), size_(0) {
  try {
    for (auto& value : list) {
      List<T, AllocatorType>::insert(end(), value);
    }
  } catch (...) {
    List<T, AllocatorType>::clear();
    throw;
  }
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>::~List() {
  List<T, AllocatorType>::clear();
}

template <typename T, typename AllocatorType>
List<T, AllocatorType>& List<T, AllocatorType>::operator=(
        const List<T, AllocatorType>& list) {
  if constexpr (std::allocator_traits<AllocatorType>::
                propagate_on_container_copy_assignment::value) {
    List<T, AllocatorType> tmp(list, node_alloc(list.alloc));
    List<T, AllocatorType>::swap(tmp);
  } else {
    List<T, AllocatorType> tmp(list, List<T, AllocatorType>::alloc);
    List<T, AllocatorType>::swap(tmp);
  }
  return *this;
}

template <typename T, typename AllocatorType>
AllocatorType List<T, AllocatorType>::get_allocator() {
  return List<T, AllocatorType>::alloc;
}

template <typename T, typename AllocatorType>
typename List<T, AllocatorType>::reference List<T, AllocatorType>::front() {
  return List<T, AllocatorType>::front_->next->data;
}

template <typename T, typename AllocatorType>
typename List<T, AllocatorType>::reference List<T, AllocatorType>::back() {
  return List<T, AllocatorType>::back_->prev->data;
}

template <typename T, typename AllocatorType>
size_t List<T, AllocatorType>::size() const {
  return List<T, AllocatorType>::size_;
}

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::clear() {
  while (List<T, AllocatorType>::size_ != 0) {
    List<T, AllocatorType>::erase(begin());
  }
}

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::push_back(
        typename List<T, AllocatorType>::const_reference value) {
  List<T, AllocatorType>::insert(end(), value);
}

/*template <typename T, typename AllocatorType>
void List<T, AllocatorType>::push_back(T&& value) {
  insert(end(), value);
}*/

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::pop_back() {
  List<T, AllocatorType>::erase(end() - 1);
}

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::push_front(
        typename List<T, AllocatorType>::const_reference value) {
  List<T, AllocatorType>::insert(begin(), value);
}

/*template <typename T, typename AllocatorType>
void List<T, AllocatorType>::push_front(T&& value) {
  insert(begin(), value);
}*/

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::pop_front() {
  List<T, AllocatorType>::erase(begin());
}

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::insert(
        List<T, AllocatorType>::const_iterator pos,
        typename List<T, AllocatorType>::const_reference value) {
  Node* node = node_alloc_traits::allocate(alloc, 1);
  try {
    node_alloc_traits::construct(alloc, node, pos.node, pos.node->prev, value);
    pos.node->prev->next = node;
    pos.node->prev = node;
    ++size_;
  } catch (...) {
    node_alloc_traits::deallocate(alloc, node, 1);
    throw;
  }
}

/*
template<typename T, typename AllocatorType>
void List<T, AllocatorType>::insert(List<T, AllocatorType>::const_iterator pos,
T&& value) { Node* node = node_alloc_traits::allocate(alloc, 1);
  node_alloc_traits::construct(alloc, node, pos.node, pos.node->prev,
                             std::move(new T(value)));
  pos.node->prev->next = node;
  pos.node->prev = node;
*/
/*  Node* node = node_alloc_traits::allocate(alloc, 1);
  if (size_ == 0) {
    front_ = node;
    node_alloc_traits::construct(alloc, node, back_, nullptr, std::move(value));
    back_ = node;
  } else if (pos.node == back_ && pos.is_end) {
    back_->next = node;
    node_alloc_traits::construct(alloc, node, back_, nullptr, std::move(value));
    back_ = node;
  } else if (pos.node == front_) {
    front_->prev = node;
    node_alloc_traits::construct(alloc, node, nullptr, front_,
  std::move(value)); front_ = node; } else { node_alloc_traits::construct(alloc,
  node, pos.node->prev, pos.node, std::move(value)); pos.node->prev->next =
  node; pos.node->prev = node;
  }
  *//*

  ++size_;
}
*/

template <typename T, typename AllocatorType>
template <typename... Args>
void List<T, AllocatorType>::insert(List<T, AllocatorType>::const_iterator pos,
                                    Args&... args) {
  Node* node = node_alloc_traits::allocate(alloc, 1);
  try {
    node_alloc_traits::construct(alloc, node, pos.node, pos.node->prev,
                                 args...);
    pos.node->prev->next = node;
    pos.node->prev = node;
    ++size_;
  } catch (...) {
    node_alloc_traits::deallocate(alloc, node, 1);
    throw;
  }
}

template <typename T, typename AllocatorType>
void List<T, AllocatorType>::erase(List<T, AllocatorType>::const_iterator pos) {
  if (size_ == 1) {
    List<T, AllocatorType>::front_->next = back_;
    List<T, AllocatorType>::front_->prev = back_;
    List<T, AllocatorType>::back_ = List<T, AllocatorType>::front_;
  } else if (pos == begin()) {
    List<T, AllocatorType>::front_->next->next->prev =
            List<T, AllocatorType>::front_;
    List<T, AllocatorType>::front_->next =
            List<T, AllocatorType>::front_->next->next;
    List<T, AllocatorType>::back_ = List<T, AllocatorType>::front_;
  } else {
    pos.node->next->prev = pos.node->prev;
    pos.node->prev->next = pos.node->next;
  }
  delete pos.node->data;
  // node_alloc_traits::destroy(alloc, pos.node->data);
  node_alloc_traits::destroy(alloc, pos.node);
  node_alloc_traits::deallocate(alloc, pos.node, 1);
  --size_;
}

template <typename T, typename AllocatorType>
template <bool IsConst>
class List<T, AllocatorType>::common_iterator {
 public:
  using value_type = std::conditional_t<IsConst, const T, T>;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;
  using iterator = common_iterator<false>;
  using const_iterator = common_iterator<true>;
  Node* node;
  common_iterator(Node* node) : node(node) {}
  common_iterator<IsConst>& operator++();
  common_iterator<IsConst> operator++(int);
  common_iterator<IsConst>& operator--();
  common_iterator<IsConst> operator--(int);
  common_iterator<IsConst>& operator+=(int n);
  common_iterator<IsConst>& operator-=(int n);
  common_iterator<IsConst> operator+(int n) const;
  common_iterator<IsConst> operator-(int n) const;
  bool operator==(const common_iterator& iter) const {
    return (node == iter.node);
  }
  bool operator!=(const common_iterator& iter) const {
    return (node != iter.node);
  }
  value_type& operator*() { return *(node->data); }
  value_type* operator->() { return node->data; }
  operator const_iterator() const { return const_iterator(node); }
};

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>&
List<T, AllocatorType>::common_iterator<IsConst>::operator++() {
  node = node->next;
  return *this;
}

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>
List<T, AllocatorType>::common_iterator<IsConst>::operator++(int) {
  common_iterator<IsConst> copy(*this);
  node = node->next;
  return copy;
}

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>&
List<T, AllocatorType>::common_iterator<IsConst>::operator--() {
  node = node->prev;
  return *this;
}

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>
List<T, AllocatorType>::common_iterator<IsConst>::operator--(int) {
  common_iterator<IsConst> copy(*this);
  node = node->prev;
  return copy;
}

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>&
List<T, AllocatorType>::common_iterator<IsConst>::operator+=(int n) {
  for (int i = 0; i < n; ++i) {
    ++(*this);
  }
  return *this;
}

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>&
List<T, AllocatorType>::common_iterator<IsConst>::operator-=(int n) {
  for (int i = 0; i < n; ++i) {
    --(*this);
  }
  return *this;
}

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>
List<T, AllocatorType>::common_iterator<IsConst>::operator+(int n) const {
  return (common_iterator<IsConst>(node) += n);
}

template <typename T, typename AllocatorType>
template <bool IsConst>
typename List<T, AllocatorType>::template common_iterator<IsConst>
List<T, AllocatorType>::common_iterator<IsConst>::operator-(int n) const {
  return (common_iterator<IsConst>(node) -= n);
}
