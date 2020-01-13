#include "core.hpp"


using namespace ysl;
void ysl::to_raw_data(hpos_safe& t, TChunkList<uchar>& p, PyObject* x)
{
	if (PyObject_IsInstance(x, TypeMarker)) throw ESerialization<void>(); //FI: check optimization (only for first element)
	if (x == NULL) throw ESerialization<void>();
	FirstType type = t.read<FirstType>();

	switch (type)
	{
	case YSLSC_BOOL:
	{
		if (x == Py_True)
			to_raw_data<char>(p, true);
		else if (x == Py_False)
			to_raw_data<char>(p, false);
		else throw ESerialization<void>();
		break;
	}
	case YSLSC_CHAR:
		to_raw_data<char>(p, PyLong_AsLong(x)); break;
	case YSLSC_UCHAR:
		to_raw_data<uchar>(p, PyLong_AsLong(x)); break;
	case YSLSC_SHORT:
		to_raw_data<short>(p, PyLong_AsLong(x)); break;
	case YSLSC_USHORT:
		to_raw_data<ushort>(p, PyLong_AsLong(x)); break;
	case YSLSC_INT:
		to_raw_data<int>(p, PyLong_AsLong(x)); break;
	case YSLSC_UINT:
		to_raw_data<uint>(p, PyLong_AsUnsignedLong(x)); break;
	case YSLSC_LONG:
		to_raw_data<bint>(p, PyLong_AsLongLong(x)); break;
	case YSLSC_ULONG:
		to_raw_data<ubint>(p, PyLong_AsUnsignedLongLong(x)); break;
	case YSLSC_FLOAT:
		to_raw_data<float>(p, PyFloat_AsDouble(x)); break;
	case YSLSC_DOUBLE:
		to_raw_data<double>(p, PyFloat_AsDouble(x)); break;

	case YSLSC_LIST:
	{
		size_t n = PyList_Size(x);
		hpos_safe oldt = t;
		if (n == 1)
			if (PyObject_IsInstance(PyList_GetItem(x, 0), TypeMarker))
				n = 0;
		p.write<size_t>(n);

		for (int i = 0; i < n; i++)
		{
			hpos_safe temp = oldt;
			to_raw_data(temp, p, PyList_GetItem(x, i));
		}
		// we have to move t 
		t += header_size(t);
		break;
	}
	case YSLSC_SET :
	{
		Py_INCREF(x);
		auto argtuple = PyTuple_New(1); PyTuple_SetItem(argtuple, 0, x);
		x = PyObject_Call(list_constructor, argtuple, NULL);
		Py_DECREF(argtuple);
		{
			size_t n = PyList_Size(x);
			hpos_safe oldt = t;
			if (n == 1)
				if (PyObject_IsInstance(PyList_GetItem(x, 0), TypeMarker))
					n = 0;
			p.write<size_t>(n);

			for (int i = 0; i < n; i++)
			{
				hpos_safe temp = oldt;
				to_raw_data(temp, p, PyList_GetItem(x, i));
			}
			// we have to move t 
			t += header_size(t);
		}

		Py_DECREF(x);
		break;
	}
	case YSLSC_DICT:
	{
		size_t n = PyDict_Size(x);
		hpos_safe oldt = t;
		auto items = PyDict_Items(x);
		if (n == 1)
			if (PyObject_IsInstance(PyTuple_GetItem(PyList_GetItem(items, 0), 0), TypeMarker))
				n = 0;
		p.write<size_t>(n);

		for (int i = 0; i < n; i++)
		{
			hpos_safe temp = oldt;
			to_raw_data(temp, p, PyTuple_GetItem(PyList_GetItem(items, i), 0));
			to_raw_data(temp, p, PyTuple_GetItem(PyList_GetItem(items, i), 1));
		}
		t += header_size(t);
		t += header_size(t);
		break;
	}
	case YSLSC_TUPLE:
	{
		size_t n = PyTuple_Size(x);
		auto [n_test, nouse] = t.read_size();
		if (n!= n_test) throw ESerialization<void>();

		for (int i = 0; i < n; i++)
			to_raw_data(t, p, PyTuple_GetItem(x, i));
		break;
	}
	case YSLSC_OPT:
	{
		if (x == Py_None)
		{
			p.write<uchar>(false);
			t += header_size(t);
			break;
		}
		p.write<uchar>(true);
		to_raw_data(t, p, x);
		break;
	}
	case YSLSC_STRING:
	{
		__int64 n;
		auto c = PyUnicode_AsUTF8AndSize(x, &n);
		if (c == NULL) throw ESerialization<void>();
		p.write<size_t>(n);
		memcpy(p.reserve(n), c, n);
		break;
	}
	case YSLSC_BYTES:
	{
		__int64 n = PyBytes_Size(x);
		auto c = PyBytes_AsString(x);
		if (c == NULL) throw ESerialization<void>();

		p.write<size_t>(n);
		memcpy(p.reserve(n), c, n);
		break;
	}
	default:
		throw ESerialization<void>();
	}
}

