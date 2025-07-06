#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
#include <array>

#define PI 3.141592653589793238462643383279502884197

namespace utils {
	namespace math {

		struct chunk_pos
		{
			int x, z;
		};


		template<typename T> class vector3
		{
		public:

			T x{ 0 };
			T y{ 0 };
			T z{ 0 };

			vector3<T>() {};
			vector3<T>(const T x, const T y, const T z) : x(x), y(y), z(z) {}
			vector3<T> operator + (const vector3<T>& rhs) const { return vector3<T>(x + rhs.x, y + rhs.y, z + rhs.z); }
			vector3<T> operator + (const T rhs) const { return vector3<T>(x + rhs, y + rhs, z + rhs); }
			vector3<T> operator - (const vector3<T>& rhs) const { return vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z); }
			vector3<T> operator - (const T rhs) const { return vector3<T>(x - rhs, y - rhs, z - rhs); }
			vector3<T> operator * (const vector3<T>& rhs) const { return vector3<T>(x * rhs.x, y * rhs.y, z * rhs.z); }
			vector3<T> operator * (const T& rhs) const { return vector3<T>(x * rhs, y * rhs, z * rhs); }
			vector3<T> operator / (const T& rhs) const { return vector3<T>(x / rhs, y / rhs, z / rhs); }
			vector3<T>& operator += (const vector3<T>& rhs) { return *this = *this + rhs; }
			vector3<T>& operator -= (const vector3<T>& rhs) { return *this = *this - rhs; }
			vector3<T>& operator *= (const T& rhs) { return *this = *this * rhs; }
			vector3<T>& operator /= (const T& rhs) { return *this = *this / rhs; }
			//Vector3<T> operator == (const Vector3<T>& rhs) { return this == rhs; }
			vector3<T> lerp(vector3<T> other, float detla) {
				return vector3<T>{
					x + (other.x - x) * static_cast<double>(detla),
						y + (other.y - y) * static_cast<double>(detla),
						z + (other.z - z) * static_cast<double>(detla),
				};
			}
			T length() const { return sqrt(x * x + y * y + z * z); }
			T length_sqr() const { return (x * x + y * y + z * z); }
			vector3<T> normalize() const { return *this * (1 / length()); }
			vector3<T> invert() const { return vector3<T>{ -x, -y, -z }; }
			T distance(const vector3<T>& rhs) const { return  vector3<T>(*this - rhs).length(); }
			T distance_sqr(const vector3<T>& rhs) const { return  vector3<T>(*this - rhs).length_sqr(); }
			T dist() { return std::sqrt(x * x + y * y); }
			vector3<T> mul(float sx, float sy, float sz) const {
				return { x * sx, y * sy, z * sz };
			}

			vector3<T> add(float ax, float ay, float az) const {
				return { x + ax, y + ay, z + az };
			}

		};
		template<typename T>
		struct box {
			T minX, minY, minZ;
			T maxX, maxY, maxZ;


			box<T> operator + (const box<T>& rhs) const {
				return box<T>{minX + rhs.minX,
					minY + rhs.minY,
					minZ + rhs.minZ,
					maxX + rhs.maxX,
					maxY + rhs.maxY,
					maxZ + rhs.maxZ};
			}
			box<T> operator + (const vector3<T>& rhs) const {
				return box<T>{minX + rhs.x,
					minY + rhs.y,
					minZ + rhs.z,
					maxX + rhs.x,
					maxY + rhs.y,
					maxZ + rhs.z};
			}
			box<T> operator - (const box<T>& rhs) const {
				return box<T>{minX - rhs.minX,
					minY - rhs.minY,
					minZ - rhs.minZ,
					maxX - rhs.maxX,
					maxY - rhs.maxY,
					maxZ - rhs.maxZ};
			}
			box<T> operator - (const vector3<T>& rhs) const {
				return box<T>{minX - rhs.x,
					minY - rhs.y,
					minZ - rhs.z,
					maxX - rhs.x,
					maxY - rhs.y,
					maxZ - rhs.z};
			}
			template<typename NT>
			inline box<NT> cast() const {
				return box<NT>{
					static_cast<NT>(minX),
						static_cast<NT>(minY),
						static_cast<NT>(minZ),
						static_cast<NT>(maxX),
						static_cast<NT>(maxY),
						static_cast<NT>(maxZ)
				};
			}

		};

