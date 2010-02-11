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

#include "qsql_sfdc.h"
#include "forcewrapper.h"
#include "SoapBinding.nsmap"
#include <vector>
#include <QDebug>
#include <QSqlField>
#include <QXmlStreamReader>
#include <QDate>
#include <QUrl>
using namespace std;

// Driver stuff

SFDCDriver::SFDCDriver(QObject* parent)
  : QSqlDriver(parent), m_proxy(0) {
}

SFDCDriver::~SFDCDriver() {
}

bool SFDCDriver::hasFeature(DriverFeature feature) const {
  // I think the only thing I will implement is "querysize", since I'm pretty sure I can handle that
  // many of the rest I don't even understand
  return (feature == QSqlDriver::QuerySize);
}

bool SFDCDriver::open(const QString & db, const QString &user,
		      const QString & password, const QString & host,
		      int port, const QString & options) {

  // create the proxy
  m_proxy = new ForceProxy;
  int login_errcode = m_proxy->login(static_cast<const char*>(user.toAscii()),
				     static_cast<const char*>(password.toAscii()), "");
  if (login_errcode != SOAP_OK) {
    qDebug() << "login call failed with result code " << login_errcode << endl;
    qDebug() << "regular fail string is " << m_proxy->soap_fault_string() << endl;
    qDebug() << "detail fail string is " << m_proxy->soap_fault_detail() << endl;
    delete m_proxy;
    m_proxy = 0;
    setOpenError(true);
    return false;
  }
  setOpen(true);
  setOpenError(false);
  return true;
}

void SFDCDriver::close() {
  // should I logout?

  if (m_proxy) {
    delete m_proxy;
    m_proxy = 0;
  }
  setOpen(false);

}

QSqlResult* SFDCDriver::createResult() const {
  // I guess this is OK for now?
  return new SFDCResult(this);
}

// utility function to translate SFDC field types to QVariant
QVariant::Type fieldtype2variant(ns1__fieldType t) {
  if (t == ns1__fieldType__boolean) {
    return QVariant::Bool;
  }
  else if (t == ns1__fieldType__int_) {
    return QVariant::Int;
  }
  else if (t == ns1__fieldType__double_) {
    return QVariant::Double;
  }
  else if (t == ns1__fieldType__date) {
    return QVariant::Date;
  }
  else if (t == ns1__fieldType__datetime) {
    return QVariant::DateTime;
  }
  else if (t == ns1__fieldType__time) {
    return QVariant::Time;
  }
  else if (t == ns1__fieldType__url) {
    return QVariant::Url;
  }
  // fallback
  return QVariant::String;
}

// utility function to translate QStrings to QVariants based on a particular type
// there's got to be a better way to do this...
QVariant fieldvalue2variant(QString v, QVariant::Type t) {
  if (t == QVariant::Bool) {
    return QVariant(v).toBool();
  }
  else if (t == QVariant::Int) {
    return QVariant(v).toInt();
  }
  else if (t == QVariant::Double) {
    return QVariant(v).toDouble();
  }
  else if (t == QVariant::Date) {
    return QVariant(v).toDate();
  }
  else if (t == QVariant::DateTime) {
    return QVariant(v).toDateTime();
  }
  else if (t == QVariant::Time) {
    return QVariant(v).toTime();
  }
  else if (t == QVariant::Url) {
    return QVariant(v).toUrl();
  }
  // fallback
  return QVariant(v);
}


