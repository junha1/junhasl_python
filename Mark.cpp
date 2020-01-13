#include "core.hpp"

using namespace ysl;

// return first order type
ysl::FirstType ysl::get_type(PyObject* x)
{
	if (PyLong_CheckExact(x))
		return mark<DefaultInt>::value;
	else if (PyFloat_CheckExact(x))
		return mark<DefaultFloat>::value;
	else if (PyList_CheckExact(x))
		return YSLSC_LIST;
	else if (PyDict_CheckExact(x))
		return YSLSC_DICT;
	else if (PySet_Check(x))
		return YSLSC_SET;
	else if (PyTuple_CheckExact(x))
		return YSLSC_TUPLE;
	else if (PyUnicode_CheckExact(x))
		return YSLSC_STRING;
	else if (PyBytes_CheckExact(x))
		return YSLSC_BYTES;
	else if (x == Py_None)
		return YSLSC_OPT;
	else
		throw ESerialization<void>();
}

/*
Python side's dump is only process which lacks of type. (C++'s dump and load, Python's load have type)
Thus, there are special conditions.

1. Implicit conversion:
If you put raw python object on dumps, then following rules will be applied
- All python int objects will be casted into 'int'.
- All python float objects will be casted into 'float'.
- Empty homogeneous types are not supported.
=> If you want to represent an empty homogeneout type, you should put TypeMarker as an unique element.
TypeMarker holds partial typeheader as a string. (It's not needed when you do explicit conversion)
- Optional is not supported. That is, if you put 'None', the type will not be deduce but just converted into optional<char>

2. Explicit conversion:
If you need to bypass those rules, you must provide object type explicitly.
The type should be given with string as exactly same as they're supposed to be in C++ side.
For example, tuple<int, float, vector<double>> -> (3if[d.

NOTE :
- Even though C++ side supports converison between all primitives, you should never put heterogeneous list! (for example, [3, 3.0] will crahs C++)
- Default types for integer and floating point can be altered simply.
*/

void ysl::mark_write(TChunkList<uchar>& t, PyObject* x)
{
	if (x == Py_True || x == Py_False)
	{
		t.write<FirstType>(YSLSC_BOOL);
		return;
	}

	FirstType type = get_type(x);
	switch (type)
	{
	case mark<DefaultInt>::value:
	{
		t.write<FirstType>(mark<DefaultInt>::value);
		break;
	}
	case mark<DefaultFloat>::value:
	{
		t.write<FirstType>(mark<DefaultFloat>::value);
		break;
	}
	case YSLSC_LIST:
	{
		t.write<FirstType>(YSLSC_LIST);
		int n = PyList_Size(x);
		if (n == 0) throw ESerialization_Py("Purely empty homogeneous-type is not supported");

		auto first = PyList_GetItem(x, 0);
		if (PyObject_IsInstance(first, TypeMarker))
		{
			if(n != 1) throw ESerialization_Py("You should put TypeMarker as an unique element.");
			auto th = PyObject_GetAttrString(first, "type");
			__int64 n;
			auto c = PyUnicode_AsUTF8AndSize(th, &n);
			memcpy(t.reserve(n), c, n);
			break;
		}
		else mark_write(t, first);
		break;
	}
	case YSLSC_SET:
	{
		t.write<FirstType>(YSLSC_SET);
		Py_INCREF(x);
		auto argtuple = PyTuple_New(1); PyTuple_SetItem(argtuple, 0, x);
		x = PyObject_Call(list_constructor, argtuple, NULL);
		Py_DECREF(argtuple);
		{ // same as list
			int n = PyList_Size(x);
			if (n == 0) throw ESerialization_Py("Purely empty homogeneous-type is not supported");

			auto first = PyList_GetItem(x, 0);
			if (PyObject_IsInstance(first, TypeMarker))
			{
				if (n != 1) throw ESerialization_Py("You should put TypeMarker as an unique element.");
				auto th = PyObject_GetAttrString(first, "type");
				__int64 n;
				auto c = PyUnicode_AsUTF8AndSize(th, &n);
				memcpy(t.reserve(n), c, n);
				break;
			}
			else mark_write(t, first);
		}
		Py_DECREF(x);
		break;
	}
	case YSLSC_DICT:
	{
		t.write<FirstType>(YSLSC_DICT);
		int n = PyDict_Size(x);
		if (n == 0) throw ESerialization_Py("Purely empty homogeneous-type is not supported");

		auto first = PyList_GetItem(PyDict_Items(x), 0);
		auto first_key = PyTuple_GetItem(first, 0);
		auto first_value = PyTuple_GetItem(first, 1);
		if (PyObject_IsInstance(first_key, TypeMarker))
		{
			if (n != 1) throw ESerialization_Py("You should put TypeMarker as an unique element.");
			if (!PyObject_IsInstance(first_value, TypeMarker))
				throw ESerialization_Py("You should put two valid TypeMarkers as the key-value pair");

			auto th = PyObject_GetAttrString(first_key, "type");
			__int64 n;
			auto c = PyUnicode_AsUTF8AndSize(th, &n);
			memcpy(t.reserve(n), c, n);

			th = PyObject_GetAttrString(first_value, "type");
			c = PyUnicode_AsUTF8AndSize(th, &n);
			memcpy(t.reserve(n), c, n);
			break;
		}

		mark_write(t, first_key);
		mark_write(t, first_value);
		break;
	}
	case YSLSC_TUPLE:
	{
		t.write<FirstType>(YSLSC_TUPLE);
		int n = PyTuple_Size(x);
		uchar sizesize = size_encoding_size(n);

		char buf[17];
		int m = snprintf(buf, 16, "%d", n);
		memcpy((char*)t.reserve(sizesize), buf, m);

		for (int i = 0; i < n; i++)
			mark_write(t, PyTuple_GetItem(x, i));
		break;
	}
	case YSLSC_STRING:
	{
		t.write<FirstType>(YSLSC_STRING);
		break;
	}
	case YSLSC_BYTES:
	{
		t.write<FirstType>(YSLSC_BYTES);
		break;
	}
	case YSLSC_OPT:
	{
		// we define ysl::none as optional<uchar>
		t.write<FirstType>(YSLSC_OPT);
		t.write<FirstType>(YSLSC_UCHAR);
		break;
	}
	default:
		throw ESerialization<void>();
	}
}