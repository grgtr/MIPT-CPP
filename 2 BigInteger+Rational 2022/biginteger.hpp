#pragma once

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void DelStringZeros(std::string& str) {
  while (str[0] == '0') {
    str.erase(str.begin());
  }
}

class BigInteger {
 public:
  BigInteger();

  BigInteger(std::string str);

  BigInteger(long long value);

  std::string toString() const;

  operator std::string() const;

  BigInteger operator+() const;

  BigInteger operator-() const;

  BigInteger& operator++();

  BigInteger operator++(int);

  BigInteger& operator--();

  BigInteger operator--(int);

  BigInteger& operator+=(const BigInteger& value);

  BigInteger& operator-=(const BigInteger& value);

  BigInteger& operator*=(const BigInteger& value);

  BigInteger& operator/=(const BigInteger& value);

  BigInteger& operator%=(const BigInteger& value);

  explicit operator bool() { return digits_[0] != 0; };

  static BigInteger gcd(BigInteger left, BigInteger right);

  bool isNegative() const { return is_negative_; }

  const std::vector<int>& getArr() const { return digits_; }

  static BigInteger abs(const BigInteger& value);

  static void swap(BigInteger& left, BigInteger& right);

 private:
  static const int kBaseLength = 9;
  static const int kBase = 1000000000;
  bool is_negative_ = false;
  std::vector<int> digits_;

  void removeLeadZeros();

  void shiftRight();

  void reverse();
};

void BigInteger::removeLeadZeros() {
  while (digits_.size() > 1 && digits_.back() == 0) {
    digits_.pop_back();
  }
  if (digits_.size() == 1 && digits_[0] == 0) {
    is_negative_ = false;
  }
}

void BigInteger::shiftRight() {
  if (digits_.empty()) {
    digits_.push_back(0);
    return;
  }
  digits_.push_back(digits_[digits_.size() - 1]);
  for (size_t i = digits_.size() - 2; i > 0; --i) {
    digits_[i] = digits_[i - 1];
  }
  digits_[0] = 0;
}

void BigInteger::reverse() {
  for (size_t i = 0; i < digits_.size() / 2; ++i) {
    std::swap(digits_[i], digits_[digits_.size() - i - 1]);
  }
}

void BigInteger::swap(BigInteger& left, BigInteger& right) {
  std::swap(left.digits_, right.digits_);
  std::swap(left.is_negative_, right.is_negative_);
}

BigInteger::BigInteger() { digits_.push_back(0); }

BigInteger::BigInteger(std::string str) {
  if (str[0] == '-') {
    is_negative_ = true;
    str.erase(str.begin());
  }
  DelStringZeros(str);
  int end = static_cast<int>(str.length()) - 1;
  int size = kBaseLength;
  while (end >= 0) {
    if (end - kBaseLength + 1 < 0) {
      size = end + 1;
      end = kBaseLength - 1;
    }
    digits_.push_back(stoi(str.substr(end - kBaseLength + 1, size)));
    end -= kBaseLength;
  }
  if (digits_.back() == 0) {
    is_negative_ = false;
  }
}

BigInteger::BigInteger(long long value) : is_negative_(value < 0) {
  long long tmp = std::abs(value);
  do {
    digits_.push_back(static_cast<int>(tmp % kBase));
    tmp /= kBase;
  } while (tmp != 0);
}

std::string BigInteger::toString() const {
  if (digits_.empty()) {
    return "";
  }
  if (digits_.back() == 0) {
    return "0";
  }
  std::string str;
  if (is_negative_) {
    str += '-';
  }
  std::string cstr;
  for (long long i = static_cast<long long>(digits_.size()) - 1; i >= 0; --i) {
    cstr = std::to_string(digits_[i]);
    int cnt = static_cast<int>(cstr.size());
    while (cnt < kBaseLength && i != static_cast<int>(digits_.size()) - 1) {
      str += '0';
      ++cnt;
    }
    str += cstr;
  }
  return str;
}

std::ostream& operator<<(std::ostream& ostream, const BigInteger& value) {
  ostream << value.toString();
  return ostream;
}

std::istream& operator>>(std::istream& istream, BigInteger& value) {
  std::string ans;
  istream >> ans;
  BigInteger tmp(ans);
  value = tmp;
  return istream;
}

BigInteger::operator std::string() const {
  std::stringstream stringstream;
  stringstream << *this;
  return stringstream.str();
}

bool operator==(const BigInteger& left, const BigInteger& right) {
  if (left.isNegative() != right.isNegative()) {
    return false;
  }
  if (left.getArr().empty()) {
    return (right.getArr().empty() ||
            (right.getArr().size() == 1 && right.getArr()[0] == 0));
  }
  if (right.getArr().empty()) {
    return (left.getArr().size() == 1 && left.getArr()[0] == 0);
  }
  if (left.getArr().size() != right.getArr().size()) {
    return false;
  }
  return left.getArr() == right.getArr();
}

