#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class BigInteger;
BigInteger operator*(const BigInteger&, const BigInteger&);
BigInteger operator/(const BigInteger&, const BigInteger&);
void plus(std::vector<long long>&, const std::vector<long long>&, long long);
void minus(std::vector<long long>&, const std::vector<long long>&, long long);
bool less(const std::vector<long long>&, const std::vector<long long>&);

class BigInteger {
  friend bool operator<(const BigInteger&, const BigInteger&);
  friend bool operator==(const BigInteger&, const BigInteger&);
 private:
  static const long long MOD = 1'000'000'000;
  static const int LOGMOD = 9;
  void removeLeadZeroes() {
    while (digits.size() > 1 && digits.back() == 0) { digits.pop_back(); }
    if (digits.size() == 1 && digits.back() == 0) { sign = 0; }
  }
  int sign;
  std::vector<long long> digits;
  void levelUp(long long x) {
    digits.push_back(0);
    for (int i = static_cast<int>(digits.size()) - 1; i >= 1; --i) {
      if (digits[i - 1]) { sign = 1; }
      digits[i] = digits[i - 1];
    }
    digits[0] = x;
    if (x) { sign = 1; }
    removeLeadZeroes();
  }
 public:
  int size() const { return static_cast<int>(digits.size()); }
  int sgn() const { return sign; }
  BigInteger abs() const {
    if (sign == -1) { return -*this; }
    return *this;
  }
  BigInteger(long long n) {
    if (n) {
      sign = (n > 0 ? 1 : -1);
      if (n < 0) { n *= -1; }
      while (n > 0) {
        digits.push_back(n % MOD);
        n /= MOD;
      }
    } else {
      sign = 0;
      digits.push_back(0);
    }
  }
  BigInteger(): BigInteger(0) {}
  BigInteger(const std::string& s) {
    sign = 1;
    int st = 0;
    if (s[0] == '-') {
      sign = -1;
      st = 1;
    }
    if (s[st] == '0') { sign = 0; }
    for (int i = static_cast<int>(s.length()); i > st; i -= LOGMOD) {
      if (i < LOGMOD + st) { digits.push_back(stoi(s.substr(st, i - st))); }
      else { digits.push_back(stoi(s.substr(i - LOGMOD, LOGMOD))); }
    }
  }
  explicit operator bool() const { return sign != 0; }
  void changeSign() { sign *= -1; }
  BigInteger& operator+=(const BigInteger& other) {
    if (sign * other.sign >= 0) {
      plus(digits, other.digits, MOD);
      removeLeadZeroes();
      return *this;
    }
    if (!less(digits, other.digits)) {
      minus(digits, other.digits, MOD);
    } else {
      std::vector<long long> tmp = other.digits;
      minus(tmp, digits, MOD);
      digits = tmp;
      changeSign();
    }
    removeLeadZeroes();
    return *this;
  }
  BigInteger& operator-=(const BigInteger& other) {
    return *this += (-other);
  }
  BigInteger& operator*=(const BigInteger& other) {
    sign *= other.sign;
    std::vector<long long> res(digits.size() + other.size() + 1);
    for (int i = 0; i < static_cast<int>(digits.size()); ++i) {
      long long carry = 0;
      for (int j = 0; j < other.size() || carry; ++j) {
        long long cur = res[i + j] + digits[i] * (j < other.size() ? other.digits[j] : 0) + carry;
        res[i + j] = cur % MOD;
        carry = cur / MOD;
      }
    }
    digits = res;
    removeLeadZeroes();
    return *this;
  }
  BigInteger& operator/=(const BigInteger& other) {
    if (sign == 0) { return *this; }
    BigInteger mod_a = abs(), mod_b = other.abs();
    if (mod_a < mod_b) return *this = 0;
    std::vector<long long> tmp;
    BigInteger cur;
    int i = static_cast<int>(digits.size()) - 1;
    while (i >= 0) {
      if (cur == 0) {
        while (i >= 0 && !digits[i]) {
          --i;
          tmp.push_back(0);
        }
      }
      if (i >= 0) {
        int cnt = 0;
        while (i >= 0 && cur < mod_b) {
          cur.levelUp(mod_a.digits[i--]);
          ++cnt;
          if (cnt > 1) { tmp.push_back(0); }
        }
        long long l = 0, r = MOD;
        while (r - l > 1) {
          long long m = (l + r) / 2;
          if (cur < mod_b * m) {
            r = m;
          } else {
            l = m;
          }
        }
        tmp.push_back(l);
        cur -= mod_b * l;
      }
    }
    std::reverse(tmp.begin(), tmp.end());
    digits = tmp;
    sign *= other.sign;
    removeLeadZeroes();
    return *this;
  }
  BigInteger& operator%=(const BigInteger& other) { return *this -= other * (*this / other); }
  BigInteger& operator++() { return *this += 1; }
  BigInteger operator++(int) {
    BigInteger tmp = *this;
    ++(*this);
    return tmp;
  }
  BigInteger& operator--() { return *this -= 1; }
  BigInteger operator--(int) {
    BigInteger tmp = *this;
    --(*this);
    return tmp;
  }
  BigInteger operator-() const {
    BigInteger tmp = *this;
    tmp.changeSign();
    return tmp;
  }
  BigInteger operator+() const {
    BigInteger tmp = *this;
    return tmp;
  }
  std::string toString() const {
    std::string ans;
    if (sign == -1) {
      ans += '-';
    }
    ans += std::to_string(digits.back());
    for (int i = static_cast<int>(digits.size()) - 2; i >= 0; --i) {
      std::string tmp = std::to_string(digits[i]);
      for (int j = 0; j < static_cast<int>(LOGMOD - tmp.length()); ++j) {
        ans += '0';
      }
      ans += tmp;
    }
    return ans;
  }
};

