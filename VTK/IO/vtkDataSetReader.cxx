/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkDataSetReader.h"
#include "vtkPolyDataReader.h"
#include "vtkStructuredPointsReader.h"
#include "vtkStructuredGridReader.h"
#include "vtkRectilinearGridReader.h"
#include "vtkUnstructuredGridReader.h"


vtkDataSetReader::vtkDataSetReader()
{
  this->Reader = vtkDataReader::New();
  this->Reader->SetSource(this);
}

vtkDataSetReader::~vtkDataSetReader()
{
  this->Reader->Delete();
  this->Reader = NULL;
}

vtkDataSet * vtkDataSetReader::GetOutput() 
{
  // check to see if an execute is necessary.
  if (this->Outputs && this->Outputs[0] && 
      this->Outputs[0]->GetUpdateTime() > this->GetMTime())
    {
    return (vtkDataSet *)(this->Outputs[0]);
    }
  
  // The filename might have changed (changing the output).
  // We need to re execute.
  if (this->Reader->GetFileName() == NULL && 
      (this->Reader->GetReadFromInputString() == 0 || 
       this->Reader->GetInputString() == NULL))
    {
    vtkWarningMacro(<< "FileName must be set");
    return (vtkDataSet *) NULL;
    }

  this->Execute();
  return (vtkDataSet *)this->Outputs[0];
}

unsigned long int vtkDataSetReader::GetMTime()
{
  unsigned long int t1, t2;
  
  t1 = this->vtkSource::GetMTime();
  t2 = this->Reader->GetMTime();
  if (t2 > t1)
    {
    t1 = t2;
    }
  return t1;
}

// Specify file name of vtk data file to read.
void vtkDataSetReader::SetFileName(char *name) 
{
  this->Reader->SetFileName(name);
}

char *vtkDataSetReader::GetFileName() 
{
  return this->Reader->GetFileName();
}

// Get the type of file (VTK_ASCII or VTK_BINARY).
int vtkDataSetReader::GetFileType() 
{
  return this->Reader->GetFileType();
}

// Set the name of the scalar data to extract. If not specified, first 
// scalar data encountered is extracted.
void vtkDataSetReader::SetScalarsName(char *name) 
{
  this->Reader->SetScalarsName(name);
}
char *vtkDataSetReader::GetScalarsName() 
{
  return this->Reader->GetScalarsName();
}

// Set the name of the vector data to extract. If not specified, first 
// vector data encountered is extracted.
void vtkDataSetReader::SetVectorsName(char *name) 
{
  this->Reader->SetVectorsName(name);
}
char *vtkDataSetReader::GetVectorsName() 
{
  return this->Reader->GetVectorsName();
}

// Set the name of the tensor data to extract. If not specified, first 
// tensor data encountered is extracted.
void vtkDataSetReader::SetTensorsName(char *name) 
{
  this->Reader->SetTensorsName(name);
}
char *vtkDataSetReader::GetTensorsName() 
{
  return this->Reader->GetTensorsName();
}

// Set the name of the normal data to extract. If not specified, first 
// normal data encountered is extracted.
void vtkDataSetReader::SetNormalsName(char *name) 
{
  this->Reader->SetNormalsName(name);
}
char *vtkDataSetReader::GetNormalsName() 
{
  return this->Reader->GetNormalsName();
}

// Set the name of the texture coordinate data to extract. If not specified,
// first texture coordinate data encountered is extracted.
void vtkDataSetReader::SetTCoordsName(char *name) 
{
  this->Reader->SetTCoordsName(name);
}
char *vtkDataSetReader::GetTCoordsName() 
{
  return this->Reader->GetTCoordsName();
}

// Set the name of the lookup table data to extract. If not specified, uses 
// lookup table named by scalar. Otherwise, this specification supersedes.
void vtkDataSetReader::SetLookupTableName(char *name) 
{
  this->Reader->SetLookupTableName(name);
}
char *vtkDataSetReader::GetLookupTableName() 
{
  return this->Reader->GetLookupTableName();
}

