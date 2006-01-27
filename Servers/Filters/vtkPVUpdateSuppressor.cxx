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
#include "vtkPVUpdateSuppressor.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkCollection.h"
#include "vtkCommand.h"
#include "vtkDataSet.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSource.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUpdateSuppressorPipeline.h"

// Only required for the prototype streaming feature.
#include "vtkPolyDataStreamer.h"

vtkCxxRevisionMacro(vtkPVUpdateSuppressor, "$Revision$");
vtkStandardNewMacro(vtkPVUpdateSuppressor);

//----------------------------------------------------------------------------
vtkPVUpdateSuppressor::vtkPVUpdateSuppressor()
{
  this->UpdatePiece = 0;
  this->UpdateNumberOfPieces = 1;

  this->CachedGeometry = NULL;
  this->CachedGeometryLength = 0;

  this->OutputType = 0;
  this->PreviousUpdateWasBlockedByStreaming = 0;  
}

//----------------------------------------------------------------------------
vtkPVUpdateSuppressor::~vtkPVUpdateSuppressor()
{
  this->RemoveAllCaches();
  this->SetOutputType(0);
}

//----------------------------------------------------------------------------
void vtkPVUpdateSuppressor::ForceUpdate()
{  
  vtkDataSet *input = vtkDataSet::SafeDownCast(this->GetInput());
  if (input == 0)
    {
    vtkErrorMacro("No valid input.");
    return;
    }
  input->UpdateInformation();
  input =  vtkDataSet::SafeDownCast(this->GetInput());

  // Make sure that output type matches input type
  this->UpdateInformation();
  vtkDataSet *output = this->GetOutput();

  // int fixme; // I do not like this hack.  How can we get rid of it?
  // Assume the input is the collection filter.
  // Client needs to modify the collection filter because it is not
  // connected to a pipeline.
  vtkAlgorithm *source = input->GetProducerPort()->GetProducer();
  if (source &&
      (source->IsA("vtkMPIMoveData") ||
       source->IsA("vtkCollectPolyData") ||
       source->IsA("vtkMPIDuplicatePolyData") ||
       source->IsA("vtkM2NDuplicate") ||
       source->IsA("vtkM2NCollect") ||
       source->IsA("vtkMPIDuplicateUnstructuredGrid") ||
       source->IsA("vtkPVDuplicatePolyData") ||
       source->IsA("vtkOrderedCompositeDistributor")))
    {
    source->Modified();
    }

  input->SetUpdatePiece(this->UpdatePiece);
  input->SetUpdateNumberOfPieces(this->UpdateNumberOfPieces);
  input->SetUpdateGhostLevel(0);

  // This conditional is only used for the temporary streaming feature.
  // It can be removed when the streaming feature is removed.
  vtkPolyData *pInput = vtkPolyData::SafeDownCast(input);
  // TODO: For now I am disabling Streaming.
  if (/*vtkPVProcessModule::GetGlobalStreamBlock()*/ 0)
    {
    this->PreviousUpdateWasBlockedByStreaming = 1;  
    input->SetUpdatePiece(this->UpdatePiece*200);
    input->SetUpdateNumberOfPieces(this->UpdateNumberOfPieces*200);
    input->Update();
    }
  else if (this->PreviousUpdateWasBlockedByStreaming  && pInput)
    { // stream
    vtkPolyDataStreamer* streamer = vtkPolyDataStreamer::New();
    streamer->SetInput(pInput);
    streamer->SetNumberOfStreamDivisions(200);
    streamer->Update();
    pInput->ShallowCopy(streamer->GetOutput());
    streamer->Delete();
    }
  else
    { // This would stay when removing streaming.
    input->Update();
    this->PreviousUpdateWasBlockedByStreaming = 0;
    }

  unsigned long t2 = 0;
  vtkDemandDrivenPipeline *ddp = 0;
  if (source)
    {
    ddp = vtkDemandDrivenPipeline::SafeDownCast(source->GetExecutive());
    }
  else
    {
    vtkInformation* pipInf = input->GetPipelineInformation();
    ddp = vtkDemandDrivenPipeline::SafeDownCast(
      pipInf->GetExecutive(vtkExecutive::PRODUCER()));
    }
  if (ddp)
    {
    ddp->UpdateInformation();
    t2 = ddp->GetPipelineMTime();
    }
  if (t2 > this->UpdateTime || output->GetDataReleased())
    {
    output->ShallowCopy(input);
    this->UpdateTime.Modified();
    }
}

