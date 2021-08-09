#pragma once

#include "Common.h"

enum class Process { Reader, Writer };

class CRWLock {
	HANDLE m_readerEvent;
	HANDLE m_globalEvent;
	HANDLE m_writersMutex;
	long m_counter=-1;
public:
	CRWLock();
	~CRWLock();

	CRWLock(CRWLock& other) = delete;
	CRWLock& operator=(CRWLock& other) = delete;

	void Claim(Process type);
	void Release(Process type);



};