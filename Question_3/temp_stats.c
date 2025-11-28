
 #include <Python.h>
 #include <math.h>
 

 static PyObject* min_temp(PyObject* self, PyObject* args) {
     PyObject* temp_list;
     
     // Parse Python argument (expecting a list)
     if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &temp_list)) {
         return NULL;
     }
     
     Py_ssize_t size = PyList_Size(temp_list);
     

     if (size == 0) {
         PyErr_SetString(PyExc_ValueError, "Temperature list cannot be empty");
         return NULL;
     }
     
     PyObject* first_item = PyList_GetItem(temp_list, 0);
     double min_value = PyFloat_AsDouble(first_item);
     
     // Traverse array to find minimum
     for (Py_ssize_t i = 1; i < size; i++) {
         PyObject* item = PyList_GetItem(temp_list, i);
         double value = PyFloat_AsDouble(item);
         
         if (value < min_value) {
             min_value = value;
         }
     }
     
     return PyFloat_FromDouble(min_value);
 }
 
 static PyObject* max_temp(PyObject* self, PyObject* args) {
     PyObject* temp_list;
     
     if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &temp_list)) {
         return NULL;
     }
     
     Py_ssize_t size = PyList_Size(temp_list);
     
     if (size == 0) {
         PyErr_SetString(PyExc_ValueError, "Temperature list cannot be empty");
         return NULL;
     }
     
     // Initialize maximum with first element
     PyObject* first_item = PyList_GetItem(temp_list, 0);
     double max_value = PyFloat_AsDouble(first_item);
     
     // Traverse array to find maximum
     for (Py_ssize_t i = 1; i < size; i++) {
         PyObject* item = PyList_GetItem(temp_list, i);
         double value = PyFloat_AsDouble(item);
         
         if (value > max_value) {
             max_value = value;
         }
     }
     
     return PyFloat_FromDouble(max_value);
 }
 

 static PyObject* avg_temp(PyObject* self, PyObject* args) {
     PyObject* temp_list;
     
     if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &temp_list)) {
         return NULL;
     }
     
     Py_ssize_t size = PyList_Size(temp_list);
     
     if (size == 0) {
         PyErr_SetString(PyExc_ValueError, "Temperature list cannot be empty");
         return NULL;
     }
     
     // Calculate sum
     double sum = 0.0;
     for (Py_ssize_t i = 0; i < size; i++) {
         PyObject* item = PyList_GetItem(temp_list, i);
         sum += PyFloat_AsDouble(item);
     }
     
     // Calculate average
     double average = sum / size;
     
     return PyFloat_FromDouble(average);
 }
 

 static PyObject* variance_temp(PyObject* self, PyObject* args) {
     PyObject* temp_list;
     
     if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &temp_list)) {
         return NULL;
     }
     
     Py_ssize_t size = PyList_Size(temp_list);
     
     if (size < 2) {
         PyErr_SetString(PyExc_ValueError, 
             "Need at least 2 readings for variance calculation");
         return NULL;
     }
     
     // First pass: Calculate mean
     double sum = 0.0;
     for (Py_ssize_t i = 0; i < size; i++) {
         PyObject* item = PyList_GetItem(temp_list, i);
         sum += PyFloat_AsDouble(item);
     }
     double mean = sum / size;
     
     // Second pass: Calculate sum of squared differences
     double variance_sum = 0.0;
     for (Py_ssize_t i = 0; i < size; i++) {
         PyObject* item = PyList_GetItem(temp_list, i);
         double value = PyFloat_AsDouble(item);
         double diff = value - mean;
         variance_sum += diff * diff;
     }
     
     // Sample variance uses n-1 denominator
     double variance = variance_sum / (size - 1);
     
     return PyFloat_FromDouble(variance);
 }

 static PyObject* count_readings(PyObject* self, PyObject* args) {
     PyObject* temp_list;
     
     if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &temp_list)) {
         return NULL;
     }
     
     Py_ssize_t size = PyList_Size(temp_list);
     
     return PyLong_FromSsize_t(size);
 }
 
//method defin table
 static PyMethodDef TempStatsMethods[] = {
     {"min_temp", min_temp, METH_VARARGS, 
      "Find minimum temperature in the list"},
     {"max_temp", max_temp, METH_VARARGS, 
      "Find maximum temperature in the list"},
     {"avg_temp", avg_temp, METH_VARARGS, 
      "Calculate average temperature"},
     {"variance_temp", variance_temp, METH_VARARGS, 
      "Calculate sample variance of temperatures"},
     {"count_readings", count_readings, METH_VARARGS, 
      "Count total number of temperature readings"},
     {NULL, NULL, 0, NULL}  // Sentinel
 };

 static struct PyModuleDef tempstatsmodule = {
     PyModuleDef_HEAD_INIT,
     "temp_stats",                           // Module name
     "Temperature statistics C extension",   // Module documentation
     -1,                                     // Module state size
     TempStatsMethods                        // Method table
 };

 PyMODINIT_FUNC PyInit_temp_stats(void) {
     return PyModule_Create(&tempstatsmodule);
 }