std::ostream& operator<<(std::ostream& out, const BigInteger& BigInteger) {
  out << BigInteger.toString();
  return out;
}
std::istream& operator>>(std::istream& in, BigInteger& a) {
  std::string s;
  in >> s;
  a = s;
  return in;
}
BigInteger operator "" _bi(const char* str, size_t) { return BigInteger(str); }
bool operator<(const BigInteger& a, const BigInteger& b) {
  if (a.sgn() < b.sgn()) { return true; }
  if (a.sgn() > b.sgn()) { return false; }
  if (a.sgn() >= 0) { return less(a.digits, b.digits); }
  return less(b.digits, a.digits);
}
bool operator==(const BigInteger& a, const BigInteger& b) {
  if (a.sgn() != b.sgn() || a.size() != b.size()) { return false; }
  for (int i = 0; i < a.size(); ++i) {
    if (a.digits[i] != b.digits[i]) { return false; }
  }
  return true;
}
bool operator!=(const BigInteger& a, const BigInteger& b) { return !(a == b); }
bool operator>(const BigInteger& a, const BigInteger& b) { return b < a; }
bool operator<=(const BigInteger& a, const BigInteger& b) { return !(b < a); }
bool operator>=(const BigInteger& a, const BigInteger& b) { return !(a < b); }
BigInteger operator+(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res += b;
  return res;
}
BigInteger operator-(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res -= b;
  return res;
}
BigInteger operator*(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res *= b;
  return res;
}
BigInteger operator/(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res /= b;
  return res;
}
BigInteger operator%(const BigInteger& a, const BigInteger& b) {
  BigInteger res = a;
  res %= b;
  return res;
}
void plus(std::vector<long long>& a, const std::vector<long long>& b, long long MOD) {
  long long carry = 0;
  for (int i = 0; (i < static_cast<int>(a.size()) || i < static_cast<int>(b.size()) || carry)
    && (i < static_cast<int>(b.size()) || carry); ++i) {
    if (i == static_cast<int>(a.size())) { a.push_back(0); }
    a[i] += (i < static_cast<int>(b.size()) ? b[i] : 0) + carry;
    carry = a[i] / MOD;
    a[i] %= MOD;
  }
}
void minus(std::vector<long long>& a, const std::vector<long long>& b, long long MOD) {
  long long carry = 0;
  for (int i = 0; i < static_cast<int>(a.size()) && (i < static_cast<int>(b.size()) || carry); ++i) {
    a[i] -= (i < static_cast<int>(b.size()) ? b[i] : 0) + carry;
    carry = a[i] < 0;
    if (carry) { a[i] += MOD; }
  }
}
bool less(const std::vector<long long>& a, const std::vector<long long>& b) {
  if (a.size() < b.size()) { return true; }
  if (a.size() > b.size()) { return false; }
  for (int i = static_cast<int>(a.size()) - 1; i >= 0; --i) {
    if (a[i] < b[i]) { return true; }
    if (a[i] > b[i]) { return false; }
  }
  return false;
}

