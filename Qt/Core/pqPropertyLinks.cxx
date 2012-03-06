/*=========================================================================

   Program: ParaView
   Module:    $RCSfile$

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
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

========================================================================*/
#include "pqPropertyLinks.h"

#include "pqPropertyLinksConnection.h"
#include "vtkSMProperty.h"
#include "vtkSMProxy.h"

#include <QMap>
#include <QtDebug>
#include <set>

class pqPropertyLinks::pqInternals
{
public:
  QList<QPointer<pqPropertyLinksConnection> > Connections;
  bool IgnoreSMChanges;
  pqInternals(): IgnoreSMChanges(false) { }
};

//-----------------------------------------------------------------------------
pqPropertyLinks::pqPropertyLinks(QObject* parentObject)
  : Superclass(parentObject),
  Internals(new pqPropertyLinks::pqInternals()),
  UseUncheckedProperties(false),
  AutoUpdateVTKObjects(true)
{
}

//-----------------------------------------------------------------------------
pqPropertyLinks::~pqPropertyLinks()
{
  // this really isn't necessary, but no harm done.
  this->clear();

  delete this->Internals;
  this->Internals = 0;
}

//-----------------------------------------------------------------------------
void pqPropertyLinks::setUseUncheckedProperties(bool val)
{
  if(val == this->UseUncheckedProperties)
    {
    // nothing to change
    return;
    }

  foreach(pqPropertyLinksConnection *connection, this->Internals->Connections)
    {
    connection->setUseUncheckedProperties(val);
/*
    // do not emit modifed signals from the QtObject if we are
    // in the process of creating the connection
    if(this->Internal->CreatingConnection)
      {
      previousBlockValue = this->Internal->QtObject->blockSignals(true);
      }

    // get the property of the object
    QVariant old;
    old = this->Internal->QtObject->property(this->Internal->QtProperty);
    QVariant prop;
    switch(pqSMAdaptor::getPropertyType(this->Internal->Property))
      {
    case pqSMAdaptor::PROXY:
    case pqSMAdaptor::PROXYSELECTION:
        {
        pqSMProxy p;
        p = pqSMAdaptor::getProxyProperty(this->Internal->Property, propertyValueType);
        prop.setValue(p);
        if(prop != old)
          {
          this->Internal->QtObject->setProperty(this->Internal->QtProperty, 
            prop);
          }
        }
      break;
    case pqSMAdaptor::ENUMERATION:
        {
        prop = pqSMAdaptor::getEnumerationProperty(this->Internal->Property, propertyValueType);
        if(prop != old)
          {
          this->Internal->QtObject->setProperty(this->Internal->QtProperty, 
            prop);
          }
        }
      break;
    case pqSMAdaptor::DATA_EXPORT:
    case pqSMAdaptor::SINGLE_ELEMENT:
        {
        prop = pqSMAdaptor::getElementProperty(this->Internal->Property, propertyValueType);
        if(prop != old)
          {
          this->Internal->QtObject->setProperty(this->Internal->QtProperty, 
            prop);
          }
        }
      break;
    case pqSMAdaptor::FILE_LIST:
        {
        prop = pqSMAdaptor::getFileListProperty(this->Internal->Property, propertyValueType);
        if(prop != old)
          {
          this->Internal->QtObject->setProperty(this->Internal->QtProperty,
            prop);
          }
        }
      break;
    case pqSMAdaptor::SELECTION:
        {
        if(this->Internal->Index == -1)
          {
          QList<QList<QVariant> > newVal =
            pqSMAdaptor::getSelectionProperty(this->Internal->Property, propertyValueType);
          if(newVal != old.value<QList<QList<QVariant> > >())
            {
            prop.setValue(newVal);
            this->Internal->QtObject->setProperty(this->Internal->QtProperty, 
              prop);
            }
          }
        else
          {
          QList<QVariant> sel;
          sel = pqSMAdaptor::getSelectionProperty(this->Internal->Property,
                                                  this->Internal->Index,
                                                  propertyValueType);

          if(sel.size() == 2 && sel[1] != old)
            {
            this->Internal->QtObject->setProperty(this->Internal->QtProperty, 
              sel[1]);
            }
          }
        }
      break;
    case pqSMAdaptor::MULTIPLE_ELEMENTS:
    case pqSMAdaptor::COMPOSITE_TREE:
    case pqSMAdaptor::SIL:
        {
        if(this->Internal->Index == -1)
          {
            prop = pqSMAdaptor::getMultipleElementProperty(this->Internal->Property,
                                                           propertyValueType);

          if(prop != old)
            {
            this->Internal->QtObject->setProperty(this->Internal->QtProperty,
              prop);
            }
          }
        else
          {
            prop = pqSMAdaptor::getMultipleElementProperty(this->Internal->Property,
                                                           this->Internal->Index,
                                                           propertyValueType);

          if(prop != old)
            {
            this->Internal->QtObject->setProperty(this->Internal->QtProperty, 
              prop);
            }
          }
        }
      break;
    case pqSMAdaptor::FIELD_SELECTION:
        {
        prop = pqSMAdaptor::getFieldSelection(this->Internal->Property,
                                              propertyValueType);

        if(prop != old)
          {
          this->Internal->QtObject->setProperty(this->Internal->QtProperty,
                                                prop);
          }
        }
    case pqSMAdaptor::UNKNOWN:
    case pqSMAdaptor::PROXYLIST:
      break;
    case pqSMAdaptor::COMMAND:
      // to prevent commands being triggered after initial creation etc
      // we squash this event
      this->Internal->OutOfSync = false;
      break;
      }

    // re-enable signals from the QtObject if we blocked
    // them because we were creating the connection
    if(this->Internal->CreatingConnection)
      {
      this->Internal->QtObject->blockSignals(previousBlockValue);
      }
*/
    }

  this->UseUncheckedProperties = val;
}