// supply the names of all the fields in a record for the given table
// note: not listed as one of the mandatory things to define, but required for QSqlTableModel (or setTable will fail)
QSqlRecord SFDCDriver::record ( const QString & tableName ) const {

  if (record_cache.contains(tableName)) {
    // no need to do extra SOAP call
    return record_cache.value(tableName);
  }

  // call describeSObject on the given table name
  _ns1__describeSObject describe_req; describe_req.sObjectType = static_cast<const char*>(tableName.toAscii());
  _ns1__describeSObjectResponse describe_resp;
  int dso_errcode;
  if ((dso_errcode = m_proxy->describeSObject(&describe_req, &describe_resp)) != SOAP_OK) {
    qDebug() << "describeSObject on table " << tableName << " failed with error code " << dso_errcode << " and string " << m_proxy->soap_fault_string();
    return QSqlRecord();
  }

  QSqlRecord field_desc_record;
  // iterate over fields, creating a QSqlField for each and inserting into the return value
  for (vector<ns1__Field*>::iterator field_it = describe_resp.result->fields.begin();
       field_it != describe_resp.result->fields.end(); ++field_it) {
    QSqlField field_desc(QString((*field_it)->name.c_str()), fieldtype2variant((*field_it)->type));
    // add properties from the response object
    field_desc.setAutoValue((*field_it)->autoNumber);
    field_desc.setReadOnly((*field_it)->calculated);   // but does not consider the "nillable" property
    if ((*field_it)->type == ns1__fieldType__double_) {
      field_desc.setPrecision((*field_it)->precision);
    }
    // not sure what to do with length, byteLength, digits, etc. fields at this time

    // store the "field label" from SF as the "value" of the field
    field_desc.setValue(QString((*field_it)->label.c_str()));
    field_desc_record.append(field_desc);
  }

  return field_desc_record;
}

ForceProxy* SFDCDriver::getProxy() const {
  return m_proxy;
}

// Result stuff

SFDCResult::SFDCResult(const SFDCDriver *driver) : QSqlResult(driver) {
}


SFDCResult::~SFDCResult() {
  // what do I do here?  Call base class destructor or is that automatic?
}

// return the data for the given field in the "current" row
QVariant SFDCResult::data(int index ) {
  if ((index < stored_query_fields.count()) && !isNull(index)) {
    // so far so good.  Let's see if there's any data
    QXmlStreamReader xmlr(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") +  // fake out encoding
			  QString(stored_query_results[stored_query_pos]->__any[index]));
    // keep it from complaining about the "sf" or "xsi" namespaces in front of the fieldname
    xmlr.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("sf", "whatever"));
    xmlr.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("xsi", "whatever"));
    xmlr.readNext();   // go to first token (representing the document start)
    xmlr.readNext();   // this should be the "element"
    // construct a QVariant of the appropriate type
    return fieldvalue2variant(xmlr.readElementText(), stored_query_fields.field(index).type());
  }

  return QVariant();
}

// return true if the given field in the current row is null
bool SFDCResult::isNull(int index ) {
  if ((index < 0) || (index >= stored_query_fields.count())) {
    return true;
  }

  if (!(stored_query_results[stored_query_pos]->__any[index])) {
    return true;
  }

  // also return true if the field has no contents (string returned, but no text in the element)
  QXmlStreamReader xmlr(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>") + // fake out encoding
			QString(stored_query_results[stored_query_pos]->__any[index]));
  // keep it from complaining about the "sf" and "xsi" namespaces in front of the fieldname
  xmlr.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("sf", "whatever"));
  xmlr.addExtraNamespaceDeclaration(QXmlStreamNamespaceDeclaration("xsi", "whatever"));
  xmlr.readNext();   // go to first token (representing the document start)
  xmlr.readNext();   // this should be the "element"
  if (QString(xmlr.readElementText()).isEmpty()) {
    return true;
  }

  return false;

}