		struct rotation {
			rotation operator - (const rotation& rhs) const { return rotation(x_rot - rhs.x_rot, y_rot - rhs.y_rot); }
			rotation Invert() const { return rotation{ -x_rot, -y_rot }; }
			float x_rot{ 0 };
			float y_rot{ 0 };
		};
		template<typename T> struct vector2
		{

			vector2 operator - (const vector2& rhs) const { return vector2(x - rhs.x, y - rhs.y); }
			vector2 Invert() const { return vector2{ -x, -y }; }
			T x{ 0 };
			T y{ 0 };
		};

		using vector3d = vector3<double>;
		/*struct Vector3D {
			double x, y, z;

			double distance(const Vector3D& other) const
			{
				return sqrt(pow(x - other.x, 2.0) + pow(y - other.y, 2.0) + pow(z - other.z, 2.0));
			}
			Vector3D operator - (const Vector3D& rhs) const { return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z); }
			Vector3D operator + (const Vector3D& rhs) const { return Vector3D(x + rhs.x, y + rhs.y, z + rhs.z); }
			Vector3D operator += (const Vector3D& rhs) { return *this = *this + rhs; }
			Vector3D& operator -= (const Vector3D& rhs) { return *this = *this - rhs; }

			Vector3D operator * (const float& rhs) const { return Vector3D(x * rhs, y * rhs, z * rhs); }
		};*/


		template<typename T>	struct Vector4
		{
			T x{ 0 };
			T y{ 0 };
			T z{ 0 };
			T w{ 0 };
		};

		struct Rect
		{
			float left{ FLT_MAX };
			float top{ FLT_MAX };
			float right{ FLT_MIN };
			float bottom{ FLT_MIN };
			inline bool vaild() {
				return left != FLT_MAX && top != FLT_MAX && right != FLT_MIN && bottom != FLT_MIN;
			}
		};

		// https://github.com/Marcelektro/MCP-919/blob/main/src/minecraft/net/minecraft/util/Matrix4f.java
		// For readability & maybe security purposes.
		struct matrix
		{
			float m00, m01, m02, m03;
			float m10, m11, m12, m13;
			float m20, m21, m22, m23;
			float m30, m31, m32, m33;
		};

		struct bb {
			double minX;
			double minY;
			double minZ;
			double maxX;
			double maxY;
			double maxZ;
		};

		struct aabb {
			float minX, minY, minZ, maxX, maxY, maxZ;
		};

		inline std::vector<double> struct_to_vector(const matrix& matrix)
		{

			//column-major order

			std::vector<double> result;
			result.reserve(16);

			result.push_back(matrix.m00);
			result.push_back(matrix.m10);
			result.push_back(matrix.m20);
			result.push_back(matrix.m30);

			result.push_back(matrix.m01);
			result.push_back(matrix.m11);
			result.push_back(matrix.m21);
			result.push_back(matrix.m31);

			result.push_back(matrix.m02);
			result.push_back(matrix.m12);
			result.push_back(matrix.m22);
			result.push_back(matrix.m32);

			result.push_back(matrix.m03);
			result.push_back(matrix.m13);
			result.push_back(matrix.m23);
			result.push_back(matrix.m33);


			return result;
		}

		inline std::array<float, 16> struct_to_array(const matrix& matrix)
		{

			//column-major order

			std::array<float, 16> result{
				matrix.m00 ,matrix.m10,matrix.m20,matrix.m30,
				matrix.m01 ,matrix.m11,matrix.m21,matrix.m31,
				matrix.m02 ,matrix.m12,matrix.m22,matrix.m32,
				matrix.m03 ,matrix.m13,matrix.m23,matrix.m33,
			};


			return result;
		}

		inline std::vector<double> struct_to_vector2(const matrix& matrix)
		{
			std::vector<double> result;
			result.reserve(16);


			result.push_back(matrix.m00);
			result.push_back(matrix.m01);
			result.push_back(matrix.m02);
			result.push_back(matrix.m03);

			result.push_back(matrix.m10);
			result.push_back(matrix.m11);
			result.push_back(matrix.m12);
			result.push_back(matrix.m13);

			result.push_back(matrix.m20);
			result.push_back(matrix.m21);
			result.push_back(matrix.m22);
			result.push_back(matrix.m23);

			result.push_back(matrix.m30);
			result.push_back(matrix.m31);
			result.push_back(matrix.m32);
			result.push_back(matrix.m33);


			return result;
		}

