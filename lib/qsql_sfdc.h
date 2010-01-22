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

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlResult>
#include <QtSql/QSqlRecord>
#include <QVector>

class ForceProxy;
class ns2__sObject;

class SFDCDriver : public QSqlDriver
{
public:
  SFDCDriver(QObject* parent=0);
  ~SFDCDriver();

  bool hasFeature(DriverFeature /* feature */) const;
  bool open(const QString & /* db */, const QString & /* user */,
	    const QString & /* password */, const QString & /* host */,
	    int /* port */, const QString & /* options */);
  void close();
  QSqlResult*   createResult() const;
  QSqlRecord 	record ( const QString & tableName ) const;
  ForceProxy*   getProxy() const;

 private:
  ForceProxy*                 m_proxy;
  QMap<QString, QSqlRecord>   record_cache;   // so I don't have to call describeSObject on every query

};

class SFDCResult : public QSqlResult
{
public:
  SFDCResult(const SFDCDriver *driver);
  ~SFDCResult();

protected:
  QVariant data(int /* index */);
  bool isNull(int /* index */);
  bool reset(const QString & /* query */);
  bool fetch(int /* index */);
  bool fetchFirst();
  bool fetchLast();
  int size();
  int numRowsAffected();
  QSqlRecord record() const;

 private:
  std::vector<ns2__sObject*> stored_query_results;
  std::string            stored_query_locator;
  int                    stored_query_pos;
  QSqlRecord             stored_query_fields;
  bool                   stored_query_done;
  int                    stored_query_match_cnt;
};
