#include <vecl/poly.hpp>
#include <gtest/gtest.h>

struct Shape
{
	template <typename Base>
	struct Interface : public Base
	{
		void draw() const { vecl::PolyCall<0>(*this); }
		int transform(float f) const { return vecl::PolyCall<1>(*this, f); }
	};

	template <typename T>
	using Members = vecl::PolyMembers<&T::draw, &T::transform>;
};

struct Square
{
	void draw() const { std::cout << "Draw Square" << '\n'; }
	int transform(float) const { std::cout << "Transform Square" << '\n'; return 1; }
};

struct Circle
{
	void draw() const { std::cout << "Draw Circle" << '\n'; }
	int transform(float) const { std::cout << "Transform Circle" << '\n'; return 1; }
};


TEST(_poly, poly)
{
	std::vector<vecl::Poly<Shape>> shapes;

	shapes.emplace_back(Square{});
	shapes.emplace_back(Circle{});

	for (const auto& shape : shapes)
		shape.draw();
}