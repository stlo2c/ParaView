/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVPythonInterpretorWrapper
// .SECTION Description
// Helper header to used to wrap the interpretor object into python so that the
// error/output messaages generated by the interpretor can be collected by the
// vtkPVPythonInterpretor object.

#ifndef __vtkPVPythonInterpretorWrapper_h
#define __vtkPVPythonInterpretorWrapper_h

#include "structmember.h"

struct vtkPVPythonInterpretorWrapper
{
  PyObject_HEAD
  int softspace;  // Used by print to keep track of its state.
  vtkPVPythonInterpretor* Interpretor;
  bool DumpToError;

  // vtkPVPythonInterpretorWrapper() 
  //   {
  //   }

  void Write(const char* string)
    {
    if (this->Interpretor)
      {
      if (this->DumpToError)
        {
        this->Interpretor->DumpError(string);
        }
      else
        {
        this->Interpretor->DumpOutput(string);
        }
      }
    }
};

static PyObject* vtkWrite(PyObject* self, PyObject* args);

// const_cast since older versions of python are not const correct.
static PyMethodDef vtkPVPythonInterpretorWrapperMethods[] = {
    {const_cast<char*>("write"), vtkWrite, METH_VARARGS, const_cast<char*>("Dump message")},
    {0, 0, 0, 0}
};

static PyObject* vtkPVPythonInterpretorWrapperNew(
  PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
  return type->tp_alloc(type, 0);
}

static PyMemberDef vtkPVPythonInterpretorWrapperMembers[] = {
  { const_cast<char*>("softspace"),
    T_INT, offsetof(vtkPVPythonInterpretorWrapper, softspace), 0,
    const_cast<char *>("Placeholder so print can keep state.") },
  { 0, 0, 0, 0, 0 }
};

static PyTypeObject vtkPVPythonInterpretorWrapperType = {
    PyObject_HEAD_INIT(NULL)
    0,                         // ob_size
    const_cast<char*>("vtkPVPythonInterpretorWrapper"),   // tp_name
    sizeof(vtkPVPythonInterpretorWrapper), // tp_basicsize
    0,                         // tp_itemsize
    0,                         // tp_dealloc
    0,                         // tp_print
    0,                         // tp_getattr
    0,                         // tp_setattr
    0,                         // tp_compare
    0,                         // tp_repr
    0,                         // tp_as_number
    0,                         // tp_as_sequence
    0,                         // tp_as_mapping
    0,                         // tp_hash 
    0,                         // tp_call
    0,                         // tp_str
    PyObject_GenericGetAttr,   // tp_getattro
    PyObject_GenericSetAttr,   // tp_setattro
    0,                         // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    const_cast<char*>("vtkPVPythonInterpretorWrapper"),   //  tp_doc 
    0,                         //  tp_traverse 
    0,                         //  tp_clear 
    0,                         //  tp_richcompare 
    0,                         //  tp_weaklistoffset 
    0,                         //  tp_iter 
    0,                         //  tp_iternext 
    vtkPVPythonInterpretorWrapperMethods, //  tp_methods 
    vtkPVPythonInterpretorWrapperMembers, //  tp_members 
    0,                         //  tp_getset 
    0,                         //  tp_base 
    0,                         //  tp_dict 
    0,                         //  tp_descr_get 
    0,                         //  tp_descr_set 
    0,                         //  tp_dictoffset 
    0,                         //  tp_init 
    0,                         //  tp_alloc 
    vtkPVPythonInterpretorWrapperNew,  //  tp_new
    0, // freefunc tp_free; /* Low-level free-memory routine */
    0, // inquiry tp_is_gc; /* For PyObject_IS_GC */
    0, // PyObject *tp_bases;
    0, // PyObject *tp_mro; /* method resolution order */
    0, // PyObject *tp_cache;
    0, // PyObject *tp_subclasses;
    0, // PyObject *tp_weaklist;
#if PYTHON_API_VERSION >= 1012
    0  // tp_del
#endif
};

static PyObject* vtkWrite(PyObject* self, PyObject* args)
{
  if(!self || !PyObject_TypeCheck(self, &vtkPVPythonInterpretorWrapperType))
    {
    return 0;
    }

  vtkPVPythonInterpretorWrapper* wrapper = 
    reinterpret_cast<vtkPVPythonInterpretorWrapper*>(self);
  
  char *string;
  // const_cast since older versions of python are not const correct.
  if (wrapper && PyArg_ParseTuple(args, const_cast<char*>("s"), &string))
    {
    wrapper->Write(string);
    }
  return Py_BuildValue(const_cast<char*>(""));
}

static vtkPVPythonInterpretorWrapper* vtkWrapInterpretor(vtkPVPythonInterpretor* interpretor)
{
  if(PyType_Ready(&vtkPVPythonInterpretorWrapperType) < 0)
    {
    return 0;
    }

  vtkPVPythonInterpretorWrapper* wrapper = 
    PyObject_New(vtkPVPythonInterpretorWrapper, &vtkPVPythonInterpretorWrapperType);
  if (wrapper)
    {
    wrapper->Interpretor = interpretor;
    }

  return wrapper;
}

#endif

