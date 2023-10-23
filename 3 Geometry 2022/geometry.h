#pragma once

#include <math.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

const int kPig = 180;
const double kPid = 180.0;
const long long kConst = 1000000000000;
const int kTrvert = 3;
const double kEps = 0.000001;

struct Point {
  double x;  //  NOLINT
  double y;  //  NOLINT

  Point() = default;

  Point(double x, double y) : x(x), y(y) {}  //  NOLINT

  explicit Point(std::pair<double, double> pair)
          : x(pair.first), y(pair.second) {}  //  NOLINT

  ~Point() = default;
};

bool operator==(Point point1, Point point2) {
  return (point1.x == point2.x) && (point1.y == point2.y);
}

bool operator!=(Point point1, Point point2) { return !(point1 == point2); }

bool operator<(Point point1, Point point2) {
  if (point1.x == point2.x) {
    return point1.y < point2.y;
  }
  return point1.x < point2.x;
}

std::ostream& operator<<(std::ostream& ostream, const Point& point) {
  ostream << point.x << "," << point.y;
  return ostream;
}

class Line {
 public:
  Line() = default;

  Line(Point point1, Point point2)
          : coefficient_(point2.x != point1.x
                         ? (point2.y - point1.y) / (point2.x - point1.x)
                         : 0),
            shift_(point1.y - coefficient_ * point1.x),
            isVertical_(point2.x == point1.x),
            vertical_(point2.x == point1.x ? point1.x : 0) {}

  Line(double coefficient, double shift)
          : coefficient_(coefficient),
            shift_(shift),
            isVertical_(false),
            vertical_(0) {}

  Line(Point point, double coefficient)
          : coefficient_(coefficient),
            shift_(point.y - coefficient * point.x),
            isVertical_(false),
            vertical_(0) {}

  double getCoefficient() const { return coefficient_; }

  double getShift() const { return shift_; }

  bool isVertical() const { return isVertical_; }

  double getVerticalShift() const { return vertical_; }

  Point intersection(const Line& other) const;

  ~Line() = default;

 protected:
  double coefficient_;
  double shift_;
  bool isVertical_;
  double vertical_;
};

Point Line::intersection(const Line& other) const {
  if (coefficient_ == other.coefficient_) {
    if (shift_ == other.shift_) {
      throw("Lines match");
    }
    throw("No intersection");
  }
  if (isVertical()) {
    return Point(
            getVerticalShift(),
            getVerticalShift() * other.getCoefficient() + other.getShift());
  }
  if (other.isVertical()) {
    return Point(other.getVerticalShift(),
                 other.getVerticalShift() * getCoefficient() + getShift());
  }
  return Point((other.shift_ - shift_) / (coefficient_ - other.coefficient_),
               coefficient_ * (other.shift_ - shift_) /
               (coefficient_ - other.coefficient_) +
               shift_);
}

bool operator==(const Line& line1, const Line& line2) {
  return (line1.getCoefficient() == line2.getCoefficient()) &&
         (line1.getShift() == line2.getShift());
}

bool operator!=(const Line& line1, const Line& line2) {
  return !(line1 == line2);
}

double LineLength(Point first, Point second) {
  return sqrt((first.x - second.x) * (first.x - second.x) +
              (first.y - second.y) * (first.y - second.y));
}

double SquareTriangleGeron(double side1, double side2, double side3) {
  double perimeterpol = (side1 + side2 + side3) / 2;
  return sqrt(perimeterpol * (perimeterpol - side1) * (perimeterpol - side2) *
              (perimeterpol - side3));
}

void RotatePoint(Point center, double angle, Point& vertice) {
  vertice = Point(
          round(((vertice.x - center.x) * cos(angle / kPig * M_PI) -
                 (vertice.y - center.y) * sin(angle / kPig * M_PI) + center.x) *
                kConst) /
          kConst,
          round(((vertice.x - center.x) * sin(angle / kPig * M_PI) +
                 (vertice.y - center.y) * cos(angle / kPig * M_PI) + center.y) *
                kConst) /
          kConst);
}

void ReflectPoint(Point center, Point& vertice) {
  vertice = Point(2 * center.x - vertice.x, 2 * center.y - vertice.y);
}