//-----------------------------------------------------------------------------
bool pqPropertyLinks::addPropertyLink(
  QObject* qobject, const char* qproperty, const char* qsignal,
  vtkSMProxy* smproxy, vtkSMProperty* smproperty, int smindex)
{
  if (!qobject || !qproperty || !qsignal || !smproxy || !smproperty)
    {
    qCritical() << "Invalid parameters to pqPropertyLinks::addPropertyLink";
    qDebug() << "(" << qobject << ", " << qproperty << ", " << qsignal
      << ") <==> ("
      << (smproxy? smproxy->GetXMLName() : "(none)")
      << "," << (smproperty? smproperty->GetXMLLabel() : "(none)")
      << smindex << ")";
    return false;
    }

  pqPropertyLinksConnection* connection = new pqPropertyLinksConnection(
    qobject, qproperty, qsignal, smproxy, smproperty, smindex,
    this->useUncheckedProperties(), this);

  // Avoid adding duplicates.
  foreach (pqPropertyLinksConnection* existing, this->Internals->Connections)
    {
    if (*existing == *connection)
      {
      qDebug() << "Skipping duplicate connection: "
        << "(" << qobject << ", " << qproperty << ", " << qsignal
        << ") <==> ("
        << smproxy << "," << (smproperty? smproperty->GetXMLLabel() : "(none)")
        << smindex;
      delete connection;
      return true;
/*
    case pqSMAdaptor::PROXY:
    case pqSMAdaptor::PROXYSELECTION:
        {
        if(this->Internal->UseUncheckedProperties)
          {
          pqSMAdaptor::setUncheckedProxyProperty(this->Internal->Property,
            prop.value<pqSMProxy>());
          }
        else
          {
          pqSMAdaptor::setProxyProperty(this->Internal->Property,
            prop.value<pqSMProxy>());
          if(this->Internal->AutoUpdate)
            {
            this->Internal->Proxy->UpdateVTKObjects();
            }
          }
        }

      break;
    case pqSMAdaptor::ENUMERATION:
        pqSMAdaptor::setEnumerationProperty(this->Internal->Property,
                                            prop,
                                            propertyValueType);

        if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
          {
          this->Internal->Proxy->UpdateVTKObjects();
          }
      break;
    case pqSMAdaptor::DATA_EXPORT:
    case pqSMAdaptor::SINGLE_ELEMENT:
        pqSMAdaptor::setElementProperty(this->Internal->Property,
                                        prop,
                                        propertyValueType);

        if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
          {
          this->Internal->Proxy->UpdateVTKObjects();
          }

      break;
    case pqSMAdaptor::FILE_LIST:
        if (!prop.canConvert<QStringList>())
          {
          qWarning() << "File list is not a list.";
          }
        else
          {
          pqSMAdaptor::setFileListProperty(this->Internal->Property,
                                           prop.value<QStringList>(),
                                           propertyValueType);

          if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
            {
            this->Internal->Proxy->UpdateVTKObjects();
            }
          }

      break;
    case pqSMAdaptor::SELECTION:
        if(this->Internal->Index == -1)
          {
          QList<QList<QVariant> > theProp = prop.value<QList<QList<QVariant> > >();

          pqSMAdaptor::setSelectionProperty(this->Internal->Property,
                                            theProp,
                                            propertyValueType);

          if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
            {
            this->Internal->Proxy->UpdateVTKObjects();
            }
          }
        else
          {
          QList<QVariant> domain;
          domain = pqSMAdaptor::getSelectionPropertyDomain(this->Internal->Property);
          QList<QVariant> selection;
          selection.append(domain[this->Internal->Index]);
          selection.append(prop);

          pqSMAdaptor::setSelectionProperty(this->Internal->Property,
                                            selection,
                                            propertyValueType);

          if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
            {
            this->Internal->Proxy->UpdateVTKObjects();
            }
          }

      break;
    case pqSMAdaptor::SIL:
    case pqSMAdaptor::MULTIPLE_ELEMENTS:
    case pqSMAdaptor::COMPOSITE_TREE:
        if(this->Internal->Index == -1)
          {
          pqSMAdaptor::setMultipleElementProperty(this->Internal->Property,
                                                  prop.toList(),
                                                  propertyValueType);

          if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
            {
            this->Internal->Proxy->UpdateVTKObjects();
            }
          }
        else
          {
          pqSMAdaptor::setMultipleElementProperty(this->Internal->Property,
                                                  this->Internal->Index,
                                                  prop,
                                                  propertyValueType);

          if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
            {
            this->Internal->Proxy->UpdateVTKObjects();
            }
          }

      break;
    case pqSMAdaptor::FIELD_SELECTION:
        pqSMAdaptor::setFieldSelection(this->Internal->Property,
                                       prop.toStringList(),
                                       propertyValueType);

          if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
            {
            this->Internal->Proxy->UpdateVTKObjects();
            }

        break;
    case pqSMAdaptor::UNKNOWN:
    case pqSMAdaptor::PROXYLIST:
      break;
    case pqSMAdaptor::COMMAND:
        pqSMAdaptor::setCommandProperty(this->Internal->Property,
                                            propertyValueType);

        if(this->Internal->AutoUpdate && !this->Internal->UseUncheckedProperties)
          {
          this->Internal->Proxy->UpdateVTKObjects();
          }
      break;
*/
      }
    }

  this->Internals->Connections.push_back(connection);

  // initialize the Qt widget using the SMProperty values.
  connection->copyValuesFromServerManagerToQt(this->useUncheckedProperties());

  QObject::connect(connection, SIGNAL(qtpropertyModified()),
    this, SLOT(onQtPropertyModified()));
  QObject::connect(connection, SIGNAL(smpropertyModified()),
    this, SLOT(onSMPropertyModified()));
  return true;
}

