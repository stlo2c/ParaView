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
// .NAME vtkCPPythonScriptPipeline - Class for coprocessing python script pipelines.
// .SECTION Description
// Class that creates a coprocessing pipeline starting from a coprocessing
// script.  This class only does operations with respect to the script
// and uses the name of the script as the module to hide its definitions
// from other python modules.  Python is primarily set up in 
// vtkCPPythonHelper where it loads servermanager, the coprocessing library
// and the trivial producer used to get the grid into the script.

#ifndef vtkCPPythonScriptPipeline_h
#define vtkCPPythonScriptPipeline_h

#include "vtkCPPipeline.h"
#include "vtkStdString.h"  // for the string

class vtkCPDataDescription;
class vtkCPPythonHelper;

class VTK_EXPORT vtkCPPythonScriptPipeline : public vtkCPPipeline
{
public:
  static vtkCPPythonScriptPipeline* New();
  vtkTypeRevisionMacro(vtkCPPythonScriptPipeline,vtkCPPipeline);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initialize this pipeline from given the file name of a 
  // python script. Returns 1 for success and 0 for failure.
  int Initialize(const char* FileName);
  
  // Description:
  // Configuration Step:
  // The coprocessor first determines if any coprocessing needs to be done
  // at this TimeStep/Time combination returning 1 if it does and 0
  // otherwise.  If coprocessing does need to be performed this time step
  // it fills in the FieldNames array that the coprocessor requires
  // in order to fulfill all the coprocessing requests for this
  // TimeStep/Time combination.
  virtual int RequestDataDescription(vtkCPDataDescription* DataDescription);

  // Description:
  // Execute the pipeline. Returns 1 for success and 0 for failure.
  virtual int CoProcess(vtkCPDataDescription* DataDescription);

protected:
  vtkCPPythonScriptPipeline();
  virtual ~vtkCPPythonScriptPipeline();

  // Description:
  // Return the address of Pointer for the python script.
  vtkStdString GetPythonAddress(void* Pointer);

  // Description:
  // Set/get macro functinos for setting PythonScriptName.
  vtkSetStringMacro(PythonScriptName);
  vtkGetStringMacro(PythonScriptName);

private:
  vtkCPPythonScriptPipeline(const vtkCPPythonScriptPipeline&); // Not implemented
  void operator=(const vtkCPPythonScriptPipeline&); // Not implemented

  // Description:
  // The object that runs the python scripts.
  static vtkCPPythonHelper* PythonHelper;

  // Description:
  // The name of the python script (without the path or extension)
  // that is used as the namespace of the functions of the script.
  char* PythonScriptName;
};



#endif
