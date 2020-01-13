#include "core.hpp"


PyDoc_STRVAR(ysl_example_doc, "example(obj, number)\
\
Example function");

/* 
- ysl is not thread-safe.
- before doing anything, call ysl.register(typemarker) first.
*/


PyObject* TypeMarker;
PyObject* list_constructor;

using namespace ysl;

PyObject* ysl_register(PyObject* self, PyObject* args)
{

	list_constructor = PyDict_GetItemString(PyEval_GetBuiltins(), "list");

	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "You must put 1 argument");
		return NULL;
	}
	TypeMarker = PyTuple_GetItem(args, 0);
	
	Py_INCREF(Py_None);
	return Py_None;
}
PyObject* ysl_check(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "You must put 1 argument to serialize");
		return NULL;
	}
	PyObject* obj = PyTuple_GetItem(args, 0);

	__int64 hsize;
	const char* buf = PyUnicode_AsUTF8AndSize(obj, &hsize);
	if (buf == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "You should put str");
		return NULL;
	}

	bytes header_real(hsize); memcpy(&header_real[0], buf, hsize);
	hpos_safe hpos_test(header_real);
	try {
		if (header_size(hpos_test) != hsize)
			throw ysl::hpos_safe::ill_formed();
	}
	catch(ysl::hpos_safe::ill_formed& e)
	{
		Py_INCREF(Py_False);
		return Py_False;
	}
	
	Py_INCREF(Py_True);
	return Py_True;
}
PyObject* ysl_header_size(PyObject* self, PyObject* args)
{
	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "You must put 1 argument to serialize");
		return NULL;
	}
	PyObject* obj = PyTuple_GetItem(args, 0);

	__int64 hsize;
	const char* buf = PyUnicode_AsUTF8AndSize(obj, &hsize);
	if (buf == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "You should put str");
		return NULL;
	}

	bytes header_real(hsize); memcpy(&header_real[0], buf, hsize);
	hpos_safe hpos_test(header_real);
	try {
		return PyLong_FromLongLong(header_size(hpos_test));
	}
	catch (ysl::hpos_safe::ill_formed& e)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid typestr");
		return NULL;
	}
}

PyObject* ysl_dumps(PyObject* self, PyObject* args, PyObject* kwargs) 
{
	static_assert(std::is_same_v<PyCFunctionWithKeywords, decltype(&ysl_dumps)>);
	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "You must put 1 argument to serialize");
		return NULL;
	}
	PyObject* obj = PyTuple_GetItem(args, 0);

	try
	{
		if (kwargs == NULL) // implicit type
		{
			TChunkList<uchar> header;
			TChunkList<uchar> data;
			mark_write(header, obj);

			bytes header_real(header.size());
			header.concat(&header_real[0]);
			hpos_safe hpos_real(header_real);

			to_raw_data(hpos_real, data, obj);

			auto hsize = header.size();
			bytes result(hsize + data.size());
			header.concat(&result[0]);
			data.concat(&result[hsize]);

			return PyBytes_FromStringAndSize((char*)& result[0], result.size());
		}
		else // explicit type
		{
			if (PyDict_Size(kwargs) > 1)
			{
				PyErr_SetString(PyExc_TypeError, "Invalid Options");
				return NULL;
			}

			char err[500];

			auto pstr = PyDict_GetItemWithError(kwargs, PyUnicode_DecodeUTF8("type", 4, err));
			if (pstr == NULL) return NULL;
			__int64 hsize;
			const char* buf = PyUnicode_AsUTF8AndSize(pstr, &hsize);
			if (buf == NULL) return NULL;

			TChunkList<uchar> data;
			bytes header_real(hsize); memcpy(&header_real[0], buf, hsize);
			hpos_safe hpos_test(header_real);
			if (header_size(hpos_test) != hsize)
			{
				PyErr_SetString(PyExc_ValueError, "Invalid explicit type");
				return NULL;
			}
			hpos_safe hpos_real(header_real);

			to_raw_data(hpos_real, data, obj);

			bytes result(hsize + data.size());
			memcpy(&result[0], &header_real[0], hsize);
			data.concat(&result[hsize]);

			return PyBytes_FromStringAndSize((char*)& result[0], result.size());
		}
	}
	catch (const ESerialization<void>& e)
	{
		PyErr_SetObject(PyExc_ValueError, obj);
		return NULL;
	}
	catch (const hpos_safe::ill_formed& e)
	{
		PyErr_SetObject(PyExc_TypeError, obj);
		return NULL;
	}
}

PyObject* ysl_loads(PyObject* self, PyObject* args) {
	/* Shared references that do not need Py_DECREF before returning. */

	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "You must put a bytes object that is serialized");
		return NULL;
	}
	PyObject* obj = PyTuple_GetItem(args, 0);
	if (!PyBytes_CheckExact(obj))
	{
		PyErr_SetString(PyExc_TypeError, "You must put a bytes object that is serialized");
		return NULL;
	}
	try
	{
		size_t len = PyBytes_Size(obj);
		auto buf = (uchar*)PyBytes_AsString(obj);
		bytes data_all(len); memcpy(&data_all[0], buf, len);
		hpos_safe hpos_test(data_all);
		auto hsize = header_size(hpos_test);

		hpos_safe header(data_all, 0, hsize);
		hpos_safe data(data_all, hsize, len);

		return from_raw_data(header, data);
	}
	catch (const ESerialization<void>& e)
	{
		PyErr_SetObject(PyExc_ValueError, obj);
		return NULL;
	}
	catch (const hpos_safe::ill_formed& e)
	{
		PyErr_SetObject(PyExc_TypeError, obj);
		return NULL;
	}
}
/*
 * List of functions to add to ysl in exec_ysl().
 */
static PyMethodDef ysl_functions[] = {
    { "dumps", (PyCFunction)ysl_dumps, METH_VARARGS | METH_KEYWORDS, ysl_example_doc },
	{ "loads", (PyCFunction)ysl_loads, METH_VARARGS, ysl_example_doc },
	{ "check", (PyCFunction)ysl_check, METH_VARARGS, ysl_example_doc },
	{ "register", (PyCFunction)ysl_register, METH_VARARGS, ysl_example_doc },
	{ "header_size", (PyCFunction)ysl_header_size, METH_VARARGS, ysl_example_doc },
    { NULL, NULL, 0, NULL } /* marks end of array */
};


PyDoc_STRVAR(ysl_doc, "The ysl module");


static PyModuleDef ysl_def = {
    PyModuleDef_HEAD_INIT,
    "ysl",
    ysl_doc,
    -1,              /* m_size */
	ysl_functions           /* m_methods */
};

PyMODINIT_FUNC PyInit_ysl() {

	PyObject* module = PyModule_Create(&ysl_def);
	if (module == NULL) return NULL;

	PyModule_AddStringConstant(module, "__author__", "JunhaYang");
	PyModule_AddStringConstant(module, "__version__", "1.0.0");
	PyModule_AddIntConstant(module, "year", 2019);
	

	return module;
}