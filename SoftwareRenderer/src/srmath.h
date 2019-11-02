#pragma once

#include <assert.h>
#include <math.h>

template<int row, int col, typename T>
struct Matrix {
	T mat[row][col];

	void setRow(int idx, T new_row[col]) {
		assert(idx < row);
		for (int i = 0; i < col; i++) {
			mat[idx][i] = new_row[i];
		}
	}

	void setRow(int idx, T p0, T p1, T p2, T p3) {
		assert(idx < row);
		mat[idx][0] = p0;
		mat[idx][1] = p1;
		mat[idx][2] = p2;
		mat[idx][3] = p3;
	}

	void setDiagonal(T value) {
		mat[0][0] = value;
		mat[1][1] = value;
		mat[2][2] = value;
		mat[3][3] = 1;
	}

	T *operator[](int rowIdx) {
		return mat[rowIdx];
	}

	Matrix operator*(Matrix B) {
		Matrix ret = { };
		for (int r = 0; r < row; r++) {
			for (int c = 0; c < col; c++) {
				ret[r][c] = mat[r][c] * B[c][r];
			}
		}
		return ret;
	}
};

typedef Matrix<4, 4, float> Mat4f;

struct Vec3f {
	union {
		struct {
			float x, y, z;
		};
		float r[3];
	};

	float operator[](int i) {
		return r[i];
	}

	Vec3f operator*(Mat4f mat) {
		Vec3f vec = { };

		vec.x =   x * mat[0][0] + y * mat[1][0] + z * mat[2][0] + mat[3][0];
		vec.y =   x * mat[0][1] + y * mat[1][1] + z * mat[2][1] + mat[3][1];
		vec.z =   x * mat[0][2] + y * mat[1][2] + z * mat[2][2] + mat[3][2];
		float w = x * mat[0][3] + y * mat[1][3] + z * mat[2][3] + mat[3][3];
		
		if (w != 0) {
			vec.y /= w;
			vec.z /= w;
			vec.x /= w;
		}

		return vec;
	}

	float operator*(Vec3f vec) {
		return vec.x * x + vec.y * y + vec.z * z;
	}

	Vec3f operator*(float scalar) {
		return Vec3f({ x * scalar, y * scalar, z * scalar });
	}

	Vec3f operator+(Vec3f b) {
		return Vec3f({ x + b.x, y + b.y, z + b.z });
	}

	Vec3f operator+(float p) {
		return Vec3f({ x + p, y + p, z + p });
	}

	Vec3f operator-(Vec3f b) {
		return Vec3f({ x - b.x, y - b.y, z - b.z });
	}

	void normalize() {
		x = x / (sqrt(x * x + y * y + z * z));
		y = y / (sqrt(x * x + y * y + z * z));
		z = z / (sqrt(x * x + y * y + z * z));
	}
};

struct Vec3i {
	union {
		struct {
			int x, y, z;
		};
		int r[3];
	};

	int operator[](int i) {
		return r[i];
	}

	Vec3i operator-(Vec3i b) {
		return { x - b.x, y - b.y, z - b.z };
	}
};

Mat4f identity() {
	Mat4f mat = {  };

	mat[0][0] = 1.0f;
	mat[1][1] = 1.0f;
	mat[2][2] = 1.0f;
	mat[3][3] = 1.0f;

	return mat;
}

Mat4f scale(float scaleValue) {
	Mat4f mat = {  };
	mat.setDiagonal(scaleValue);
	return mat;
}

Mat4f translate(float x, float y, float z) {
	Mat4f mat = identity();
	float values[4] = { x, y, z, 1.0f };
	mat.setRow(3, values);
	return mat;
}

// Angle in radians
Mat4f rotationY(float angle) {
	Mat4f mat = identity();

	float values0[4] = {  cos(angle),	0,	sin(angle),	0 };
	float values1[4] = {		   0,	1,			 0,	0 };
	float values2[4] = { -sin(angle),	0,	cos(angle), 0 };
	float values3[4] = {		   0,	0,			 0,	1 };

	mat.setRow(0, values0);
	mat.setRow(1, values1);
	mat.setRow(2, values2);
	mat.setRow(3, values3);

	return mat;
}

Mat4f rotationX(float angle) {
	Mat4f mat = identity();

	float values0[4] = { 1,			 0,			  0,	0 };
	float values1[4] = { 0,	cos(angle),	-sin(angle),	0 };
	float values2[4] = { 0,	sin(angle),	 cos(angle),	0 };
	float values3[4] = { 0,			 0,			  0,	1 };

	mat.setRow(0, values0);
	mat.setRow(1, values1);
	mat.setRow(2, values2);
	mat.setRow(3, values3);

	return mat;
}

