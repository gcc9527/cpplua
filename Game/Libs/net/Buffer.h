#pragma once

#include <stddef.h>


class Buffer
{
public:
	Buffer();
	~Buffer();
	void zero();
	char* headAddr();
	void expand();
	void moveBuff();
	inline char* getHeadPos() { return data_ + wPos_; }
	inline void reset() { rPos_ = 0; wPos_ = 0; }
	inline size_t size() { return len_; }
	inline size_t writePos() { return wPos_; }
	inline size_t readPos() { return rPos_; }
	inline size_t leftSpace() { return len_ - rPos_; }
	inline size_t validLen() { return rPos_ - wPos_; } 
	inline void updateReadPos(size_t len) { rPos_ += len; }
	inline void updateWritePos(size_t len) { wPos_ += len; }
	inline char* readData() { return data_ + rPos_; }
	inline char* data() { return data_; }

private:
	char* data_;
	size_t wPos_ = 0;
	size_t rPos_ = 0;
	size_t len_ = 0;
};

