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
// .NAME vtkFileSeriesWriter - meta-writer for writing a file series using writers
// that are not time-aware.
// .SECTION Description:
// vtkFileSeriesWriter is a meta-writer that enables writing a file series using
// writers that are not time-aware.

#ifndef __vtkFileSeriesWriter_h
#define __vtkFileSeriesWriter_h

#include "vtkDataObjectAlgorithm.h"

class VTK_EXPORT vtkFileSeriesWriter : public vtkDataObjectAlgorithm
{
public:
  static vtkFileSeriesWriter* New();
  vtkTypeRevisionMacro(vtkFileSeriesWriter, vtkDataObjectAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the internal writer.
  virtual void SetWriter(vtkAlgorithm*);
  vtkGetObjectMacro(Writer, vtkAlgorithm);

  // Description:
  // Return the MTime also considering the internal writer.
  virtual unsigned long GetMTime();

  // Description:
  // Name of the method used to set the file name of the internal
  // writer. By default, this is SetFileName.
  vtkSetStringMacro(FileNameMethod);
  vtkGetStringMacro(FileNameMethod);

  // Description:
  // Get/Set the name of the output file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Invoke the writer.  Returns 1 for success, 0 for failure.
  int Write();

  // Description:
  // Must be set to true to write all timesteps, otherwise only the current
  // timestep will be written out. Off by default.
  vtkGetMacro(WriteAllTimeSteps, int);
  vtkSetMacro(WriteAllTimeSteps, int);
  vtkBooleanMacro(WriteAllTimeSteps, int);

  //BTX
  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);
  //ETX
protected:
//BTX
  vtkFileSeriesWriter();
  ~vtkFileSeriesWriter();

  int RequestInformation(vtkInformation* request,
                         vtkInformationVector** inputVector,
                         vtkInformationVector* outputVector);
  int RequestUpdateExtent(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);
  int RequestData(vtkInformation* request,
                  vtkInformationVector** inputVector,
                  vtkInformationVector* outputVector);

private:
  vtkFileSeriesWriter(const vtkFileSeriesWriter&); // Not implemented.
  void operator=(const vtkFileSeriesWriter&); // Not implemented.
  
  void SetWriterFileName(const char* fname);
  void WriteATimestep(vtkDataObject*);
  void WriteInternal();

  vtkAlgorithm* Writer;
  char* FileNameMethod;

  int WriteAllTimeSteps;
  int NumberOfTimeSteps;
  int CurrentTimeIndex;

  // The name of the output file.
  char* FileName;
//ETX
};

#endif

