/**************************************************************************\
 *
 *  This file is part of the SIM Quarter extension library for Coin.
 *  Copyright (C) 1998-2007 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License ("GPL") version 2
 *  as published by the Free Software Foundation.  See the file COPYING
 *  at the root directory of this source distribution for additional
 *  information about the GNU GPL.
 *
 *  For using SIM Quarter with software that can not be combined with
 *  the GNU GPL, and for taking advantage of the additional benefits of
 *  our support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org/> for more information.
 *
 *  Systems in Motion AS, Bygd�y all� 5, N-0257 Oslo, NORWAY. (www.sim.no)
 *
\**************************************************************************/

/*!
  \class SIM::Coin3D::Quarter::DragDropHandler DragDropHandler.h Quarter/devices/DragDropHandler.h

  \brief The DragDropHandler class provides drag and drop
  functionality to the QuarterWidget. It is not registered with the
  DeviceManager by default.
*/

#include <Quarter/devices/DragDropHandler.h>

#include <QUrl>
#include <QFileInfo>
#include <QStringList>
#include <QDragEnterEvent>
#include <QDropEvent>

#include <Inventor/SoInput.h>
#include <Inventor/nodes/SoSeparator.h>

#include <Quarter/QuarterWidget.h>
#include <Quarter/devices/DeviceManager.h>
#include <stdlib.h>

namespace SIM { namespace Coin3D { namespace Quarter {

class DragDropHandlerP {
public:
  DragDropHandlerP(DragDropHandler * master) {
    this->master = master;
  }
  void dragEnterEvent(QDragEnterEvent * event);
  void dropEvent(QDropEvent * event);
  
  QStringList suffixes;
  DragDropHandler * master;
};

}}} // namespace

#define PRIVATE(obj) obj->pimpl
#define PUBLIC(obj) obj->master

using namespace SIM::Coin3D::Quarter;

DragDropHandler::DragDropHandler(void)
{
  PRIVATE(this) = new DragDropHandlerP(this);
  PRIVATE(this)->suffixes << "iv" << "wrl";
}

DragDropHandler::~DragDropHandler()
{
  delete PRIVATE(this);
}

/*! Detects a QDragEnterEvent and if the event is the dropping of a
  valid Inventor or VRML it opens the file, reads in the scenegraph
  and calls setSceneGraph on the QuarterWidget
 */
const SoEvent * 
DragDropHandler::translateEvent(QEvent * event)
{
  switch (event->type()) {
  case QEvent::DragEnter:
    PRIVATE(this)->dragEnterEvent((QDragEnterEvent *) event);
    break;
  case QEvent::Drop:
    PRIVATE(this)->dropEvent((QDropEvent *) event);
    break;
  default:
    break;
  }
  return NULL;
}
  
void 
DragDropHandlerP::dragEnterEvent(QDragEnterEvent * event)
{
  const QMimeData * mimedata = event->mimeData();
  if (!mimedata->hasUrls() & !mimedata->hasText()) return;

  if (mimedata->hasUrls()) { 
    QFileInfo fileinfo(mimedata->urls().takeFirst().path());
    QString suffix = fileinfo.suffix().toLower();
    if (!this->suffixes.contains(suffix)) { return; }
  }
  
  event->acceptProposedAction();
}

void 
DragDropHandlerP::dropEvent(QDropEvent * event)
{
  const QMimeData * mimedata = event->mimeData();
  
  SoSeparator * root;
  SoInput in;
  
  if (mimedata->hasUrls()) { 
    QString path = mimedata->urls().takeFirst().path();
    // attempt to open file
    if (!in.openFile(path.toLatin1().constData())) return;

  } else if (mimedata->hasText()) {
    QByteArray bytes = mimedata->text().toUtf8();
    in.setBuffer((void *) bytes.constData(), bytes.size());
    if (!in.isValidBuffer()) return;
  }

  // attempt to import it
  root = SoDB::readAll(&in);
  if (root == NULL) return;
  
  // get QuarterWidget and set new scenegraph
  QuarterWidget * quarterwidget = (QuarterWidget *) PUBLIC(this)->manager->getWidget();
  quarterwidget->setSceneGraph(root);
  quarterwidget->updateGL();
}

#undef PRIVATE
#undef PUBLIC
