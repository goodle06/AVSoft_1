#include "WindowsThread.h"


WindowsThread::WindowsThread(unsigned long func(void*), void* param ) {
	m_threadHandle=CreateThread(m_security, m_stackSz, func, param, m_flags, m_threadId);
	if (m_threadHandle == NULL) {
		throw std::runtime_error("failed to create a thread");
	}
}

WindowsThread::~WindowsThread() {
	CloseHandle(m_threadHandle);
}