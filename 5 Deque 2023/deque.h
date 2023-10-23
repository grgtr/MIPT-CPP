#include <cstring>
#include <stdexcept>
#include <vector>

template <typename T>
class Deque {
 private:
  static const int kArraySize = 32;

  int front_index_;
  int back_index_;
  int size_;
  int sz_of_non_empty_arrs_;
  std::vector<T*> data_;

  void reserve(int size);

 public:
  template <bool IsConst>
  class common_iter;

  Deque()
          : front_index_(0), back_index_(0), size_(0), sz_of_non_empty_arrs_(0) {
    reserve(1);
  }

  Deque(const Deque& other)
          : front_index_(other.front_index_),
            back_index_(other.back_index_),
            size_(other.size_),
            sz_of_non_empty_arrs_(other.sz_of_non_empty_arrs_) {
    int size = static_cast<int>(other.data_.size());
    data_.resize(size);
    for (int i = 0; i < static_cast<int>(data_.size()); ++i) {
      data_[i] = reinterpret_cast<T*>(new char[kArraySize * sizeof(T)]);
      std::copy(other.data_[i], other.data_[i] + kArraySize, data_[i]);
    }
  }

  explicit Deque(int n);

  Deque(int n, const T& value);

  ~Deque();

  Deque& operator=(const Deque& other) {
    if (this != &other) {
      while (size_ > 0) {
        Deque::pop_front();
      }
      for (int i = 0; i < static_cast<int>(data_.size()); ++i) {
        delete[] reinterpret_cast<char*>(data_[i]);
      }
      int size = static_cast<int>(other.data_.size());
      data_.resize(size);
      for (int i = 0; i < static_cast<int>(data_.size()); ++i) {
        data_[i] = reinterpret_cast<T*>(new char[kArraySize * sizeof(T)]);
        std::copy(other.data_[i], other.data_[i] + kArraySize, data_[i]);
      }
      front_index_ = other.front_index_;
      back_index_ = other.back_index_;
      size_ = other.size_;
      sz_of_non_empty_arrs_ = other.sz_of_non_empty_arrs_;
    }
    return *this;
  }

  size_t size() const { return size_; }

  void push_back(const T& value) {
    if (((back_index_ + 1) % kArraySize == 0) ||
        ((back_index_) % kArraySize == 0 && size_ == 0)) {
      ++sz_of_non_empty_arrs_;
    }
    if ((size_ == static_cast<int>(data_.size()) * kArraySize) ||
        ((back_index_ + 1) == kArraySize * static_cast<int>(data_.size()) &&
         size_ != 0)) {
      reserve(sz_of_non_empty_arrs_ - 1);
    }
    if (size_ != 0) {
      ++back_index_;
    }
    new (data_[back_index_ / kArraySize] + back_index_ % kArraySize) T(value);
    ++size_;
  }

  void push_front(const T& value) {
    if ((front_index_) % kArraySize == 0) {
      ++sz_of_non_empty_arrs_;
    }
    if ((size_ == static_cast<int>(data_.size()) * kArraySize) ||
        (front_index_ == 0 && size_ != 0)) {
      reserve(sz_of_non_empty_arrs_ - 1);
    }
    if (size_ != 0) {
      --front_index_;
    }
    new (data_[front_index_ / kArraySize] + front_index_ % kArraySize) T(value);
    ++size_;
  }

  void pop_back() {
    if (back_index_ % kArraySize == 0) {
      --sz_of_non_empty_arrs_;
    }
    if (size_ == 0) {
      throw std::out_of_range("Deque is empty");
    }
    (data_[back_index_ / kArraySize] + back_index_ % kArraySize)->~T();
    --back_index_;
    --size_;
  }

  void pop_front() {
    if (front_index_ + 1 % kArraySize == 0) {
      --sz_of_non_empty_arrs_;
    }
    if (size_ == 0) {
      throw std::out_of_range("Deque is empty");
    }
    (data_[front_index_ / kArraySize] + front_index_ % kArraySize)->~T();
    ++front_index_;
    --size_;
  }

  T& operator[](int index) {
    return *(data_[(front_index_ + index) / kArraySize] +
             (front_index_ + index) % kArraySize);
  }