class Rational {
  friend bool operator<(const Rational&, const Rational&);
  friend bool operator==(const Rational&, const Rational&);
 private:
  BigInteger num;
  BigInteger enom;
  BigInteger gcd(BigInteger a, BigInteger b) {
    if (a.sgn() < 0) { a.changeSign(); }
    if (b.sgn() < 0) { b.changeSign(); }
    while (b) {
      a %= b;
      std::swap(a, b);
    }
    return a;
  }
  void normalize() {
    BigInteger g = gcd(num, enom);
    if (g != 1) {
      num /= g;
      enom /= g;
    }
  }
 public:
  Rational(): Rational(0) {}
  Rational(BigInteger n): num(n), enom(1) {}
  Rational operator-() const {
    Rational tmp = *this;
    tmp.num.changeSign();
    return tmp;
  }
  Rational(int n): Rational(static_cast<BigInteger>(n)) {}
  Rational(const Rational& other): num(other.num), enom(other.enom) {}
  Rational& operator=(Rational other) {
    std::swap(num, other.num);
    std::swap(enom, other.enom);
    return *this;
  }
  Rational& operator+=(const Rational& other) {
    num *= other.enom;
    num += enom * other.num;
    enom *= other.enom;
    normalize();
    return *this;
  }
  Rational& operator-=(const Rational& other) { return *this += (-other); }
  Rational& operator*=(const Rational& other) {
    num *= other.num;
    enom *= other.enom;
    normalize();
    return *this;
  }
  Rational& operator/=(const Rational& other) {
    num *= other.enom;
    enom *= other.num;
    if (enom.sgn() == -1) {
      enom.changeSign();
      num.changeSign();
    }
    normalize();
    return *this;
  }
  std::string toString() const {
    if (enom == 1) { return num.toString(); }
    std::string ans = num.toString();
    ans += '/';
    ans += enom.toString();
    return ans;
  }
  std::string asDecimal(size_t precision = 0) const {
    if (num.sgn() == 0) { return "0"; }
    BigInteger tmp = ("1" + std::string(precision, '0'));
    BigInteger new_num = num * tmp;
    if (new_num.sgn() < 0) { new_num.changeSign(); }
    tmp = new_num / enom;
    std::string str = tmp.toString();
    if (str.size() <= precision) {
      str = std::string(precision - str.size(), '0') + str;
      std::string ans = "0." + str;
      if (num.sgn() < 0) { ans = '-' + ans; }
      return ans;
    }
    std::string ans = str.substr(0, str.size() - precision);
    ans += '.';
    ans += str.substr(str.size() - precision, precision);
    if (ans[0] == '.') { ans = '0' + ans; }
    if (num.sgn() < 0) { ans = '-' + ans; }
    return ans;
  }
  explicit operator double() const { return std::stod(asDecimal(5)); }
};

Rational operator+(const Rational& a, const Rational& b) {
  Rational res = a;
  return res += b;
}
Rational operator-(const Rational& a, const Rational& b) {
  Rational res = a;
  return res -= b;
}
Rational operator*(const Rational& a, const Rational& b) {
  Rational res = a;
  return res *= b;
}
Rational operator/(const Rational& a, const Rational& b) {
  Rational res = a;
  return res /= b;
}
bool operator<(const Rational& a, const Rational& b) { return a.num * b.enom < b.num * a.enom; }
bool operator==(const Rational& a, const Rational& b) { return a.num * b.enom == b.num * a.enom; }
bool operator!=(const Rational& a, const Rational& b) { return !(a == b); }
bool operator>(const Rational& a, const Rational& b) { return (b < a); }
bool operator<=(const Rational& a, const Rational& b) { return !(b < a); }
bool operator>=(const Rational& a, const Rational& b) { return !(a < b); }
