#pragma once
#include <string>
namespace Sandbox
{
	namespace Math
	{
		class Vector2
		{
		public:
			float X;
			float Y;

			Vector2(float x, float y) : X(x), Y(y) {}

			float Magnitude() const;
			float Magnitude2() const;

			void operator+=(const Vector2& other);
			void operator-=(const Vector2& other);

			void operator*=(const Vector2& other);
			void operator/=(const Vector2& other);

			void operator*=(const float& scalar);
			void operator/=(const float& scalar);

			std::string ToString() const;
		};
		using Vec2 = Vector2;
		inline std::string format_as(const Vector2& v) {
			return v.ToString();
		}
		inline bool operator==(const Vector2& left, const Vector2& right)
		{
			return (left.X == right.X && left.Y == right.Y);
		}
		inline bool operator!=(const Vector2& left, const Vector2& right)
		{
			return !(left == right);
		}

		inline Vector2 operator+(const Vector2& left, const Vector2& right)
		{
			return Vector2(left.X + right.X, left.Y + right.Y);
		}
		inline Vector2 operator-(const Vector2& left, const Vector2& right)
		{
			return Vector2(left.X - right.X, left.Y - right.Y);
		}
		inline Vector2 operator/(const Vector2& left, const Vector2& right)
		{
			return Vector2(left.X / right.X, left.Y / right.Y);
		}
		inline Vector2 operator*(const Vector2& left, const Vector2& right)
		{
			return Vector2(left.X * right.X, left.Y * right.Y);
		}

		inline Vector2 operator/(const Vector2& left, const float& scalar)
		{
			float mult = 1.0f / scalar;
			return Vector2(left.X * mult, left.Y * mult);
		}
		inline Vector2 operator*(const Vector2& left, const float& scalar)
		{
			return Vector2(left.X * scalar, left.Y * scalar);
		}

		inline Vector2 Normalize(const Vector2& input)
		{
			float mag = input.Magnitude();
			return input / mag;
		}

		inline float Dot(const Vector2& left, const Vector2& right)
		{
			return (left.X * right.X + left.Y * right.Y);
		}

		class Vector3
		{
		public:
			float X;
			float Y;
			float Z;

			Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

			float Magnitude() const;
			float Magnitude2() const;

			void operator+=(const Vector3& other);
			void operator-=(const Vector3& other);

			void operator*=(const Vector3& other);
			void operator/=(const Vector3& other);

			void operator*=(const float& scalar);
			void operator/=(const float& scalar);

			std::string ToString() const;
		};
		using Vec3 = Vector3;
		inline std::string format_as(const Vector3& v) {
			return v.ToString();
		}
		inline bool operator==(const Vector3& left, const Vector3& right)
		{
			return (left.X == right.X && left.Y == right.Y && left.Z == right.Z);
		}
		inline bool operator!=(const Vector3& left, const Vector3& right)
		{
			return !(left == right);
		}

		inline Vector3 operator+(const Vector3& left, const Vector3& right)
		{
			return Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);
		}
		inline Vector3 operator-(const Vector3& left, const Vector3& right)
		{
			return Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
		}
		inline Vector3 operator/(const Vector3& left, const Vector3& right)
		{
			return Vector3(left.X / right.X, left.Y / right.Y, left.Z / right.Z);
		}
		inline Vector3 operator*(const Vector3& left, const Vector3& right)
		{
			return Vector3(left.X * right.X, left.Y * right.Y, left.Z * right.Z);
		}

		inline Vector3 operator/(const Vector3& left, const float& scalar)
		{
			float mult = 1.0f / scalar;
			return Vector3(left.X * mult, left.Y * mult, left.Z * mult);
		}
		inline Vector3 operator*(const Vector3& left, const float& scalar)
		{
			return Vector3(left.X * scalar, left.Y * scalar, left.Z * scalar);
		}

		inline Vector3 Normalize(const Vector3& input)
		{
			float mag = input.Magnitude();
			return input / mag;
		}

		inline float Dot(const Vector3& left, const Vector3& right)
		{
			return (left.X * right.X + left.Y * right.Y + left.Z * right.Z);
		}

		inline Vector3 Cross(const Vector3& left, const Vector3& right)
		{
			float x = (left.Y * right.Z) - (right.Y * left.Z);
			float y = (left.Z * right.X) - (right.Z * left.X);
			float z = (left.X * right.Y) - (right.X * left.Y);

			return Vector3(x, y, z);
		}
	}
}