void ReflectPoint(const Line& axis, Point& vertice) {
  if (axis.getCoefficient() == 0) {
    if (axis.isVertical()) {
      vertice.x = -vertice.x;
    } else {
      vertice.y = -vertice.y;
    }
  } else {
    double coeff = -1 / axis.getCoefficient();
    Line oxis(vertice, coeff);
    Point intersection_point = oxis.intersection(axis);
    ReflectPoint(intersection_point, vertice);
  }
}

void ScalePoint(Point center, double coefficient, Point& vertice) {
  double len = coefficient * LineLength(center, vertice);
  Line line(center, vertice);
  if (vertice < center) {
    len *= -1;
  }
  if (line.isVertical()) {
    vertice.y = center.y + len;
  } else {
    vertice.x = center.x +
                len / sqrt(1 + line.getCoefficient() * line.getCoefficient());
    vertice.y = vertice.x * line.getCoefficient() + line.getShift();
  }
}

class Shape {
 public:
  virtual double area() = 0;

  virtual double perimeter() = 0;

  virtual std::string showFigureType() = 0;

  virtual void rotate(Point center, double angle) = 0;

  virtual void reflect(Point center) = 0;

  virtual void reflect(const Line& axis) = 0;

  virtual void scale(Point center, double coefficient) = 0;

  virtual bool containsPoint(Point point) = 0;

  virtual bool isSimilarTo(const Shape& another) = 0;

  virtual bool isCongruentTo(const Shape& another) = 0;

  virtual ~Shape() = default;
};

bool LeftFromLine(Point point1, Point point2, Point point3) {
  bool left_from_line;
  Line line(point1, point3);
  if (line.isVertical()) {
    left_from_line = point2.x <= line.getVerticalShift();
    if (point3.y < point1.y) {
      left_from_line = !left_from_line;
    }
  } else {
    left_from_line =
            point2.y >= (line.getCoefficient() * point2.x + line.getShift());
    if (line.getCoefficient() < 0 && line.getCoefficient() != 0) {
      left_from_line = !left_from_line;
    }
    if ((point3.y < point1.y && line.getCoefficient() != 0) ||
        (line.getCoefficient() == 0 && point3.x < point1.x)) {
      left_from_line = !left_from_line;
    }
  }
  return left_from_line;
}

class Polygon : public Shape {
 public:
  Polygon() = default;

  template <typename... Points>
  Polygon(const Point& point, const Points&... tail);

  explicit Polygon(const std::vector<Point>& vertices) : vertices_(vertices) {}

  size_t verticesCount() const { return vertices_.size(); }

  bool isConvex();

  double area() override;

  double perimeter() override;

  std::string showFigureType() override { return typeid(*this).name(); }

  std::vector<Point> getVertices() const { return vertices_; }

  void rotate(Point center, double angle) override;

  void reflect(Point center) override;

  void reflect(const Line& axis) override;

  void scale(Point center, double coefficient) override;

  bool containsPoint(Point point) override;

  bool isSimilarTo(const Shape& another) override;

  bool isCongruentTo(const Shape& another) override;

  ~Polygon() = default;

  bool isSame(const Polygon& polygon);

 protected:
  std::vector<Point> vertices_;
  int convex_ = 0;
};

template <typename... Points>
Polygon::Polygon(const Point& point, const Points&... tail) : Polygon(tail...) {
  static_assert(std::is_same_v<Point, Point>);
  vertices_.push_back(point);
}

bool Polygon::isConvex() {
  if (convex_ != 0) {
    return convex_ == 1;
  }
  if (vertices_.size() == kTrvert) {
    convex_ = 1;
    return true;
  }
  bool left_from_line_last;
  for (size_t i = 0; i < vertices_.size(); ++i) {
    Point point1 = vertices_[i];
    Point point2 = vertices_[(i + 1) % vertices_.size()];
    Point point3 = vertices_[(i + 2) % vertices_.size()];
    bool left_from_line = LeftFromLine(point1, point2, point3);
    if (i != 0) {
      if (left_from_line != left_from_line_last) {
        convex_ = -1;
        return false;
      }
    }
    left_from_line_last = left_from_line;
  }
  convex_ = 1;
  return true;
}

