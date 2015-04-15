#ifndef FUTEX_TEST_EVENT_H
#define FUTEX_TEST_EVENT_H

#include <atomic>

#ifdef bool
#undef bool
#endif


class Event {
public:
	static const int kInfinite = -1;

	Event();

	bool wait(int msecs = kInfinite);
	void post();

//private:
	std::atomic<int> count_;
};


#endif // FUTEX_TEST_EVENT_H