bool operator<(const BigInteger& left, const BigInteger& right) {
  bool both_is_negative = false;
  if (left == right) {
    return false;
  }
  if (left.isNegative()) {
    if (right.isNegative()) {
      both_is_negative = true;
    }
    return true;
  }
  if (right.isNegative()) {
    return false;
  }

  if (left.getArr().size() != right.getArr().size()) {
    return (left.getArr().size() < right.getArr().size()) ^ both_is_negative;
  }
  for (long long i = static_cast<long long>(left.getArr().size()) - 1; i >= 0;
       --i) {
    if (left.getArr()[i] != right.getArr()[i]) {
      return (left.getArr()[i] < right.getArr()[i]) ^ both_is_negative;
    }
  }
  return both_is_negative;
}

bool operator!=(const BigInteger& left, const BigInteger& right) {
  return !(left == right);
}

bool operator<=(const BigInteger& left, const BigInteger& right) {
  return (left < right || left == right);
}

bool operator>(const BigInteger& left, const BigInteger& right) {
  return !(left <= right);
}

bool operator>=(const BigInteger& left, const BigInteger& right) {
  return !(left < right);
}

BigInteger operator""_bi(unsigned long long val) {
  BigInteger value(val);
  return value;
}

BigInteger operator""_bi(const char* val, size_t size) {
  BigInteger value(val);
  ++size;
  return value;
}

BigInteger BigInteger::operator+() const { return BigInteger(*this); }

BigInteger BigInteger::operator-() const {
  BigInteger tmp(*this);
  tmp.is_negative_ = !tmp.is_negative_;
  return tmp;
}

BigInteger operator+(const BigInteger& left, const BigInteger& right) {
  BigInteger ans = left;
  ans += right;
  return ans;
}

BigInteger operator-(const BigInteger& left, const BigInteger& right) {
  BigInteger ans = left;
  ans -= right;
  return ans;
}

BigInteger operator*(const BigInteger& left, const BigInteger& right) {
  BigInteger ans = left;
  ans *= right;
  return ans;
}

BigInteger operator/(const BigInteger& left, const BigInteger& right) {
  BigInteger ans = left;
  ans /= right;
  return ans;
}

BigInteger operator%(const BigInteger& left, const BigInteger& right) {
  BigInteger ans = left;
  ans %= right;
  return ans;
}

BigInteger& BigInteger::operator++() { return (*this += 1); }

BigInteger BigInteger::operator++(int) {
  *this += 1;
  return *this - 1;
}

BigInteger& BigInteger::operator--() { return *this -= 1; }

BigInteger BigInteger::operator--(int) {
  *this -= 1;
  return *this + 1;
}

BigInteger& BigInteger::operator+=(const BigInteger& value) {
  if (is_negative_) {
    is_negative_ = false;
    BigInteger tmp = abs(value);
    value.is_negative_ ? * this += tmp : * this -= tmp;
    is_negative_ = value.is_negative_ || tmp < *this;
    return *this;
  }
  if (value.is_negative_) {
    *this -= abs(value);
    return *this;
  }
  int carry = 0;
  for (size_t i = 0;
       i < std::max(digits_.size(), value.digits_.size()) || carry != 0; ++i) {
    if (i == digits_.size()) {
      digits_.push_back(0);
    }
    digits_[i] += carry + (i < value.digits_.size() ? value.digits_[i] : 0);
    carry = static_cast<int>(digits_[i] >= kBase);
    if (carry != 0) {
      digits_[i] -= kBase;
    }
  }
  return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& value) {
  if (is_negative_) {
    is_negative_ = false;
    value.is_negative_ ? * this -= abs(value) : * this += abs(value);
    is_negative_ = true;
    return *this;
  }
  if (value.is_negative_) {
    *this += abs(value);
    return *this;
  }
  if (*this < value) {
    BigInteger tmp = value;
    swap(*this, tmp);
    *this -= tmp;
    is_negative_ = true;
    return *this;
  }
  int carry = 0;
  for (size_t i = 0; i < value.digits_.size() || carry != 0; ++i) {
    digits_[i] -= carry + (i < value.digits_.size() ? value.digits_[i] : 0);
    carry = static_cast<int>(digits_[i] < 0);
    if (carry != 0) {
      digits_[i] += kBase;
    }
  }
  removeLeadZeros();
  return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& value) {
  BigInteger ans;
  ans.digits_.resize(digits_.size() + value.digits_.size());
  for (size_t i = 0; i < digits_.size(); ++i) {
    int carry = 0;
    for (size_t j = 0; j < value.digits_.size() || carry != 0; ++j) {
      long long cur =
              ans.digits_[i + j] +
              digits_[i] * 1LL * (j < value.digits_.size() ? value.digits_[j] : 0) +
              carry;
      ans.digits_[i + j] = static_cast<int>(cur % kBase);
      carry = static_cast<int>(cur / kBase);
    }
  }
  ans.is_negative_ = is_negative_ != value.is_negative_;
  ans.removeLeadZeros();
  *this = ans;
  return *this;
}