double Polygon::area() {
  if (vertices_.size() == kTrvert) {
    return SquareTriangleGeron(LineLength(vertices_[0], vertices_[1]),
                               LineLength(vertices_[1], vertices_[2]),
                               LineLength(vertices_[2], vertices_[0]));
  }

  double ans = 0;
  int sign;
  bool left_from_line_first;
  for (size_t i = 0; i < vertices_.size() - 2; ++i) {
    sign = 1;
    Point point1 = vertices_[0];
    Point point2 = vertices_[i + 1];
    Point point3 = vertices_[i + 2];
    bool left_from_line = LeftFromLine(point1, point2, point3);
    if (i != 0) {
      if (left_from_line != left_from_line_first) {
        sign = -1;
      }
    } else {
      left_from_line_first = left_from_line;
    }
    ans += sign * SquareTriangleGeron(LineLength(point1, point2),
                                      LineLength(point2, point3),
                                      LineLength(point3, point1));
  }
  return abs(ans);
}

double Polygon::perimeter() {
  double ans = 0;
  for (size_t i = 0; i < vertices_.size(); ++i) {
    Point point1 = vertices_[i];
    Point point2 = vertices_[(i + 1) % vertices_.size()];
    ans += LineLength(point1, point2);
  }
  return ans;
}

void Polygon::rotate(Point center, double angle) {
  for (auto& vertice : vertices_) {
    RotatePoint(center, angle, vertice);
  }
}

void Polygon::reflect(Point center) {
  for (auto& vertice : vertices_) {
    ReflectPoint(center, vertice);
  }
}

void Polygon::reflect(const Line& axis) {
  for (auto& vertice : vertices_) {
    ReflectPoint(axis, vertice);
  }
}

void Polygon::scale(Point center, double coefficient) {
  for (auto& vertice : vertices_) {
    ScalePoint(center, coefficient, vertice);
  }
}

bool Polygon::containsPoint(Point point) {
  bool last = LeftFromLine(vertices_[0], point, vertices_[1]);
  for (size_t i = 1; i < vertices_.size(); ++i) {
    bool left_from_line = LeftFromLine(vertices_[i], point,
                                       vertices_[(i + 1) % vertices_.size()]);
    if (left_from_line != last) {
      return false;
    }
    last = left_from_line;
  }
  return true;
}

bool Polygon::isSame(const Polygon& polygon) {
  std::vector<Point> arr1 = vertices_;
  std::vector<Point> arr2 = polygon.vertices_;
  if (arr1.size() != arr2.size()) {
    return false;
  }
  std::sort(arr1.begin(), arr1.end());
  std::sort(arr2.begin(), arr2.end());
  for (size_t i = 0; i < arr1.size(); ++i) {
    if (arr1[i] != arr2[i]) {
      return false;
    }
  }
  return true;
}

class Ellipse : public Shape {
 public:
  Ellipse() = default;

  Ellipse(Point focus1, Point focus2, double sum)
          : focuses_(std::make_pair(focus1, focus2)),
            big_semi_axis_(sum / 2),
            focal_length_(LineLength(focus1, focus2) / 2),
            eccentricity_(focal_length_ / big_semi_axis_),
            small_semi_axis_(big_semi_axis_ *
                             sqrt(1 - eccentricity_ * eccentricity_)),
            directrices_(
                    std::make_pair(Line(Point(big_semi_axis_ / eccentricity_, -1),
                                        Point(big_semi_axis_ / eccentricity_, +1)),
                                   Line(Point(-big_semi_axis_ / eccentricity_, -1),
                                        Point(-big_semi_axis_ / eccentricity_, +1)))),
            center_(
                    Point(focus1.x / 2 + focus2.x / 2, focus1.y / 2 + focus2.y / 2)) {}

  std::pair<Point, Point> focuses() const { return focuses_; }

  std::pair<Line, Line> directrices() const { return directrices_; }

  double eccentricity() const { return eccentricity_; }

  Point center() const { return center_; }

  double area() override;

  double perimeter() override;

  std::string showFigureType() override { return typeid(*this).name(); }

  void rotate(Point center, double angle) override;

  void reflect(Point center) override;

  void reflect(const Line& axis) override;

  void scale(Point center, double coefficient) override;

  bool containsPoint(Point point) override;

