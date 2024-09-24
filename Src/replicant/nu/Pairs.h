#pragma once
namespace nu
{

	template <class A, class B>
	class Pair
	{
	public:
		Pair() {}
		Pair(const A &_a, const B &_b) : first(_a), second(_b) {}

		A first;
		B second;
	};
}
