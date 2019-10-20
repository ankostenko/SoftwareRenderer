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

	// If it doesn't have any cost I let it be
	float operator[](int i) {
		return r[i];
	}

	// TODO: rewrite this thing
	Vec3f operator*(Mat4f mat) {
		Vec3f vec = { };

		vec.x = x * mat[0][0] + y * mat[1][0] + z * mat[2][0] + mat[3][0];
		vec.y = x * mat[0][1] + y * mat[1][1] + z * mat[2][1] + mat[3][1];
		vec.z = x * mat[0][2] + y * mat[1][2] + z * mat[2][2] + mat[3][2];
		float w = x * mat[0][3] + y * mat[1][3] + z * mat[2][3] + mat[3][3];

		vec.x /= w;
		vec.y /= w;
		vec.z /= w;

		return vec;
	}

	Vec3f operator+(Vec3f b) {
		return { x + b.x, y + b.y, z + b.z };
	}

	Vec3f operator-(Vec3f b) {
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

Mat4f translate(float values[4]) {
	Mat4f mat = identity();
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

Vec3f cross(Vec3f v0, Vec3f v1) {
	return { v0.y * v1.z - v0.z * v1.y, v0.z * v1.x - v0.x * v1.z, v0.x * v1.y - v0.y * v1.x };
}


Mat4f lookAt(Vec3f from, Vec3f to) {
	Vec3f forward = from - to;
	Vec3f right = cross({ 0.0f, 1.0f, 0.0f }, forward);
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