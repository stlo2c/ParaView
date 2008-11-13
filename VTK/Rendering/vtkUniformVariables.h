/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkUniformVariables - GLSL uniform variables
// .SECTION Description
// vtkUniformVariables is a list of uniform variables attached to either a
// vtkShader2 object or to a vtkShaderProgram2. Uniform variables on
// a vtkShaderProgram2 override values of uniform variables on a vtkShader2.

// .SECTION See Also
// vtkShader2 vtkShaderProgram2

#ifndef __vtkUniformVariables_h
#define __vtkUniformVariables_h

#include "vtkObject.h"

class vtkUniformVariablesMap; // internal

class VTK_RENDERING_EXPORT vtkUniformVariables : public vtkObject
{
public:
  static vtkUniformVariables *New();
  vtkTypeRevisionMacro(vtkUniformVariables,vtkObject);
  void PrintSelf(ostream &os, vtkIndent indent);
  
  // Description:
  // 
  // \pre name_exists: name!=0
  // \pre value_exists: value!=0
  // \pre valid_numberOfComponents: numberOfComponents>=1 && numberOfComponents<=4
  void SetUniformi(const char *name,
                   int numberOfComponents,
                   int *value);

  // Description:
  // \pre name_exists: name!=0
  // \pre value_exists: value!=0
  // \pre valid_numberOfComponents: numberOfComponents>=1 && numberOfComponents<=4
  void SetUniformf(const char *name,
                   int numberOfComponents,
                   float *value);

  // Description:
  //
  // \pre name_exists: name!=0
  // \pre value_exists: value!=0
  // \pre valid_rows:  rows>=2 && rows<=4
  // \pre valid_colums: colums>=2 && colums<=4
  void SetUniformMatrix(const char *name,
                        int rows,
                        int colums,
                        float *value);
  
  // Description:
  // Remove uniform `name' from the list.
  void RemoveUniform(const char *name);
  
  // Description:
  // \pre need a valid OpenGL context and a shader program in use.
  void Send(const char *name,
            int uniformIndex);
  
  // Description:
  // Place the internal cursor on the first uniform.
  void Start();
  
  // Description:
  // Is the iteration done?
  bool IsAtEnd();
  
  // Description:
  // Name of the uniform at the current cursor position.
  // \pre not_done: !this->IsAtEnd()
  const char *GetCurrentName();
 
  // Description:
  // \pre need a valid OpenGL context and a shader program in use.
  // \pre not_done: !this->IsAtEnd()
  void SendCurrentUniform(int uniformIndex);
  
  // Description:
  // Move the cursor to the next uniform.
  // \pre not_done: !this->IsAtEnd()
  void Next();
  
protected:
  vtkUniformVariables();
  virtual ~vtkUniformVariables();

private:
  vtkUniformVariables(const vtkUniformVariables&);  // Not implemented.
  void operator=(const vtkUniformVariables&);  // Not implemented.
  
  vtkUniformVariablesMap *Map;
};
#endif