// despite the name this means "execute the query"
bool SFDCResult::reset(const QString & querystr ) {
  // extract the field list from the query (and do some syntax checking)

  // locate the beginning and end of the field list
  QString after_select = querystr;
  after_select.remove(QRegExp("^(select|SELECT)\\s+"));
  if (after_select == querystr) {
    // doesn't start with select?
    return false;
  }

  int frompos = after_select.indexOf(QRegExp(" (from|FROM)"));
  if (frompos < 0) {
    // no "from" keyword = fail
    return false;
  }

  QString fieldlist = after_select.mid(0, frompos);

  QStringList query_fieldnames = fieldlist.split(QRegExp("\\s?,\\s?"));  // split on commas with optional spaces


  // perform the query through SOAP
  const SFDCDriver* drv = dynamic_cast<const SFDCDriver*>(driver());  // a base class method, so must dyn cast to derived
  ForceProxy* proxy = drv->getProxy();

  _ns1__query query; query.queryString = static_cast<const char*>(querystr.toAscii());
  _ns1__queryResponse query_resp;
  int q_errcode;
  if ((q_errcode = proxy->query(&query, &query_resp)) != SOAP_OK) {
    qDebug() << "query failed\n";
    return false;
  }

  // some info from a random SF website board
  // apparently "done" tells you if you have pulled all the data down yet
  // "size" is how many records matched (whether or not you have all of them)
  // if records.size() is different from size, it will be presumably lower and equal to the current batch size

  // record results
  stored_query_results = query_resp.result->records;
  stored_query_done = query_resp.result->done;
  if (query_resp.result->queryLocator) {
    stored_query_locator = query_resp.result->queryLocator->c_str();
  }
  else {
    stored_query_locator = "";
  }
  stored_query_match_cnt = query_resp.result->size;

  // find the corresponding types from the original fields
  // BOZO does not handle references to other objects, e.g. Contact.Account.Name
  QString objname = query_resp.result->records[0]->type.c_str();
  QSqlRecord object_fields = drv->record(objname);
  stored_query_fields.clear();
  for (int i = 0; i < query_fieldnames.size(); ++i) {
    if (!object_fields.contains(query_fieldnames.at(i))) {
      qDebug() << "could not find definition of field " << query_fieldnames.at(i) << " in object " << objname;
      stored_query_fields.append(QSqlField(query_fieldnames.at(i), QVariant::Invalid));
    }
    else {
      stored_query_fields.append(object_fields.field(query_fieldnames.at(i)));
    }
  }

  setAt(QSql::BeforeFirstRow);
  setActive(true);
  setSelect(true);

  return true;
}

// position current row to the given index and call setAt(). return false if out of range
bool SFDCResult::fetch(int index ) {
  if (index < size()) {
    while ((index >= stored_query_results.size()) && !stored_query_done) {
      const SFDCDriver* drv = dynamic_cast<const SFDCDriver*>(driver());  // a base class method, so must dyn cast to derived
      ForceProxy* proxy = drv->getProxy();
      // we don't have all the records downloaded and have to get additional records from the API
      // BOZO totally untested
      _ns1__queryMore querymore; querymore.queryLocator = stored_query_locator.c_str();
      _ns1__queryMoreResponse querymore_resp;
      int qm_errcode;
      if ((qm_errcode = proxy->queryMore(&querymore, &querymore_resp)) != SOAP_OK) {
	qDebug() << "queryMore failed";
	return false;
      }
      // update results
      copy(querymore_resp.result->records.begin(),
	   querymore_resp.result->records.end(),
	   back_inserter(stored_query_results));
      stored_query_done = querymore_resp.result->done;
      if (querymore_resp.result->queryLocator) {
	stored_query_locator = querymore_resp.result->queryLocator->c_str();
      }
      else {
	stored_query_locator = "";
      }
    }

    stored_query_pos = index;
    setAt(index);
    return true;
  }

  return false;
}

// like fetch, but to row 0.  why does this even exist?
bool SFDCResult::fetchFirst() {
  return fetch(0);
}

// fetch, but last row.  I guess if you don't know how many rows there are this could be useful
bool SFDCResult::fetchLast() {

  if (size() <= 0) {
    return false;
  }

  return fetch(size() - 1);
}

// number of rows in the result, or -1 if there are none
int SFDCResult::size() {
  int numrecs = stored_query_match_cnt;
  if (numrecs == 0) {
    return -1;
  }

  return numrecs;
}

// this one's kind of bogus for us as no SOQL statement can modify records
int SFDCResult::numRowsAffected() {
  return -1;
}

// return the current "record", which is really the names and types of all the fields in each record of the query result
QSqlRecord SFDCResult::record() const {
  if (stored_query_results.empty()) {
    // nothing to do
    return QSqlRecord();
  }

  return stored_query_fields;

}