  bool isSimilarTo(const Shape& another) override;

  bool isCongruentTo(const Shape& another) override;

  bool isSame(const Ellipse& other) const;

  ~Ellipse() = default;

 protected:
  std::pair<Point, Point> focuses_;
  double big_semi_axis_;
  double focal_length_;
  double eccentricity_;
  double small_semi_axis_;
  std::pair<Line, Line> directrices_;
  Point center_;
};

double Ellipse::area() { return M_PI * big_semi_axis_ * small_semi_axis_; }

double Ellipse::perimeter() {
  return M_PI * (kTrvert * (big_semi_axis_ + small_semi_axis_) -
                 sqrt((kTrvert * big_semi_axis_ + small_semi_axis_) *
                      (kTrvert * small_semi_axis_ + big_semi_axis_)));
}

void Ellipse::rotate(Point center, double angle) {
  RotatePoint(center, angle, focuses_.first);
  RotatePoint(center, angle, focuses_.second);
  *this = Ellipse(focuses_.first, focuses_.second, 2 * big_semi_axis_);
}

void Ellipse::reflect(Point center) {
  ReflectPoint(center, focuses_.first);
  ReflectPoint(center, focuses_.second);
  *this = Ellipse(focuses_.first, focuses_.second, 2 * big_semi_axis_);
}

void Ellipse::reflect(const Line& axis) {
  ReflectPoint(axis, focuses_.first);
  ReflectPoint(axis, focuses_.second);
  *this = Ellipse(focuses_.first, focuses_.second, 2 * big_semi_axis_);
}

void Ellipse::scale(Point center, double coefficient) {
  ScalePoint(center, coefficient, focuses_.first);
  ScalePoint(center, coefficient, focuses_.second);
  *this = Ellipse(focuses_.first, focuses_.second, 2 * big_semi_axis_);
}

bool Ellipse::containsPoint(Point point) {
  return (((point.x - center_.x) / big_semi_axis_) *
          ((point.x - center_.x) / big_semi_axis_) +
          ((point.y - center_.y) / small_semi_axis_) *
          ((point.y - center_.y) / small_semi_axis_)) < 1;
}

bool Ellipse::isSame(const Ellipse& other) const {
  return (focuses_.first == other.focuses_.first) &&
         (focuses_.second == other.focuses_.second);
}

class Circle : public Ellipse {
 public:
  Circle() = default;

  Circle(Point center, double radius) : Ellipse(center, center, 2 * radius){};

  double radius() const { return big_semi_axis_; }

  ~Circle() = default;
};

class Rectangle : public Polygon {
 public:
  Rectangle() = default;

  Rectangle(Point point1, Point point2, double coef);

  Point center() { return center_; }

  std::pair<Line, Line> diagonals() { return diagonals_; }

  ~Rectangle() = default;

 protected:
  Point center_;
  std::pair<Line, Line> diagonals_;
  double len_;  // between diagonals1
};

Rectangle::Rectangle(Point point1, Point point2, double coef) {
  Point point3 = point1;
  Point point4 = point2;
  double alpha = atan(1 / coef);
  double gamma = acos(1 - 2 * sin(alpha) * sin(alpha)) * kPig / M_PI;
  center_ = {point1.x / 2 + point2.x / 2, point1.y / 2 + point2.y / 2};
  RotatePoint(center_, -gamma, point3);
  RotatePoint(center_, -gamma, point4);
  vertices_.push_back(point1);
  vertices_.push_back(point4);
  vertices_.push_back(point2);
  vertices_.push_back(point3);
  diagonals_ = std::make_pair(Line(point1, point2), Line(point3, point4));
  len_ = LineLength(point1, point2);
}

class Square : public Rectangle {
 public:
  Square() = default;

  Square(Point point1, Point point2) : Rectangle(point1, point2, 1) {}

  Circle circumscribedCircle() { return Circle(center_, len_ / 2); }

  Circle inscribedCircle() {
    return Circle(center_, len_ * sin(kPid / (2 * 2) / kPid * M_PI));
  }

  ~Square() = default;
};

class Triangle : public Polygon {
 public:
  Triangle() = default;

  Triangle(Point point1, Point point2, Point point3);

  Circle circumscribedCircle();

  Circle inscribedCircle();

