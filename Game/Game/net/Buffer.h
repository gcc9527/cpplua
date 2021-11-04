#pragma once

#include <stddef.h>
#include <string.h>

class Buffer
{
public:
	Buffer();
	~Buffer();
	char* headAddr();
	void expand();
	void moveBuff();
	inline char* getHeadPos()
	{
		return data_ + wPos_; 
	}
	inline void reset()
	{ 
		rPos_ = 0;
		wPos_ = 0; 
	}
	inline size_t size() 
	{
		return len_; 
	}
	inline size_t writePos() 
	{
		return wPos_; 
	}
	inline size_t readPos()
	{ 
		return rPos_;
	}
	inline size_t leftSpace() 
	{
		return len_ - rPos_;
	}
	inline size_t validWriteLen() 
	{ 
		return rPos_ - wPos_; 
	} 
	inline void updateReadPos(size_t len)
	{ 
		rPos_ += len; 
	}
	inline void updateWritePos(size_t len) 
	{
		wPos_ += len;
	}
	inline char* readData()
	{ 
		return data_ + rPos_;
	}
	inline char* writeData() 
	{
		return data_ + wPos_;
	}
	inline char* data() 
	{ 
		return data_;
	}
	void copyData(char* dst, size_t lens);



private:
	char* data_;
	size_t wPos_ = 0;
	size_t rPos_ = 0;
	size_t len_ = 0;
};

