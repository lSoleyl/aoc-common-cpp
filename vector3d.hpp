#pragma once

// Vector class for 3D vectors


#include <xhash>
#include <iostream>
#include <algorithm> // std::clamp


namespace {
  template<typename T>
  T pMod(T a, T b) {
    auto result = a % b;
    return result < 0 ? result + b : result;
  }
}


template<typename T>
struct VectorT3D {
  VectorT3D(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}

  static const VectorT3D Zero;


  VectorT3D operator+(const VectorT3D& other) const { return VectorT3D(x + other.x, y + other.y, z + other.z); }
  VectorT3D& operator+=(const VectorT3D& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  VectorT3D operator-(const VectorT3D& other) const { return VectorT3D(x - other.x, y - other.y, z - other.z); }
  VectorT3D& operator-=(const VectorT3D& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  // Important: the following operation performs integer division!
  VectorT3D operator/(T divisor) const { return VectorT3D(x / divisor, y / divisor, z / divisor); }
  VectorT3D& operator/(T divisor) {
    x /= divisor;
    y /= divisor;
    z /= divisor;
    return *this;
  }
  VectorT3D operator*(T factor) const { return VectorT3D(x * factor, y * factor, z * factor); }
  VectorT3D& operator*=(T factor) {
    x *= factor;
    y *= factor;
    z *= factor;
    return *this;
  }

  VectorT3D operator%(T divisor) const { return VectorT3D(x % divisor, y % divisor, z % divisor); }
  VectorT3D& operator%=(T divisor) {
    x %= divisor;
    y %= divisor;
    z %= divisor;
    return *this;
  }

  // element wise modulo
  VectorT3D operator%(const VectorT3D& other) const { return VectorT3D(x % other.x, y % other.y, z % other.z); }
  VectorT3D& operator%=(const VectorT3D& other) {
    x %= other.x;
    y %= other.y;
    z %= other.z;
    return *this;
  }

  // positive modulo
  VectorT3D pMod(T divisor) const { return VectorT3D(::pMod(x, divisor), ::pMod(y, divisor), ::pMod(z, divisor)); }

  // element wise positive modulo
  VectorT3D pMod(const VectorT3D& other) const { return VectorT3D(::pMod(x, other.x), ::pMod(y, other.y), ::pMod(z, other.z)); }


  // Calculate the number of single steps needed to reach other from this vector
  auto stepDistance(const VectorT3D& other) const {
    return std::abs(x - other.x) + std::abs(y - other.y) + std::abs(z - other.z);
  }

  // Apply given functor to each component and return the result
  template<typename MapFn>
  VectorT3D apply(MapFn mapFn) const { return VectorT3D(mapFn(x), mapFn(y), mapFn(z)); }

  // Performs a component wise compare and returns a result vector with -1,0,1 for each component
  VectorT3D compare(const VectorT3D& other) const { return (*this - other).apply([](int value) { return std::clamp(value, -1, 1); }); }

  bool operator==(const VectorT3D& other) const = default;
  bool operator!=(const VectorT3D& other) const = default;

  // This ordering is only defined to enable sorting vectors and using them in std::set and std::map and has no further semantic meaning
  std::strong_ordering operator<=>(const VectorT3D& other) const {
    auto result = (z <=> other.z);
    if (result == std::strong_ordering::equal) {
      result = (y <=> other.y);
    }
    if (result == std::strong_ordering::equal) {
      result = (x <=> other.x);
    }
    return result;
  }

  T x, y, z;
};

template<typename T>
VectorT3D<T> operator*(T factor, const VectorT3D<T>& vector) {
  return vector * factor;
}

template<typename T> const VectorT3D<T> VectorT3D<T>::Zero(0, 0);

namespace std {
  template<typename T>
  struct hash<VectorT3D<T>> {
    size_t operator()(const VectorT3D<T>& vec) const { return ((vec.z << 16) ^ (vec.y << 8) ^ vec.x); }
  };
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const VectorT3D<T>& v) {
  return out << '(' << v.x << ", " << v.y << ',' << v.z << ')';
}

using Vector3D = VectorT3D<int>;

