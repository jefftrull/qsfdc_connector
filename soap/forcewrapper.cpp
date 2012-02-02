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

#include "forcewrapper.h"
using namespace std;

ForceProxy::ForceProxy() : SoapBindingProxy(SOAP_IO_CHUNK | SOAP_IO_KEEPALIVE | SOAP_ENC_ZLIB), m_logged_in(false) {
}

ForceProxy::~ForceProxy() {};

int ForceProxy::login(const string& uid,
		       const string& pw,
		       const string& token) {
  _ns1__login login_req;
  login_req.username = uid;
  login_req.password = pw + token;
  _ns1__loginResponse login_resp;

  int login_errcode;
  if ((login_errcode = SoapBindingProxy::login(&login_req, &login_resp)) != SOAP_OK)
    return login_errcode;
  
  // store returned values

  // server URL into "endpoint"
  serverUrl = *(login_resp.result->serverUrl);
  soap_endpoint = serverUrl.c_str();

  // overwrite the session header sessionId with the returned value
  sessionHeader.sessionId = *(login_resp.result->sessionId);

  // finish by restoring the proxy header
  restoreProxyHeader();

  m_logged_in = true;

  return login_errcode;

}

int ForceProxy::describeSObject(_ns1__describeSObject *ns1__describeSObject,
				_ns1__describeSObjectResponse *ns1__describeSObjectResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::describeSObject(ns1__describeSObject, ns1__describeSObjectResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::describeSObjects(_ns1__describeSObjects *ns1__describeSObjects,
				 _ns1__describeSObjectsResponse *ns1__describeSObjectsResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::describeSObjects(ns1__describeSObjects, ns1__describeSObjectsResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::describeGlobal(_ns1__describeGlobal *ns1__describeGlobal,
			       _ns1__describeGlobalResponse *ns1__describeGlobalResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::describeGlobal(ns1__describeGlobal, ns1__describeGlobalResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::describeLayout(_ns1__describeLayout *ns1__describeLayout,
			       _ns1__describeLayoutResponse *ns1__describeLayoutResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::describeLayout(ns1__describeLayout, ns1__describeLayoutResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::describeTabs(_ns1__describeTabs *ns1__describeTabs,
			     _ns1__describeTabsResponse *ns1__describeTabsResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::describeTabs(ns1__describeTabs, ns1__describeTabsResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::create(_ns1__create *ns1__create,
		       _ns1__createResponse *ns1__createResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::create(ns1__create, ns1__createResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::update(_ns1__update *ns1__update,
		       _ns1__updateResponse *ns1__updateResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::update(ns1__update, ns1__updateResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::upsert(_ns1__upsert *ns1__upsert,
		       _ns1__upsertResponse *ns1__upsertResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::upsert(ns1__upsert, ns1__upsertResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}
int ForceProxy::merge(_ns1__merge *ns1__merge,
		      _ns1__mergeResponse *ns1__mergeResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::merge(ns1__merge, ns1__mergeResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::delete_(_ns1__delete *ns1__delete,
			_ns1__deleteResponse *ns1__deleteResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::delete_(ns1__delete, ns1__deleteResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::retrieve(_ns1__retrieve *ns1__retrieve,
			 _ns1__retrieveResponse *ns1__retrieveResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::retrieve(ns1__retrieve, ns1__retrieveResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::process(_ns1__process *ns1__process,
			_ns1__processResponse *ns1__processResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::process(ns1__process, ns1__processResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::query(_ns1__query *ns1__query,
		      _ns1__queryResponse *ns1__queryResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::query(ns1__query, ns1__queryResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::queryAll(_ns1__queryAll *ns1__queryAll,
			 _ns1__queryAllResponse *ns1__queryAllResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::queryAll(ns1__queryAll, ns1__queryAllResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::queryMore(_ns1__queryMore *ns1__queryMore,
			  _ns1__queryMoreResponse *ns1__queryMoreResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::queryMore(ns1__queryMore, ns1__queryMoreResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}

int ForceProxy::search(_ns1__search *ns1__search,
		       _ns1__searchResponse *ns1__searchResponse) {
  int retcode;
  if ((retcode = SoapBindingProxy::search(ns1__search, ns1__searchResponse)) == SOAP_OK)
    restoreProxyHeader();
  return retcode;

}


void ForceProxy::restoreProxyHeader() {
  // re-set the soap header from the private data
  // This is ugly, but if I don't do this the soap header becomes NULL again after every call,
  // and we lose the session ID (and our requests fail!)
  // is there some better way?
  soap_header(&allOrNoneHeader, &allowFieldTruncationHeader, &assignmentRuleHeader,
	      &callOptions,    // Partner WSDL only
	      &debuggingHeader, &debuggingInfo,	&disableFeedTrackingHeader,
	      &emailHeader, &localeOptions,
	      &loginScopeHeader, &mruHeader, &packageVersionHeader,
	      &queryOptions, &sessionHeader,
#ifdef STREAMING_API_PRESENT
	      &streamingEnabledHeader,
#endif
	      &userTerritoryDeleteHeader);
}

bool ForceProxy::isLoggedIn() const {
  return m_logged_in;
}
