/**
 * @file vector_template_functions.hpp
 * @brief Class for mathematical vector (template functions)
 */

#ifndef S2E_LIBRARY_MATH_VECTOR_TFS_HPP_
#define S2E_LIBRARY_MATH_VECTOR_TFS_HPP_

#include <cmath>

namespace libra {

template <size_t N, typename T>
Vector<N, T>::Vector(const T& n) {
  for (size_t i = 0; i < N; ++i) {
    vector_[i] = n;
  }
}

template <size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator+=(const Vector<N, T>& v) {
  for (size_t i = 0; i < N; ++i) {
    vector_[i] += v.vector_[i];
  }
  return *this;
}

template <size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator-=(const Vector<N, T>& v) {
  for (size_t i = 0; i < N; ++i) {
    vector_[i] -= v.vector_[i];
  }
  return *this;
}

template <size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator*=(const T& n) {
  for (size_t i = 0; i < N; ++i) {
    vector_[i] *= n;
  }
  return *this;
}

template <size_t N, typename T>
Vector<N, T>& Vector<N, T>::operator/=(const T& n) {
  return operator*=(1.0 / n);
}

template <size_t N, typename T>
Vector<N, T> Vector<N, T>::operator-() const {
  Vector<N, T> temp = *this;
  temp *= -1;
  return temp;
}

template <size_t N, typename T>
void FillUp(Vector<N, T>& v, const T& n) {
  for (size_t i = 0; i < N; ++i) {
    v[i] = n;
  }
}

template <size_t N, typename T>
void Print(const Vector<N, T>& v, char delimiter, std::ostream& stream) {
  stream << v[0];
  for (size_t i = 1; i < N; ++i) {
    stream << delimiter << v[i];
  }
}

template <size_t N, typename T>
const Vector<N, T> operator+(const Vector<N, T>& lhs, const Vector<N, T>& rhs) {
  Vector<N, T> temp;
  for (size_t i = 0; i < N; ++i) {
    temp[i] = lhs[i] + rhs[i];
  }
  return temp;
}

template <size_t N, typename T>
const Vector<N, T> operator-(const Vector<N, T>& lhs, const Vector<N, T>& rhs) {
  Vector<N, T> temp;
  for (size_t i = 0; i < N; ++i) {
    temp[i] = lhs[i] - rhs[i];
  }
  return temp;
}

template <size_t N, typename T>
const Vector<N, T> operator*(const T& lhs, const Vector<N, T>& rhs) {
  Vector<N, T> temp;
  for (size_t i = 0; i < N; ++i) {
    temp[i] = lhs * rhs[i];
  }
  return temp;
}

template <size_t N, typename T>
const T InnerProduct(const Vector<N, T>& lhs, const Vector<N, T>& rhs) {
  T temp = 0;
  for (size_t i = 0; i < N; ++i) {
    temp += lhs[i] * rhs[i];
  }
  return temp;
}

template <typename T>
const Vector<3, T> OuterProduct(const Vector<3, T>& lhs, const Vector<3, T>& rhs) {
  Vector<3, T> temp;
  temp[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
  temp[1] = lhs[2] * rhs[0] - lhs[0] * rhs[2];
  temp[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
  return temp;
}

template <size_t N>
double CalcNorm(const Vector<N, double>& v) {
  double temp = 0.0;
  for (size_t i = 0; i < N; ++i) {
    temp += pow(v[i], 2.0);
  }
  return sqrt(temp);
}

template <size_t N>
Vector<N, double>& Normalize(Vector<N, double>& v) {
  double n = CalcNorm(v);
  if (n == 0.0) {
    return v;
  }

  n = 1.0 / n;
  for (size_t i = 0; i < N; ++i) {
    v[i] *= n;
  }

  return v;
}

template <size_t N>
double CalcAngleTwoVectors_rad(const Vector<N, double>& v1, const Vector<N, double>& v2) {
  double cos = InnerProduct(v1, v2) / (CalcNorm(v1) * CalcNorm(v2));
  return acos(cos);
}

}  // namespace libra

#endif  // S2E_LIBRARY_MATH_VECTOR_TFS_HPP_
