#pragma once

#include <cmath>
#include <algorithm>
#include <ostream>
#include <iostream>

typedef struct float2 {
	float x;
	float y;

	float2 operator- (float2 other) {
		float2 out;
		out.x = x - other.x;
		out.y = y - other.y;
		return out;
	}

	float2 operator* (float other) {
		float2 out;
		out.x = other * x;
		out.y = other * y;
		return out;
	}

	float2 operator/ (float other) {
	    float2 out;
	    out.x = x / other;
	    out.y = y / other;
	    return out;
	}
} float2;


typedef struct float3 {
	float x;
	float y;
	float z;

	float3(float vx, float vy, float vz) {
		x = vx;
		y = vy;
		z = vz;
	}

	float3() {

	}

	float3 operator- (float3 other) {
		float3 out;
		out.x = x - other.x;
		out.y = y - other.y;
		out.z = z - other.z;
		return out;
	}

	float3 operator+ (float3 other) {
		float3 out;
		out.x = x + other.x;
		out.y = y + other.y;
		out.z = z + other.z;
		return out;
	}

	float3 operator* (float3 other) {
		float3 out;
		out.x = other.x * x;
		out.y = other.y * y;
		out.z = other.z * z;
		return out;
	}

	float3 operator/ (float divisor) {
	    float3 out;
	    out.x = x / divisor;
        out.y = y / divisor;
        out.z = z / divisor;
        return out;
	}

	bool operator== (float3 other) {
		return
			(x == other.x) &&
			(y == other.y) &&
			(z == other.z);
	}
} float3;


std::ostream & operator<<(std::ostream & os, float3 vec);

typedef struct float4 {
	float x;
	float y;
	float z;
	float w;

	float4 operator- (float4 other) {
		float4 out;
		out.x = x - other.x;
		out.y = y - other.y;
		out.z = z - other.z;
		out.w = w - other.w;
		return out;
	}

	float4 operator+ (float4 other) {
		float4 out;
		out.x = x + other.x;
		out.y = y + other.y;
		out.z = z + other.z;
		out.w = w + other.w;
		return out;
	}

	float4 operator* (float4 other) {
		float4 out;
		out.x = x * other.x;
		out.y = y * other.y;
		out.z = z * other.z;
		out.w = w * other.w;
		return out;
	}

	float4 operator/ (float4 other) {
		float4 out;
		out.x = x / other.x;
		out.y = y / other.y;
		out.z = z / other.z;
		out.w = w / other.w;
		return out;
	}

	bool operator!= (float4 other) {
		return (x != other.x) || (y != other.y) || (z != other.z) || (w != other.w);
	}

	template <class T>
	float4& operator=(T other) {
		x = other;
		y = other;
		z = other;
		w = other;
		return *this;
	}

	template <class T>
	bool operator!= (T other) {
		float4 tmp;
		tmp = other;
		return *this != tmp;
	}

	float4& operator= (float4 other) {
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}

	float3 toFloat3() {
		float3 vec;
		vec.x = x;
		vec.y = y;
		vec.z = z;
		return vec;
	}
} float4;

typedef struct mat4x4 {

	// Format: m<row><column>
	float m00, m01, m02, m03;
	float m10, m11, m12, m13;
	float m20, m21, m22, m23;
	float m30, m31, m32, m33;



	mat4x4(
	float v00, float v01, float v02, float v03,
	float v10, float v11, float v12, float v13,
	float v20, float v21, float v22, float v23,
	float v30, float v31, float v32, float v33) {
		m00 = v00; m01 = v01; m02 = v02; m03 = v03;
		m10 = v10; m11 = v11; m12 = v12; m13 = v13;
		m20 = v20; m21 = v21; m22 = v22; m23 = v23;
		m30 = v30; m31 = v31; m32 = v32; m33 = v33;
	};

	mat4x4() {

	}

	mat4x4 operator* (mat4x4 mat2) {
		mat4x4 mat1 = *this;

		return mat4x4(
			mat2.m00 * mat1.m00 + mat2.m10 * mat1.m01 + mat2.m20 * mat1.m02 + mat2.m30 * mat1.m03,
			mat2.m01 * mat1.m00 + mat2.m11 * mat1.m01 + mat2.m21 * mat1.m02 + mat2.m31 * mat1.m03,
			mat2.m02 * mat1.m00 + mat2.m12 * mat1.m01 + mat2.m22 * mat1.m02 + mat2.m32 * mat1.m03,
			mat2.m03 * mat1.m00 + mat2.m13 * mat1.m01 + mat2.m23 * mat1.m02 + mat2.m33 * mat1.m03,
			mat2.m00 * mat1.m10 + mat2.m10 * mat1.m11 + mat2.m20 * mat1.m12 + mat2.m30 * mat1.m13,
			mat2.m01 * mat1.m10 + mat2.m11 * mat1.m11 + mat2.m21 * mat1.m12 + mat2.m31 * mat1.m13,
			mat2.m02 * mat1.m10 + mat2.m12 * mat1.m11 + mat2.m22 * mat1.m12 + mat2.m32 * mat1.m13,
			mat2.m03 * mat1.m10 + mat2.m13 * mat1.m11 + mat2.m23 * mat1.m12 + mat2.m33 * mat1.m13,
			mat2.m00 * mat1.m20 + mat2.m10 * mat1.m21 + mat2.m20 * mat1.m22 + mat2.m30 * mat1.m23,
			mat2.m01 * mat1.m20 + mat2.m11 * mat1.m21 + mat2.m21 * mat1.m22 + mat2.m31 * mat1.m23,
			mat2.m02 * mat1.m20 + mat2.m12 * mat1.m21 + mat2.m22 * mat1.m22 + mat2.m32 * mat1.m23,
			mat2.m03 * mat1.m20 + mat2.m13 * mat1.m21 + mat2.m23 * mat1.m22 + mat2.m33 * mat1.m23,
			mat2.m00 * mat1.m30 + mat2.m10 * mat1.m31 + mat2.m20 * mat1.m32 + mat2.m30 * mat1.m33,
			mat2.m01 * mat1.m30 + mat2.m11 * mat1.m31 + mat2.m21 * mat1.m32 + mat2.m31 * mat1.m33,
			mat2.m02 * mat1.m30 + mat2.m12 * mat1.m31 + mat2.m22 * mat1.m32 + mat2.m32 * mat1.m33,
			mat2.m03 * mat1.m30 + mat2.m13 * mat1.m31 + mat2.m23 * mat1.m32 + mat2.m33 * mat1.m33);
	}

	float4 operator* (float4 other) {
		float4 transformed;

		transformed.x = m00 * other.x + m01 * other.y + m02 * other.z + m03 * other.w;
		transformed.y = m10 * other.x + m11 * other.y + m12 * other.z + m13 * other.w;
		transformed.z = m20 * other.x + m21 * other.y + m22 * other.z + m23 * other.w;
		transformed.w = m30 * other.x + m31 * other.y + m32 * other.z + m33 * other.w;

		return transformed;
	}
} mat4x4;



float length(float2 vec);
float length(float3 vec);

inline float2 make_float2(float x, float y) {
    float2 out;
    out.x = x;
    out.y = y;
    return out;
}

inline float3 make_float3(float x, float y, float z) {
    float3 out;
    out.x = x;
    out.y = y;
    out.z = z;
    return out;
}

float2 to_float2(float3 vec);

float2 normalize(float2 in);
float3 normalize(float3 in);

float3 operator* (float3 vec, float other);
float3 operator* (float other, float3 vec);

int clamp(int value, int lower, int upper);
