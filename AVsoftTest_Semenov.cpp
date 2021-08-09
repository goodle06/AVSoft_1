// AVsoftTest_Semenov.cpp : Defines the entry point for the application.
//

#include "AVsoftTest_Semenov.h"
#include "WindowsThread.h"
#include "CRWLock.h"
#include <string>
#include <vector>
#include <random>
#include <chrono>

#define READERS_COUNT 3
#define WRITERS_COUNT 3
#define REPEATS 10
#define DELAY_MSC 50


HANDLE allSyncEvent;
auto base_time = std::chrono::steady_clock::now();

struct ThreadInfo {
	ThreadInfo (Process p, CRWLock* lk, int delay) : process_type(p), lock(lk), initial_delay(delay) {}
	Process process_type;
	CRWLock* lock;
	int initial_delay = 0;
	std::vector <long> thread_timings=std::vector<long>( REPEATS );
	long thread_id = 0;
};

unsigned long WINAPI DoStuff(ThreadInfo* data);
void AnalyseResults(std::vector<ThreadInfo*>& data);

int main()
{
	CRWLock *lock= new CRWLock;
	allSyncEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (allSyncEvent == NULL)
		throw std::runtime_error("Unable to create event\n");

	std::vector<WindowsThread*> threads;
	std::vector<ThreadInfo*> params;
	const int total_threads = READERS_COUNT + WRITERS_COUNT;
	HANDLE threadHandles[total_threads];

	//randomize initial delay time of the thread
	std::random_device device;
	std::mt19937_64 eng(device());
	std::uniform_int_distribution<int> dist(0,2000);

	//create readers
	for (int i = 0; i < READERS_COUNT; i++) {
		params.push_back(new ThreadInfo { Process::Reader,lock, dist(eng) });
		threads.push_back(new WindowsThread((LPTHREAD_START_ROUTINE)DoStuff, (void*)params.back()));
		threadHandles[i] = threads.back()->GetThreadHandle();
	}
	//create writers
	for (int i = 0; i < WRITERS_COUNT; i++) {
		params.push_back(new ThreadInfo { Process::Writer,lock, dist(eng) });
		threads.push_back(new WindowsThread((LPTHREAD_START_ROUTINE)DoStuff, (void*)params.back()));
		threadHandles[READERS_COUNT+i] = threads.back()->GetThreadHandle();
	}

	//synchronizes all threads to start at the same time
	SetEvent(allSyncEvent);

	//wait for all threads to finish
	auto res=WaitForMultipleObjects(threads.size(), threadHandles, TRUE, INFINITE);
	switch (res)
	{
	case WAIT_OBJECT_0:
		std::cout << "Wait succeded\n";
		break;
	case WAIT_ABANDONED_0:
		std::cout << "Abandoned mutex\n";
		break;
	case WAIT_FAILED:
		std::cout << "Wait failed, error code #" << std::to_string(GetLastError()) << "\n";
		break;
	default:
		break;
	}

	AnalyseResults(params);

	//cleanup
	for (int i = 0; i < total_threads; i++) {
		delete threads[i];
		delete params[i];
	}
	delete lock;
	return 0;
}


unsigned long WINAPI DoStuff(ThreadInfo* data) {
	//synchronizes all threads to start at the same time
	WaitForSingleObject(allSyncEvent, INFINITE);

	ThreadInfo* info = data;
	Process id = info->process_type;
	CRWLock* lock = info->lock;

	std::string procstr = "Writer";
	if (id == Process::Reader) procstr = "Reader";

	//sleep for random initial delay
	Sleep(info->initial_delay);

	info->thread_id = GetCurrentThreadId();
	for (int i = 0; i < REPEATS; i++) {
		lock->Claim(id);
		//log time of aquiring resource for reading (not exclusive for other readers, exclusive to writers) or writing (exclusive to all)
		info->thread_timings[i] = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - base_time).count();
		Sleep(DELAY_MSC);
		lock->Release(id);
	}
	return 0;
}

void AnalyseResults(std::vector<ThreadInfo*>& threadData) {

	struct TimeTable
	{
		Process type;
		long id;
		long tm;
	};

	//merging timings of all threads together to sort and create a timeline
	std::vector < TimeTable > timetable;
	for (int i = 0; i < READERS_COUNT + WRITERS_COUNT; i++) {
		for (int j = 0; j < REPEATS; j++) {
			auto data = threadData[i];
			timetable.push_back({ data->process_type,data->thread_id, data->thread_timings[j] });
		}
	}
	std::sort(timetable.begin(), timetable.end(), [](auto a, auto b) {return a.tm < b.tm; });


	for (int i = 0; i < timetable.size(); i++) {
			auto data = timetable[i];
			std::cout << "Thread #" << data.id << ", start time " << data.tm << ", end time " << data.tm + DELAY_MSC << ", type ";
			if (data.type == Process::Reader)
				std::cout << "Reader\n";
			else
				std::cout << "Writer\n";
		}


}