#ifndef COMMON_EVENT_H
#define COMMON_EVENT_H

#include <atomic>

#ifdef bool
#undef bool
#endif


class Event {
public:
	static const int kInfinite = -1;

	Event();
	~Event();

	bool wait(int msecs = kInfinite);
	void post();

private:
	std::atomic<int> count_;
};


#endif // COMMON_EVENT_H