  const T& operator[](int index) const {
    return *(data_[(front_index_ + index) / kArraySize] +
             (front_index_ + index) % kArraySize);
  }

  T& at(size_t index) {
    if (index >= static_cast<size_t>(size_)) {
      throw std::out_of_range("Index out of range");
    }
    return *(data_[(front_index_ + index) / kArraySize] +
             (front_index_ + index) % kArraySize);
  }

  const T& at(size_t index) const {
    if (index >= static_cast<size_t>(size_)) {
      throw std::out_of_range("Index out of range");
    }
    return *(data_[(front_index_ + index) / kArraySize] +
             (front_index_ + index) % kArraySize);
  }

  using iterator = common_iter<false>;
  using const_iterator = common_iter<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() {
    auto ptr = data_.begin() + front_index_ / kArraySize;
    return common_iter<false>(ptr, front_index_ % kArraySize);
  }

  iterator end() {
    if (size_ == 0) {
      auto ptr = data_.begin() + (back_index_) / kArraySize;
      return common_iter<false>(ptr, (back_index_) % kArraySize);
    }
    auto ptr = data_.begin() + (back_index_ + 1) / kArraySize;
    return common_iter<false>(ptr, (back_index_ + 1) % kArraySize);
  }

  const_iterator cbegin() const {
    return common_iter<true>(data_.begin() + front_index_ / kArraySize,
                             front_index_ % kArraySize);
  }

  const_iterator cend() const {
    if (size_ == 0) {
      return common_iter<true>(data_.begin() + (back_index_) / kArraySize,
                               (back_index_) % kArraySize);
    }
    return common_iter<true>(data_.begin() + (back_index_ + 1) / kArraySize,
                             (back_index_ + 1) % kArraySize);
  }

  const_iterator begin() const {
    return const_iterator(data_.begin() + front_index_ / kArraySize,
                          front_index_ % kArraySize);
  }

  const_iterator end() const {
    if (size_ == 0) {
      return const_iterator(data_.begin() + (back_index_) / kArraySize,
                            (back_index_) % kArraySize);
    }
    return const_iterator(data_.begin() + (back_index_ + 1) / kArraySize,
                          (back_index_ + 1) % kArraySize);
  }

  reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }

  reverse_iterator rend() { return std::make_reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const {
    return std::make_reverse_iterator(cend());
  }

  const_reverse_iterator rend() const {
    return std::make_reverse_iterator(cbegin());
  }

  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(cend());
  }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(cbegin());
  }

  void insert(iterator iter, const T& value);

  void erase(iterator iter);
};

template <typename T>
Deque<T>::Deque(int n)
        : front_index_(0),
          back_index_(0),
          size_(0),
          sz_of_non_empty_arrs_(n / kArraySize + (n % kArraySize == 0 ? 0 : 1)) {
  reserve(sz_of_non_empty_arrs_);
  int index;
  try {
    for (index = 0; index < n; ++index) {
      Deque::push_back(T());
    }
  } catch (...) {
    for (int j = index; j > 0; --j) {
      Deque::pop_back();
    }
    for (int i = 0; i < static_cast<int>(data_.size()); ++i) {
      delete[] reinterpret_cast<char*>(data_[i]);
    }
    throw;
  }
}

template <typename T>
Deque<T>::Deque(int n, const T& value)
        : front_index_(0),
          back_index_(0),
          size_(0),
          sz_of_non_empty_arrs_(n / kArraySize + (n % kArraySize == 0 ? 0 : 1)) {
  reserve(sz_of_non_empty_arrs_);
  for (int i = 0; i < n; ++i) {
    Deque::push_back(value);
  }
}

template <typename T>
Deque<T>::~Deque() {
  while (size_ > 0) {
    Deque::pop_front();
  }
  for (int i = 0; i < static_cast<int>(data_.size()); ++i) {
    delete[] reinterpret_cast<char*>(data_[i]);
  }
}

