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
#include "vtkXMLPVAnimationWriter.h"

#include "vtkDataSet.h"
#include "vtkErrorCode.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLWriter.h"

#include <vtkstd/map>
#include <vtkstd/string>
#include <vtkstd/vector>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkXMLPVAnimationWriter);
vtkCxxRevisionMacro(vtkXMLPVAnimationWriter, "$Revision$");

//----------------------------------------------------------------------------
class vtkXMLPVAnimationWriterInternals
{
public:
  // The name of the group to which each input belongs.
  typedef vtkstd::vector<vtkstd::string> InputGroupNamesType;
  InputGroupNamesType InputGroupNames;
  
  // The part number each input has been assigned in its group.
  typedef vtkstd::vector<int> InputPartNumbersType;
  InputPartNumbersType InputPartNumbers;
  
  // The modified time when each input was last written in a previous
  // animation step.
  typedef vtkstd::vector<unsigned long> InputMTimesType;
  InputMTimesType InputMTimes;
  
  // The number of times each input has changed during this animation
  // sequence.
  typedef vtkstd::vector<int> InputChangeCountsType;
  InputChangeCountsType InputChangeCounts;
  
  // Count the number of parts in each group.
  typedef vtkstd::map<vtkstd::string, int> GroupMapType;
  GroupMapType GroupMap;
  
  // Create the file name for the given input during this animation
  // step.
  vtkstd::string CreateFileName(int index, const char* prefix,
                                const char* ext);
};

//----------------------------------------------------------------------------
vtkXMLPVAnimationWriter::vtkXMLPVAnimationWriter()
{
  this->Internal = new vtkXMLPVAnimationWriterInternals;
  this->StartCalled = 0;
  this->FinishCalled = 0;
  this->FileNamesCreated = 0;
  this->NumberOfFileNamesCreated = 0;
}

//----------------------------------------------------------------------------
vtkXMLPVAnimationWriter::~vtkXMLPVAnimationWriter()
{
  delete this->Internal;
  this->DeleteFileNames();
}

//----------------------------------------------------------------------------
void vtkXMLPVAnimationWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  if(this->GetNumberOfInputs() > 0)
    {
    os << indent << "Input Detail:\n";
    vtkIndent nextIndent = indent.GetNextIndent();
    int i;
    for(i=0; i < this->GetNumberOfInputs(); ++i)
      {
      os << nextIndent << i << ": group \""
         << this->Internal->InputGroupNames[i].c_str()
         << "\" part " << this->Internal->InputPartNumbers[i]
         << "\n";
      }
    }
}

//----------------------------------------------------------------------------
void vtkXMLPVAnimationWriter::AddInput(vtkDataSet* pd, const char* group)
{
  // Add the input to the pipeline structure.
  this->vtkProcessObject::AddInput(pd);
  
  // Find the part number for this input.
  int partNum = 0;
  vtkXMLPVAnimationWriterInternals::GroupMapType::iterator s =
    this->Internal->GroupMap.find(group);
  if(s != this->Internal->GroupMap.end())
    {
    partNum = s->second++;
    }
  else
    {
    vtkXMLPVAnimationWriterInternals::GroupMapType::value_type v(group, 1);
    this->Internal->GroupMap.insert(v);
    }
  this->Internal->InputPartNumbers.push_back(partNum);
  
  // Add the group name for this input.
  this->Internal->InputGroupNames.push_back(group);
  
  // Allocate the mtime table entry for this input.
  this->Internal->InputMTimes.push_back(0);
  
  // Allocate the change count entry for this input.
  this->Internal->InputChangeCounts.push_back(0);
}

//----------------------------------------------------------------------------
void vtkXMLPVAnimationWriter::AddInput(vtkDataSet* ds)
{
  this->AddInput(ds, "");
}

//----------------------------------------------------------------------------
void vtkXMLPVAnimationWriter::AddInput(vtkDataObject* d)
{
  this->Superclass::AddInput(d);
}

//----------------------------------------------------------------------------
void vtkXMLPVAnimationWriter::Start()
{
  // Do not allow double-start.
  if(this->StartCalled)
    {
    vtkErrorMacro("Cannot call Start() twice before calling Finish().");
    return;
    }
  
  // Make sure we have a file name.
  if(!this->FileName || !this->FileName[0])
    {
    vtkErrorMacro("No FileName has been set.");
    return;
    }
  
  // Initialize input change tables.
  int i;
  for(i=0; i < this->GetNumberOfInputs(); ++i)
    {
    this->Internal->InputMTimes[i] = 0;
    this->Internal->InputChangeCounts[i] = 0;
    }
  
  // Clear the animation entries from any previous run.
  this->DeleteAllEntries();
  
  // Clear the file names from any previous run.
  this->DeleteFileNames();
  
  // Split the file name into a directory and file prefix.
  this->SplitFileName();
  
  // Create a writer for each input.
  this->CreateWriters();
  
  // Create the subdirectory for the internal files.
  vtkstd::string subdir = this->GetFilePath();
  subdir += this->GetFilePrefix();
  this->MakeDirectory(subdir.c_str());
  
  this->StartCalled = 1;
}