BigInteger& BigInteger::operator/=(const BigInteger& value) {
  BigInteger tmp = abs(value);
  BigInteger ans;
  BigInteger current;
  ans.digits_.resize(digits_.size());
  for (long long i = static_cast<long long>(digits_.size()) - 1; i >= 0; --i) {
    current.shiftRight();
    current.digits_[0] = digits_[i];
    current.removeLeadZeros();
    int val = 0;
    int left = 0;
    int right = kBase;
    while (left <= right) {
      int medium = (left + right) / 2;
      BigInteger gist = tmp * medium;
      if (gist <= current) {
        val = medium;
        left = medium + 1;
      } else {
        right = medium - 1;
      }
    }
    ans.digits_[i] = val;
    current = current - tmp * val;
  }
  ans.is_negative_ = is_negative_ != value.is_negative_;
  ans.removeLeadZeros();
  *this = ans;
  return *this;
}

BigInteger& BigInteger::operator%=(const BigInteger& value) {
  BigInteger bi1(*this);
  bi1 /= value;
  bi1 *= value;
  *this -= bi1;
  return *this;
}

BigInteger BigInteger::gcd(BigInteger left, BigInteger right) {
  left.is_negative_ = false;
  right.is_negative_ = false;
  while (right != 0) {
    left %= right;
    swap(left, right);
  }
  return left;
}

BigInteger BigInteger::abs(const BigInteger& value) {
  BigInteger tmp = value;
  tmp.is_negative_ = false;
  return tmp;
}

class Rational {
 public:
  Rational() = default;

  Rational(const Rational& other) = default;

  Rational(int other)
          : nominator_(abs(other)), denominator_(1), is_negative_(other < 0){};

  Rational(const BigInteger& other)
          : nominator_(BigInteger::abs(other)),
            denominator_(1),
            is_negative_(other.isNegative()){};

  Rational(const BigInteger& bi1, const BigInteger& bi2);

  Rational& operator=(int other);

  Rational& operator=(const BigInteger& other);

  Rational& operator=(const Rational& other);

  Rational& operator+=(const Rational& other);

  Rational& operator-=(const Rational& other);

  Rational& operator*=(const Rational& other);

  Rational& operator/=(const Rational& other);

  Rational operator+() const;

  Rational operator-() const;

  BigInteger getNominator() const { return nominator_; }

  BigInteger getDenominator() const { return denominator_; }

  bool isNegative() const { return is_negative_; }

  std::string toString() const;

  std::string asDecimal(size_t precision);

  explicit operator double() {
    return std::stod((*this).asDecimal(Rational::kDecimal));
  }

  void checkNull() {
    if (nominator_ == 0) {
      is_negative_ = false;
    }
  }

  void setSign(bool val) { is_negative_ = !val; }

 private:
  BigInteger nominator_, denominator_;
  bool is_negative_ = false;
  const int kDecimal = 300;
  const int kNum = 10;

  static void swap(Rational& left, Rational& right);

  void rationalOperatorsGcd(Rational& tmp, const Rational& other, int sig);

  static void rationalGcd(Rational& tmp);
};

bool operator==(const Rational& left, const Rational& right) {
  return left.isNegative() == right.isNegative() and
         BigInteger::abs(left.getNominator()) ==
         BigInteger::abs(right.getNominator()) and
         BigInteger::abs(left.getDenominator()) ==
         BigInteger::abs(right.getDenominator());
}

bool operator!=(const Rational& left, const Rational& right) {
  return !(left == right);
}

bool operator<(const Rational& left, const Rational& right) {
  if (left.isNegative() ^ right.isNegative()) {
    return static_cast<int>(left.isNegative()) >
           static_cast<int>(right.isNegative());
  }
  return right.getNominator() * left.getDenominator() >
         left.getNominator() * right.getDenominator();
}

bool operator>(const Rational& left, const Rational& right) {
  return !(left < right) and (left != right);
}

bool operator<=(const Rational& left, const Rational& right) {
  return !(left > right);
}

bool operator>=(const Rational& left, const Rational& right) {
  return !(left < right);
}

Rational operator+(const Rational& left, const Rational& right) {
  Rational tmp = left;
  tmp += right;
  tmp.checkNull();
  return tmp;
}

Rational operator-(const Rational& left, const Rational& right) {
  Rational tmp = left;
  tmp -= right;
  tmp.checkNull();
  return tmp;
}

Rational operator/(const Rational& left, const Rational& right) {
  Rational tmp = left;
  tmp /= right;
  tmp.checkNull();
  return tmp;
}

