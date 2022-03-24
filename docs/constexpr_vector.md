# Constexpr Vector
The Constexpr Vector mimics the interface and implementation of a fixed_vector,
with extra compile time capabilities. For the sake of brevity, we will only
show the use cases of the container in compile time. For the full interface,
please see [fixed_vector](fixed_vector.md).


# Quick Start
As mentioned previously, the interface of constexpr_vector mimics fixed_vector,
but usable in compile time.

With POD-types.
```c++
constexpr auto create_constexpr_vector()
{
    vecl::constexpr_vector<int, MAX_SIZE> cv;

	cv.push_back(0);
	cv.push_back(1);

	a.append({ 2,3,4 });

	return cv; // returns {0,1,2,3,4}
}

constexpr auto create_array()
{
    constexpr auto cv = create_constexpr_vector();

	std::array<int, cv.size()> arr;

	for (int i = 0; i < cv.size(); ++i)
		arr[i] = cv[i];

	return arr; // returns {0,1,2,3,4}
}

int main()
{
    // std::array<int,5> = {0,1,2,3,4}
    constexpr auto arr = create_array();
}
```

With objects.

```c++
struct pts
{
	int x, y;

	constexpr pts& operator=(const pts& p)
	{
		x = p.x, y = p.y;
		return *this;
	}
};

constexpr auto create_constexpr_vector()
{
    vecl::constexpr_vector<pts, MAX_SIZE> cv;

	a.push_back(pts{ 1,2 });
	a.push_back(pts{ 3,4 });

	return cv; // returns {{1,2},{3,4}}
}

constexpr auto create_array()
{
    constexpr auto cv = create_constexpr_vector();

	std::array<pts, cv.size()> arr;

	for (int i = 0; i < cv.size(); ++i)
		arr[i] = cv[i];

	return arr; // returns {{1,2},{3,4}}
}

int main()
{
    // std::array<pts,2> = {{1,2},{3,4}}
    constexpr auto arr = create_array();
}
```

# Further Reading
Originally, fixed vector was planned to have compile time capabilities. However,
the implementation of such a container required some insight and trickery to avoid dynamic casts, both of which I have little. 

Cutting a long story short, I gained both after watching David Stone's talk about
static_vector at CppCon2021.

- CppCon
    - https://www.youtube.com/watch?v=I8QJLGI0GOE (David Stone's talk on Implementing static_vector)