//-----------------------------------------------------------------------------
bool pqPropertyLinks::removePropertyLink(
  QObject* qobject, const char* qproperty, const char* qsignal,
  vtkSMProxy* smproxy, vtkSMProperty* smproperty, int smindex)
{
  // remove has to be a little flexible about input arguments. It accepts null
  // qobject and smproperty.

  pqPropertyLinksConnection connection(
    qobject, qproperty, qsignal, smproxy, smproperty, smindex,
    this->useUncheckedProperties(), this);

  // Avoid adding duplicates.
  foreach (pqPropertyLinksConnection* existing, this->Internals->Connections)
    {
    if (*existing == connection)
      {
      this->Internals->Connections.removeOne(existing);
      delete existing;
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void pqPropertyLinks::clear()
{
  foreach (pqPropertyLinksConnection* connection, this->Internals->Connections)
    {
    delete connection;
    }
  this->Internals->Connections.clear();
}

//-----------------------------------------------------------------------------
void pqPropertyLinks::onQtPropertyModified()
{
  pqPropertyLinksConnection* connection =
    qobject_cast<pqPropertyLinksConnection*>(this->sender());
  if (connection)
    {
    connection->copyValuesFromQtToServerManager(
      this->useUncheckedProperties());
    if (this->autoUpdateVTKObjects() && connection->proxy())
      {
      connection->proxy()->UpdateVTKObjects();
      }
    emit this->qtWidgetChanged();

    // although unintuitive, several panels e.g. pqDisplayProxyEditor expect
    // smPropertyChanged() whenever the SMProperty is chnaged, either by the GUI
    // or from ServerManager itself.
    // pqPropertyLinksConnection::copyValuesFromQtToServerManager() doesn't fire
    // any smpropertyModified() signal. It is fired only when ServerManager
    // changes the SMProperty. To avoid breaking old code, we fire this signal
    // explicitly. I'd like not provide some deprecation mechanism for this
    // behavior.
    if (this->autoUpdateVTKObjects() && connection->proxy())
      {
      emit this->smPropertyChanged();
      }
    }
}

//-----------------------------------------------------------------------------
void pqPropertyLinks::onSMPropertyModified()
{
  if (!this->Internals->IgnoreSMChanges)
    {
    pqPropertyLinksConnection* connection =
      qobject_cast<pqPropertyLinksConnection*>(this->sender());
    if (connection)
      {
      connection->copyValuesFromServerManagerToQt(
        this->useUncheckedProperties());
      emit this->smPropertyChanged();
      }
    }
}

//-----------------------------------------------------------------------------
void pqPropertyLinks::accept()
{
  std::set<vtkSMProxy*> proxies_to_update;

  // In some cases, pqPropertyLinks is used to connect multiple elements of a
  // property to different widgets (e.g. the VOI widgets for ExtractVOI filter).
  // In such cases when we "accept" changes from Index 0, for example, the Index
  // 1 ends up resetting its value using the current value from the property.
  // To avoid such incorrect feedback when we are accepting all changes, we use
  // IgnoreSMChanges flag.
  this->Internals->IgnoreSMChanges = true;

  foreach (pqPropertyLinksConnection* connection, this->Internals->Connections)
    {
    if (connection && connection->proxy())
      {
      connection->copyValuesFromQtToServerManager(false);
      proxies_to_update.insert(connection->proxy());
      }
    }
  this->Internals->IgnoreSMChanges = false;

  for (std::set<vtkSMProxy*>::iterator iter = proxies_to_update.begin();
    iter != proxies_to_update.end(); ++iter)
    {
    (*iter)->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
void pqPropertyLinks::reset()
{
  foreach (pqPropertyLinksConnection* connection, this->Internals->Connections)
    {
    if (connection && connection->proxy())
      {
      connection->copyValuesFromServerManagerToQt(false);
      }
    }
}
