#pragma once

#include "Common.h"


class WindowsThread {
	LPSECURITY_ATTRIBUTES m_security = NULL;
	SIZE_T m_stackSz = 0;
	LPTHREAD_START_ROUTINE	m_threadDoWork = nullptr;
	LPVOID m_funcParam = nullptr;
	DWORD m_flags=0;
	LPDWORD m_threadId = 0;
	HANDLE m_threadHandle = NULL;

public:
	WindowsThread(unsigned long(void*), void * params);
	~WindowsThread();

	WindowsThread(WindowsThread& other) = delete;
	WindowsThread& operator=(WindowsThread& rhs) = delete;

	HANDLE GetThreadHandle() { return m_threadHandle; }
};