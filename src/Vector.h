#ifndef __VECTOR
#define __VECTOR

#include <iostream>
#include <iosfwd>
#include <cmath>

template <class T> class Vector3 {
public:
	Vector3() {set(T(), T(), T());}
	Vector3(T new_x, T new_y, T new_z) {set(new_x, new_y, new_z);}
	explicit Vector3(T value) {set(value, value, value);}

	void set(T new_x, T new_y, T new_z) {
		x() = new_x;
		y() = new_y;
		z() = new_z;
	}

	T& x(){return _c[0];}
	T& y(){return _c[1];}
	T& z(){return _c[2];}
	T x() const{return _c[0];}
	T y() const{return _c[1];}
	T z() const{return _c[2];}

	void operator=(T value) {set(value, value, value);}
	void operator+=(const Vector3& other) {set(x() + other.x(), y() + other.y(), z() + other.z());}
	void operator-=(const Vector3& other) {set(x() - other.x(), y() - other.y(), z() - other.z());}
	void operator*=(T factor) {set(x() * factor, y() * factor, z() * factor);}
	void operator/=(T factor) {set(x() / factor, y() / factor, z() / factor);}
	T lengthSquared() const {return x()*x() + y()*y() + z()*z();}
	T length() const {return std::sqrt(lengthSquared());}
	void invert() {x() = -x();y() = -y();z() = -z();}
	operator const T*() const {return _c;}

	inline void normalize();
	inline Vector3<T> normalized();

			template <class U>
	Vector3<U> convert() const {return Vector3<U>(static_cast<U>(x()), static_cast<U>(y()), static_cast<U>(z()));}

private:
	T _c[3];
};

template <class T> inline const Vector3<T> operator+(const Vector3<T>& one, const Vector3<T>& two) {
	return Vector3<T>(one.x() + two.x(), one.y() + two.y(), one.z() + two.z());
}

template <class T> inline const Vector3<T> operator-(const Vector3<T>& one, const Vector3<T>& two) {
	return Vector3<T>(one.x() - two.x(), one.y() - two.y(), one.z() - two.z());
}

template <class T> inline const Vector3<T> operator-(const Vector3<T>& one) {
	return Vector3<T>(-one.x(), -one.y(), -one.z());
}

template <class T> inline const Vector3<T> operator*(const Vector3<T>& one, T factor) {
	return Vector3<T>(one.x() * factor, one.y() * factor, one.z() * factor);
}

template <class T> inline const Vector3<T> operator*(T factor, const Vector3<T>& one) {
	return Vector3<T>(one.x() * factor, one.y() * factor, one.z() * factor);
}

template <class T> inline const Vector3<T> operator/(const Vector3<T>& one, T factor) {
	return Vector3<T>(one.x() / factor, one.y() / factor, one.z() / factor);
}

template <class T> inline T operator*(const Vector3<T>& one, const Vector3<T>& two) {
	return one.x()*two.x() + one.y()*two.y() + one.z()*two.z();
}

template <class T> inline Vector3<T> operator%(const Vector3<T>& one, const Vector3<T> two) {
	return Vector3<T>(one.y()*two.z() - one.z()*two.y(), one.z()*two.x() - one.x()*two.z(), one.x()*two.y() - one.y()*two.x());
}

template <class T> inline void Vector3<T>::normalize() {
	T len = length();
	x()/=len;
	y()/=len;
	z()/=len;
}

template <class T> inline Vector3<T> Vector3<T>::normalized() {
	T len = length();
	x()/=len;
	y()/=len;
	z()/=len;
	return (*this);
}

template <class T> inline bool operator==(const Vector3<T>& one, const Vector3<T>& two) {
	return one.x() == two.x() and one.y() == two.y() and one.z() == two.z();
}

template <class T> inline bool operator!=(const Vector3<T>& one, const Vector3<T>& two) {
	return !(one == two);
}

template<class T> std::ostream& operator<<(std::ostream& os, const Vector3<T>& vec) {
	os << '(' << vec.x() << ',' << ' ' << vec.y() << ',' << ' ' << vec.z() << ')';
	return os;
}

typedef Vector3<double> Vector3d;
typedef Vector3<float> Vector3f;
typedef Vector3<long> Vector3i;
typedef Vector3<size_t> Vector3u;

#endif
