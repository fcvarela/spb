#ifndef __QUATERNION
#define __QUATERNION

#include <string.h>
#include <Vector.h>
#include <GL/glfw.h>

template <class T> class Quaternion {
public:
	Quaternion(void);
	Quaternion(T wi, T xi, T yi, T zi);
	Quaternion(T w, const Vector3<T>& v);
	Quaternion(const Quaternion<T>& other);
	Quaternion(const Vector3<T>& vec, T angle);
	~Quaternion();

			// quaternion operators
	Quaternion<T> operator = (const Quaternion<T>& other);
	Quaternion<T> operator + (const Quaternion<T>& other);
	Quaternion<T> operator - (const Quaternion<T>& other);
	Quaternion<T> operator * (const Quaternion<T>& other);
	Quaternion<T> operator / (const Quaternion<T>& other);
	Quaternion<T> operator += (const Quaternion<T>& other);
	Quaternion<T> operator -= (const Quaternion<T>& other);
	Quaternion<T> operator *= (const Quaternion<T>& other);
	Quaternion<T> operator /= (const Quaternion<T>& other);
	bool operator != (const Quaternion<T>& other);
	bool operator == (const Quaternion<T>& other);

			// quat operations
	T norm();
	T magnitude();
	Quaternion<T> scale(T s);
	Quaternion<T> inverse();
	Quaternion<T> conjugate();
	Quaternion<T> unit();
	void rotate(Vector3<T> &v);
	void glMatrix(GLdouble *mat);

	// std stream support
	friend inline std::ostream& operator << (std::ostream& output, const Quaternion<T>& quat) {
		output << "[" << quat.w << ", (" << quat.x << ", " << quat.y << ", " << quat.z <<")]";
		return output;
	}

private:
	T w, x, y, z;
};

template<class T> Quaternion<T>::Quaternion(void) {
	x = y = z = w = 0;
}

template<class T> Quaternion<T>::Quaternion(T wi, T xi, T yi, T zi) {
	x = xi;
	y = yi;
	z = zi;
	w = wi;
}

template<class T> Quaternion<T>::Quaternion(T wi, const Vector3<T>& v) {
	w = wi;
	x = v.x();
	y = v.y();
	z = v.z();
}

template<class T> Quaternion<T>::Quaternion(const Quaternion<T>& other) {
	z = other.z;
	w = other.w;
	x = other.x;
	y = other.y;
}

template<class T> Quaternion<T>::Quaternion(const Vector3<T>& vec, T angle) {
	angle = 0.5 * (angle * M_PI/180.0);
	Vector3<T> vn(vec);
	vn.normalize();
	T sinAngle = sin(angle);
	x = (vn.x() * sinAngle);
	y = (vn.y() * sinAngle);
	z = (vn.z() * sinAngle);
	w = cos(angle);
}

template<class T> Quaternion<T>::~Quaternion() {}

template<class T> Quaternion<T> Quaternion<T>::operator = (const Quaternion<T>& other) {
	w = other.w;
	x = other.x;
	y = other.y;
	z = other.z;

	return (*this);
}

template<class T> Quaternion<T> Quaternion<T>::operator + (const Quaternion<T>& other) {
	return Quaternion(w+other.w, x+other.x, y+other.y, z+other.z);
}

template<class T> Quaternion<T> Quaternion<T>::operator - (const Quaternion<T>& other) {
	return Quaternion(w-other.w, x-other.x, y-other.y, z-other.z);
}

template<class T> Quaternion<T> Quaternion<T>::operator * (const Quaternion<T>& other) {
	return Quaternion(
		w*other.w - x*other.x - y*other.y - z*other.z,
		w*other.x + x*other.w + y*other.z - z*other.y,
		w*other.y + y*other.w + z*other.x - x*other.z,
		w*other.z + z*other.w + x*other.y - y*other.x);
}

template<class T> Quaternion<T> Quaternion<T>::operator / (const Quaternion<T>& other) {
	return ((*this) * (other.inverse()));;
}

template<class T> Quaternion<T> Quaternion<T>::operator += (const Quaternion<T>& other) {
	w+=other.w;
	x+=other.x;
	y+=other.y;
	z+=other.z;

	return (*this);
}

template<class T> Quaternion<T> Quaternion<T>::operator -= (const Quaternion<T>& other) {
	w-=other.w;
	x-=other.x;
	y-=other.y;
	z-=other.z;

	return (*this);
}

template<class T> Quaternion<T> Quaternion<T>::operator *= (const Quaternion<T>& other) {
	T w_ = w*other.w - x*other.x - y*other.y - z*other.z;
	T x_ = w*other.x + x*other.w + y*other.z - z*other.y;
	T y_ = w*other.y + y*other.w + z*other.x - x*other.z;
	T z_ = w*other.z + z*other.w + x*other.y - y*other.x;

	w = w_;
	x = x_;
	y = y_;
	z = z_;

	return (*this);
}

template<class T> Quaternion<T> Quaternion<T>::operator /= (const Quaternion<T>& other) {
	(*this) = (*this)*other.inverse();
	return (*this);
}

template<class T> bool Quaternion<T>::operator != (const Quaternion<T>& other) {
	return (w!=other.w || x!=other.x || y!=other.y || z!=other.z) ? true : false;
}

template<class T> bool Quaternion<T>::operator == (const Quaternion<T>& other) {
	return (w==other.w && x==other.x && y==other.y && z==other.z) ? true : false;
}

template<class T> T Quaternion<T>::magnitude() {
	return sqrt(norm());
}

template<class T> T Quaternion<T>::norm() {
	return (w*w + x*x + y*y + z*z);
}

template<class T> Quaternion<T> Quaternion<T>::scale(T s) {
	return Quaternion(w*s, x*s, y*s, z*s);
}

template<class T> Quaternion<T> Quaternion<T>::inverse() {
	return conjugate().scale(1/norm());
}

template<class T> Quaternion<T> Quaternion<T>::conjugate() {
	return Quaternion(w, -x, -y, -z);
}

template<class T> Quaternion<T> Quaternion<T>::unit() {
	return (*this).scale(1/(*this).magnitude());
}

template<class T> void Quaternion<T>::rotate(Vector3<T> &v) {
	Quaternion<T> quat_v(0, v.x(), v.y(), v.z());
	Quaternion<T> quat_m = (*this) * quat_v * (*this).inverse();

	v.set(quat_m.x, quat_m.y, quat_m.z);
}

template<class T> void Quaternion<T>::glMatrix(GLdouble *mat) {
	GLdouble x2 = x * x;
	GLdouble y2 = y * y;
	GLdouble z2 = z * z;
	GLdouble xy = x * y;
	GLdouble xz = x * z;
	GLdouble yz = y * z;
	GLdouble wx = w * x;
	GLdouble wy = w * y;
	GLdouble wz = w * z;

	GLdouble _mat[16] = {
		1.0f - 2.0f * (y2 + z2), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
		2.0f * (xy + wz), 1.0f - 2.0f * (x2 + z2), 2.0f * (yz - wx), 0.0f,
		2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (x2 + y2), 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f};

	memcpy(mat, _mat, sizeof(GLdouble)*16);
}

typedef Quaternion<double> Quatd;

#endif