Rational operator*(const Rational& left, const Rational& right) {
  Rational tmp = left;
  tmp *= right;
  tmp.checkNull();
  return tmp;
}

Rational::Rational(const BigInteger& bi1, const BigInteger& bi2) {
  nominator_ /= BigInteger::gcd(bi1, bi2);
  denominator_ /= BigInteger::gcd(bi1, bi2);
  is_negative_ = bi1.isNegative() ^ bi2.isNegative();
}

Rational& Rational::operator=(int other) {
  Rational tmp(std::to_string(other));
  *this = tmp;
  checkNull();
  return *this;
}

Rational& Rational::operator=(const BigInteger& other) {
  Rational tmp(other);
  swap(*this, tmp);
  checkNull();
  return *this;
}

Rational& Rational::operator=(const Rational& other) {
  Rational tmp(other);
  swap(*this, tmp);
  checkNull();
  return *this;
}

void Rational::rationalGcd(Rational& tmp) {
  BigInteger gcd = BigInteger::gcd(tmp.nominator_, tmp.denominator_);
  tmp.nominator_ /= gcd;
  tmp.denominator_ /= gcd;
}

void Rational::rationalOperatorsGcd(Rational& tmp, const Rational& other,
                                    int sig) {
  if (sig == 1 or sig == -1) {
    tmp.denominator_ = denominator_ * other.denominator_;
    tmp.nominator_ =
            nominator_ * other.denominator_ + sig * denominator_ * other.nominator_;
    rationalGcd(tmp);
  } else {
    is_negative_ = is_negative_ ^ other.is_negative_;
    if (sig == 0) {
      denominator_ = denominator_ * other.denominator_;
      nominator_ = nominator_ * other.nominator_;
    } else {
      denominator_ = denominator_ * other.nominator_;
      nominator_ = nominator_ * other.denominator_;
    }
    rationalGcd(*this);
  }
}

Rational& Rational::operator+=(const Rational& other) {
  Rational tmp;
  if ((is_negative_ ^ other.is_negative_) == 0) {
    tmp.is_negative_ = ((static_cast<int>(is_negative_) &
                         static_cast<int>(other.is_negative_)) != 0);
    rationalOperatorsGcd(tmp, other, 1);
    *this = tmp;
    checkNull();
    return *this;
  }
  if (is_negative_ or other.is_negative_) {
    tmp = is_negative_ ? *this : other;
    tmp.is_negative_ = is_negative_ ? !is_negative_ : !other.is_negative_;
    *this = is_negative_ ? other - tmp : *this - tmp;
    tmp.is_negative_ = false;
    is_negative_ = is_negative_ ? other < tmp : *this < tmp;
  }
  checkNull();
  return *this;
}

Rational& Rational::operator-=(const Rational& other) {
  Rational tmp;
  if ((is_negative_ ^ other.is_negative_) == 0) {
    tmp.is_negative_ = *this < other;
    rationalOperatorsGcd(tmp, other, -1);
    *this = tmp;
    checkNull();
    return *this;
  }
  tmp = other;
  tmp.is_negative_ = !other.is_negative_;
  *this += tmp;
  checkNull();
  return *this;
}

Rational& Rational::operator*=(const Rational& other) {
  rationalOperatorsGcd(*this, other, 0);
  checkNull();
  return *this;
}

Rational& Rational::operator/=(const Rational& other) {
  rationalOperatorsGcd(*this, other, 2);
  checkNull();
  return *this;
}

Rational Rational::operator+() const { return Rational(*this); }

Rational Rational::operator-() const {
  Rational tmp(*this);
  tmp.is_negative_ = !tmp.is_negative_;
  return tmp;
}

std::string Rational::toString() const {
  std::string ans;
  ans = is_negative_ ? "-" : "";
  ans += nominator_.toString();
  ans += denominator_ != 1 ? '/' + denominator_.toString() : "";
  return ans;
}

std::string Rational::asDecimal(size_t precision) {
  Rational tmp = *this;
  tmp.setSign(true);
  std::string ans;
  ans += isNegative() ? "-" : "";
  ans += (tmp.nominator_ / tmp.denominator_).toString();
  tmp.nominator_ %= tmp.denominator_;
  ans += precision != 0 ? "." : "";
  std::string element;
  for (size_t i = 0; i < precision; ++i) {
    tmp.nominator_ *= kNum;
    element = (tmp.nominator_ / tmp.denominator_).toString();
    ans += element;
    tmp.nominator_ %= tmp.denominator_;
  }
  return ans;
}

void Rational::swap(Rational& left, Rational& right) {
  BigInteger::swap(left.nominator_, right.nominator_);
  BigInteger::swap(left.denominator_, right.denominator_);
  std::swap(left.is_negative_, right.is_negative_);
}