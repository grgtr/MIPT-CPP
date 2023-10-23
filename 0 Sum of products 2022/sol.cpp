#include <iostream>

template <typename T>
class Vector {
 public:
  Vector();

  explicit Vector(size_t size);

  Vector(size_t size, T element);

  Vector(const Vector& other);

  Vector(Vector&& other)

  noexcept;

  size_t size() const;

  void resize(size_t size);

  void push_back(T element);

  void remove(size_t index);

  void pop_back();

  long long int find(T element);

  void delete_arr();

  void clear();

  T* begin();

  const T* begin() const;

  T* end();

  const T* end() const;

  Vector& operator=(const Vector& other);

  Vector& operator=(Vector&& other)

  noexcept;

  T& operator[](size_t index) { return arr_[index]; }

  const T& operator[](size_t index) const { return arr_[index]; }

  ~Vector();

 private:
  size_t size_;
  size_t cap_;
  T* arr_ = nullptr;

  void resize_memory(size_t size);

  static void swap(Vector<T>& vector1, Vector<T>& vector2);
};

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector<T>& other) {
  Vector<T> tmp(other);
  swap(*this, tmp);
  return *this;
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector<T>&& other)

noexcept {
Vector<T> tmp(std::move(other));
swap(*this, tmp);
return *this;
}

template <typename T>
void Vector<T>::resize_memory(size_t size) {
  //вызывается после изменения размера, передаётся старый размер, чтобы
  // скопировать старые значения
  T* tmp = arr_;
  arr_ = new T[cap_];
  for (size_t i = 0; i < size; ++i) {
    arr_[i] = tmp[i];
  }
  delete[] tmp;
}

template <typename T>
Vector<T>::Vector() : size_(0), cap_(1), arr_(new T[cap_]) {}

template <typename T>
Vector<T>::Vector(size_t size)
        : size_(size), cap_(2 * size), arr_(new T[cap_]) {}

template <typename T>
Vector<T>::Vector(size_t size, T element)
        : size_(size), cap_(2 * size), arr_(new T[cap_]) {
  for (size_t i = 0; i < size_; ++i) {
    arr_[i] = element;
  }
}

template <typename T>
Vector<T>::Vector(const Vector& other) {
  if (this != &other) {
    arr_ = new T[other.cap_];
    for (size_t i = 0; i < other.size_; ++i) {
      arr_[i] = other.arr_[i];
    }
    size_ = other.size_;
    cap_ = other.cap_;
  }
}

template <typename T>
Vector<T>::Vector(Vector&& other)

noexcept {
if (this != &other) {
arr_ = other.arr_;
size_ = other.size_;
cap_ = other.cap_;
other.arr_ = nullptr;
other.size_ = other.cap_ = 0;
}
}

template <typename T>
size_t Vector<T>::size() const {
  return size_;
}

template <typename T>
void Vector<T>::resize(size_t size) {
  size_t last_size = size_;
  size_ = size;
  if (size_ > last_size) {
    if (cap_ <= size_) {
      do {
        cap_ *= 2;
      } while (cap_ <= size_);
      resize_memory(last_size);
    }
  } else if (size_ < last_size) {
    if (cap_ / 2 > size_) {
      cap_ /= 2;
      resize_memory(size_);
    }
  }
}

template <typename T>
void Vector<T>::push_back(T element) {
  ++size_;
  if (cap_ <= size_) {
    cap_ *= 2;
    resize_memory(size_);
  }
  arr_[size_ - 1] = element;
}

template <typename T>
void Vector<T>::remove(size_t index) {
  for (size_t i = index + 1; i < size_; ++i) {
    arr_[i - 1] = arr_[i];
  }
  --size_;
}

template <typename T>
void Vector<T>::pop_back() {
  --size_;
  resize_memory(size_);
}

template <typename T>
long long Vector<T>::find(T element) {
  for (unsigned int i = 0; i < size_; ++i) {
    if (element == arr_[i]) {
      return i;
    }
  }
  return -1;
}

template <typename T>
void Vector<T>::delete_arr() {
  cap_ = 0;
  resize_memory(0);
  delete[] arr_;
}

template <typename T>
void Vector<T>::clear() {
  resize(0);
}

template <typename T>
T* Vector<T>::begin() {
  return &arr_[0];
}

template <typename T>
const T* Vector<T>::begin() const {
  return &arr_[0];
}

template <typename T>
T* Vector<T>::end() {
  return &arr_[size_];
}

template <typename T>
const T* Vector<T>::end() const {
  return &arr_[size_];
}

template <typename T>
Vector<T>::~Vector() {
  delete[] arr_;
}

template <typename T>
void Vector<T>::swap(Vector<T>& vector1, Vector<T>& vector2) {
  std::swap(vector1.size_, vector2.size_);
  std::swap(vector1.cap_, vector2.cap_);
  std::swap(vector1.arr_, vector2.arr_);
}

template <typename T>
std::ostream& operator<<(std::ostream& ostream, const Vector<T>& arr) {
  for (auto& element : arr) {
    ostream << element << " ";
  }
  return ostream;
}

template <typename T>
std::ostream& operator<<(std::ostream& ostream, const Vector<Vector<T>>& arr) {
  for (auto& index : arr) {
    for (T element : index) {
      ostream << element << " ";
    }
    ostream << "\n";
  }
  return ostream;
}

template <typename T>
inline std::istream& operator>>(std::istream& istream, Vector<T>& arr) {
  for (auto& element : arr) {
    istream >> element;
  }
  return istream;
}

template <typename T>
inline std::istream& operator>>(std::istream& istream, Vector<Vector<T>>& arr) {
  for (auto& index : arr) {
    for (T& element : index) {
      istream >> element;
    }
  }
  return istream;
}

template <typename T>
long long Find(const Vector<T>& arr, T index) {
  for (ssize_t i = 0; i < static_cast<T>(arr.size()); ++i) {
    if (arr[i] == index) {
      return i;
    }
  }
  return -1;
}

long long ToLl(const std::string& str) { return atoll(str.c_str()); }

void OneMoreFor(long long& ans, size_t index,
                const Vector<Vector<long long>>& arr, long long& product,
                Vector<int>& indexes) {
  for (int j = 0; j < static_cast<int>(arr[index].size()); ++j) {
    if (Find(indexes, j) == -1) {
      long long last_product = product;
      product *= arr[index][j];
      if (index == arr.size() - 1) {
        ans += product;
      } else {
        indexes.push_back(j);
        OneMoreFor(ans, index + 1, arr, product, indexes);
        indexes.pop_back();
      }
      if (arr[index][j] != 0) {
        product /= arr[index][j];
      } else {
        product = last_product;
      }
    }
  }
}

int main(int argc, char** argv) {
  Vector<size_t> sizes;
  for (int i = 1; i < argc; ++i) {
    sizes.push_back(ToLl(argv[i]));
  }

  Vector<Vector<long long>> arr(sizes.size());

  size_t element = 0;
  for (auto& vec : arr) {
    vec.resize(sizes[element]);
    ++element;
  }
  std::cin >> arr;
  long long ans = 0;
  long long product = 1;
  Vector<int> indexes;
  indexes.push_back(-1);
  OneMoreFor(ans, 0, arr, product, indexes);
  std::cout << ans;
}