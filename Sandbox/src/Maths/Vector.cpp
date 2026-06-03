#include "Vector.h"
#include <cmath>
namespace Sandbox::Math
{
	// Vector2
	// -------
	float Vector2::Magnitude() const
	{
		return std::sqrt((X * X) + (Y + Y));
	}

	float Vector2::Magnitude2() const
	{
		return (X * X) + (Y + Y);
	}

	void Vector2::operator+=(const Vector2& other)
	{
		X += other.X;
		Y += other.Y;
	}

	void Vector2::operator-=(const Vector2& other)
	{
		X -= other.X;
		Y -= other.Y;
	}

	void Vector2::operator*=(const Vector2& other)
	{
		X *= other.X;
		Y *= other.Y;
	}

	void Vector2::operator/=(const Vector2& other)
	{
		X /= other.X;
		Y /= other.Y;
	}

	void Vector2::operator*=(const float& scalar)
	{
		X *= scalar;
		Y *= scalar;
	}

	void Vector2::operator/=(const float& scalar)
	{
		float mult = 1.0f / scalar;
		X *= mult;
		Y *= mult;
	}

	std::string Vector2::ToString() const
	{
		return "X: " + std::to_string(X) + " Y: " + std::to_string(Y);
	}


	// Vector3
	// -------
	float Vector3::Magnitude() const
	{
		return std::sqrt((X * X) + (Y * Y) + (Z * Z));
	}
	float Vector3::Magnitude2() const
	{
		return (X * X) + (Y * Y) + (Z * Z);
	}

	void Vector3::operator+=(const Vector3& other)
	{
		X += other.X;
		Y += other.Y;
		Z += other.Z;
	}

	void Vector3::operator-=(const Vector3& other)
	{
		X -= other.X;
		Y -= other.Y;
		Z -= other.Z;
	}

	void Vector3::operator*=(const Vector3& other)
	{
		X *= other.X;
		Y *= other.Y;
		Z *= other.Z;
	}

	void Vector3::operator/=(const Vector3& other)
	{
		X /= other.X;
		Y /= other.Y;
		Z /= other.Z;
	}

	void Vector3::operator*=(const float& scalar)
	{
		X *= scalar;
		Y *= scalar;
		Z *= scalar;
	}

	void Vector3::operator/=(const float& scalar)
	{
		float mult = 1.0f / scalar;
		X *= mult;
		Y *= mult;
		Z *= mult;
	}
	std::string Vector3::ToString() const
	{
		return "X: " + std::to_string(X) + " Y: " + std::to_string(Y) + " Z: " + std::to_string(Z);
	}
}