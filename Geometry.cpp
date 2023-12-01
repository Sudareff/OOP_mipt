#include <iostream>
#include <vector>
#include <math.h>

namespace constants {
const long double pi = 3.14159265358979323846;
const long double equalityPrecision = 1e-6;
const long double squarePrecision = 1e-1;
const long double flatAngle = 180;
}

class Line;
class Polygon;
class Rectangle;
class Square;
class Triangle;
class Ellipse;
class Circle;
bool isEqualZero(double d) { return abs(d) < constants::equalityPrecision; }
struct Point {
 public:
  long double x, y;
  Point(long double x, long double y): x(x), y(y) {}
  void rotate(const Point& center, long double angle) {
    angle *= constants::pi / constants::flatAngle;
    long double x_tmp = x - center.x, y_tmp = y - center.y;
    x = x_tmp * cos(angle) - y_tmp * sin(angle) + center.x;
    y = x_tmp * sin(angle) + y_tmp * cos(angle) + center.y;
  }
  Point(): Point(0, 0) {}
  void reflect(const Point& center) {
    x = 2 * center.x - x;
    y = 2 * center.y - y;
  }
  void reflect(const Line& axis);
  void scale(const Point& center, long double coefficient) {
    x = (x - center.x) * coefficient + center.x;
    y = (y - center.y) * coefficient + center.y;
  }
  long double mod() { return sqrt(x * x + y * y); }
};
bool operator==(const Point& A, const Point& B) { return isEqualZero(A.x - B.x) && isEqualZero(A.y - B.y); }
bool operator!=(const Point& A, const Point& B) { return !(A == B); }
Point operator+(Point A, Point B) { return Point(A.x + B.x, A.y + B.y); }
Point operator-(Point A, Point B) { return Point(A.x - B.x, A.y - B.y); }
Point operator*(long double d, Point A) { return Point(A.x * d, A.y * d); }
long double scalar(const Point& A, const Point& B) { return A.x * B.x + A.y * B.y; }
long double vect(const Point A, const Point& B) { return A.x * B.y - A.y * B.x; }
long double dist(const Point& A, const Point& B) { return sqrt(scalar(A - B, A - B)); }
long double area(const Point& A, const Point& B, const Point& C) {
  Point CA = A - C;
  Point CB = B - C;
  return 0.5 * abs(vect(CA, CB));
}
class Line {
 public:
  long double a;
  long double b;
  long double c;
  Line(const Point& A, const Point& B): a(A.y - B.y), b(B.x - A.x), c(A.x * B.y - A.y * B.x) {}
  Line(long double k, long double m): a(k), b(-1), c(m) {}
  Line(const Point& A, long double k): a(k), b(-1), c(A.y - k * A.x) {}
};
long double dist(const Line& l, const Point& A) {
  return abs(l.a * A.x + l.b * A.y + l.c) / sqrt(l.a * l.a + l.b * l.b + l.c * l.c);
}
bool operator==(const Line& l, const Line& m) {
  if (!isEqualZero(l.a * m.b - l.b * m.a)) { return false; }
  return isEqualZero(l.a * m.c - l.c * m.a);
}
void Point::reflect(const Line& axis) {
  long double x_tmp = x, y_tmp = y;
  x = (axis.b * axis.b - axis.a * axis.a) * x_tmp - 2 * axis.a * axis.b * y_tmp - 2 * axis.a * axis.c;
  x /= (axis.b * axis.b + axis.a * axis.a);
  y = (axis.a * axis.a - axis.b * axis.b) * y_tmp - 2 * axis.a * axis.b * x_tmp - 2 * axis.b * axis.c;
  y /= (axis.b * axis.b + axis.a * axis.a);
}

class Shape {
 public:
  virtual long double perimeter() const = 0;
  virtual long double area() const = 0;
  virtual bool operator==(const Shape& another) const = 0;
  virtual bool operator!=(const Shape& another) const = 0;
  virtual bool isCongruentTo(const Shape& another) const = 0;
  virtual bool isSimilarTo(const Shape& another) const = 0;
  virtual bool containsPoint(const Point& point) const = 0;
  virtual void rotate(const Point& center, long double angle) = 0;
  virtual void reflect(const Point& center) = 0;
  virtual void reflect(const Line& axis) = 0;
  virtual void scale(const Point& center, long double coefficient) = 0;
  virtual ~Shape() = default;
};

