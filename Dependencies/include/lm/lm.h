#pragma once
#include <iostream>
#include <list>


namespace lm {
	// Constant variables
	const double PI = 3.141592653589793;
	const double E = 2.718281828459045;
	const double PIrad = PI / 180;

	// Converting euler degrees and radians
	float degrees2radians(float angle);
	float radians2degrees(float radians);

	// Vector types
	struct vec2 {
	public:
		float x, y;
		vec2() {
			x = 0; y = 0;
		}
		vec2(float value) {
			x = value; y = value;
		}
		vec2(float xVal, float yVal) {
			x = xVal; y = yVal;
		}

		float& operator[] (int idx) {
			if (idx > 1) {
				std::cerr << "vec2: Index out of range 0-2." << std::endl;
				throw std::out_of_range("Index out of range 0-2.");
			}
			return (&x)[idx];
		}

		vec2& operator=(vec2 v) {
			x = v.x;
			y = v.y;
			return *this;
		}
		vec2& operator=(float v) {
			x = v;
			y = v;
			return *this;
		}

		vec2& operator++(int) {
			x++;
			y++;
			return *this;
		}
		vec2& operator--(int) {
			x--;
			y--;
			return *this;
		}
	};
	struct vec3 {
	public:
		float x, y, z;
		vec3() {
			x = 0; y = 0; z = 0;
		}
		vec3(float value) {
			x = value; y = value; z = value;
		}
		vec3(float xVal, float yVal) {
			x = xVal; y = yVal; z = 0;
		}
		vec3(float xVal, float yVal, float zVal) {
			x = xVal; y = yVal; z = zVal;
		}
		vec3(vec2 v) {
			x = v.x; y = v.y; z = 0;
		}

		vec2 cutVec2() {
			return { x, y };
		}

		float& operator[] (int idx) {
			if (idx > 2) {
				std::cerr << "vec3: Index out of range 0-3." << std::endl;
				throw std::out_of_range("Index out of range 0-3.");
			}
			return (&x)[idx];
		}

		vec3& operator=(vec2 v) {
			x = v.x;
			y = v.y;
			return *this;
		}
		vec3& operator=(vec3 v) {
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
		vec3& operator=(float v) {
			x = v;
			y = v;
			z = v;
			return *this;
		}

		vec3& operator++(int) {
			x++;
			y++;
			z++;
			return *this;
		}
		vec3& operator--(int) {
			x--;
			y--;
			z--;
			return *this;
		}
	};
	struct vec4 {
	public:
		float x, y, z, w;
		vec4() {
			x = 0; y = 0; z = 0; w = 0;
		}
		vec4(float value) {
			x = value; y = value; z = value; w = value;
		}
		vec4(float xVal, float yVal) {
			x = xVal; y = yVal; z = 0; w = 0;
		}
		vec4(float xVal, float yVal, float zVal) {
			x = xVal; y = yVal; z = zVal; w = 0;
		}
		vec4(float xVal, float yVal, float zVal, float wVal) {
			x = xVal; y = yVal; z = zVal; w = wVal;
		}
		vec4(vec2 v) {
			x = v.x; y = v.y; z = 0; w = 0;
		}
		vec4(vec3 v) {
			x = v.x; y = v.y; z = v.z; w = 0;
		}

		vec2 cutVec2() {
			return { x, y };
		}
		vec3 cutVec3() {
			return { x, y, z };
		}

		float& operator[] (int idx) {
			if (idx > 3) {
				std::cerr << "vec4: Index out of range 0-4." << std::endl;
				throw std::out_of_range("Index out of range 0-4.");
			}
			return (&x)[idx];
		}

		vec4& operator=(vec2 v) {
			x = v.x;
			y = v.y;
			return *this;
		}
		vec4& operator=(vec3 v) {
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}
		vec4& operator=(vec4 v) {
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}
		vec4& operator=(float v) {
			x = v;
			y = v;
			z = v;
			w = v;
			return *this;
		}

		vec4& operator++(int) {
			x++;
			y++;
			z++;
			w++;
			return *this;
		}
		vec4& operator--(int) {
			x--;
			y--;
			z--;
			w--;
			return *this;
		}
	};

	// Vector math functions
	// Dot product
	float dot(vec2 v1, vec2 v2);
	float dot(vec3 v1, vec3 v2);
	float dot(vec4 v1, vec4 v2);
	
	// Cross product
	float cross(vec2 v1, vec2 v2);
	vec3 cross(vec3 v1, vec3 v2);

	// Magnitude
	float magnitude(vec2 vector);
	float magnitude(vec3 vector);
	float magnitude(vec4 vector);

	// Normalize
	vec2 normalize(vec2 vector);
	vec3 normalize(vec3 vector);
	vec4 normalize(vec4 vector);

	// Power
	vec2 vecPow(vec2 vector, const int pow);
	vec3 vecPow(vec3 vector, const int pow);
	vec4 vecPow(vec4 vector, const int pow);

	// Square root
	vec2 vecSqrt(vec2 vector);
	vec3 vecSqrt(vec3 vector);
	vec4 vecSqrt(vec4 vector);