// Set the name of the field data to extract. If not specified, uses 
// first field data encountered in file.
void vtkDataSetReader::SetFieldDataName(char *name) 
{
  this->Reader->SetFieldDataName(name);
}
char *vtkDataSetReader::GetFieldDataName() 
{
  return this->Reader->GetFieldDataName();
}

void vtkDataSetReader::Execute()
{
  char line[256];
  vtkDataObject *output;
  
  vtkDebugMacro(<<"Reading vtk dataset...");

  if ( this->Debug ) 
    {
    this->Reader->DebugOn();
    }
  else 
    {
    this->Reader->DebugOff();
    }
  if (!this->Reader->OpenVTKFile() || !this->Reader->ReadHeader())
    {
    return;
    }

  // Determine dataset type
  //
  if (!this->Reader->ReadString(line))
    {
    vtkErrorMacro(<< "Premature EOF reading dataset keyword");
    return;
    }

  if ( !strncmp(this->Reader->LowerCase(line),"dataset",(unsigned long)7) )
    {

    // See if type is recognized.
    //
    if (!this->Reader->ReadString(line))
      {
      vtkErrorMacro(<< "Premature EOF reading type");
      this->Reader->CloseVTKFile ();
      return;
      }

    this->Reader->CloseVTKFile();
    if ( ! strncmp(this->Reader->LowerCase(line),"polydata",8) )
      {
      vtkPolyDataReader *preader = vtkPolyDataReader::New();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkPolyData") == 0)
	{
	preader->SetOutput((vtkPolyData *)(output));
	}
      preader->SetFileName(this->Reader->GetFileName());
      preader->SetInputString(this->Reader->GetInputString(),
			      this->Reader->GetInputStringLength());
      preader->SetReadFromInputString(this->Reader->GetReadFromInputString());
      preader->SetScalarsName(this->Reader->GetScalarsName());
      preader->SetVectorsName(this->Reader->GetVectorsName());
      preader->SetNormalsName(this->Reader->GetNormalsName());
      preader->SetTensorsName(this->Reader->GetTensorsName());
      preader->SetTCoordsName(this->Reader->GetTCoordsName());
      preader->SetLookupTableName(this->Reader->GetLookupTableName());
      preader->SetFieldDataName(this->Reader->GetFieldDataName());
      preader->Update();
      // whether we used the old output or not, we need to set the output.
      this->SetOutput(0, preader->GetOutput());
      preader->Delete();
      }

    else if ( ! strncmp(line,"structured_points",17) )
      {
      vtkStructuredPointsReader *preader = vtkStructuredPointsReader::New();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkStructuredPoints") == 0)
	{
	preader->SetOutput((vtkStructuredPoints *)(output));
	}
      preader->SetFileName(this->Reader->GetFileName());
      preader->SetInputString(this->Reader->GetInputString(),
			      this->Reader->GetInputStringLength());
      preader->SetReadFromInputString(this->Reader->GetReadFromInputString());
      preader->SetScalarsName(this->Reader->GetScalarsName());
      preader->SetVectorsName(this->Reader->GetVectorsName());
      preader->SetNormalsName(this->Reader->GetNormalsName());
      preader->SetTensorsName(this->Reader->GetTensorsName());
      preader->SetTCoordsName(this->Reader->GetTCoordsName());
      preader->SetLookupTableName(this->Reader->GetLookupTableName());
      preader->SetFieldDataName(this->Reader->GetFieldDataName());
      preader->Update();
      // whether we used the old output or not, we need to set the output.
      this->SetOutput(0, preader->GetOutput());
      preader->Delete();
      }

    else if ( ! strncmp(line,"structured_grid",15) )
      {
      vtkStructuredGridReader *preader = vtkStructuredGridReader::New();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkStructuredGrid") == 0)
	{
	preader->SetOutput((vtkStructuredGrid *)(output));
	}
      preader->SetFileName(this->Reader->GetFileName());
      preader->SetInputString(this->Reader->GetInputString(),
			      this->Reader->GetInputStringLength());
      preader->SetReadFromInputString(this->Reader->GetReadFromInputString());
      preader->SetScalarsName(this->Reader->GetScalarsName());
      preader->SetVectorsName(this->Reader->GetVectorsName());
      preader->SetNormalsName(this->Reader->GetNormalsName());
      preader->SetTensorsName(this->Reader->GetTensorsName());
      preader->SetTCoordsName(this->Reader->GetTCoordsName());
      preader->SetLookupTableName(this->Reader->GetLookupTableName());
      preader->SetFieldDataName(this->Reader->GetFieldDataName());
      preader->Update();
      // whether we used the old output or not, we need to set the output.
      this->SetOutput(0, preader->GetOutput());
      preader->Delete();
      }

    else if ( ! strncmp(line,"rectilinear_grid",16) )
      {
      vtkRectilinearGridReader *preader = vtkRectilinearGridReader::New();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkRectilinearGrid") == 0)
	{
	preader->SetOutput((vtkRectilinearGrid *)(output));
	}
      preader->SetFileName(this->Reader->GetFileName());
      preader->SetInputString(this->Reader->GetInputString());
      preader->SetInputString(this->Reader->GetInputString(),
			      this->Reader->GetInputStringLength());
      preader->SetScalarsName(this->Reader->GetScalarsName());
      preader->SetVectorsName(this->Reader->GetVectorsName());
      preader->SetNormalsName(this->Reader->GetNormalsName());
      preader->SetTensorsName(this->Reader->GetTensorsName());
      preader->SetTCoordsName(this->Reader->GetTCoordsName());
      preader->SetLookupTableName(this->Reader->GetLookupTableName());
      preader->SetFieldDataName(this->Reader->GetFieldDataName());
      preader->Update();
      // whether we used the old output or not, we need to set the output.
      this->SetOutput(0, preader->GetOutput());
      preader->Delete();
      }

    else if ( ! strncmp(line,"unstructured_grid",17) )
      {
      vtkUnstructuredGridReader *preader = vtkUnstructuredGridReader::New();
      // Can we use the old output?
      output = this->Outputs ? this->Outputs[0] : NULL;
      if (output && strcmp(output->GetClassName(), "vtkUnstructuredGrid") == 0)
	{
	preader->SetOutput((vtkUnstructuredGrid *)(output));
	}
      preader->SetFileName(this->Reader->GetFileName());
      preader->SetInputString(this->Reader->GetInputString());
      preader->SetInputString(this->Reader->GetInputString(),
			      this->Reader->GetInputStringLength());
      preader->SetScalarsName(this->Reader->GetScalarsName());
      preader->SetVectorsName(this->Reader->GetVectorsName());
      preader->SetNormalsName(this->Reader->GetNormalsName());
      preader->SetTensorsName(this->Reader->GetTensorsName());
      preader->SetTCoordsName(this->Reader->GetTCoordsName());
      preader->SetLookupTableName(this->Reader->GetLookupTableName());
      preader->SetFieldDataName(this->Reader->GetFieldDataName());
      preader->Update();
      // whether we used the old output or not, we need to set the output.
      this->SetOutput(0, preader->GetOutput());
      preader->Delete();
      }
    
    else
      {
      vtkErrorMacro(<< "Cannot read dataset type: " << line);
      return;
      }
    }

  else if ( !strncmp(this->Reader->LowerCase(line),"field",(unsigned long)5) )
    {
    vtkErrorMacro(<<"This object can only read datasets, not fields");
    }
  
  else
    {
    vtkErrorMacro(<<"Expecting DATASET keyword, got " << line << " instead");
    }
  

  return;
}

//----------------------------------------------------------------------------
void vtkDataSetReader::Update()
{
  if (this->GetOutput())
    {
    this->GetOutput()->Update();
    }
  else
    {
    // execute creates an output
    this->InternalUpdate(NULL);
    }
}

static int recursing = 0;
void vtkDataSetReader::PrintSelf(ostream& os, vtkIndent indent)
{
  // the reader ivar's source will be this reader. we must do this to prevent infinite printing
  if (!recursing)
    { 
    vtkSource::PrintSelf(os,indent);
    recursing = 1;
    os << indent << "Reader:\n";
    this->Reader->PrintSelf(os,indent.GetNextIndent());
    }
    recursing = 0;
}