//----------------------------------------------------------------------------
void vtkXMLPVAnimationWriter::WriteTime(double time)
{
  if(!this->StartCalled)
    {
    vtkErrorMacro("Must call Start() before WriteTime().");
    return;
    }
  
  // Consider every input.
  int i;
  for(i=0; i < this->GetNumberOfInputs(); ++i)
    {
    // Make sure the pipeline mtime is up to date.
    this->Inputs[i]->UpdateInformation();
    
    // If the input has been modified since the last animation step,
    // increment its file number.
    int changed = 0;
    if(this->Inputs[i]->GetPipelineMTime() > this->Internal->InputMTimes[i])
      {
      this->Internal->InputMTimes[i] = this->Inputs[i]->GetPipelineMTime();
      this->Internal->InputChangeCounts[i] += 1;
      changed = 1;
      }
    
    // Create this animation entry.
    vtkXMLWriter* writer = this->GetWriter(i);
    vtkstd::string fname =
      this->Internal->CreateFileName(i, this->GetFilePrefix(),
                                     writer->GetDefaultFileExtension());
    ostrstream entry_with_warning_C4701;
    entry_with_warning_C4701
      << "<DataSet timestep=\"" << time
      << "\" group=\"" << this->Internal->InputGroupNames[i].c_str()
      << "\" part=\"" << this->Internal->InputPartNumbers[i]
      << "\" file=\"" << fname.c_str()
      << "\"/>" << ends;
    this->AppendEntry(entry_with_warning_C4701.str());
    entry_with_warning_C4701.rdbuf()->freeze(0);
    
    // Write this step's file if its input has changed.
    if(changed)
      {
      vtkstd::string fullName = this->GetFilePath();
      fullName += fname;
      writer->SetFileName(fullName.c_str());
      this->AddFileName(fullName.c_str());
      writer->Write();
      if (writer->GetErrorCode() == vtkErrorCode::OutOfDiskSpaceError)
        {
        this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
        break;
        }
      }
    }
  
  if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
    {
    this->DeleteFiles();
    }
}

//----------------------------------------------------------------------------
void vtkXMLPVAnimationWriter::Finish()
{
  if(!this->StartCalled)
    {
    vtkErrorMacro("Must call Start() before Finish().");
    return;
    }
  
  this->StartCalled = 0;
  this->FinishCalled = 1;
  
  // Just write the output file with the current set of entries.
  this->Write();

  if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
    {
    this->DeleteFiles();
    }
}

//----------------------------------------------------------------------------
int vtkXMLPVAnimationWriter::WriteInternal()
{
  if(!this->FinishCalled)
    {
    vtkErrorMacro("Do not call Write() directly.  Call Finish() instead.");
    return 0;
    }
  
  this->FinishCalled = 0;
  
  // Write the animation file.
  return this->WriteCollectionFileIfRequested();
}

//----------------------------------------------------------------------------
vtkstd::string
vtkXMLPVAnimationWriterInternals::CreateFileName(int index,
                                                 const char* prefix,
                                                 const char* ext)
{ 
  // Start with the directory and file name prefix.
  ostrstream fn_with_warning_C4701;
  fn_with_warning_C4701 << prefix << "/" << prefix << "_";
  
  // Add the group name.
  fn_with_warning_C4701 << this->InputGroupNames[index].c_str();
  
  // Construct the part/time portion.  Add a part number if there is
  // more than one part in this group.
  char pt[100];  
  if(this->GroupMap[this->InputGroupNames[index]] > 1)
    {
    sprintf(pt, "P%02dT%04d",
            this->InputPartNumbers[index],
            this->InputChangeCounts[index]-1);
    }
  else
    {
    sprintf(pt, "T%04d", this->InputChangeCounts[index]-1);
    }
  fn_with_warning_C4701 << pt;
  
  // Add the file extension.
  fn_with_warning_C4701 << "." << ext << ends;
  
  // Return the result.
  vtkstd::string fname = fn_with_warning_C4701.str();
  fn_with_warning_C4701.rdbuf()->freeze(0);
  return fname;
}

void vtkXMLPVAnimationWriter::AddFileName(const char *fileName)
{
  int size = this->NumberOfFileNamesCreated;
  char **newFileNameList = new char *[size];
  
  int i;
  for (i = 0; i < size; i++)
    {
    newFileNameList[i] = new char[strlen(this->FileNamesCreated[i]) + 1];
    strcpy(newFileNameList[i], this->FileNamesCreated[i]);
    delete [] this->FileNamesCreated[i];
    }
  delete [] this->FileNamesCreated;
  
  this->FileNamesCreated = new char *[size+1];
  
  for (i = 0; i < size; i++)
    {
    this->FileNamesCreated[i] = new char[strlen(newFileNameList[i]) + 1];
    strcpy(this->FileNamesCreated[i], newFileNameList[i]);
    delete [] newFileNameList[i];
    }
  delete [] newFileNameList;
  
  this->FileNamesCreated[size] = new char[strlen(fileName) + 1];
  strcpy(this->FileNamesCreated[size], fileName);
  this->NumberOfFileNamesCreated++;
}

void vtkXMLPVAnimationWriter::DeleteFileNames()
{
  int i;
  if (this->FileNamesCreated)
    {
    for (i = 0; i < this->NumberOfFileNamesCreated; i++)
      {
      delete [] this->FileNamesCreated[i];
      }
    delete [] this->FileNamesCreated;
    this->FileNamesCreated = 0;
    }
  this->NumberOfFileNamesCreated = 0;
}

void vtkXMLPVAnimationWriter::DeleteFiles()
{
  for (int i = 0; i < this->NumberOfFileNamesCreated; i++)
    {
    this->DeleteFile(this->FileNamesCreated[i]);
    }
  this->DeleteFile(this->FileName);
  vtkstd::string subdir = this->GetFilePath();
  subdir += this->GetFilePrefix();
  this->RemoveDirectory(subdir.c_str());
}
