/***********************************************************************************
 * Copyright (C) 2010 Jeffrey Elliot Trull <linmodemstudent@gmail.com>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
  ************************************************************************************/

#include "sfdcresource.h"
#include "forcewrapper.h"
#include "SoapBinding.nsmap"

#include "settings.h"
#include "settingsadaptor.h"

#include <KWindowSystem>
#include <akonadi/changerecorder.h>
#include <akonadi/itemfetchscope.h>
#include <kabc/addressee.h>
#include <QtDBus/QDBusConnection>
#include <fstream>
#include <string>

using namespace Akonadi;
using namespace std;

SFDCResource::SFDCResource( const QString &id )
  : ResourceBase( id ), m_proxy(0), m_ui(0), m_dlg(0)
{
  new SettingsAdaptor( Settings::self() );
  QDBusConnection::sessionBus().registerObject( QLatin1String( "/Settings" ),
                            Settings::self(), QDBusConnection::ExportAdaptors );

  changeRecorder()->itemFetchScope().fetchFullPayload();

  // TODO: you can put any resource specific initialization code here.
  // google resource does "new" of the gcal object but does not login

  m_proxy = new ForceProxy();
  m_dlg = new QDialog;
  m_ui = new Ui::dlgSFDCConf;
  m_ui->setupUi(m_dlg);
  connect(m_ui->buttonBox, SIGNAL(accepted()), m_dlg, SLOT(accept()));
  connect(m_ui->buttonBox, SIGNAL(rejected()), m_dlg, SLOT(reject()));

  status(Idle, "Resource constructed");

}

SFDCResource::~SFDCResource()
{
  delete m_dlg;
  delete m_proxy;

}

// google resource defines "retrieveTimestamp" and "saveTimestamp" methods here

void SFDCResource::retrieveCollections()
{
  status(Running, "retrieveCollections called");

  // google just provides a skeleton that gives a single collection.  No communication done.
  Collection c;
  c.setParent(Collection::root());
  c.setRemoteId("Salesforce.com-contacts");
  c.setName(name());

  status(Running, "Collection created and set up");

  QStringList mimeTypes;
  mimeTypes << "text/directory";
  c.setContentMimeTypes(mimeTypes);

  status(Running, "mime stuff done");

  Collection::List list;
  list << c;
  status(Running, "collection list created");
  collectionsRetrieved(list);
  status(Running, "collectionsRetrieved set, all done");


}

// google defines an authenticationError method here

void SFDCResource::retrieveItems( const Akonadi::Collection &collection )
{
  Q_UNUSED( collection );

  // TODO: this method is called when Akonadi wants to know about all the
  // items in the given collection. You can but don't have to provide all the
  // data for each item, remote ID and MIME type are enough at this stage.
  // Depending on how your resource accesses the data, there are several
  // different ways to tell Akonadi when you are done.

  // google ignores the collection argument
  // google calls the configure method if we are not authenticated
  // it then tries to fetch only the updated items and otherwise fetches all of them, updating the stored timestamp
  // then it takes all the known contacts locally stored, makes a KABC::Addressee out of each and puts them into "items"
  // then calls itemsRetrieved(items) and is done

  status(Running, "retrieveItems called");

  if (!m_proxy->isLoggedIn()) {
    status(Idle, "NOT logged in");
    configure(0);
    if (m_proxy->isLoggedIn()) {
      status(Running, "done configuring, reading to retrieve items");
    }
    else {
      return;
    }
  }
  else {
    status(Running, "logged in already; retrieving items now");
  }

  // for now, just get all of them.  Ugh.
  QStringList queryfields;
  queryfields << "Id" << "Name" << "Phone" << "Email";
  queryfields << "MailingStreet" << "MailingCity" << "MailingState" << "MailingPostalCode" << "MailingCountry";
  QString querystr("select ");
  querystr += queryfields.join(", ");
  querystr += " from Contact";

  _ns1__query query; query.queryString = static_cast<const char*>(querystr.toAscii());
  _ns1__queryResponse query_resp;
  int q_errcode;
  if ((q_errcode = m_proxy->query(&query, &query_resp)) != SOAP_OK) {
    status(Broken, "query failed");
    qDebug() << "query call failed with code " << q_errcode << " and error string " << m_proxy->soap_fault_string();
    return;
  }

  Item::List items;
  QString timestamp;
  QByteArray t_byte;

  for (vector<ns2__sObject*>::iterator rec_it = query_resp.result->records.begin();
       rec_it != query_resp.result->records.end(); ++rec_it) {
    // create a new Item for this record
    Item item(QLatin1String("text/directory"));

    // for each field in this record
    //    verify the name of this field is in the field list
    //    if field has a value {
    //      based on which field it is, store in different ways
    //    }

    KABC::Addressee addressee;
    KABC::Address address;

    for (vector<char*>::const_iterator field_it = (*rec_it)->__any.begin();
	 field_it != (*rec_it)->__any.end(); ++field_it) {
      QXmlStreamReader xmlr(*field_it);
      xmlr.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("sf", "whatever"));
      xmlr.readNext();   // go to first token (representing the document start)
      xmlr.readNext();   // this should be the "element"
      QString fname = xmlr.name().toString();
      QString ftext = xmlr.readElementText();
      if (!queryfields.contains(fname)) {
	qDebug() << "don't know field " << fname << "; skipping";
	continue;
      }
      if (fname == "Id") {
	item.setRemoteId(ftext);
      }
      else if (fname == "Name") {
	addressee.setNameFromString(ftext);
      }
      else if (fname == "Phone") {
	KABC::PhoneNumber number;
	number.setNumber(ftext);
	addressee.insertPhoneNumber(number);
      }
      else if (fname == "Email") {
	addressee.insertEmail(ftext);
      }
      else if (fname == "MailingStreet") {
	address.setStreet(ftext);
      }
      else if (fname == "MailingCity") {
	address.setLocality(ftext);
      }
      else if (fname == "MailingState") {
	address.setRegion(ftext);
      }
      else if (fname == "MailingPostalCode") {
	address.setPostalCode(ftext);
      }
      else if (fname == "MailingCountry") {
	address.setCountry(ftext);
      }
    }

    if (!address.isEmpty())
      addressee.insertAddress(address);

    item.setPayload<KABC::Addressee>(addressee);
    
    items << item;
  }
  itemsRetrieved(items);
  QString ststr("retrieved ");
  ststr += items.size();
  ststr += " items";
  
  status(Running, ststr);
}