PyObject* ysl::from_raw_data(hpos_safe& t, hpos_safe& p)
{
	FirstType type = t.read<FirstType>();

	switch (type)
	{
	case YSLSC_BOOL:
	{
		bool b = p.read<char>();
		if (b)
		{
			Py_INCREF(Py_True);
			return Py_True;
		}
		else
		{
			Py_INCREF(Py_False);
			return Py_False;
		}
	}
		
	case YSLSC_CHAR:
		return PyLong_FromLong(p.read<char>());
	case YSLSC_UCHAR:
		return PyLong_FromLong(p.read<uchar>());
	case YSLSC_SHORT:
		return PyLong_FromLong(p.read<short>());
	case YSLSC_USHORT:
		return PyLong_FromLong(p.read<ushort>());
	case YSLSC_INT:
		return PyLong_FromLong(p.read<int>());
	case YSLSC_UINT:
		return PyLong_FromUnsignedLong(p.read<uint>());
	case YSLSC_LONG:
		return PyLong_FromLongLong(p.read<bint>());
	case YSLSC_ULONG:
		return PyLong_FromUnsignedLongLong(p.read<ubint>());
	case YSLSC_FLOAT:
		return PyFloat_FromDouble(p.read<float>());
	case YSLSC_DOUBLE:
		return PyFloat_FromDouble(p.read<double>());

	case YSLSC_LIST:
	{
		size_t n = p.read<size_t>();
		auto l = PyList_New(n);

		hpos_safe oldt = t;
		for (int i = 0; i < n; i++)
		{
			hpos_safe temp = oldt;
			PyList_SetItem(l, i, from_raw_data(temp, p));
		}
		t += header_size(t);
		return l;
	}
	case YSLSC_SET:
	{
		size_t n = p.read<size_t>();
		auto l = PySet_New(NULL);

		hpos_safe oldt = t;
		for (int i = 0; i < n; i++)
		{
			hpos_safe temp = oldt;
			PySet_Add(l, from_raw_data(temp, p));
		}
		t += header_size(t);
		return l;
	}
	case YSLSC_DICT:
	{
		size_t n = p.read<size_t>();
		auto l = PyDict_New();

		hpos_safe oldt = t;
		for (int i = 0; i < n; i++)
		{
			hpos_safe temp = oldt;
			auto key = from_raw_data(temp, p);
			auto value = from_raw_data(temp, p);
			PyDict_SetItem(l, key, value);
		}
		t += header_size(t); // twice
		t += header_size(t);
		return l;
	}
	case YSLSC_TUPLE:
	{
		size_t n = t.read_size().first;
		auto tp = PyTuple_New(n);
		for (int i = 0; i < n; i++)
			PyTuple_SetItem(tp, i, from_raw_data(t, p));
		return tp;
	}
	case YSLSC_OPT:
	{
		uchar exist = p.read<uchar>();
		if (exist)
		{
			return from_raw_data(t, p);
		}
		else
		{
			t += header_size(t);
			Py_INCREF(Py_None);
			return Py_None;
		}
	}
	case YSLSC_STRING:
	{
		size_t n = p.read<size_t>();
		auto oldindex = p.index;
		p += n;
		char err[512];

		return PyUnicode_DecodeUTF8((const char*)& p.buffer[oldindex], n, err);
	}
	case YSLSC_BYTES:
	{
		size_t n = p.read<size_t>();
		auto oldindex = p.index;
		p += n;
		if (n == 0)
		{
			char c = 0;
			return PyBytes_FromStringAndSize(&c, 0);
		}
		return PyBytes_FromStringAndSize((const char*)&p.buffer[oldindex], n);
	}
	default:
		throw ESerialization<void>();
	}
}

