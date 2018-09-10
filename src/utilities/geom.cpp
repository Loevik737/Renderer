#include "geom.hpp"



/*float3 cross(float3 a, float3 b) {
	float3 out;
	out.x =
}*/

float2 to_float2(float3 vec) {
	return make_float2(vec.x, vec.y);
}

float length(float2 vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y);
}

float length(float3 vec) {
	return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}