bool SFDCResource::retrieveItem( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_UNUSED( item );
  Q_UNUSED( parts );

  // google does nothing in this one because it provides all the data at once (in retrieveItems)

  status(Idle, "retrieveItem called");

  return true;
}

void SFDCResource::aboutToQuit()
{
  // TODO: any cleanup you need to do while there is still an active
  // event loop. The resource will terminate after this method returns

  // logout here, I guess?
  status(Idle, "aboutToQuit called");

}

void SFDCResource::configure( WId windowId )
{
  Q_UNUSED( windowId );

  // TODO: this method is usually called when a new resource is being
  // added to the Akonadi setup. You can do any kind of user interaction here,
  // e.g. showing dialogs.
  // The given window ID is usually useful to get the correct
  // "on top of parent" behavior if the running window manager applies any kind
  // of focus stealing prevention technique

  // here is where google does its login
  status(Idle, "about to show dialog");
  if (windowId && m_dlg)
    KWindowSystem::setMainWindow(m_dlg, windowId);
  m_dlg->exec();

  QString ststr("dialog executed: uid=");
  ststr += m_ui->eAccount->text();
  ststr += " pw=";
  ststr += m_ui->ePass->text();
  ststr += " (";
  if (m_dlg->result() == QDialog::Accepted) {
    ststr += "Accepted)";
  }
  else {
    status(Idle, "Login Cancelled");
    return;
  }

  // attempt to log in
  int login_errcode = m_proxy->login(static_cast<const char*>(m_ui->eAccount->text().toAscii()),
				     static_cast<const char*>(m_ui->ePass->text().toAscii()),
				     static_cast<const char*>(m_ui->eToken->text().toAscii()));

  if (login_errcode != SOAP_OK) {
    qDebug() << "login call failed with result code " << login_errcode;
    qDebug() << "regular fail string is " << m_proxy->soap_fault_string();
    qDebug() << "detail fail string is " << m_proxy->soap_fault_detail();
    emit error("login to SFDC server failed");
    emit status(Broken, "login to SFDC server failed");
    return;
  }

  ststr = "session ID ";
  ststr += m_proxy->soap_header()->ns1__SessionHeader->sessionId.c_str();

  status(Running, ststr);

  synchronize();

}

void SFDCResource::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  Q_UNUSED( item );
  Q_UNUSED( collection );

  // TODO: this method is called when somebody else, e.g. a client application,
  // has created an item in a collection managed by your resource.

  // NOTE: There is an equivalent method for collections, but it isn't part
  // of this template code to keep it simple

  // let's not implement this initially.  From looking at the Google code I suspect it wouldn't be hard, though

}

void SFDCResource::itemChanged( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_UNUSED( item );
  Q_UNUSED( parts );

  // TODO: this method is called when somebody else, e.g. a client application,
  // has changed an item managed by your resource.

  // NOTE: There is an equivalent method for collections, but it isn't part
  // of this template code to keep it simple

  // google doesn't use the "parts" argument.  I wonder if there is a standard for this?

}

void SFDCResource::itemRemoved( const Akonadi::Item &item )
{
  Q_UNUSED( item );

  // TODO: this method is called when somebody else, e.g. a client application,
  // has deleted an item managed by your resource.

  // NOTE: There is an equivalent method for collections, but it isn't part
  // of this template code to keep it simple
}

AKONADI_RESOURCE_MAIN( SFDCResource )

#include "sfdcresource.moc"
