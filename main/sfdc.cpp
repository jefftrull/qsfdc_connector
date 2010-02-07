/***********************************************************************************
 * Copyright (C) 2010 Jeffrey Elliot Trull <linmodemstudent@gmail.com>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ************************************************************************************/
 
#include <QApplication>
#include "qsql_sfdc.h"
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QDialog>
#include <QTableView>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QHeaderView>
#include "ui_sfdc.h"

int main( int argc, char **argv ) {
  QApplication app(argc, argv);

  SFDCDriver* sfdc_driver = new SFDCDriver;
  QSqlDatabase db = QSqlDatabase::addDatabase(sfdc_driver, "salesforce.com");

  // use UI dialog to get passwords
  QDialog dlg;
  Ui::dlgSFDCConf ui;
  ui.setupUi(&dlg);
  QObject::connect(ui.buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
  QObject::connect(ui.buttonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));
  dlg.exec();

  if (dlg.result() == QDialog::Rejected) {
    qDebug() << "quitting because login dialog was cancelled";
    return 1;
  }

  db.open(ui.eAccount->text(),
	  ui.ePass->text() + ui.eToken->text());

  // not using a QSqlTableModel here because it really seems to want to pull all the columns in
  // even removing the columns causes it to issue new sObject describe requests
  QSqlQueryModel *model = new QSqlQueryModel;

  // get table and chosen fields from user
  // hardcoded for now
  QString tablename("Contact");
  QStringList queryfields;
  queryfields << "Id" << "Name" << "Phone" << "Email";
  queryfields << "MailingStreet" << "MailingCity" << "MailingState" << "MailingPostalCode" << "MailingCountry";
  QString querystr("select ");
  querystr += queryfields.join(", ");
  querystr += " from " + tablename;
  model->setQuery(querystr, db);

  // replace header names with user-friendly "label" of each field
  QSqlRecord field_info = sfdc_driver->record(tablename);
  int colno = 0;
  while (colno < model->columnCount()) {
    QString colname = model->headerData(colno, Qt::Horizontal).toString();
    if (queryfields.contains(colname)) {
      // replace with label
      model->setHeaderData(colno, Qt::Horizontal, field_info.field(colname).value().toString());
      ++colno;
    }
  }
    
  QTableView *view = new QTableView;
  view->setModel(model);
  view->resizeColumnsToContents();
  view->resize(view->horizontalHeader()->length(), 200);
  view->setWindowTitle("Salesforce Contacts");
  view->show();

  return app.exec();

}
