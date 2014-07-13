#include "dataport.h"

#include <cerrno>
#include <cstring>
#include <ctime>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "logger.h"


namespace Airwave {


DataPort::DataPort() :
	id_(-1),
	frameSize_(0),
	buffer_(nullptr)
{
}


DataPort::~DataPort()
{
	disconnect();
}


bool DataPort::create(size_t frameSize)
{
	if(!isNull()) {
		LOG("Unable to create, port is already created.");
		return false;
	}

	size_t bufferSize = sizeof(ControlBlock) + frameSize;

	id_ = shmget(IPC_PRIVATE, bufferSize, S_IRUSR | S_IWUSR);
	if(id_ < 0) {
		LOG("Unable to allocate %d bytes of shared memory.", bufferSize);
		return false;
	}

	buffer_ = shmat(id_, nullptr, 0);
	if(buffer_ == reinterpret_cast<void*>(-1)) {
		LOG("Unable to attach shared memory segment with id %d.", id_);
		shmctl(id_, IPC_RMID, nullptr);
		id_ = -1;
		return false;
	}

	ControlBlock* control = controlBlock();
	sem_init(&control->request, 1, 0);
	sem_init(&control->response, 1, 0);

	frameSize_ = frameSize;
	return true;
}


bool DataPort::connect(int id)
{
	if(!isNull()) {
		LOG("Unable to connect on already initialized port.");
		return false;
	}

	buffer_ = shmat(id, nullptr, 0);
	if(buffer_ == reinterpret_cast<void*>(-1)) {
		LOG("Unable to attach shared memory segment with id %d.", id);
		return false;
	}

	shmid_ds info;
	if(shmctl(id, IPC_STAT, &info) != 0) {
		LOG("Unable to get info about shared memory segment with id %d.", id);
		shmdt(buffer_);
		id_ = -1;
		return false;
	}

	size_t bufferSize = info.shm_segsz;
	frameSize_ = bufferSize - sizeof(ControlBlock);

	id_ = id;
	return true;
}


void DataPort::disconnect()
{
	if(!isNull()) {
		if(!isConnected()) {
			ControlBlock* control = controlBlock();
			sem_destroy(&control->request);
			sem_destroy(&control->response);
		}

		shmdt(buffer_);
		shmctl(id_, IPC_RMID, nullptr);
		id_ = -1;
		buffer_ = nullptr;
		frameSize_ = 0;
	}
}


bool DataPort::isNull() const
{
	return id_ < 0;
}


bool DataPort::isConnected() const
{
	shmid_ds info;

	if(shmctl(id_, IPC_STAT, &info) != 0) {
		LOG("Unable to get info about shared memory segment with id %d.", id_);
		return false;
	}

	return info.shm_nattch > 1;
}


int DataPort::id() const
{
	return id_;
}


size_t DataPort::frameSize() const
{
	return frameSize_;
}


void* DataPort::frameBuffer()
{
	return controlBlock() + sizeof(ControlBlock);
}


void DataPort::sendRequest()
{
	if(!isNull()) {
		if(sem_post(&controlBlock()->request) != 0)
			LOG("sem_post() call failed: %s", strerror(errno));
	}
}


void DataPort::sendResponse()
{
	if(!isNull()) {
		if(sem_post(&controlBlock()->response) != 0)
			LOG("sem_post() call failed: %s", strerror(errno));
	}
}


bool DataPort::waitRequest(int msecs)
{
	return waitForReady(&controlBlock()->request, msecs);
}


bool DataPort::waitResponse(int msecs)
{
	return waitForReady(&controlBlock()->response, msecs);
}


DataPort::ControlBlock* DataPort::controlBlock()
{
	return static_cast<ControlBlock*>(buffer_);
}


bool DataPort::waitForReady(sem_t* semaphore, int msecs)
{
	if(isNull())
		return false;

	if(msecs < 0) {
		if(sem_wait(semaphore) != 0) {
			LOG("sem_wait() call failed: %d", errno);
			return false;
		}
	}
	else {
		int seconds = msecs / 1000;
		msecs %= 1000;

		timespec tm;
		clock_gettime(CLOCK_REALTIME, &tm);
		tm.tv_sec += seconds;
		tm.tv_nsec += msecs * 1000000;

		if(tm.tv_nsec >= 1000000000L) {
			tm.tv_sec++;
			tm.tv_nsec -= 1000000000L;
		}

		if(sem_timedwait(semaphore, &tm) != 0) {
			if(errno != ETIMEDOUT)
				LOG("sem_timedwait() call failed: %d", errno);

			return false;
		}
	}

	return true;
}


} // namespace Airwave
