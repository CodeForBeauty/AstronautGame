#include "Types.h"

using namespace std;

Quaternion::Quaternion(const lm::vec3& euler)
	: w(cos(euler.y / 2) * cos(euler.x / 2) * cos(euler.z / 2) - sin(euler.y / 2) * sin(euler.x / 2) * sin(euler.z / 2)),
	  y(sin(euler.y / 2) * cos(euler.x / 2) * cos(euler.z / 2) + cos(euler.y / 2) * sin(euler.x / 2) * sin(euler.z / 2)),
	  x(cos(euler.y / 2) * sin(euler.x / 2) * cos(euler.z / 2) - sin(euler.y / 2) * cos(euler.x / 2) * sin(euler.z / 2)),
	  z(cos(euler.y / 2) * cos(euler.x / 2) * sin(euler.z / 2) + sin(euler.y / 2) * sin(euler.x / 2) * cos(euler.z / 2)) {
}

Quaternion::Quaternion(const lm::vec3& axis, float angle) 
	: w(cos(angle / 2)),
	  x(sin(angle / 2) * axis.x),
	  y(sin(angle / 2) * axis.y),
	  z(sin(angle / 2) * axis.z) {

}

Quaternion& Quaternion::operator=(const Quaternion& other) {
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}

Quaternion& Quaternion::operator=(const lm::vec4& other) {
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}

Quaternion Quaternion::operator*(const Quaternion& other) const {
	return {
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y + y * other.w + z * other.x - x * other.z,
		w * other.z + z * other.w + x * other.y - y * other.x,
		w * other.w - x * other.x - y * other.y - z * other.z
	};
}

void Quaternion::Normalize() {
	float magnitude = sqrt(x * x + y * y + z * z + w * w);

	x /= magnitude;
	y /= magnitude;
	z /= magnitude;
	w /= magnitude;
}

void Quaternion::Inverse() {
	x = -x;
	y = -y;
	z = -z;
}

float Quaternion::Magnitude() {
	return sqrt(x * x + y * y + z * z + w * w);
}

lm::vec3 Quaternion::GetEuler() const {
	float p0 = w, p1 = y, p2 = x, p3 = z;

	float angleY = asin(2 * (p0 * p2 + p3 * p1));

	float angleX = 0, angleZ = 0;
	if (fabs(fabs(angleY) - lm::degrees2radians(90)) < 0.0001f) {
		angleX = atan2(p2, p0);
		angleZ = 0;
	}
	else {
		angleX = atan2((2 * (p0 * p1 - p2 * p3)), (1 - 2 * (p1 * p1 + p2 * p2)));
		angleZ = atan2((2 * (p0 * p3 - p1 * p2)), (1 - 2 * (p2 * p2 + p3 * p3)));
	}
	return {
		angleY,
		angleX,
		angleZ,
	};
}

lm::mat3 Quaternion::GetMatrix() const {
	return {
		{ 2 * (w * w + x * x) - 1, 2 * (x * y - w * z),     2 * (x * z + w * y) },
		{ 2 * (x * y + w * z),     2 * (w * w + y * y) - 1, 2 * (y * z - w * x) },
		{ 2 * (x * z - w * y),     2 * (y * z + w * x),     2 * (w * w + z * z) - 1 },
	};
}

Quaternion Quaternion::operator-() const {
	return { -x, -y, -z, -w };
}

lm::vec3 operator*(const Quaternion& quat, const lm::vec3& vec) {
	lm::vec3 complex = { quat.x, quat.y, quat.z };
	float real = quat.w;
	return 2.0f * lm::dot(complex, vec) * complex
		+ (real * real - lm::dot(complex, complex)) * vec
		+ 2.0f * real * lm::cross(complex, vec);
}

Quaternion slerp(const Quaternion& a, const Quaternion& b, float t) {
	float cosom = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

	Quaternion end = b;
	if (cosom < 0.0f) {
		cosom = -cosom;
		end.x = -end.x;
		end.y = -end.y;
		end.z = -end.z;
		end.w = -end.w;
	}

	float sclp, sclq;

	float omega, sinom;
	omega = std::acos(cosom);
	sinom = std::sin(omega);
	sclp = std::sin((1.0f - t) * omega) / sinom;
	sclq = std::sin(t * omega) / sinom;
	if ((1.0f - cosom) > 0.0001) {
		float omega, sinom;
		omega = std::acos(cosom);
		sinom = std::sin(omega);
		sclp = std::sin((1.0f - t) * omega) / sinom;
		sclq = std::sin(t * omega) / sinom;
	}
	else {
		sclp = 1.0f - t;
		sclq = t;
	}
	return {
		sclp * a.x + sclq * end.x,
		sclp * a.y + sclq * end.y,
		sclp * a.z + sclq * end.z,
		sclp * a.w + sclq * end.w
	};
}

std::ostream& operator<<(std::ostream& os, const Quaternion& v) {
	os << "x = " << v.x << " y = " << v.y << " z = " << v.z << " w = " << v.w;
	return os;
}