	// Matrix types
	// Square matricies
	struct mat2 {
	public:
		vec2 x, y;

		mat2() {
			x = {}; y = {};
		}
		mat2(vec2 x1, vec2 y1) {
			x = x1; y = y1;
		}

		vec2 column(int idx) {
			return vec2(x[idx], y[idx]);
		}


		mat2& operator=(const mat2& value) {
			x = value.x;
			y = value.y;
			return *this;
		}

		vec2& operator[] (int idx) {
			if (idx > 1) {
				std::cerr << "vec4: Index out of range 0-2." << std::endl;
				throw std::out_of_range("Index out of range 0-2.");
			}
			return (&x)[idx];
		}

		operator float* () {
			return &x.x;
		}
	};
	struct mat3 {
	public:
		vec3 x, y, z;

		mat3() {
			x = {}; y = {}; z = {};
		}
		mat3(const mat2& value) {
			x = value.x; y = value.y; z = {};
		}
		mat3(const mat2& value, vec3 z1) {
			x = value.x; y = value.y; z = z1;
		}
		mat3(vec3 x1, vec3 y1, vec3 z1) {
			x = x1; y = y1; z = z1;
		}


		vec3 column(int idx) {
			return vec3(x[idx], y[idx], z[idx]);
		}


		mat3& operator=(const mat3& value) {
			x = value.x;
			y = value.y;
			z = value.z;
			return *this;
		}
		mat3& operator=(const mat2& value) {
			x = value.x;
			y = value.y;
			return *this;
		}

		vec3& operator[] (int idx) {
			if (idx > 2) {
				std::cerr << "vec4: Index out of range 0-3." << std::endl;
				throw std::out_of_range("Index out of range 0-3.");
			}
			return (&x)[idx];
		}

		operator float* () {
			return &x.x;
		}
	};
	struct mat4 {
	public:
		vec4 x, y, z, w;

		mat4() {
			x = {}; y = {}; z = {}; w = {};
		}
		mat4(const mat2& value) {
			x = value.x; y = value.y; z = {}; w = {};
		}
		mat4(const mat2& value, vec4 z1, vec4 w1) {
			x = value.x; y = value.y; z = z1; w = w1;
		}
		mat4(const mat3& value) {
			x = value.x; y = value.y; z = value.z; w = {};
		}
		mat4(const mat3& value, vec4 w1) {
			x = value.x; y = value.y; z = value.z; w = w1;
		}
		mat4(vec4 x1, vec4 y1, vec4 z1, vec4 w1) {
			x = x1; y = y1; z = z1; w = w1;
		}


		vec4 column(int idx) {
			return vec4(x[idx], y[idx], z[idx], w[idx]);
		}


		mat4& operator=(const mat4& value) {
			x = value.x;
			y = value.y;
			z = value.z;
			w = value.w;
			return *this;
		}
		mat4& operator=(const mat2& value) {
			x = value.x;
			y = value.y;
			return *this;
		}
		mat4& operator=(const mat3& value) {
			x = value.x;
			y = value.y;
			z = value.z;
			return *this;
		}

		vec4& operator[] (int idx) {
			if (idx > 4) {
				std::cerr << "vec4: Index out of range 0-4." << std::endl;
				throw std::out_of_range("Index out of range 0-4.");
			}
			return (&x)[idx];
		}

		operator float* () {
			return &x.x;
		}
	};

	// Useful matricies
	mat4 position3d(vec3 position);
	mat3 position2d(vec2 position);

	mat3 rotation3d(vec3 rotation);
	mat2 rotation2d(float rotation);

	mat4 viewMatrix(vec3 at, vec3 eye, vec3 up = { 0, 1, 0 });
	mat4 orthographic(float right, float left, float top, float bottom, float far, float near);
	mat4 orthographic(float width, float height, float far, float near);
	mat4 orthographic(float width, float height, float depth);

	mat4 perspective(float fov, float near, float far, float width, float height);
	mat4 perspective(float fov, float near, float far, float ratio = 0.0f);



	// Non square matricies
	struct Matrix {
		double* data;
		size_t rowsSize, columnsSize;

		Matrix(const int& rows, const int& columns) {
			rowsSize = rows; columnsSize = columns;
			data = new double[rows * columns];
		}
		~Matrix() {
			delete data;
		}

		double& operator() (int row, int column) {
			return data[row * columnsSize + column];
		}

		Matrix& operator= (const Matrix& mat) {
			delete data;
			data = std::copy(mat.data, mat.data + (mat.rowsSize * mat.columnsSize), data);
			return *this;
		}
		Matrix& operator= (const mat2& mat) {
			delete data;
			data = new double[4] {mat.x.x, mat.x.y, mat.y.x, mat.y.y};
			return *this;
		}
		Matrix& operator= (const mat3& mat) {
			delete data;
			data = new double[9] {mat.x.x, mat.x.y, mat.x.z, mat.y.x, mat.y.y, mat.y.z, mat.z.x, mat.z.y, mat.z.z};
			return *this;
		}
		Matrix& operator= (const mat4& mat) {
			delete data;
			data = new double[16] {mat.x.x, mat.x.y, mat.x.z, mat.x.w, mat.y.x, mat.y.y, mat.y.z, mat.y.w, mat.z.x, mat.z.y, mat.z.z, mat.z.w, mat.w.x, mat.w.y, mat.w.z, mat.w.w};
			return *this;
		}
	};



