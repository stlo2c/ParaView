/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.1. 

   See License_v1.1.txt for the full ParaView license.
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

#include "pqTreeWidget.h"

#include <QApplication>
#include <QPainter>
#include <QStyle>
#include <QTimer>
#include <QHeaderView>
#include <QLayout>

// enum for different pixmap types
enum pqTreeWidgetPixmap
{
  pqCheck               = 0,
  pqPartialCheck        = 1,
  pqUnCheck             = 2,
  
  // All active states in lower half
  pqCheck_Active        = 3,
  pqPartialCheck_Active = 4,
  pqUnCheck_Active      = 5,
 
  pqMaxCheck            = 6
};

// array of style corresponding with the pqTreeWidgetPixmap enum
static const QStyle::State pqTreeWidgetPixmapStyle[] =
{
  QStyle::State_On | QStyle::State_Enabled,
  QStyle::State_NoChange | QStyle::State_Enabled,
  QStyle::State_Off | QStyle::State_Enabled,
  QStyle::State_On | QStyle::State_Enabled | QStyle::State_Active,
  QStyle::State_NoChange | QStyle::State_Enabled | QStyle::State_Active,
  QStyle::State_Off | QStyle::State_Enabled | QStyle::State_Active
};

QPixmap pqTreeWidget::pixmap(Qt::CheckState cs, bool active)
{
  int offset = active ? pqMaxCheck/2 : 0;
  switch(cs)
    {
  case Qt::Checked:
    return *this->CheckPixmaps[offset + pqCheck];
  case Qt::Unchecked:
    return *this->CheckPixmaps[offset + pqUnCheck];
  case Qt::PartiallyChecked:
    return *this->CheckPixmaps[offset + pqPartialCheck];
    }
  return QPixmap();
}

pqTreeWidget::pqTreeWidget(QWidget* p)
  : QTreeWidget(p)
{
  QStyleOptionButton option;
  QRect r = this->style()->subElementRect(QStyle::SE_CheckBoxIndicator, 
                                          &option, this);
  option.rect = QRect(QPoint(0,0), r.size());

  this->CheckPixmaps = new QPixmap*[6];
  for(int i=0; i<pqMaxCheck; i++)
    {
    this->CheckPixmaps[i] = new QPixmap(r.size());
    this->CheckPixmaps[i]->fill(QColor(0,0,0,0));
    QPainter painter(this->CheckPixmaps[i]);
    option.state = pqTreeWidgetPixmapStyle[i];
    
    this->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, 
                         &painter, this);
    }
  
  QObject::connect(this->header(), SIGNAL(sectionClicked(int)),
                   this, SLOT(doToggle(int)),
                   Qt::QueuedConnection);
  
  this->header()->setClickable(true);

  QObject::connect(this->model(), SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                   this, SLOT(updateCheckState()));
  QObject::connect(this->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                   this, SLOT(updateCheckState()));

  QObject::connect(this->model(), SIGNAL(rowsInserted(QModelIndex, int, int)),
                   this, SLOT(invalidateLayout()));
  QObject::connect(this->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)),
                   this, SLOT(invalidateLayout()));
  QObject::connect(this->model(), SIGNAL(modelReset()),
                   this, SLOT(invalidateLayout()));
}

pqTreeWidget::~pqTreeWidget()
{
  for(int i=0; i<pqMaxCheck; i++)
    {
    delete this->CheckPixmaps[i];
    }
  delete [] this->CheckPixmaps;
}

bool pqTreeWidget::event(QEvent* e)
{
  if(e->type() == QEvent::FocusIn ||
     e->type() == QEvent::FocusOut)
    {
    bool convert = false;
    int cs = this->headerItem()->data(0, Qt::CheckStateRole).toInt(&convert);
    if(convert)
      {
      bool active = e->type() == QEvent::FocusIn;
      this->headerItem()->setData(0, Qt::DecorationRole, 
                            pixmap(static_cast<Qt::CheckState>(cs), active));
      }
    }

  return Superclass::event(e);
}

void pqTreeWidget::updateCheckState()
{
  Qt::CheckState newState = Qt::Checked;
  int numChecked = 0;
  int numPartial = 0;
  int numUnchecked = 0;
  QAbstractItemModel* m = this->model();
  int numRows = m->rowCount(QModelIndex());
  for(int i=0; i<numRows; i++)
    {
    QModelIndex idx = m->index(i, 0);
    bool convert = 0;
    int v = m->data(idx, Qt::CheckStateRole).toInt(&convert);
    if(convert)
      {
      if(v == Qt::Checked)
        {
        numChecked++;
        }
      else if (v == Qt::PartiallyChecked)
        { 
        numPartial++;
        }
      else
        {
        numUnchecked++;
        }
      }
    }

  // if there are no check boxes at all
  if(0 == (numUnchecked + numPartial + numChecked))
    {
    return;
    }
  
  if(numChecked != numRows)
    {
      newState = ((numChecked==0) && (numPartial==0)) 
        ? Qt::Unchecked : Qt::PartiallyChecked;
    }

  this->headerItem()->setCheckState(0, newState);
  this->headerItem()->setData(0, Qt::DecorationRole, 
                              pixmap(newState, this->hasFocus()));
}

void pqTreeWidget::allOn()
{
  QTreeWidgetItem* item;
  int i, end = this->topLevelItemCount();
  for(i=0; i<end; i++)
    {
    item = this->topLevelItem(i);
    item->setCheckState(0, Qt::Checked);
    }
}

void pqTreeWidget::allOff()
{
  QTreeWidgetItem* item;
  int i, end = this->topLevelItemCount();
  for(i=0; i<end; i++)
    {
    item = this->topLevelItem(i);
    item->setCheckState(0, Qt::Unchecked);
    }
}


void pqTreeWidget::doToggle(int column)
{
  if(column == 0)
    {
    bool convert = false;
    int cs = this->headerItem()->data(0, Qt::CheckStateRole).toInt(&convert);
    if(convert)
      {
      if(cs == Qt::Checked)
        {
        this->allOff();
        }
      else
        {
        // both unchecked and partial checked go here
        this->allOn();
        }
      }
    }
}

QSize pqTreeWidget::sizeHint() const
{
  // lets show X items before we get a scrollbar
  // probably want to make this a member variable
  // that a caller has access to
  int maxItemHint = 10;
  // for no items, let's give a space of X pixels
  int minItemHeight = 20;

  int num = this->topLevelItemCount();
  num = qMin(num, maxItemHint);
  
  int pix = minItemHeight;

  if(num)
    {
    pix = qMax(pix, this->sizeHintForRow(0) * num);
    }

  int margin[4];
  this->getContentsMargins(margin, margin+1, margin+2, margin+3);
  int h = pix + margin[1] + margin[3] + this->header()->frameSize().height();
  return QSize(156, h);
}

QSize pqTreeWidget::minimumSizeHint() const
{
  return this->sizeHint();
}

void pqTreeWidget::invalidateLayout()
{
  // sizeHint is dynamic, so we need to invalidate parent layouts
  // when items are added or removed
  for(QWidget* w = this->parentWidget();
      w && w->layout();
      w = w->parentWidget())
    {
    w->layout()->invalidate();
    }
}