class Ellipse: public Shape {
 protected:
  Point F1;
  Point F2;
  long double a;
  long double c;
  long double b;
 public:
  Ellipse(const Point& A, const Point& B, long double d)
    : F1(A), F2(B), a(d / 2), c(dist(F1, F2) / 2), b(sqrt(a * a - c * c)) {}
  long double area() const override { return constants::pi * a * b; }
  long double perimeter() const override { return constants::pi * (3 * (a + b) - sqrt((3 * a + b) * (a + 3 * b))); }
  std::pair<Point, Point> focuses() { return {F1, F2}; }
  std::pair<Line, Line> directrices() {
    Line axis = {F1, F2};
    Point v = {-axis.b, axis.a};
    v = (1.0 / v.mod()) * v;
    Point A1 = center() + a / eccentricity() * v, A2 = center() - a / eccentricity() * v;
    return {{A1, axis.a / axis.b}, {A2, axis.a / axis.b}};
  }
  Point center() { return 0.5 * (F1 + F2); }
  long double eccentricity() { return c / a; }
  void rotate(const Point& center, long double angle) override {
    F1.rotate(center, angle);
    F2.rotate(center, angle);
  }
  void reflect(const Point& center) override {
    F1.reflect(center);
    F2.reflect(center);
  }
  void reflect(const Line& axis) override {
    F1.reflect(axis);
    F2.reflect(axis);
  }
  void scale(const Point& center, long double coefficient) override {
    F1.scale(center, coefficient);
    F2.scale(center, coefficient);
    a = abs(a * coefficient);
    b = abs(b * coefficient);
    c = abs(c * coefficient);
  }
  bool containsPoint(const Point& point) const override { return (dist(point, F1) + dist(point, F2) <= 2 * a); }
  bool isCongruentTo(const Shape& another) const override {
    const Ellipse* p = dynamic_cast<const Ellipse*>(&another);
    if (p == nullptr) { return false; }
    return (isEqualZero(p->a - a) && isEqualZero(p->b - b));
  }
  bool isSimilarTo(const Shape& another) const override {
    const Ellipse* p = dynamic_cast<const Ellipse*>(&another);
    if (p == nullptr) { return false; }
    return isEqualZero(a * p->b - b * p->a);
  }
  bool operator==(const Shape& another) const override {
    const Ellipse* p = dynamic_cast<const Ellipse*>(&another);
    if (p == nullptr) { return false; }
    return (F1 == p->F1 && F2 == p->F2 && a == p->a);
  }
  bool operator==(const Ellipse& another) const {
    return (F1 == another.F1 && F2 == another.F2 && a == another.a);
  }
  bool operator!=(const Ellipse& another) const {
    return !(*this == another);
  }
  bool operator!=(const Shape& another) const override {
    return !(*this == another);
  }
};

class Circle: public Ellipse {
 public:
  Circle(const Point& A, long double r): Ellipse(A, A, 2 * r) {}
  long double perimeter() const override { return 2 * constants::pi * a; }
  long double radius() { return a; }
};

