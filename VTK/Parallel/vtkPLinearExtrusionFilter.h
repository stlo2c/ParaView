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
// .NAME vtkPLinearExtrusionFilter - Subclass that handles piece invariance.
// .SECTION Description
// vtkPLinearExtrusionFilter is a parallel version of vtkLinearExtrusionFilter.

// .SECTION See Also
// vtkLinearExtrusionFilter

#ifndef __vtkPLinearExtrusionFilter_h
#define __vtkPLinearExtrusionFilter_h

#include "vtkLinearExtrusionFilter.h"

class VTK_PARALLEL_EXPORT vtkPLinearExtrusionFilter : public vtkLinearExtrusionFilter 
{
public:
  vtkTypeRevisionMacro(vtkPLinearExtrusionFilter,vtkLinearExtrusionFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create an object with PieceInvariant off.
  static vtkPLinearExtrusionFilter *New();

  // To get piece invariance, this filter has to request an 
  // extra ghost level.  Since piece invariance is not very 
  // important for this filter,  it is optional.  Without invariance,
  // Internal surfaces will be generated.  These surface
  // Are hidden by the normal surface generated by this filter.
  // By default, PieceInvariance is off.
  vtkSetMacro(PieceInvariant, int);
  vtkGetMacro(PieceInvariant, int);
  vtkBooleanMacro(PieceInvariant, int);

protected:
  vtkPLinearExtrusionFilter();
  ~vtkPLinearExtrusionFilter() {};

  void Execute();
  void ComputeInputUpdateExtents(vtkDataObject *output);

  int PieceInvariant;
private:
  vtkPLinearExtrusionFilter(const vtkPLinearExtrusionFilter&);  // Not implemented.
  void operator=(const vtkPLinearExtrusionFilter&);  // Not implemented.
};

#endif
