#pragma once

#include <ranges>
#include <iterator>
#include <iostream>
#include <string>
#include <vector>
#include <optional>

#include "vector.hpp"

int int_div_round(int numerator, int denominator) {
  // perform rounding with integer division by calculating floor(numerator/denominator + 1/2)
  return (2 * numerator + denominator) / (2 * denominator);
}


int int_div_ceil(int numerator, int denominator) {
  // perform celing integer division by calculating the division as follows
  // a/b = n + r/b. (We want to round up whenever r > 0) 
  // --> we must add b-1 to the numerator
  // thus arriving at floor(numerator+denominator-1 / denominator)
  // The added branch is necessary to correctly round up in case both numerator and denominator are negative in 
  // which case we need floor(numerator+denominator+1 / denominator)
  return (numerator + denominator + (denominator > 0 ? -1 : 1)) / denominator;
}


template<typename Element>
struct FieldT {
  FieldT(int width, int height, Element fill) : size(width, height) {
    data.resize(width * height, fill);
  }

  FieldT(std::istream&& source) : FieldT(source) {}
  FieldT(std::istream& source) : size(0, 0) {
    for (std::string line; std::getline(source, line);) {
      if (line.empty()) { // special case for Day 15 where the field is followed by a newline and instructions
        break;
      }
      for (char ch : line) {
        data.emplace_back(ch); // Element must be constructible from a single char
      }
      size.x = static_cast<int>(line.length());
      ++size.y;
    }
  }

  template<typename Self>
  auto& operator[](this Self&& self, const Vector& pos) { return self.data[self.toOffset(pos)]; }
  bool validPosition(const Vector& pos) const { return pos.x >= 0 && pos.y >= 0 && pos.x < size.x && pos.y < size.y; }
  bool isAt(const Element& element, const Vector& pos) const { return validPosition(pos) && (*this)[pos] == element; }
  /** checked field access, which returns a copy to the field value if the position is valid */
  std::optional<Element> at(const Vector& pos) const { return validPosition(pos) ? std::optional<Element>(data[toOffset(pos)]) : std::nullopt; }
  Element at(const Vector& pos, Element defaultValue) const { return validPosition(pos) ? data[toOffset(pos)] : defaultValue; }

  int toOffset(const Vector& pos) const { return pos.y * size.x + pos.x; }
  Vector fromOffset(size_t offset) const { return Vector(static_cast<int>(offset) % size.x, static_cast<int>(offset) / size.x); }
  size_t findOffset(const Element& element, size_t startOffset = 0) const {
    auto pos = std::find(data.begin() + startOffset, data.end(), element);
    return pos != data.end() ? std::distance(data.begin(), pos) : std::numeric_limits<size_t>::max();
  }
  
  Vector topLeft() const { return Vector(0, 0); }
  Vector topRight() const { return Vector(size.x - 1, 0); }
  Vector bottomLeft() const { return Vector(0, size.y - 1); }
  Vector bottomRight() const { return size - Vector(1, 1); }


  struct iterator {
    using element_type = Element;
    using reference = Element&;
    using pointer = Element*;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = int;

    iterator() : field(nullptr) {}
    iterator(FieldT& field, const Vector& pos, const Vector& direction = Vector(0, 0)) : field(&field), pos(pos), direction(direction) {}

    // Sentinel type, which can alternatively be used in place of an end() iterator
    struct sentinel {};

    bool operator==(const iterator& other) const { return pos == other.pos; }
    bool operator!=(const iterator& other) const { return pos != other.pos; }

    // We must use negate the condition due to the partial ordering of vectors
    bool operator==(sentinel) const { return !valid(); }
    bool valid() const { return field->validPosition(pos); }

    iterator& operator++() { pos += direction; return *this; }
    iterator operator++(int) { auto copy = *this; ++(*this); return copy; }
    iterator& operator-() { pos -= direction; return *this; }
    iterator operator--(int) { auto copy = *this; --(*this); return copy; }
    Element& operator*() const { return (*field)[pos]; }

    iterator& operator+=(int offset) { pos += direction * offset; return *this; }
    iterator& operator-=(int offset) { pos -= direction * offset; return *this; }
    iterator operator+(int offset) const { auto copy = *this; copy += offset; return copy; }
    iterator operator-(int offset) const { auto copy = *this; copy -= offset; return copy; }
    
