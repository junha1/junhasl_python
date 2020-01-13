#pragma once

#include <Python.h>

#include "YSL_Common.h"
#include "YSL_Types.h"

#include "TChunkList.h"

namespace ysl
{

	using DefaultInt = int;
	using DefaultFloat = float;

	FirstType get_type(PyObject* x);
	void mark_write(TChunkList<uchar>& t, PyObject* x);

	void to_raw_data(hpos_safe& t, TChunkList<uchar>& p, PyObject* x);
	PyObject* from_raw_data(hpos_safe& t, hpos_safe& p);

	// Primitive
	template <class T>
	void to_raw_data(TChunkList<uchar>& p, T x)
	{
		static_assert(ysl::IsPrimitive<T>);
		p.write<T>(x);
	}

	// Primitives are special : they are all castable with each other.
	template <bool R, bool W, class T>
	PyObject* from_raw_data(hpos_safe & t, cbpos & p)
	{
		RW_Assert;
		FirstType type = t.read<FirstType>();
		p += read_primitive<R, W, T>(x, type, p); // implicit conversion may occur here
	}

	struct ESerialization_Py : public ESerialization<void> 
	{ 
		string msg;  
		ESerialization_Py(const string& s)
		{
			msg = s;
		}
	};
}

extern PyObject* TypeMarker;
extern PyObject* list_constructor;