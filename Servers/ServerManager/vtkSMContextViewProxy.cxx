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
#include "vtkSMContextViewProxy.h"

#include "vtkContextView.h"

#include "vtkObjectFactory.h"
#include "vtkQtChartInteractorSetup.h"
#include "vtkQtChartMouseSelection.h"
#include "vtkQtChartWidget.h"
#include "vtkQtChartView.h"
#include "vtkSMChartOptionsProxy.h"

#include "QVTKWidget.h"
#include <QPointer>

//-----------------------------------------------------------------------------
// Minimal storage class for STL containers etc.
class vtkSMContextViewProxy::Private
{
public:
  Private() { this->Widget = new QVTKWidget; }
  ~Private()
  {
    if (this->Widget)
      {
      delete this->Widget;
      this->Widget = NULL;
      }
  }

  QPointer<QVTKWidget> Widget;
};


vtkCxxRevisionMacro(vtkSMContextViewProxy, "$Revision$");
//----------------------------------------------------------------------------
vtkSMContextViewProxy::vtkSMContextViewProxy()
{
  this->ChartView = NULL;
  this->Storage = NULL;
}

//----------------------------------------------------------------------------
vtkSMContextViewProxy::~vtkSMContextViewProxy()
{
  if (this->ChartView)
    {
    this->ChartView->Delete();
    this->ChartView = NULL;
    }
  delete this->Storage;
  this->Storage = NULL;
}

//----------------------------------------------------------------------------
void vtkSMContextViewProxy::CreateVTKObjects()
{
  if (this->ObjectsCreated)
    {
    return;
    }

  this->Storage = new Private;
  this->ChartView = vtkContextView::New();
  this->ChartView->SetInteractor(this->Storage->Widget->GetInteractor());
  this->Storage->Widget->SetRenderWindow(this->ChartView->GetRenderWindow());

  this->NewChartView();

  this->Superclass::CreateVTKObjects();
}

//----------------------------------------------------------------------------
QVTKWidget* vtkSMContextViewProxy::GetChartWidget()
{
  return this->Storage->Widget;
}

//----------------------------------------------------------------------------
vtkContextView* vtkSMContextViewProxy::GetChartView()
{
  return this->ChartView;
}

//----------------------------------------------------------------------------
bool vtkSMContextViewProxy::WriteImage(const char*)
{
//  this->PerformRender();
//  return this->ChartView->SaveImage(filename);
  return false;
}

//----------------------------------------------------------------------------
void vtkSMContextViewProxy::PerformRender()
{
  int size[2];
  this->GetGUISize(size);
  cout << "Perform render called in ContextViewProxy." << endl;
}

//----------------------------------------------------------------------------
void vtkSMContextViewProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


