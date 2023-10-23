#pragma once

#include <cstring>
#include <iostream>

class String {
 public:
  String();

  String(const char* str);

  String(size_t size);

  String(size_t size, char element);

  String(const String& other);

  void push_back(char element);

  void pop_back() { arr_[--size_] = '\0'; }

  void clear() { size_ = 0; }

  void shrink_to_fit();

  String substr(size_t start, size_t count) const;

  size_t find(const String& substr) const;

  size_t rfind(const String& substr) const;

  [[nodiscard]] size_t length() const { return size_; }

  [[nodiscard]] size_t size() const { return length() + 1; }

  [[nodiscard]] size_t capacity() const { return cap_; }

  [[nodiscard]] bool empty() const { return size_ == 0; }

  [[nodiscard]] char* data() { return &arr_[0]; }

  [[nodiscard]] const char* data() const { return &arr_[0]; }

  [[nodiscard]] char& front() { return arr_[0]; }

  [[nodiscard]] const char& front() const { return arr_[0]; }

  [[nodiscard]] char& back() { return arr_[size_ - 1]; }

  [[nodiscard]] const char& back() const { return arr_[size_ - 1]; }

  [[nodiscard]] char* begin() { return &arr_[0]; }

  [[nodiscard]] const char* begin() const { return &arr_[0]; }

  [[nodiscard]] char* end() { return &arr_[size_]; }

  [[nodiscard]] const char* end() const { return &arr_[size_]; }

  String& operator=(const String& other);

  String& operator=(const char* other);

  String& operator+=(const String& str);

  String& operator+=(char str);

  char& operator[](size_t index) { return arr_[index]; }

  const char& operator[](size_t index) const { return arr_[index]; }

  ~String() { delete[] arr_; }

 private:
  size_t size_{};
  size_t cap_{};
  char* arr_{nullptr};

  static void swap(String& str1, String& str2);

  void resize_memory(size_t size);

  bool in_str(size_t index, const String& str) const;
};

void String::swap(String& str1, String& str2) {
  std::swap(str1.size_, str2.size_);
  std::swap(str1.cap_, str2.cap_);
  std::swap(str1.arr_, str2.arr_);
}

void String::resize_memory(size_t size) {
  char* tmp = new char[cap_];
  std::copy(arr_, arr_ + size, tmp);
  tmp[size_] = '\0';
  delete[] arr_;
  arr_ = tmp;
}

bool String::in_str(size_t index, const String& str) const {
  size_t size = size_ > str.length() ? str.length() : size_;
  for (size_t i = index; i < index + size; ++i) {
    if (str[i - index] != arr_[i]) {
      return false;
    }
  }
  return true;
}

String::String() : size_(0), cap_(1), arr_(new char[1]) { arr_[size_] = '\0'; }

String::String(const char* str)
        : size_(strlen(str)), cap_(size()), arr_(new char[size()]) {
  std::copy(str, str + size_, arr_);
  arr_[size_] = '\0';
}

String::String(size_t size)
        : size_(size), cap_(size_ + 1), arr_(new char[cap_]) {
  arr_[size_] = '\0';
}

String::String(size_t size, char element)
        : size_(size), cap_(size + 1), arr_(new char[size + 1]) {
  std::fill(arr_, arr_ + size_, element);
  arr_[size_] = '\0';
}

String::String(const String& other)
        : size_(other.size_), cap_(other.cap_), arr_(new char[other.cap_]) {
  std::copy(other.arr_, other.arr_ + other.size_, arr_);
  arr_[size_] = '\0';
}

void String::push_back(char element) {
  ++size_;
  if (cap_ <= size_) {
    cap_ = 2 * cap_ + 1;
    resize_memory(size_ - 1);
  }
  arr_[size_ - 1] = element;
  arr_[size_] = '\0';
}

void String::shrink_to_fit() {
  if (cap_ - 1 != size_) {
    cap_ = size_ + 1;
    resize_memory(size_);
  }
}

String String::substr(size_t start, size_t count) const {
  String ans(count);
  std::copy(arr_ + start, arr_ + start + count, ans.arr_);
  return ans;
}

size_t String::find(const String& str) const {
  if (str.size_ > size_) {
    return size_;
  }
  for (size_t i = 0; i < size_ - str.size_ + 1; ++i) {
    if (in_str(i, str)) {
      return i;
    }
  }
  return size_;
}

size_t String::rfind(const String& str) const {
  if (str.size_ > size_) {
    return size_;
  }
  for (size_t i = size_ - str.size_; true; --i) {
    if (in_str(i, str)) {
      return i;
    }
    if (i == 0) {
      break;
    }
  }
  return size_;
}

String& String::operator=(const String& other) {
  String tmp(other);
  swap(*this, tmp);
  return *this;
}

String& String::operator=(const char* other) {
  String tmp(other);
  swap(*this, tmp);
  return *this;
}

String& String::operator+=(const String& str) {
  if (cap_ < size_ + str.size_) {
    cap_ = (size_ + str.size_) * 2;
    resize_memory(size_);
  }
  std::copy(str.arr_, str.arr_ + str.size_, arr_ + size_);
  size_ += str.size_;
  return *this;
}

String& String::operator+=(char str) {
  this->push_back(str);
  return *this;
}

bool operator==(const String& str1, const String& str2) {
  if (str1.length() != str2.length()) {
    return false;
  }
  size_t size = str1.length() > str2.length() ? str1.length() : str2.length();
  for (size_t i = 0; i < size; ++i) {
    if (str1[i] != str2[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const String& str1, const String& str2) {
  return !(str1 == str2);
}

inline std::istream& operator>>(std::istream& istream, String& str) {
  char symbol = istream.get();
  while (symbol == ' ') {
    symbol = istream.get();
  }
  if (isspace(symbol) == 0) {
    str.push_back(symbol);
  }
  symbol = istream.get();
  while (symbol != ' ' and symbol != '\n' and symbol != '\t' and
         !istream.eof()) {
    str.push_back(symbol);
    symbol = istream.get();
  }
  return istream;
}

inline std::ostream& operator<<(std::ostream& ostream, const String& str) {
  if (str.length() == 0) {
    return ostream;
  }
  ostream << str.data();
  return ostream;
}

String operator+(const String& str1, const String& str2) {
  String copy = str1;
  copy += str2;
  return copy;
}

String operator+(const String& str, char symbol) {
  String copy = str;
  copy.push_back(symbol);
  return copy;
}

String operator+(char symbol, const String& str) {
  String ans;
  ans.push_back(symbol);
  ans += str;
  return ans;
}

bool operator<(const String& str1, const String& str2) {
  size_t size = str1.length() > str2.length() ? str2.length() : str1.length();
  for (size_t i = 0; i < size; ++i) {
    if (str1[i] > str2[i]) {
      return false;
    }
    if (str1[i] < str2[i]) {
      return true;
    }
  }
  return str1.length() < str2.length();
}

bool operator>(const String& str1, const String& str2) { return str2 < str1; }

bool operator>=(const String& str1, const String& str2) {
  return !(str2 < str1);
}

bool operator<=(const String& str1, const String& str2) {
  return !(str2 > str1);
}