template <typename T>
void Deque<T>::reserve(int size) {  // size is the number of arrays
  int index_of_front_array = front_index_ / kArraySize;
  int index_of_back_array = back_index_ / kArraySize;
  for (int i = 0; i < static_cast<int>(data_.size()); ++i) {
    if (i < index_of_front_array or i > index_of_back_array) {
      delete[] reinterpret_cast<char*>(data_[i]);
    }
  }
  std::vector<T*> new_data = data_;
  bool is_empty = data_.empty();
  data_.clear();
  data_.resize(3 * size);
  front_index_ += size * kArraySize - index_of_front_array * kArraySize;
  back_index_ += size * kArraySize - index_of_front_array * kArraySize;
  for (auto iter = data_.begin(); iter != data_.end(); ++iter) {
    int index = iter - data_.begin();
    if (index >= size && 2 * size > index && !is_empty) {
      data_[index] = new_data[index - size + index_of_front_array];
    } else {
      data_[index] = reinterpret_cast<T*>(new char[kArraySize * sizeof(T)]);
    }
  }
}

template <typename T>
template <bool IsConst>
class Deque<T>::common_iter {
 public:
  using value_type = std::conditional_t<IsConst, const T, T>;
  using difference_type = int;
  using iterator_category = std::random_access_iterator_tag;
  using cond_t =
          std::conditional_t<IsConst, typename std::vector<T*>::const_iterator,
                  typename std::vector<T*>::iterator>;
  using pointer = value_type*;
  using reference = value_type&;
  using commit = common_iter<IsConst>;

  reference operator*() { return *(*pointer_ + shift_); }

  explicit common_iter<IsConst>(const cond_t& ptr, size_t shift)
          : pointer_(ptr), shift_(shift) {}

  operator const_iterator() {
    return const_iterator(pointer_, shift_);
  }  // conversion from non-const to const availability

  commit& operator++() {
    ++shift_;
    if (shift_ == kArraySize) {
      ++pointer_;
      shift_ = 0;
    }
    return *this;
  }

  commit operator++(int) {
    commit prev = *this;
    ++(*this);
    return prev;
  }

  commit& operator--() {
    if (shift_ == 0) {
      --pointer_;
      shift_ = kArraySize;
    }
    --shift_;
    return *this;
  }

  commit operator--(int) {
    commit prev = *this;
    --(*this);
    return prev;
  }

  commit& operator+=(const int kValue) {
    if (kValue < 0) {
      return *this -= -kValue;
    }
    pointer_ += (kValue + shift_) / kArraySize;
    shift_ = (kValue + shift_) % kArraySize;
    return *this;
  }

  commit& operator-=(const int kValue) {
    if (kValue < 0) {
      return *this += -kValue;
    }
    int qqqqq = (shift_ - kValue) / kArraySize;  //  не трогать иначе падает
    pointer_ += qqqqq;
    shift_ = ((shift_ - kValue) % kArraySize + kArraySize) % kArraySize;
    return *this;
  }

  commit operator+(const int kValue) const {
    auto iter = *this;
    iter += kValue;
    return iter;
  }

  commit operator-(const int kValue) const {
    auto iter = *this;
    iter -= kValue;
    return iter;
  }

  pointer operator->() { return *pointer_ + shift_; }

  int operator-(const common_iter& other) {
    return (pointer_ - other.pointer_) * kArraySize + shift_ - other.shift_;
  }

  bool operator==(const Deque<T>::common_iter<IsConst>& other) const {
    return (pointer_ == other.pointer_ && shift_ == other.shift_);
  }

  bool operator!=(const Deque<T>::common_iter<IsConst>& other) const {
    return !(*this == other);
  }

  bool operator<(const Deque<T>::common_iter<IsConst>& other) {
    return (*this - other) < 0;
  }

  bool operator>(const Deque<T>::common_iter<IsConst>& other) {
    return (*this - other) > 0;
  }

  bool operator>=(const Deque<T>::common_iter<IsConst>& other) {
    return !(*this < other);
  }

  bool operator<=(const Deque<T>::common_iter<IsConst>& other) {
    return !(*this > other);
  }

 private:
  cond_t pointer_;
  size_t shift_;
};

template <typename T>
void Deque<T>::erase(iterator iter) {
  (iter)->~T();
  ++iter;
  while (iter != (*this).end()) {
    *(iter - 1) = *iter;
    ++iter;
  }
  Deque::pop_back();
}

template <typename T>
void Deque<T>::insert(iterator iter, const T& value) {
  if (iter == (*this).end()) {
    Deque::push_back(value);
    return;
  }
  T copy = *iter;
  *iter = value;
  insert(++iter, copy);
}