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

// A class that's like SoapBindingProxy but automatically re-sets the header info for you after each API call

#ifndef forcewrapper_H
#define forcewrapper_H

#include "soapSoapBindingProxy.h"

class ForceProxy : public SoapBindingProxy {
public:
  ForceProxy();
  ~ForceProxy();

  // give a simple interface for logging in
  int login(const std::string& uid, const std::string& pw, const std::string& token);

  // restore proxy header, which for some reason in gSoap gets removed after each API call
  void restoreProxyHeader();

  // provide our own implementations of a few key calls
  int describeSObject(_ns1__describeSObject *ns1__describeSObject, _ns1__describeSObjectResponse *ns1__describeSObjectResponse);
  int describeSObjects(_ns1__describeSObjects *ns1__describeSObjects, _ns1__describeSObjectsResponse *ns1__describeSObjectsResponse);
  int describeGlobal(_ns1__describeGlobal *ns1__describeGlobal, _ns1__describeGlobalResponse *ns1__describeGlobalResponse);
  int describeLayout(_ns1__describeLayout *ns1__describeLayout, _ns1__describeLayoutResponse *ns1__describeLayoutResponse);
  int describeTabs(_ns1__describeTabs *ns1__describeTabs, _ns1__describeTabsResponse *ns1__describeTabsResponse);
  int create(_ns1__create *ns1__create, _ns1__createResponse *ns1__createResponse);
  int update(_ns1__update *ns1__update, _ns1__updateResponse *ns1__updateResponse);
  int upsert(_ns1__upsert *ns1__upsert, _ns1__upsertResponse *ns1__upsertResponse);
  int merge(_ns1__merge *ns1__merge, _ns1__mergeResponse *ns1__mergeResponse);
  int delete_(_ns1__delete *ns1__delete, _ns1__deleteResponse *ns1__deleteResponse);
  int retrieve(_ns1__retrieve *ns1__retrieve, _ns1__retrieveResponse *ns1__retrieveResponse);
  int process(_ns1__process *ns1__process, _ns1__processResponse *ns1__processResponse);
  int query(_ns1__query *ns1__query, _ns1__queryResponse *ns1__queryResponse);
  int queryAll(_ns1__queryAll *ns1__queryAll, _ns1__queryAllResponse *ns1__queryAllResponse);
  int queryMore(_ns1__queryMore *ns1__queryMore, _ns1__queryMoreResponse *ns1__queryMoreResponse);
  int search(_ns1__search *ns1__search, _ns1__searchResponse *ns1__searchResponse);

  bool isLoggedIn() const;

private:
  bool m_logged_in;

  std::string serverUrl;           // server URL after successful login

  // Header values.  For some reason we must re-set the proxy header from these before every call
  // create a set of headers with default values

  _ns1__SessionHeader sessionHeader;  // In here we record the session ID after a successful login

  // the rest of the header values (allowed to default)
  _ns1__AllOrNoneHeader allOrNoneHeader;
  _ns1__AllowFieldTruncationHeader allowFieldTruncationHeader;
  _ns1__AssignmentRuleHeader assignmentRuleHeader;
  _ns1__CallOptions callOptions;   // Partner WSDL only
  _ns1__DebuggingHeader debuggingHeader;
  _ns1__DebuggingInfo debuggingInfo;
  _ns1__DisableFeedTrackingHeader disableFeedTrackingHeader;
  _ns1__EmailHeader emailHeader;
  _ns1__PackageVersionHeader packageVersionHeader;
  _ns1__MruHeader mruHeader;
  _ns1__QueryOptions queryOptions;
  _ns1__UserTerritoryDeleteHeader userTerritoryDeleteHeader;
  _ns1__StreamingEnabledHeader streamingEnabledHeader;
  _ns1__LocaleOptions localeOptions;
  _ns1__LoginScopeHeader loginScopeHeader;

};

#endif