  Point centroid() { return centroid_; }

  Point orthocenter() { return orthocenter_; }

  Line EulerLine();  //  NOLINT

  Circle ninePointsCircle();

  ~Triangle() = default;

 private:
  Point centroid_;
  Point orthocenter_;
};

std::vector<Point> ReverseVertices(Point point1, Point point2, Point point3) {
  std::vector<Point> vertices = {point1, point2, point3};
  std::sort(vertices.begin(), vertices.end());
  std::reverse(vertices.begin(), vertices.end());
  return vertices;
}

Triangle::Triangle(Point point1, Point point2, Point point3) {
  vertices_ = ReverseVertices(point1, point2, point3);
  Point mid1(vertices_[0].x / 2 + vertices_[1].x / 2,
             vertices_[0].y / 2 + vertices_[1].y / 2);
  Point mid2(vertices_[1].x / 2 + vertices_[2].x / 2,
             vertices_[1].y / 2 + vertices_[2].y / 2);
  Line midl1(mid1, vertices_[2]);
  Line midl2(mid2, vertices_[0]);
  centroid_ = midl1.intersection(midl2);
  Line side1(vertices_[0], vertices_[1]);
  Line height1 = {vertices_[2], 0};
  if (side1.isVertical() or side1.getCoefficient() == 0) {
    if (!side1.isVertical()) {
      Point tmp(vertices_[2].x, vertices_[2].y - 1);
      height1 = {vertices_[2], tmp};
    }
  } else {
    height1 = {vertices_[2], -1 / side1.getCoefficient()};
  }
  Line side2(vertices_[1], vertices_[2]);
  Line height2 = {vertices_[0], 0};
  if (side2.isVertical() or side2.getCoefficient() == 0) {
    if (!side2.isVertical()) {
      Point tmp(vertices_[0].x, vertices_[0].y - 1);
      height2 = {vertices_[0], tmp};
    }
  } else {
    height2 = {vertices_[0], -1 / side2.getCoefficient()};
  }
  orthocenter_ = height1.intersection(height2);
}

Circle Triangle::circumscribedCircle() {
  Point mid1(vertices_[0].x / 2 + vertices_[1].x / 2,
             vertices_[0].y / 2 + vertices_[1].y / 2);
  Line side1(vertices_[0], vertices_[1]);
  Line midl1 = {mid1, 0};
  if (side1.isVertical() or side1.getCoefficient() == 0) {
    if (!side1.isVertical()) {
      Point tmp(mid1.x, mid1.y - 1);
      midl1 = {mid1, tmp};
    }
  } else {
    midl1 = {mid1, -1 / side1.getCoefficient()};
  }
  Point mid2(vertices_[1].x / 2 + vertices_[2].x / 2,
             vertices_[1].y / 2 + vertices_[2].y / 2);
  Line side2(vertices_[1], vertices_[2]);
  Line midl2 = {mid2, 0};
  if (side2.isVertical() or side2.getCoefficient() == 0) {
    if (!side2.isVertical()) {
      Point tmp(mid2.x, mid2.y - 1);
      midl2 = {mid2, tmp};
    }
  } else {
    midl2 = {mid2, -1 / side2.getCoefficient()};
  }
  return Circle(midl1.intersection(midl2),
                LineLength(midl1.intersection(midl2), vertices_[0]));
}

Circle Triangle::inscribedCircle() {
  double len_side1 = LineLength(vertices_[0], vertices_[1]);
  double len_side2 = LineLength(vertices_[1], vertices_[2]);
  double len_side3 = LineLength(vertices_[2], vertices_[0]);
  double angle12 = acos((len_side1 * len_side1 + len_side2 * len_side2 -
                         len_side3 * len_side3) /
                        (2 * len_side1 * len_side2)) *
                   kPig / 2 / M_PI;
  double angle23 = acos((len_side3 * len_side3 + len_side2 * len_side2 -
                         len_side1 * len_side1) /
                        (2 * len_side3 * len_side2)) *
                   kPig / 2 / M_PI;
  Point point1 = vertices_[0];
  RotatePoint(vertices_[1], angle12, point1);
  Point point2 = vertices_[1];
  RotatePoint(vertices_[2], angle23, point2);
  Line bis1(vertices_[1], point1);
  Line bis2(vertices_[2], point2);
  return Circle(bis1.intersection(bis2), area() / perimeter() * 2);
}