//----------------------------------------------------------------------------
int vtkPVUpdateSuppressor::RequestDataObject(
  vtkInformation* request, 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  if (!this->OutputType)
    {
    return this->Superclass::RequestDataObject(
      request, inputVector, outputVector);
    }

  // for each output
  for(int i=0; i < this->GetNumberOfOutputPorts(); ++i)
    {
    vtkInformation* info = outputVector->GetInformationObject(i);
    vtkDataObject *output = info->Get(vtkDataObject::DATA_OBJECT());
    
    if (!output || !output->IsA(this->OutputType)) 
      {
      output = vtkDemandDrivenPipeline::NewDataObject(this->OutputType);
      if (!output)
        {
        return 0;
        }
      output->SetPipelineInformation(info);
      output->Delete();
      this->GetOutputPortInformation(0)->Set(
        vtkDataObject::DATA_EXTENT_TYPE(), output->GetExtentType());
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkPVUpdateSuppressor::RemoveAllCaches()
{
  int idx;

  for (idx = 0; idx < this->CachedGeometryLength; ++idx)
    {
    if (this->CachedGeometry[idx])
      {
      this->CachedGeometry[idx]->Delete();
      this->CachedGeometry[idx] = NULL;
      }
    }

  if (this->CachedGeometry)
    {
    delete [] this->CachedGeometry;
    this->CachedGeometry = NULL;
    }
  this->CachedGeometryLength = 0;
}

//----------------------------------------------------------------------------
void vtkPVUpdateSuppressor::CacheUpdate(int idx, int num)
{
  vtkDataSet *pd;
  vtkDataSet *output;
  int j;

  if (num == -1)
    {
    return;
    }

  if (idx < 0 || idx >= num)
    {
    vtkErrorMacro("Bad cache index: " << idx << " of " << num);
    return;
    }

  if (num != this->CachedGeometryLength)
    {
    this->RemoveAllCaches();
    this->CachedGeometry = new vtkDataSet*[num];
    for (j = 0; j < num; ++j)
      {
      this->CachedGeometry[j] = NULL;
      }
    this->CachedGeometryLength = num;
    }

  output = this->GetOutput();
  pd = this->CachedGeometry[idx];
  if (pd == NULL)
    { // we need to update and save.
    this->ForceUpdate();
    pd = output->NewInstance();
    pd->ShallowCopy(output);
    //  Compositing seems to update the input properly.
    //  But this update is needed when doing animation without compositing.
    pd->Update(); 
    this->CachedGeometry[idx] = pd;
    pd->Register(this);
    pd->Delete();
    }
  else
    { // Output generated previously.
    output->ShallowCopy(pd);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
vtkExecutive* vtkPVUpdateSuppressor::CreateDefaultExecutive()
{
  return vtkUpdateSuppressorPipeline::New();
}


//----------------------------------------------------------------------------
int vtkPVUpdateSuppressor::RequestData(vtkInformation *request,
                                       vtkInformationVector **inputVector,
                                       vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()) > 1)
    {
    input->SetUpdateNumberOfPieces(outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
    input->SetUpdatePiece(outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
    input->SetUpdateGhostLevel(outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS()));
    input->Update();
    output->ShallowCopy(input);
    return 1;
    }

  return this->Superclass::RequestData(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
void vtkPVUpdateSuppressor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "UpdatePiece: " << this->UpdatePiece << endl;
  os << indent << "UpdateNumberOfPieces: " << this->UpdateNumberOfPieces << endl;
  os << indent << "OutputType: " 
     << (this->OutputType?this->OutputType:"(none)")
     << endl;
         
}
