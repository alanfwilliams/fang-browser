#include <Python.h>

// This is based on the example from the Python documentation:
// https://docs.python.org/3/extending/extending.html
// This is to understand how to write a Python module in C,
// which will be used to start development of the web browser 
// from scratch.

static PyObject *
spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    return PyLong_FromLong(sts);
}