		struct matrix4 {
			std::array<std::array<float, 4>, 4> mat;

			matrix4(std::initializer_list<float> values) {
				auto it = values.begin();
				for (int i = 0; i < 4; ++i) {
					for (int j = 0; j < 4; ++j) {
						mat[i][j] = *it++;
					}
				}
			}
		};

		inline Vector4<float> multiply(Vector4<float> v, matrix m) {
			return Vector4{
				v.x * m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30,
				v.x * m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31,
				v.x * m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32,
				v.x * m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33
			};
		}

		inline Vector4<double> multiply(Vector4<double> v, matrix m) {
			return Vector4<double>{
				v.x* m.m00 + v.y * m.m10 + v.z * m.m20 + v.w * m.m30,
					v.x* m.m01 + v.y * m.m11 + v.z * m.m21 + v.w * m.m31,
					v.x* m.m02 + v.y * m.m12 + v.z * m.m22 + v.w * m.m32,
					v.x* m.m03 + v.y * m.m13 + v.z * m.m23 + v.w * m.m33
			};
		}




		inline float wrap_angle_to_180(float angle)
		{
			angle = std::fmod(angle, 360.0f);
			if (angle >= 180.0f) {
				angle -= 360.0f;
			}

			if (angle < -180.0f) {
				angle += 360.0f;
			}

			return angle;
		}
		inline float radiants_to_deg(float x)
		{
			return x * 180.f / PI;
		}
		inline float deg_to_radiants(float x)
		{
			return x * PI / 180.f;
		}
		inline double get_direction(float rotationYaw, double moveForward, double moveStrafing) {
			if (moveForward < 0.f) rotationYaw += 180.f;

			float forward = 1.f;

			if (moveForward < 0.f) forward = -0.5F;
			else if (moveForward > 0.f) forward = 0.5F;

			if (moveStrafing > 0.f) rotationYaw -= 90.f * forward;
			if (moveStrafing < 0.f) rotationYaw += 90.f * forward;
			return deg_to_radiants(rotationYaw);
		}
		inline math::vector2<float> get_angles(vector3<float> pos, vector3<float> pos1)
		{
			double d_x = pos1.x - pos.x;
			double d_y = pos1.y - pos.y;
			double d_z = pos1.z - pos.z;

			double hypothenuse = sqrt(d_x * d_x + d_z * d_z);
			float yaw = radiants_to_deg(atan2(d_z, d_x)) - 90.f;
			float pitch = radiants_to_deg(-atan2(d_y, hypothenuse));

			return math::vector2<float>(yaw, pitch);
		}
		template<typename T1, typename T2>
		inline math::vector2<T1> get_angles(vector3<T2> pos, vector3<T2> pos1)
		{
			T1 d_x = pos1.x - pos.x;
			T1 d_y = pos1.y - pos.y;
			T1 d_z = pos1.z - pos.z;

			T1 hypothenuse = sqrt(d_x * d_x + d_z * d_z);
			float yaw = radiants_to_deg(atan2(d_z, d_x)) - 90.f;
			float pitch = radiants_to_deg(-atan2(d_y, hypothenuse));

			return math::vector2< T1>(yaw, pitch);
		}
		template<typename T>
		inline math::vector2< T> vec_wrap_angle_to_180(math::vector2<T> angle)
		{
			return math::vector2<T>{
				wrap_angle_to_180(angle.x),
					wrap_angle_to_180(angle.y),
			};
		}

		inline float coterminal(float angle) {
			return std::fmod(angle, 180) < 0 ? angle + 170 : angle;
		}
		inline utils::math::vector3d get_closet_point(utils::math::vector3d start, utils::math::box<double> aabb) {
			return utils::math::vector3d{
				std::clamp(start.x, aabb.minX, aabb.maxX),
				std::clamp(start.y, aabb.minY, aabb.maxY),
				std::clamp(start.z, aabb.minZ, aabb.maxZ)
			};
		}

	}

}
