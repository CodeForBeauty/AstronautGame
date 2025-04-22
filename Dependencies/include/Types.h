#pragma once

#include <lm/lm.h>


struct int2 {
	int x;
	int y;
};
struct int3 {
	int x;
	int y;
	int z;
};
struct int4 {
	int x;
	int y;
	int z;
	int w;
};

	
struct Quaternion {
	Quaternion() : x(0), y(0), z(0), w(1) {};
	Quaternion(float x1, float y1, float z1, float w1) : x(x1), y(y1), z(z1), w(w1) {};
	Quaternion(const lm::vec3& euler);
	Quaternion(const lm::vec3& axis, const float angle);

	float w, x, y, z;

	Quaternion& operator=(const Quaternion& other);
	Quaternion& operator=(const lm::vec4& other);

	Quaternion operator*(const Quaternion& other) const;

	void Normalize();
	void Inverse();
	float Magnitude();

	lm::vec3 GetEuler() const;

	lm::mat3 GetMatrix() const;

	Quaternion operator-() const;
};

lm::vec3 operator*(const Quaternion& quat, const lm::vec3& vec);

Quaternion slerp(const Quaternion& a, const Quaternion& b, float t);

std::ostream& operator<<(std::ostream& os, const Quaternion& v);