    // Determine the difference between two positions
    // Assumption: other iterator will pass through this position otherwise behavior is undefined
    difference_type operator-(const iterator& other) const { 
      auto delta = this->pos - other.pos;
      return other.direction.x != 0 ? delta.x / other.direction.x : delta.y / other.direction.y;
    }

    Element& operator[](int index) const { return (*field)[pos + (direction * index)]; }

    Vector pos, direction;
    FieldT* field;
  };

  auto rangeFromPositionAndDirection(const Vector& position, const Vector& direction) {
    iterator begin(*this, position, direction);
    if (!validPosition(position)) {
      // starting at invalid position -> return an empty range
      return std::ranges::subrange(begin, begin);
    }

    int dx = std::max(direction.x > 0 ? int_div_ceil(size.x - position.x, direction.x) :
                      direction.x < 0 ? int_div_ceil(-1 - position.x, direction.x) : std::numeric_limits<int>::max(), 0);

    int dy = std::max(direction.y > 0 ? int_div_ceil(size.y - position.y, direction.y) :
                      direction.y < 0 ? int_div_ceil(-1 - position.y, direction.y) : std::numeric_limits<int>::max(), 0);

    auto distanceToEnd = std::min(dx, dy);
    return std::ranges::subrange(begin, begin + distanceToEnd);
  }

  // Return a range representing the i'th row (0-based) left to right
  auto row(int row) {
    iterator begin(*this, Vector(0, row), Vector::Right);
    return std::ranges::subrange(begin, begin + size.x);
  }

  // Returns a range representing the i'th column (0-based) top to bottom
  auto column(int column) {
    iterator begin(*this, Vector(column, 0), Vector::Down);
    return std::ranges::subrange(begin, begin + size.y);
  }

  struct rows_columns_iterator {
    using element_type = std::ranges::subrange<iterator, iterator>;
    using reference = element_type&;
    using pointer = element_type*;
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = int;
    using self = rows_columns_iterator;
    using subrange_method = element_type (FieldT::*)(int idx);

    rows_columns_iterator() : field(nullptr), idx(0), method(nullptr) {}
    rows_columns_iterator(FieldT* field, int idx, subrange_method method) : field(field), idx(idx), method(method) {}

    bool operator==(const self& other) const { return idx == other.idx; } // assumption: no two iterators from different directions will be compared
    bool operator!=(const self& other) const { return idx != other.idx; }

    self& operator++() { ++idx; return *this; }
    self operator++(int) { auto copy = *this; ++(*this); return copy; }
    self& operator-() { --idx; return *this; }
    self operator--(int) { auto copy = *this; --(*this); return copy; }
    element_type operator*() const { return (field->*method)(idx); }

    self& operator+=(int offset) { idx += offset; return *this; }
    self& operator-=(int offset) { idx -= offset; return *this; }
    self operator+(int offset) const { auto copy = *this; copy += offset; return copy; }
    self operator-(int offset) const { auto copy = *this; copy -= offset; return copy; }

    difference_type operator-(const iterator& other) const { return idx - other.idx; }
    element_type operator[](int index) const { return (field->*method)(idx + index); }

    FieldT* field;
    int idx;
    subrange_method method;
  };

  /** Returns an iterator over all rows of this field top to bottom (each row being returned as a range) */
  auto rows() {
    return std::ranges::subrange<rows_columns_iterator, rows_columns_iterator> {
      rows_columns_iterator(this, 0, &FieldT::row),
      rows_columns_iterator(this, size.y, &FieldT::row),
    };
  }

  /** Returns an iterator over all columns of this field left to right (each row being returned as a range) */
  auto columns() {
    return std::ranges::subrange<rows_columns_iterator, rows_columns_iterator> {
      rows_columns_iterator(this, 0, &FieldT::column),
      rows_columns_iterator(this, size.x, &FieldT::column),
    };
  }

  Vector size;
  std::vector<Element> data;
};

using Field = FieldT<char>;


template<typename Element>
std::ostream& operator<<(std::ostream& out, FieldT<Element>& field) {
  for (int y = 0; y < field.size.y; ++y) {
    for (int x = 0; x < field.size.x; ++x) {
      out << field[Vector(x, y)];
    }
    out << "\n";
  }
  return out;
}