Circle Triangle::ninePointsCircle() {
  Point mid1(vertices_[0].x / 2 + vertices_[1].x / 2,
             vertices_[0].y / 2 + vertices_[1].y / 2);
  Point mid2(vertices_[1].x / 2 + vertices_[2].x / 2,
             vertices_[1].y / 2 + vertices_[2].y / 2);
  Point mid3(vertices_[0].x / 2 + vertices_[2].x / 2,
             vertices_[0].y / 2 + vertices_[2].y / 2);
  Triangle mid_tr(mid1, mid2, mid3);
  return mid_tr.circumscribedCircle();
}

Line Triangle::EulerLine() {
  return Line(ninePointsCircle().center(), centroid());
}  //  NOLINT
bool Polygon::isSimilarTo(const Shape& another) {
  if ((typeid(another).name() == typeid(Ellipse).name()) ||
      (typeid(another).name() == typeid(Circle).name())) {
    return false;
  }
  Polygon* other = dynamic_cast<Polygon*>(const_cast<Shape*>(&another));
  double coeff = other->perimeter() / perimeter();
  return (other->verticesCount() == verticesCount()) &&
         (abs(other->area() / area() - coeff * coeff) < kEps);
}

bool Polygon::isCongruentTo(const Shape& another) {
  if ((typeid(another).name() == typeid(Ellipse).name()) ||
      (typeid(another).name() == typeid(Circle).name())) {
    return false;
  }
  Polygon* other = dynamic_cast<Polygon*>(const_cast<Shape*>(&another));
  return (other->verticesCount() == verticesCount()) &&
         (abs(other->area() - area()) < kEps) &&
         (abs(other->perimeter() - perimeter()) < kEps);
}

bool Ellipse::isSimilarTo(const Shape& another) {
  if ((typeid(another).name() != typeid(Ellipse).name()) ||
      (typeid(another).name() != typeid(Circle).name())) {
    return false;
  }
  Ellipse* other = dynamic_cast<Ellipse*>(const_cast<Shape*>(&another));

  return big_semi_axis_ * other->small_semi_axis_ ==
         small_semi_axis_ * other->big_semi_axis_;
}

bool Ellipse::isCongruentTo(const Shape& another) {
  if ((typeid(another).name() != typeid(Ellipse).name()) ||
      (typeid(another).name() != typeid(Circle).name())) {
    return false;
  }
  Ellipse* other = dynamic_cast<Ellipse*>(const_cast<Shape*>(&another));
  if (other->showFigureType() != showFigureType()) {
    return false;
  }
  return (big_semi_axis_ == other->big_semi_axis_) &&
         (small_semi_axis_ == other->small_semi_axis_);
}

bool operator==(const Shape& figure1, const Shape& figure2) {
  if ((typeid(figure1).name() == typeid(Ellipse).name() ||
       typeid(figure1).name() == typeid(Circle).name()) &&
      (typeid(figure2).name() == typeid(Ellipse).name() ||
       typeid(figure2).name() == typeid(Circle).name())) {
    Ellipse* el1 = dynamic_cast<Ellipse*>(const_cast<Shape*>(&figure1));
    Ellipse* el2 = dynamic_cast<Ellipse*>(const_cast<Shape*>(&figure2));
    return el1->isSame(*el2);
  }
  if ((typeid(figure1).name() == typeid(Rectangle).name() ||
       typeid(figure1).name() == typeid(Square).name() ||
       typeid(figure1).name() == typeid(Triangle).name() ||
       typeid(figure1).name() == typeid(Polygon).name()) &&
      (typeid(figure2).name() == typeid(Rectangle).name() ||
       typeid(figure2).name() == typeid(Square).name() ||
       typeid(figure2).name() == typeid(Triangle).name() ||
       typeid(figure2).name() == typeid(Polygon).name())) {
    Polygon* pol1 = dynamic_cast<Polygon*>(const_cast<Shape*>(&figure1));
    Polygon* pol2 = dynamic_cast<Polygon*>(const_cast<Shape*>(&figure2));
    return pol1->isSame(*pol2);
  }
  return false;
}