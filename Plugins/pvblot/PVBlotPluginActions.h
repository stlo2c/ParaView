// -*- c++ -*-
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
/*-------------------------------------------------------------------------
  Copyright 2009 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#ifndef __PVBlotPluginActions_h
#define __PVBlotPluginActions_h

#include <QActionGroup>

class pqBlotDialog;
class pqServer;

/// Adds an action for starting the PVBlot interpreter window.
class PVBlotPluginActions : public QActionGroup
{
  Q_OBJECT;
public:
  PVBlotPluginActions(QObject *p);

  virtual pqServer *activeServer();
  virtual QWidget *mainWindow();

public slots:
  virtual void startPVBlot();
  virtual void startPVBlot(const QString &filename);

protected slots:
  virtual void startPVBlot(const QStringList &filenames);

private:
  PVBlotPluginActions(const PVBlotPluginActions &);     // Not implemented
  void operator=(const PVBlotPluginActions &);          // Not implemented
};

#endif //__PVBlotPluginActions_h