Mat4f rotationXY(float angleTheta, float anglePhi) {
	Mat4f mat = identity();

	float values0[4] = { cos(angleTheta),	sin(angleTheta) * sin(anglePhi),	sin(angleTheta) * cos(anglePhi),	0 };
	float values1[4] = {			   0,					  cos(anglePhi),					 -sin(anglePhi),	0 };
	float values2[4] = { -sin(angleTheta),	cos(angleTheta) * sin(anglePhi),	cos(angleTheta) * cos(anglePhi),	0 };
	float values3[4] = {			   0,								  0,								  0,	1 };

	mat.setRow(0, values0);
	mat.setRow(1, values1);
	mat.setRow(2, values2);
	mat.setRow(3, values3);

	return mat;
}

template<typename T>
T cross(T v0, T v1) {
	return { v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x };
}

template<typename T>
Vec3f norm(T vec) {
	float length = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	return { vec.x / length, vec.y / length, vec.z / length };
}

Mat4f projection(float fov, float near, float far) {
	Mat4f proj = { };

	float s = 1 / tan(fov / 2);

	float c0 = -far / (far - near);
	float c1 = -(far * near) / (far - near);

	proj.setRow(0, s, 0,  0,  0);
	proj.setRow(1, 0, s,  0,  0);
	proj.setRow(2, 0, 0, c0, -1);
	proj.setRow(3, 0, 0, c1,  0);

	return proj;
}

Mat4f projection(float fov, float aspect, float near, float far) {

	float n = near;
	float f = far;

	float t = tan(fov / 2) * near;
	float b = -t;
	float r = t * aspect;
	float l = -r;

	Mat4f proj = { };
	proj.setRow(0,	(2 * n) / (r - l),					0,						 0,		 0);
	proj.setRow(1,					0,	(2 * n) / (t - b),						 0,		 0);
	proj.setRow(2,	(r + l) / (r - l),	(t + b) / (t - b),		-(f + n) / (f - n),		-1);
	proj.setRow(3,					0,					0,	-(2 * f * n) / (f - n),		 0);

	return proj;
}

void viewport(Vec3f &clipVert, float width, float height) {
	clipVert.x = roundf((clipVert.x + 1.0f) * 0.5f * width);
	clipVert.y = roundf((1.0f - (clipVert.y + 1.0f)  * 0.5f) * height);
}

Mat4f lookAt(Vec3f from, Vec3f to) {
	Vec3f forward = norm(from - to);
	Vec3f right = norm(cross({ 0.0f, 1.0f, 0.0f }, forward));
	Vec3f up = cross(forward, right);

	Mat4f ret = { };

	float values0[4] = {   right.x,	  right.y,	 right.z, 0.0f };
	float values1[4] = {	  up.x,		 up.y,	    up.z, 0.0f };
	float values2[4] = { forward.x, forward.y, forward.z, 0.0f };
	float values3[4] = {	from.x,	   from.y,	  from.z, 1.0f };

	ret.setRow(0, values0);
	ret.setRow(1, values1);
	ret.setRow(2, values2);
	ret.setRow(3, values3);

	return ret;
}

int LUPDecompose(float A[4][4], int N, double Tol, int *P) {
	int i, j, k, imax;
	float maxA, absA;

	for (i = 0; i <= N; i++)
		P[i] = i; //Unit permutation matrix, P[N] initialized with N

	for (i = 0; i < N; i++) {
		maxA = 0.0;
		imax = i;

		for (k = i; k < N; k++)
			if ((absA = fabs(A[k][i])) > maxA) {
				maxA = absA;
				imax = k;
			}

		if (maxA < Tol) return 0; //failure, matrix is degenerate

		if (imax != i) {
			//pivoting P
			j = P[i];
			P[i] = P[imax];
			P[imax] = j;

			//pivoting rows of A
			for (int l = 0; l < 4; l++) {
				float temp[4];
				temp[l] = A[i][l];
				A[i][l] = A[imax][l];
				A[imax][l] = temp[l];
			}

			//counting pivots starting from N (for determinant)
			P[N]++;
		}

		for (j = i + 1; j < N; j++) {
			A[j][i] /= A[i][i];

			for (k = i + 1; k < N; k++)
				A[j][k] -= A[j][i] * A[i][k];
		}
	}

	return 1;  //decomposition done 
}

void LUPInvert(float A[4][4], int *P, int N, float IA[4][4]) {
	for (int j = 0; j < N; j++) {
		for (int i = 0; i < N; i++) {
			if (P[i] == j)
				IA[i][j] = 1.0;
			else
				IA[i][j] = 0.0;

			for (int k = 0; k < i; k++)
				IA[i][j] -= A[i][k] * IA[k][j];
		}

		for (int i = N - 1; i >= 0; i--) {
			for (int k = i + 1; k < N; k++)
				IA[i][j] -= A[i][k] * IA[k][j];

			IA[i][j] = IA[i][j] / A[i][i];
		}
	}
}

Mat4f inverse(Mat4f &mat) {
	Mat4f inverted = { };
	int P[32];

	LUPDecompose(mat.mat, 4, 1e-5, P);
	LUPInvert(mat.mat, P, 4, inverted.mat);

	return inverted;
}