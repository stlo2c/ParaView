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
#include "vtkSMProxyGroupDomain.h"

#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSourceProxy.h"

#include <vtkstd/vector>

#include "vtkStdString.h"

vtkStandardNewMacro(vtkSMProxyGroupDomain);
vtkCxxRevisionMacro(vtkSMProxyGroupDomain, "$Revision$");

struct vtkSMProxyGroupDomainInternals
{
  vtkstd::vector<vtkStdString> Groups;
};

//---------------------------------------------------------------------------
vtkSMProxyGroupDomain::vtkSMProxyGroupDomain()
{
  this->PGInternals = new vtkSMProxyGroupDomainInternals;
}

//---------------------------------------------------------------------------
vtkSMProxyGroupDomain::~vtkSMProxyGroupDomain()
{
  delete this->PGInternals;
}

//---------------------------------------------------------------------------
int vtkSMProxyGroupDomain::IsInDomain(vtkSMProperty* property)
{
  if (!property)
    {
    return 0;
    }
  vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(property);
  if (pp)
    {
    unsigned int numMatches = 0;
    unsigned int numProxies = pp->GetNumberOfProxies();
    for (unsigned int i=0; i<numProxies; i++)
      {
      if (!this->IsInDomain(pp->GetProxy(i)))
        {
        return 0;
        }
      }
    return 1;
    }

  return 0;
}

//---------------------------------------------------------------------------
int vtkSMProxyGroupDomain::IsInDomain(vtkSMProxy* proxy)
{
  if (!proxy)
    {
    return 0;
    }

  vtkSMProxyManager* pm = this->GetProxyManager();
  if (pm)
    {
    vtkstd::vector<vtkStdString>::iterator it = 
      this->PGInternals->Groups.begin();
    for (; it != this->PGInternals->Groups.end(); it++)
      {
      if (pm->IsProxyInGroup(proxy, it->c_str()))
        {
        return 1;
        }
      }
    }
  return 0;
}

//---------------------------------------------------------------------------
void vtkSMProxyGroupDomain::AddGroup(const char* group)
{
  this->PGInternals->Groups.push_back(group);
}

//---------------------------------------------------------------------------
unsigned int vtkSMProxyGroupDomain::GetNumberOfGroups()
{
  return this->PGInternals->Groups.size();
}

//---------------------------------------------------------------------------
const char* vtkSMProxyGroupDomain::GetGroup(unsigned int idx)
{
  return this->PGInternals->Groups[idx].c_str();
}

//---------------------------------------------------------------------------
int vtkSMProxyGroupDomain::ReadXMLAttributes(vtkPVXMLElement* element)
{
  int found=0;
  for(unsigned int i=0; i < element->GetNumberOfNestedElements(); ++i)
    {
    vtkPVXMLElement* groupElement = element->GetNestedElement(i);
    if (strcmp(groupElement->GetName(), "Group")==0)
      {
      const char* name = groupElement->GetAttribute("name");
      if (name)
        {
        this->AddGroup(name);
        found = 1;
        }
      }
    }
    
  if (!found)
    {
    vtkErrorMacro("Required element \"Group\" (with a name attribute) "
                  "was not found.");
    return 0;
    }

  return 1;
}

//---------------------------------------------------------------------------
void vtkSMProxyGroupDomain::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

}
