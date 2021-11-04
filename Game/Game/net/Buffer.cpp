#include "Buffer.h"

#include <stdlib.h>
#include <string.h>



constexpr size_t defaultLen = 1024;

Buffer::Buffer() :
	data_((char*)malloc(defaultLen))
{
	len_ = defaultLen;
	memset(data_, 0, len_);
}

Buffer::~Buffer()
{
	if (data_)
	{
		free(data_);
		data_ = nullptr;
	}
}


char* Buffer::headAddr()
{
	return data_;
}

void Buffer::expand()
{
	if (leftSpace() == 0)
	{
		size_t len = len_ * 2;
		char* data = (char*)malloc(len);
		memset(data, 0, len);
		memcpy(data, data_, len_);
		len_ = len;
		free(data_);
		data_ = data;
	}
}

void Buffer::moveBuff()
{
	if (wPos_ == rPos_)
	{
		return;
	}
	size_t pos = rPos_ - wPos_;
	memcpy(data_, data_ + wPos_, pos);
	wPos_ = 0;
	rPos_ = pos;
}

void Buffer::copyData(char* src, size_t lens)
{
	if (leftSpace() < lens)
	{
		size_t len = len_ * 2 + lens;
		char* data = (char*)malloc(len);
		memset(data, 0, len);
		memcpy(data, data_, len_);
		len_ = len;
		free(data_);
		data_ = data;
	}

	memcpy(data_ + rPos_, src, lens);
	rPos_ += lens;
}

