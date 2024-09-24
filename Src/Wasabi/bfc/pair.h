#ifndef _PAIR_H
#define _PAIR_H

template <class A, class B>
class Pair {
public:
	Pair() {}
  Pair(A _a, B _b) : a(_a), b(_b) {}

  A a;
  B b;
};

#endif