class Polygon: public Shape {
 protected:
  std::vector<Point> vertices;
  void setter(Point v) { vertices.push_back(v); }
  template<typename... Args>
  void setter(Point v, Args... args) {
    vertices.push_back(v);
    setter(args...);
  }
  bool similarityChecker(const Polygon* p, bool orientation, int start, double k) const {
    int n = verticesCount();
    for (int i = 0; i < n; ++i) {
      Point A1 = vertices[(n + i - 1) % n], B1 = vertices[i], C1 = vertices[(i + 1) % n];
      Point A2 = p->vertices[(orientation ? n + i - 1 + start : n - 1 - (i - 1) + n + start) % n],
        B2 = p->vertices[(orientation ? i + start : n - 1 - i + start + n) % n],
        C2 = p->vertices[(orientation ? i + 1 + start : n - 1 - (i + 1) + start + n) % n];
      Point B1A1 = A1 - B1, B1C1 = C1 - B1;
      Point B2A2 = A2 - B2, B2C2 = C2 - B2;
      if (!isEqualZero(dist(B1, A1) - k * dist(B2, A2)) ||
        !isEqualZero(scalar(B1A1, B1C1) / (B1A1.mod() * B1C1.mod()) - scalar(B2A2, B2C2) / (B2A2.mod() * B2C2.mod()))) {
        return false;
      }
    }
    return true;
  }
  bool equalityChecker(const Polygon& p, bool orientation, int start) const {
    int n = verticesCount();
    for (int i = 0; i < n; ++i) {
      if (vertices[(orientation ? start + i : start + n - 1 - i) % n] != p.vertices[i]) { return false; }
    }
    return true;
  }
 public:
  Polygon(std::vector<Point> v): vertices(v) {}
  template<typename... Args>
  Polygon(Args... args) { setter(args...); }
  int verticesCount() const { return vertices.size(); }
  std::vector<Point> getVertices() const { return vertices; }
  bool isConvex() {
    int n = vertices.size();
    Point AB = vertices[n - 1] - vertices[n - 2];
    Point BC = vertices[0] - vertices[n - 1];
    int sign = vect(AB, BC);
    sign /= abs(sign);
    AB = vertices[0] - vertices[n - 1];
    BC = vertices[1] - vertices[0];
    if (vect(AB, BC) * sign <= 0) { return false; }
    for (int i = 1; i < n - 1; ++i) {
      AB = vertices[i] - vertices[i - 1];
      BC = vertices[i + 1] - vertices[i];
      if (vect(AB, BC) * sign <= 0) { return false; }
    }
    return true;
  }
  long double perimeter() const override {
    long double ans = dist(vertices.front(), vertices.back());
    for (int i = 0; i < static_cast<int>(vertices.size()) - 1; ++i) {
      ans += dist(vertices[i], vertices[i + 1]);
    }
    return ans;
  }
  long double area() const override {
    long double ans = 0;
    for (int i = 1; i < static_cast<int>(vertices.size()) - 1; ++i) {
      ans += ::area(vertices[0], vertices[i], vertices[i + 1]);
    }
    return ans;
  }
  void rotate(const Point& center, long double angle) override {
    for (auto& p : vertices) {
      p.rotate(center, angle);
    }
  }
  void reflect(const Point& center) override {
    for (auto& p : vertices) {
      p.reflect(center);
    }
  }
  void reflect(const Line& axis) override {
    for (auto& p : vertices) {
      p.reflect(axis);
    }
  }
  void scale(const Point& center, long double coefficient) override {
    for (auto& p : vertices) {
      p.scale(center, coefficient);
    }
  }
  bool containsPoint(const Point& point) const override {
    long double s = ::area(point, vertices.front(), vertices.back());
    for (int i = 0; i < static_cast<int>(vertices.size()) - 1; ++i) {
      s += ::area(point, vertices[i], vertices[i + 1]);
    }
    return area() + constants::squarePrecision >= s;
  }
  bool isCongruentTo(const Shape& another) const override {
    const Polygon* p = dynamic_cast<const Polygon*>(&another);
    if (p == nullptr) { return false; }
    if (verticesCount() != p->verticesCount()) { return false; }
    for (int start = 0; start < verticesCount(); ++start) {
      if (similarityChecker(p, true, start, 1) || similarityChecker(p, false, start, 1)) { return true; }
    }
    return false;
  }

