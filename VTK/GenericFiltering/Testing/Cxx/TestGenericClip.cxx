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
// This example demonstrates how to implement a vtkGenericDataSet
// (here vtkBridgeDataSet) and to use vtkGenericDataSetTessellator filter on
// it.
// 
// The command line arguments are:
// -I        => run in interactive mode; unless this is used, the program will
//              not allow interaction and exit
// -D <path> => path to the data; the data should be in <path>/Data/

#include "vtkActor.h"
#include "vtkDebugLeaks.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkTestUtilities.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkBridgeDataSet.h"
#include "vtkGenericClip.h"
#include "vtkGenericCellTessellator.h"
#include "vtkGenericSubdivisionErrorMetric.h"
#include <assert.h>
#include "vtkLookupTable.h"
#include "vtkDataSetMapper.h"
#include "vtkPolyData.h"
#include "vtkPlane.h"
#include "vtkGeometricErrorMetric.h"
#include "vtkAttributesErrorMetric.h"
#include "vtkSimpleCellTessellator.h"

int TestGenericClip(int argc, char* argv[])
{
  // Disable for testing
  vtkDebugLeaks::PromptUserOff();

  // Standard rendering classes
  vtkRenderer *renderer = vtkRenderer::New();
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->AddRenderer(renderer);
  vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
  iren->SetRenderWindow(renWin);

  // Load the mesh geometry and data from a file
  vtkXMLUnstructuredGridReader *reader = vtkXMLUnstructuredGridReader::New();
  char *cfname = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/quadraticTetra01.vtu");
  
//  char *cfname = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/Test2_Volume.vtu");
  reader->SetFileName( cfname );
  delete[] cfname;
  
  // Force reading
  reader->Update();

  // Initialize the bridge
  vtkBridgeDataSet *ds=vtkBridgeDataSet::New();
  ds->SetDataSet( reader->GetOutput() );
  reader->Delete();
  
  // Set the error metric thresholds:
  // 1. for the geometric error metric
  vtkGeometricErrorMetric *geometricError=vtkGeometricErrorMetric::New();
  geometricError->SetRelativeGeometricTolerance(0.01,ds);
  
  ds->GetTessellator()->GetErrorMetrics()->AddItem(geometricError);
  geometricError->Delete();
  
  // 2. for the attribute error metric
  vtkAttributesErrorMetric *attributesError=vtkAttributesErrorMetric::New();
  attributesError->SetAttributeTolerance(0.01);
  
  ds->GetTessellator()->GetErrorMetrics()->AddItem(attributesError);
  attributesError->Delete();
  
  cout<<"input unstructured grid: "<<ds<<endl;
  
  static_cast<vtkSimpleCellTessellator *>(ds->GetTessellator())->SetMaxSubdivisionLevel(100);

  vtkIndent indent;
  ds->PrintSelf(cout,indent);
  
  // Create the filter
  
  vtkPlane *implicitPlane = vtkPlane::New();
  implicitPlane->SetOrigin(0.5, 0, 0);
  implicitPlane->SetNormal(1, 1, 1);
  
  vtkGenericClip *clipper = vtkGenericClip::New();
  clipper->SetInput(ds);
  
  clipper->SetClipFunction(implicitPlane);
  implicitPlane->Delete();
  clipper->SetValue( 0.5 );
  clipper->SetInsideOut(1);
  
  clipper->Update(); //So that we can call GetRange() on the scalars
  
  assert(clipper->GetOutput()!=0);
  
  // This creates a blue to red lut.
  vtkLookupTable *lut = vtkLookupTable::New(); 
  lut->SetHueRange (0.667, 0.0);
  
  vtkDataSetMapper *mapper = vtkDataSetMapper::New();
  mapper->SetLookupTable(lut);
  mapper->SetInput( clipper->GetOutput() );
  
  if(clipper->GetOutput()->GetPointData()!=0)
    {
    if(clipper->GetOutput()->GetPointData()->GetScalars()!=0)
      {
      mapper->SetScalarRange( clipper->GetOutput()->GetPointData()->
                              GetScalars()->GetRange());
      }
    }
  
  vtkActor *actor = vtkActor::New();
  actor->SetMapper(mapper);
  renderer->AddActor(actor);
  
  // Standard testing code.
  renderer->SetBackground(0.5,0.5,0.5);
  renWin->SetSize(300,300);
  renWin->Render();
  int retVal = vtkRegressionTestImage( renWin );
  if ( retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    }

  // Cleanup
  renderer->Delete();
  renWin->Delete();
  iren->Delete();
  mapper->Delete();
  actor->Delete();
  clipper->Delete();
  ds->Delete();
  lut->Delete();
  
  return !retVal;
}
