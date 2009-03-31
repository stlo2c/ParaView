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
#include "vtkSMLineChartSeriesOptionsProxy.h"

#include "vtkObjectFactory.h"
#include "vtkQtLineChartSeriesOptions.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkStringList.h"
#include "vtkQtChartNamedSeriesOptionsModel.h"

#include <QColor>
#include <QPen>

vtkStandardNewMacro(vtkSMLineChartSeriesOptionsProxy);
vtkCxxRevisionMacro(vtkSMLineChartSeriesOptionsProxy, "$Revision$");
//----------------------------------------------------------------------------
vtkSMLineChartSeriesOptionsProxy::vtkSMLineChartSeriesOptionsProxy()
{
}

//----------------------------------------------------------------------------
vtkSMLineChartSeriesOptionsProxy::~vtkSMLineChartSeriesOptionsProxy()
{
}

//----------------------------------------------------------------------------
vtkQtChartSeriesOptions* vtkSMLineChartSeriesOptionsProxy::NewOptions()
{
  return new vtkQtLineChartSeriesOptions();
}

//----------------------------------------------------------------------------
void vtkSMLineChartSeriesOptionsProxy::SetAxisCorner(
  const char* name, int value)
{
  vtkQtLineChartSeriesOptions* options = 
    qobject_cast<vtkQtLineChartSeriesOptions*>(
      this->GetOptions(name));
  if (options)
    {
    options->setAxesCorner(static_cast<vtkQtChartLayer::AxesCorner>(value));
    }
}

//----------------------------------------------------------------------------
void vtkSMLineChartSeriesOptionsProxy::SetMarkerStyle(
  const char* name, int value)
{
  vtkQtLineChartSeriesOptions* options = 
    qobject_cast<vtkQtLineChartSeriesOptions*>(
      this->GetOptions(name));
  if (options)
    {
    options->setMarkerStyle(
      static_cast<vtkQtPointMarker::MarkerStyle>(value));
    }
}

//----------------------------------------------------------------------------
void vtkSMLineChartSeriesOptionsProxy::SetColor(
  const char* name, double r, double g, double b)
{
  vtkQtChartSeriesOptions* options = this->GetOptions(name);
  QPen pen = options->getPen();
  pen.setColor(QColor::fromRgbF(r, g, b));
  options->setPen(pen);
}

//----------------------------------------------------------------------------
void vtkSMLineChartSeriesOptionsProxy::UpdatePropertyInformationInternal(
  vtkSMProperty* prop)
{
  this->Superclass::UpdatePropertyInformationInternal(prop);

  vtkSMStringVectorProperty* svp =
    vtkSMStringVectorProperty::SafeDownCast(prop);
  if (svp && svp->GetInformationOnly())
    {
    vtkStringList* new_values = vtkStringList::New();
    vtkQtChartNamedSeriesOptionsModel* optionsModel = this->GetOptionsModel();
    int num_options = optionsModel->getNumberOfOptions();
    const char* propname = this->GetPropertyName(prop);
    bool skip = false;
    for (int cc=0; cc < num_options; cc++)
      {
      QString name = optionsModel->getSeriesName(cc);
      vtkQtLineChartSeriesOptions* options =
        qobject_cast<vtkQtLineChartSeriesOptions*>(
          this->GetOptions(name.toAscii().data()));
      if (options && strcmp(propname, "MarkerStyleInfo") == 0)
        {
        new_values->AddString(name.toAscii().data());
        new_values->AddString(QString::number(
            static_cast<int>(options->getMarkerStyle())).toAscii().data());
        }
      else if (options && strcmp(propname, "ColorInfo") == 0)
        {
        new_values->AddString(name.toAscii().data());

        QPen pen = options->getPen();
        new_values->AddString(QString::number(pen.color().redF()).toAscii().data());
        new_values->AddString(QString::number(pen.color().greenF()).toAscii().data());
        new_values->AddString(QString::number(pen.color().blueF()).toAscii().data());
        }
      else
        {
        skip = true;
        break;
        }
      }
    if (!skip)
      {
      svp->SetElements(new_values);
      }
    new_values->Delete();
    }
}


//----------------------------------------------------------------------------
void vtkSMLineChartSeriesOptionsProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