  bool isSimilarTo(const Shape& another) const override {
    const Polygon* p = dynamic_cast<const Polygon*>(&another);
    if (p == nullptr) { return false; }
    if (verticesCount() != p->verticesCount()) { return false; }
    long double k = perimeter() / p->perimeter();
    for (int start = 0; start < verticesCount(); ++start) {
      if (similarityChecker(p, true, start, k) || similarityChecker(p, false, start, k)) { return true; }
    }
    return false;
  }
  bool operator==(const Shape& another) const override {
    const Polygon* p = dynamic_cast<const Polygon*>(&another);
    if (p == nullptr) { return false; }
    return (*this == *p);
  }
  bool operator!=(const Shape& another) const override { return !(*this == another); }
  bool operator==(const Polygon& another) const {
    if (verticesCount() != another.verticesCount()) { return false; }
    for (int start = 0; start < verticesCount(); ++start) {
      if (equalityChecker(another, true, start) || equalityChecker(another, false, start)) { return true; }
    }
    return false;
  }
  bool operator!=(const Polygon& another) const { return !(*this == another); }
};
class Rectangle: public Polygon {
 public:
  Rectangle(const Point& A, const Point& C, long double d)
    : Polygon({A,
               {(C.x - A.x) / (d * d + 1) - (C.y - A.y) * d / (d * d + 1) + A.x,
                (C.y - A.y) / (d * d + 1) + (C.x - A.x) * d / (d * d + 1) + A.y},
               C,
               {(C.x - A.x) * d * d / (d * d + 1) + (C.y - A.y) * d / (d * d + 1) + A.x,
                (C.y - A.y) * d * d / (d * d + 1) - (C.x - A.x) * d / (d * d + 1) + A.y}
              }) {}
  Point center() {
    Point A = vertices[0], C = vertices[2];
    return 0.5 * (A + C);
  }
  std::pair<Line, Line> diagonals() {
    Point A = vertices[0], B = vertices[1], C = vertices[2], D = vertices[3];
    return {{A, C}, {B, D}};
  }
  long double perimeter() const override {
    Point A = vertices[0], B = vertices[1], C = vertices[2];
    return 2 * (dist(A, B) + dist(B, C));
  }
  long double area() const override {
    Point A = vertices[0], B = vertices[1], C = vertices[2];
    return dist(A, B) * dist(B, C);
  }
};

class Square: public Rectangle {
 public:
  using Rectangle::Rectangle;
  using Rectangle::Polygon;
  Square(const Point& A, const Point& C): Rectangle(A, C, 1.0) {}
  long double area() const override {
    Point A = vertices[0], B = vertices[1];
    return dist(A, B) * dist(A, B);
  }
  long double perimeter() const override {
    Point A = vertices[0], B = vertices[1];
    return 4 * dist(A, B);
  }
  Circle inscribedCircle() {
    Point A = vertices[0], B = vertices[1];
    return {center(), dist(A, B) / 2};
  }
  Circle circumscribedCircle() {
    Point A = vertices[0], B = vertices[1];
    return {center(), dist(A, B) / sqrt(2)};
  }
};

class Triangle: public Polygon {
 public:
  using Polygon::Polygon;
  Point centroid() {
    Point A = vertices[0], B = vertices[1], C = vertices[2];
    return (1.0 / 3) * (A + B + C);
  }
  Point circumscribedCircleCenter() {
    Point A = vertices[0], B = vertices[1], C = vertices[2];
    long double tmp = 2 * (A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));
    Point ans;
    ans.x = A.mod() * (B.y - C.y) + B.mod() * (C.y - A.y) + C.mod() * (A.y - B.y);
    ans.x /= tmp;
    ans.y = A.mod() * (C.x - B.x) + B.mod() * (A.x - C.x) + C.mod() * (B.x - A.x);
    ans.y /= tmp;
    return ans;
  }
  Circle circumscribedCircle() {
    Point A = vertices[0];
    Point O = circumscribedCircleCenter();
    return Circle(O, dist(A, O));
  }
  Point orthocenter() {
    Point O = circumscribedCircleCenter();
    Point M = centroid();
    return M + 2 * (O - M);
  }
  Line EulerLine() {
    Point O = circumscribedCircleCenter();
    Point M = centroid();
    return {O, M};
  }
  Circle ninePointsCircle() {
    Point A = vertices[0];
    Point O = circumscribedCircleCenter();
    Point H = orthocenter();
    Point E = 0.5 * (O + H);
    return {E, 0.5 * dist(O, A)};
  }
  Point inscribedCircleCenter() {
    Point A = vertices[0], B = vertices[1], C = vertices[2];
    long double a = dist(B, C);
    long double b = dist(C, A);
    long double c = dist(A, B);
    return (1 / (a + b + c)) * (a * A + b * B + c * C);
  }
  Circle inscribedCircle() {
    Point A = vertices[0];
    Point I = inscribedCircleCenter();
    Point O = circumscribedCircleCenter();
    long double R = dist(O, A);
    long double OI = dist(O, I);
    return {I, (R * R - OI * OI) / (2 * R)};
  }
};
