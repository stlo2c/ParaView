/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#ifndef pqPropertyHelper_h
#define pqPropertyHelper_h

#include "OverViewCoreExport.h"
#include <vtkSMPropertyHelper.h>

#include <QStringList>

class vtkStdString;

/// Helper object to simplify working with properties
class OVERVIEW_CORE_EXPORT pqPropertyHelper :
  public vtkSMPropertyHelper
{
public:
  pqPropertyHelper(vtkSMProxy* proxy, const char* name);

  // Include all the superclass's Set methods into this subclass.
  // Without this, all the other Set overrides are hidden.
  using vtkSMPropertyHelper::Set;

  void Set(const vtkStdString& value);
  void Set(unsigned int index, const vtkStdString& value);

  void Set(const QString& value);
  void Set(unsigned int index, const QString& value);
  void Set(const QStringList& value);

  const QString GetAsString(unsigned int index = 0);
  const QStringList GetAsStringList();
};

#endif // !pqPropertyHelper_h

