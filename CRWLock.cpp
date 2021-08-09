#include "CRWLock.h"

CRWLock::CRWLock() {
	m_readerEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_globalEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (m_readerEvent == NULL || m_globalEvent == NULL)
		throw std::runtime_error{ "CRWLock constructor was unable to create event\n" }; //global variable constructor won't throw

	m_writersMutex = CreateMutex(NULL, FALSE, NULL);
	if (m_writersMutex == NULL)
		throw std::runtime_error{ "CRWLock constructor was unable to create mutex\n" };
	m_counter = -1;
}

CRWLock::~CRWLock() {
	CloseHandle(m_readerEvent);
	CloseHandle(m_globalEvent);
	CloseHandle(m_writersMutex);
}

void CRWLock::Claim(Process type) {

	if (type == Process::Writer) {
		// writer waits when mutex blocked by othre writer is free, then waits for all current readers to finish reading and post an event regarding this
		WaitForSingleObject(m_writersMutex, INFINITE);
		WaitForSingleObject(m_globalEvent, INFINITE); // m_globalEvent auto resets
	}
	else {
		//increment counter when first reader arrives, wait till writer posts event when finished writing, then post event
		//stating that first reader acquired resource
		if (InterlockedIncrement(&m_counter) == 0) {
			WaitForSingleObject(m_globalEvent, INFINITE); // m_globalEvent auto resets
			SetEvent(m_readerEvent);
		}
		//all readers wait event signalling acquisition of resource by first reader
		WaitForSingleObject(m_readerEvent, INFINITE);
	}
}

void CRWLock::Release(Process type) {
	if (type == Process::Writer) {
		//writer first signals readers that writing is finished and releases mutex
		SetEvent(m_globalEvent);
		ReleaseMutex(m_writersMutex);
	}
	else {
		if (InterlockedDecrement(&m_counter) < 0) {
			//if counter is less than 0, all readers finished reading, reader event is reset, global event signal all threads that resource is free for grabs
			ResetEvent(m_readerEvent);
			SetEvent(m_globalEvent);
		}
	}
}