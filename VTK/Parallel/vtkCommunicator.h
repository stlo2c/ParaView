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
// .NAME vtkCommunicator - Used to send/receive messages in a multiprocess environment.
// .SECTION Description
// This is an abstact class which contains functionality for sending
// and receiving inter-process messages. It contains methods for marshaling
// an object into a string (currently used by the MPI communicator but
// not the shared memory communicator).

// .SECTION Caveats
// Communication between systems with different vtkIdTypes is not
// supported. All machines have to have the same vtkIdType.

// .SECTION see also
// vtkMPICommunicator

#ifndef __vtkCommunicator_h
#define __vtkCommunicator_h

#include "vtkObject.h"

class vtkDataSet;
class vtkImageData;
class vtkDataObject;
class vtkDataArray;
class vtkBoundingBox;

class VTK_PARALLEL_EXPORT vtkCommunicator : public vtkObject
{

public:

  vtkTypeRevisionMacro(vtkCommunicator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method sends a data object to a destination.  
  // Tag eliminates ambiguity
  // and is used to match sends to receives.
  virtual int Send(vtkDataObject* data, int remoteHandle, int tag);

  // Description:
  // This method sends a data array to a destination.  
  // Tag eliminates ambiguity
  // and is used to match sends to receives.
  virtual int Send(vtkDataArray* data, int remoteHandle, int tag);
  
  // Description:
  // Subclass have to supply these methods to send various arrays of data.
  virtual int Send(int* data, int length, int remoteHandle, int tag) = 0;
  virtual int Send(unsigned long* data, int length, int remoteHandle, 
                   int tag) = 0;
  virtual int Send(unsigned char* data, int length, int remoteHandle, 
                   int tag) = 0;
  virtual int Send(char* data, int length, int remoteHandle, 
                   int tag) = 0;
  virtual int Send(float* data, int length, int remoteHandle, 
                   int tag) = 0;
  virtual int Send(double* data, int length, int remoteHandle, 
                   int tag) = 0;
#ifdef VTK_USE_64BIT_IDS
  virtual int Send(vtkIdType* data, int length, int remoteHandle, 
                   int tag) = 0;
#endif


  // Description:
  // This method receives a data object from a corresponding send. It blocks
  // until the receive is finished. 
  virtual int Receive(vtkDataObject* data, int remoteHandle, int tag);

  // Description:
  // This method receives a data array from a corresponding send. It blocks
  // until the receive is finished. 
  virtual int Receive(vtkDataArray* data, int remoteHandle, int tag);

  // Description:
  // Subclass have to supply these methods to receive various arrays of data.
  virtual int Receive(int* data, int length, int remoteHandle, 
                      int tag) = 0;
  virtual int Receive(unsigned long* data, int length, int remoteHandle,
                      int tag) = 0;
  virtual int Receive(unsigned char* data, int length, int remoteHandle, 
                      int tag) = 0;
  virtual int Receive(char* data, int length, int remoteHandle, 
                      int tag) = 0;
  virtual int Receive(float* data, int length, int remoteHandle, 
                      int tag) = 0;
  virtual int Receive(double* data, int length, int remoteHandle, 
                      int tag) = 0;
#ifdef VTK_USE_64BIT_IDS
  virtual int Receive(vtkIdType* data, int length, int remoteHandle, 
                      int tag) = 0;
#endif

  static void SetUseCopy(int useCopy);

  // Description:
  // Determine the global bounds for a set of processes.  BBox is 
  // initially set (outside of the call to the local bounds of the process 
  // and will be modified to be the global bounds - this default implementation
  // views the processors as a heap tree with the root being processId = 0
  // If either rightHasBounds or leftHasBounds is not 0 then the 
  // corresponding int will be set to 1 if the right/left processor has
  // bounds else it will be set to 0
  // The last three arguements are the tags to be used when performing
  // the operation
  virtual int ComputeGlobalBounds(int processorId, int numProcesses,
                                  vtkBoundingBox *bounds,
                                  int *rightHasBounds = 0,
                                  int *leftHasBounds = 0,
                                  int hasBoundsTag = 288402, 
                                  int localBoundsTag = 288403,
                                  int globalBoundsTag = 288404);

  // Description: 
  // Some helper functions when dealing with heap tree - based
  // algorthims - we don't need a function for getting the right
  // processor since it is 1 + theLeftProcessor
  static int GetParentProcessor(int pid);
  static int GetLeftChildProcessor(int pid);

protected:

  void DeleteAndSetMarshalString(char *str, int strLength);

  // Write and read from marshal string
  // return 1 success, 0 fail
  int WriteObject(vtkDataObject *object);
  int ReadObject(vtkDataObject *object);
  
  int WriteDataArray(vtkDataArray *object);
  int ReadDataArray(vtkDataArray *object);

  vtkCommunicator();
  ~vtkCommunicator();

  char *MarshalString;
  int MarshalStringLength;
  // The data may not take up all of the string.
  int MarshalDataLength;

  static int UseCopy;

private:
  vtkCommunicator(const vtkCommunicator&);  // Not implemented.
  void operator=(const vtkCommunicator&);  // Not implemented.
};

#endif // __vtkCommunicator_h