	// Add operator overload for vectors
	vec2 operator+(vec2 v1, vec2 v2);
	vec3 operator+(vec3 v1, vec3 v2);
	vec4 operator+(vec4 v1, vec4 v2);

	// Multiply operator overload for vectors
	vec2 operator*(vec2 v1, vec2 v2);
	vec3 operator*(vec3 v1, vec3 v2);
	vec4 operator*(vec4 v1, vec4 v2);

	// Subtract operator overload for vectors
	vec2 operator-(vec2 v1, vec2 v2);
	vec3 operator-(vec3 v1, vec3 v2);
	vec4 operator-(vec4 v1, vec4 v2);

	// Divide operator overload for vectors
	vec2 operator/(vec2 v1, vec2 v2);
	vec3 operator/(vec3 v1, vec3 v2);
	vec4 operator/(vec4 v1, vec4 v2);

	// Vector operations with float
	// Add operator overload
	vec2 operator+(vec2 vector, float value);
	vec3 operator+(vec3 vector, float value);
	vec4 operator+(vec4 vector, float value);

	// Multiply operator overload
	vec2 operator*(vec2 vector, float value);
	vec3 operator*(vec3 vector, float value);
	vec4 operator*(vec4 vector, float value);

	// Subtract operator overload
	vec2 operator-(vec2 vector, float value);
	vec3 operator-(vec3 vector, float value);
	vec4 operator-(vec4 vector, float value);

	// Divide operator overload
	vec2 operator/(vec2 vector, float value);
	vec3 operator/(vec3 vector, float value);
	vec4 operator/(vec4 vector, float value);

	// Negative operator overload
	vec2 operator-(vec2 vector);
	vec3 operator-(vec3 vector);
	vec4 operator-(vec4 vector);

	// Compound assign operators overload
	// Add
	vec2& operator+=(vec2& vector, float value);
	vec3& operator+=(vec3& vector, float value);
	vec4& operator+=(vec4& vector, float value);

	// Multiply
	vec2& operator*=(vec2& vector, float value);
	vec3& operator*=(vec3& vector, float value);
	vec4& operator*=(vec4& vector, float value);

	// Subtract
	vec2& operator-=(vec2& vector, float value);
	vec3& operator-=(vec3& vector, float value);
	vec4& operator-=(vec4& vector, float value);

	// Divide
	vec2& operator/=(vec2& vector, float value);
	vec3& operator/=(vec3& vector, float value);
	vec4& operator/=(vec4& vector, float value);

	// Compound assignment with vectors
	// Add
	vec2& operator+=(vec2& v1, vec2 v2);
	vec3& operator+=(vec3& v1, vec3 v2);
	vec4& operator+=(vec4& v1, vec4 v2);

	// Multiply
	vec2& operator*=(vec2& v1, vec2 v2);
	vec3& operator*=(vec3& v1, vec3 v2);
	vec4& operator*=(vec4& v1, vec4 v2);

	// Subtract
	vec2& operator-=(vec2& v1, vec2 v2);
	vec3& operator-=(vec3& v1, vec3 v2);
	vec4& operator-=(vec4& v1, vec4 v2);

	// Divide
	vec2& operator/=(vec2& v1, vec2 v2);
	vec3& operator/=(vec3& v1, vec3 v2);
	vec4& operator/=(vec4& v1, vec4 v2);

	// Modulo operator overload
	vec2 operator%(vec2 vector, float value);
	vec3 operator%(vec3 vector, float value);
	vec4 operator%(vec4 vector, float value);

	// Cout operator overloading for vectors
	std::ostream& operator<<(std::ostream& os, vec2 v);
	std::ostream& operator<<(std::ostream& os, vec3 v);
	std::ostream& operator<<(std::ostream& os, vec4 v);



	// Matrix operator overloads
	// Square matricies
	vec2 operator*(mat2 mat, vec2 vec);
	vec3 operator*(mat3 mat, vec3 vec);
	vec4 operator*(mat4 mat, vec4 vec);

	mat2 operator*(mat2 mat1, mat2 mat2);
	mat3 operator*(mat3 mat1, mat3 mat2);
	mat4 operator*(mat4 mat1, mat4 mat2);

	// Non-square matricies
	Matrix operator*(Matrix mat1, Matrix mat2);
	std::list<double> operator*(Matrix mat1, std::list<double> vec);



	std::ostream& operator<<(std::ostream& os, mat2 v);
	std::ostream& operator<<(std::ostream& os, mat3 v);
	std::ostream& operator<<(std::ostream& os, mat4 v);

	std::ostream& operator<<(std::ostream& os, Matrix mat1);
}