#include <iostream>
#include <cstring>

class String {
 private:
  size_t cap;
  size_t sz;
  char* array;
  void relocate(size_t new_cap) {
    char* tmp = new char[new_cap + 1];
    memcpy(tmp, array, sz);
    cap = new_cap;
    delete[] array;
    array = tmp;
  }
  void swap(String& string) {
    std::swap(array, string.array);
    std::swap(sz, string.sz);
    std::swap(cap, string.cap);
  }
 public:
  String(const char* string): cap(strlen(string)), sz(strlen(string)), array(new char[cap + 1]) {
    memcpy(array, string, cap + 1);
  }

  String(size_t n, char c): cap(n), sz(n), array(new char[cap + 1]) {
    memset(array, c, n);
    array[n] = '\0';
  }

  String(): cap(0), sz(0), array(new char[cap + 1]) {
    array[0] = '\0';
  }
  String(const String& string): cap(string.cap), sz(string.sz), array(new char[cap + 1]) {
    memcpy(array, string.array, cap + 1);
  }

  size_t size() const {
    return sz;
  }

  size_t capacity() const {
    return cap;
  }

  String& operator=(String string) {
    swap(string);
    return *this;
  }

  const char& operator[](size_t i) const {
    return array[i];
  }

  char& operator[](size_t i) {
    return array[i];
  }

  size_t length() const {
    return sz;
  }

  void push_back(char c) {
    if (sz + 1 > cap) {
      relocate(2 * cap + 1);
    }
    array[sz] = c;
    array[++sz] = '\0';
  }

  void pop_back() {
    array[--sz] = '\0';
  }

  char& front() {
    return array[0];
  }

  char& back() {
    return array[sz - 1];
  }

  const char& front() const {
    return array[0];
  }

  const char& back() const {
    return array[sz - 1];
  }

  String& operator+=(char c) {
    push_back(c);
    return *this;
  }

  String& operator+=(const String& string) {
    if (sz + string.sz > cap) {
      relocate(2 * (sz + string.sz));
    }
    memcpy(array + sz, string.array, string.sz);
    sz += string.sz;
    array[sz] = '\0';
    return *this;
  }

  size_t find(const String& substring) const {
    for (size_t i = 0; i <= sz - substring.sz; ++i) {
      if (memcmp(array + i, substring.data(), substring.sz) == 0) {
        return i;
      }
    }
    return sz;
  }

  size_t rfind(const String& substring) const {
    for (size_t i = 0; i <= sz - substring.sz; ++i) {
      if (memcmp(array + (sz - substring.sz - i), substring.data(), substring.sz) == 0) {
        return sz - substring.sz - i;
      }
    }
    return sz;
  }

  String substr(size_t start, size_t count) const {
    String result(count, '.');
    memcpy(result.array, array + start, count);
    result.array[count] = '\0';
    return result;
  }

  bool empty() {
    return (sz == 0);
  }

  void clear() {
    sz = 0;
    array[0] = '\0';
  }

  void shrink_to_fit() {
    relocate(sz);
    array[sz] = '\0';
  }

  char* data() {
    return array;
  }

  const char* data() const {
    return array;
  }

  ~String() {
    delete[] array;
  }
};

String operator+(const String& string1, const String& string2) {
  String result = string1;
  result += string2;
  return result;
}

String operator+(const String& string, char c) {
  String result = string;
  result += c;
  return result;
}

String operator+(char c, String& string) {
  String result(1, c);
  result += string;
  return result;
}

bool operator<(const String& string1, const String& string2) {
  return memcmp(string1.data(), string2.data(), std::max(string1.size(), string2.size())) < 0;
}

bool operator>=(const String& string1, const String& string2) {
  return !(string1 < string2);
}

bool operator>(const String& string1, const String& string2) {
  return string2 < string1;
}

bool operator<=(const String& string1, const String& string2) {
  return !(string1 > string2);
}

bool operator==(const String& string1, const String& string2) {
  if (string1.size() != string2.size()) {
    return false;
  }
  return memcmp(string1.data(), string2.data(), string1.size()) == 0;
}

bool operator!=(const String& string1, const String& string2) {
  return !(string1 == string2);
}

std::ostream& operator<<(std::ostream& out, const String& string) {
  out << string.data();
  return out;
}

std::istream& operator>>(std::istream& in, String& string) {
  while (std::isspace(in.peek()) && !in.eof()) {
    in.get();
  }
  while (!std::isspace(in.peek()) && !in.eof()) {
    string.push_back(in.get());
  }
  return in;
}
