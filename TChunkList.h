#pragma once
#include <list>
#include <vector>
#include <memory>
using std::size_t;
using std::vector;
using std::list;
using std::shared_ptr;

template <class T>
class TChunkList
{
	struct chunk
	{
		shared_ptr <vector<T>> vec;
		size_t size; // may exceed chunksize
	};
	size_t chunksize;
	vector<chunk> data;

public:

	TChunkList(size_t s = 1024)
	{
		chunksize = s;

		chunk x; x.vec.reset(new vector<T>(chunksize));
		x.size = 0;
		data.push_back(x);
	}

	size_t size()
	{
		size_t s = 0;
		for (auto& x : data)
			s += x.size;
		return s;
	}
	void concat(T* v)
	{
		size_t count = 0;
		for (auto& x : data)
		{
			memcpy(&v[count], &x.vec->at(0), x.size);
			count += x.size;
		}
	}

	T* reserve(size_t size)
	{
		if (data.back().size + size > data.back().vec->size())
		{
			chunk x; x.vec.reset(new vector<T>(std::max(chunksize, size)));
			x.size = 0;
			data.push_back(x);
		}
		T* p = &data.back().vec->at(data.back().size);
		data.back().size += size;
		return p;
	}

	template <class V>
	void write(V t)
	{
		*(V*)reserve(sizeof(V)) = t;
	}
};