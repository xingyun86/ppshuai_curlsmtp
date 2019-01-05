/***************************************************************************
*                                  _   _ ____  _
*  Project                     ___| | | |  _ \| |
*                             / __| | | | |_) | |
*                            | (__| |_| |  _ <| |___
*                             \___|\___/|_| \_\_____|
*
* Copyright (C) 1998 - 2017, Daniel Stenberg, <daniel@haxx.se>, et al.
*
* This software is licensed as described in the file COPYING, which
* you should have received as part of this distribution. The terms
* are also available at https://curl.haxx.se/docs/copyright.html.
*
* You may opt to use, copy, modify, merge, publish, distribute and/or sell
* copies of the Software, and permit persons to whom the Software is
* furnished to do so, under the terms of the COPYING file.
*
* This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
* KIND, either express or implied.
*
***************************************************************************/

/* <DESC>
* SMTP example showing how to send mime e-mails
* </DESC>
*/

#include <vector>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#pragma comment (lib, "ws2_32.lib")
#pragma comment (lib, "mswsock.lib")
#pragma comment (lib, "wsock32.lib")
#pragma comment (lib, "wldap32.lib")
#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "normaliz.lib")
#pragma comment (lib, "libcurl.lib")

/* This is a simple example showing how to send mime mail using libcurl's SMTP
* capabilities. For an example of using the multi interface please see
* smtp-multi.c.
*
* Note that this example requires libcurl 7.56.0 or above.
*/

#define SMTP_URL		"smtp://smtp.yeah.net:25"
#define SMTPS_URL		"smtp://smtp.yeah.net:993"
//#define SMTP_URL		"smtp://smtp.office365.com:587"
//#define SMTP_URL		"smtp://smtp-mail.outlook.com:587"
//#define SMTPS_URL		"smtp://smtp.yeah.net:465"
#define USER_NAME		"xxxx@yeah.net"
//#define USER_NAME		"xxxx@outlook.com"
#define PASS_WORD		"xxxx"

#define FROM			"<" USER_NAME ">"
#define FROM_NAME		"(gz_ppshuai)"
#define TO				"<xxxx@qq.com>"
#define TO_NAME			"(xxxx)"
#define TO_CC			"<xxxx@yeah.net>"
#define TO_CC_NAME		"(xxxx)"
#define TO_CC2			"<xxxx@chacuo.net>"
#define TO_CC2_NAME		"(xxxx)"
#define TO_BCC			"<xxxx@yeah.net>"
#define TO_BCC_NAME		"(xxxx)"
#define TO_BCC2			"<xxxx@gmail.com>"
#define TO_BCC2_NAME	"(xxxx)"

#define MAIL_SUBJECT	"[NO.BETTER]Update password success"
#define MAIL_BODY_TEXT	"[NO.BETTER]Update success.Please login again!"

static const char *headers_text[] = {
	"Date: Thu, 03 Jan 2019 22:38:56 +0800",
	"To: " TO TO_NAME "",
	"From: " FROM "" FROM_NAME "",
	"Cc: " TO_CC "" TO_CC_NAME "," TO_CC2 "" TO_CC2_NAME "",
	"Bcc: " TO_BCC "" TO_BCC_NAME "," TO_BCC2 "" TO_BCC2_NAME "",
	"Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@yeah.net>",
	"Subject: " MAIL_SUBJECT,
	NULL
};

static const char inline_text[] =
"" MAIL_BODY_TEXT "\r\n";

static const char inline_html[] =
"<html><body>" MAIL_BODY_TEXT "</body></html>\r\n";

typedef struct _tagByteData {
#define call_back_data_size 0x10000
	char * p;
	unsigned int s;
	unsigned int v;
	static void * startup()
	{
		void * thiz = malloc(sizeof(struct _tagByteData));
		if (thiz)
		{
			((struct _tagByteData *)thiz)->init();
		}
		return thiz;
	}
	_tagByteData()
	{
	}
	_tagByteData(char ** _p = 0, unsigned int _s = 0, unsigned int _v = 0)
	{
		init(_p, _s, _v);
	}
	void init(char ** _p = 0, unsigned int _s = 0, unsigned int _v = 0)
	{
		p = _p ? (*_p) : 0; s = _s; v = (!p || !_v) ? call_back_data_size : _v;
		if (!p)
		{
			p = (char *)malloc(v * sizeof(char));
		}
		if (p && v > 0)
		{
			memset(p, 0, v * sizeof(char));
		}
	}
	void copy(const char * _p, unsigned int _s)
	{
		if (_s > 0)
		{
			if (s + _s > v)
			{
				v += _s + 1;
				p = (char *)realloc(p, v * sizeof(char));
				memset(p + s, 0, _s + 1);
			}
			if (p)
			{
				memcpy(p, _p, _s);
				s = _s;
			}
		}
	}
	char * append(char * _p, unsigned int _s)
	{
		if (_s > 0)
		{
			if (s + _s > v)
			{
				v += _s + 1;
				p = (char *)realloc(p, v * sizeof(char));
				memset(p + s, 0, _s + 1);
			}
			if (p)
			{
				memcpy(p + s, _p, _s);
				s += _s;
			}
		}
		return p;
	}
	void exit(char ** _p)
	{
		if (_p && (*_p))
		{
			free((*_p));
			(*_p) = 0;
		}
		s = v = 0;
	}
	void cleanup()
	{
		exit(&p);
		free(this);
	}
}BYTEDATA, *PBYTEDATA;

__inline static size_t writer_callback(void *buffer, size_t size, size_t nmemb, void *user_p)
{
	((BYTEDATA *)user_p)->append((char *)buffer, size * nmemb);
	return size * nmemb;
}

//smtp发送邮件
__inline static int curl_smtp(void)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *headers = NULL;
	struct curl_slist *recipients = NULL;
	struct curl_slist *slist = NULL;
	curl_mime *mime;
	curl_mime *alt;
	curl_mimepart *part;
	const char **cpp;

	curl = curl_easy_init();
	if (curl) 
	{
		/* This is the URL for your mailserver */
		curl_easy_setopt(curl, CURLOPT_URL, SMTP_URL);
		curl_easy_setopt(curl, CURLOPT_USERNAME, USER_NAME);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PASS_WORD);

		/* Note that this option isn't strictly required, omitting it will result
		* in libcurl sending the MAIL FROM command with empty sender data. All
		* autoresponses should have an empty reverse-path, and should be directed
		* to the address in the reverse-path which triggered them. Otherwise,
		* they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
		* details.
		*/
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

		/* Add two recipients, in this particular case they correspond to the
		* To: and Cc: addressees in the header, but they could be any kind of
		* recipient. */
		recipients = curl_slist_append(recipients, TO);
		recipients = curl_slist_append(recipients, TO_CC);
		recipients = curl_slist_append(recipients, TO_CC2);
		recipients = curl_slist_append(recipients, TO_BCC);
		recipients = curl_slist_append(recipients, TO_BCC2);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		/* Build and set the message header list. */
		for (cpp = headers_text; *cpp; cpp++)
		{
			headers = curl_slist_append(headers, *cpp);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* Build the mime message. */
		mime = curl_mime_init(curl);

		/* The inline part is an alternative proposing the html and the text
		versions of the e-mail. */
		alt = curl_mime_init(curl);
		
		/* HTML message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/html;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, inline_html, CURL_ZERO_TERMINATED);

		/* Text message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/plain;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, inline_text, CURL_ZERO_TERMINATED);

		/* Create the inline part. */
		part = curl_mime_addpart(mime);
		curl_mime_subparts(part, alt);
		curl_mime_type(part, "multipart/alternative");
		slist = curl_slist_append(NULL, "Content-Disposition:inline");
		curl_mime_headers(part, slist, 1);

		/* Add the current source program as an attachment. */
		part = curl_mime_addpart(mime);
		curl_mime_filedata(part, "curl_smtp.h");
		
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		//BYTEDATA * pReaderByteData = (BYTEDATA *)BYTEDATA::startup();
		//curl_easy_setopt(curl, CURLOPT_READDATA, pReaderByteData);
		//curl_easy_setopt(curl, CURLOPT_READFUNCTION, &reader_callback);
		//BYTEDATA * pWriterByteData = (BYTEDATA *)BYTEDATA::startup();
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, pWriterByteData);
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer_callback);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Send the message */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		//fprintf(stdout, "%s\n", pWriterByteData->p);
		//pWriterByteData->cleanup();

		/* Free lists. */
		curl_slist_free_all(recipients);
		curl_slist_free_all(headers);

		/* curl won't send the QUIT command until you call cleanup, so you should
		* be able to re-use this connection for additional messages (setting
		* CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
		* curl_easy_perform() again. It may not be a good idea to keep the
		* connection open for a very long time though (more than a few minutes
		* may result in the server timing out the connection), and you do want to
		* clean up in the end.
		*/
		curl_easy_cleanup(curl);

		/* Free multipart message. */
		curl_mime_free(alt);
		curl_mime_free(mime);
	}

	return (int)res;
}

//smtp发送邮件
__inline static int curl_smtp(
	std::string url, 
	std::string username, 
	std::string password, 
	std::string from, 
	std::string fromname,
	std::vector<std::string> tos, 
	std::vector<std::string> ccs, 
	std::vector<std::string> bccs,
	std::vector<std::string> tonames, 
	std::vector<std::string> ccnames, 
	std::vector<std::string> bccnames, 
	std::vector<std::string> headers, 
	std::vector<std::string> files, 
	std::string subject, std::string body)
{
	CURL *curl = 0;
	CURLcode res = CURLE_OK;
	struct curl_slist *headerslist = 0;
	struct curl_slist *recipientslist = 0;
	struct curl_slist *slist = 0;
	curl_mime *mime = 0;
	curl_mime *alt = 0;
	curl_mimepart *part = 0;
	time_t tt = time(0);
	char cDate[MAXCHAR] = { 0 };
	std::string strDate = ("Date:");
	std::string strTo = ("To:");
	std::string strFrom = ("From:");
	std::string strCc = ("Cc:");
	std::string strBcc = ("Bcc:");
	std::string strMessageId = ("Message-ID:");
	std::string strSubject = ("Subject:");

	curl = curl_easy_init();
	if (curl)
	{
		/* This is the URL for your mailserver */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());

		/* Note that this option isn't strictly required, omitting it will result
		* in libcurl sending the MAIL FROM command with empty sender data. All
		* autoresponses should have an empty reverse-path, and should be directed
		* to the address in the reverse-path which triggered them. Otherwise,
		* they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
		* details.
		*/
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

		/* Add two recipients, in this particular case they correspond to the
		* To: and Cc: addressees in the header, but they could be any kind of
		* recipient. */
		
		strftime(cDate, sizeof(cDate) / sizeof(*cDate), ("%#c +0000"), gmtime(&tt));
		strDate.append(cDate);

		strFrom.append("<").append(from).append(">").append("(").append(fromname).append(")");
		strMessageId.append("<").append("dcd9cb36-11db-889a-9f3a-e652a9658efd").append(">");
		strSubject.append(subject);
	
		for each (auto v in tos)
		{
			strTo.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in tonames)
		{
			strTo.append("(").append(v).append(")");
		}
		for each (auto v in ccs)
		{
			strCc.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in ccnames)
		{
			strCc.append("(").append(v).append(")");
		}
		for each (auto v in bccs)
		{
			strBcc.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in bccnames)
		{
			strBcc.append("(").append(v).append(")");
		}
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipientslist);

		/* Build and set the message header list. */
		headerslist = curl_slist_append(headerslist, strDate.c_str());
		headerslist = curl_slist_append(headerslist, strTo.c_str());
		headerslist = curl_slist_append(headerslist, strFrom.c_str());
		headerslist = curl_slist_append(headerslist, strCc.c_str());
		headerslist = curl_slist_append(headerslist, strBcc.c_str());
		headerslist = curl_slist_append(headerslist, strMessageId.c_str());
		headerslist = curl_slist_append(headerslist, strSubject.c_str());
		for each (auto v in headers)
		{
			headerslist = curl_slist_append(headerslist, v.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerslist);

		/* Build the mime message. */
		mime = curl_mime_init(curl);

		/* The inline part is an alternative proposing the html and the text
		versions of the e-mail. */
		alt = curl_mime_init(curl);

		/* HTML message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/html;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);

		/* Text message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/plain;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, ("<html><body>" + body + "</body></html>\r\n").c_str(), CURL_ZERO_TERMINATED);

		/* Create the inline part. */
		part = curl_mime_addpart(mime);
		curl_mime_subparts(part, alt);
		curl_mime_type(part, "multipart/alternative");
		slist = curl_slist_append(NULL, "Content-Disposition:inline");
		curl_mime_headers(part, slist, 1);

		for each (auto v in files)
		{
			/* Add the current source program as an attachment. */
			part = curl_mime_addpart(mime);
			curl_mime_filedata(part, v.c_str());
		}
		
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Send the message */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		/* Free lists. */
		curl_slist_free_all(recipientslist);
		curl_slist_free_all(headerslist);

		/* curl won't send the QUIT command until you call cleanup, so you should
		* be able to re-use this connection for additional messages (setting
		* CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
		* curl_easy_perform() again. It may not be a good idea to keep the
		* connection open for a very long time though (more than a few minutes
		* may result in the server timing out the connection), and you do want to
		* clean up in the end.
		*/
		curl_easy_cleanup(curl);

		/* Free multipart message. */
		curl_mime_free(alt);
		curl_mime_free(mime);
	}

	return (int)res;
}

//smtp-ssl发送邮件
__inline static int curl_smtp_ssl(void)
{
#ifndef SSL_SKIP_PEER_VERIFICATION
#define SSL_SKIP_PEER_VERIFICATION
#endif

#ifndef SSL_SKIP_HOSTNAME_VERIFICATION
#define SSL_SKIP_HOSTNAME_VERIFICATION
#endif
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *headers = NULL;
	struct curl_slist *recipients = NULL;
	struct curl_slist *slist = NULL;
	curl_mime *mime;
	curl_mime *alt;
	curl_mimepart *part;
	const char **cpp;

	curl = curl_easy_init();
	if (curl)
	{
		/* This is the URL for your mailserver */
		curl_easy_setopt(curl, CURLOPT_URL, SMTP_URL);
		//curl_easy_setopt(curl, CURLOPT_URL, SMTPS_URL);
		curl_easy_setopt(curl, CURLOPT_USERNAME, USER_NAME);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PASS_WORD);

		////////////////////////////////////////////////////////////////////////////////////////////////
		/* If you want to connect to a site who isn't using a certificate that is
		* signed by one of the certs in the CA bundle you have, you can skip the
		* verification of the server's certificate. This makes the connection
		* A LOT LESS SECURE.
		*
		* If you have a CA cert for the server stored someplace else than in the
		* default bundle, then the CURLOPT_CAPATH option might come handy for
		* you. */
#ifdef SSL_SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
#endif

		/* If the site you're connecting to uses a different host name that what
		* they have mentioned in their server certificate's commonName (or
		* subjectAltName) fields, libcurl will refuse to connect. You can skip
		* this check, but this will make the connection less secure. */
#ifdef SSL_SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
#endif

		////////////////////////////////////////////////////////////////////////////////////////////////

		/* Note that this option isn't strictly required, omitting it will result
		* in libcurl sending the MAIL FROM command with empty sender data. All
		* autoresponses should have an empty reverse-path, and should be directed
		* to the address in the reverse-path which triggered them. Otherwise,
		* they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
		* details.
		*/
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

		/* Add two recipients, in this particular case they correspond to the
		* To: and Cc: addressees in the header, but they could be any kind of
		* recipient. */
		recipients = curl_slist_append(recipients, TO);
		recipients = curl_slist_append(recipients, TO_CC);
		recipients = curl_slist_append(recipients, TO_CC2);
		recipients = curl_slist_append(recipients, TO_BCC);
		recipients = curl_slist_append(recipients, TO_BCC2);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		/* Build and set the message header list. */
		for (cpp = headers_text; *cpp; cpp++)
		{
			headers = curl_slist_append(headers, *cpp);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* Build the mime message. */
		mime = curl_mime_init(curl);

		/* The inline part is an alternative proposing the html and the text
		versions of the e-mail. */
		alt = curl_mime_init(curl);

		/* HTML message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/html;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, inline_html, CURL_ZERO_TERMINATED);

		/* Text message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/plain;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, inline_text, CURL_ZERO_TERMINATED);

		/* Create the inline part. */
		part = curl_mime_addpart(mime);
		curl_mime_subparts(part, alt);
		curl_mime_type(part, "multipart/alternative");
		slist = curl_slist_append(NULL, "Content-Disposition:inline");
		curl_mime_headers(part, slist, 1);

		/* Add the current source program as an attachment. */
		part = curl_mime_addpart(mime);
		curl_mime_filedata(part, "curl_smtp.h");

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		//BYTEDATA * pReaderByteData = (BYTEDATA *)BYTEDATA::startup();
		//curl_easy_setopt(curl, CURLOPT_READDATA, pReaderByteData);
		//curl_easy_setopt(curl, CURLOPT_READFUNCTION, &reader_callback);
		//BYTEDATA * pWriterByteData = (BYTEDATA *)BYTEDATA::startup();
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, pWriterByteData);
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer_callback);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Send the message */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		//fprintf(stdout, "%s\n", pWriterByteData->p);
		//pWriterByteData->cleanup();

		/* Free lists. */
		curl_slist_free_all(recipients);
		curl_slist_free_all(headers);

		/* curl won't send the QUIT command until you call cleanup, so you should
		* be able to re-use this connection for additional messages (setting
		* CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
		* curl_easy_perform() again. It may not be a good idea to keep the
		* connection open for a very long time though (more than a few minutes
		* may result in the server timing out the connection), and you do want to
		* clean up in the end.
		*/
		curl_easy_cleanup(curl);

		/* Free multipart message. */
		curl_mime_free(alt);
		curl_mime_free(mime);
	}

	return (int)res;
}
//smtp-ssl发送邮件
__inline static int curl_smtp_ssl(
	std::string url,
	std::string username,
	std::string password,
	std::string from,
	std::string fromname,
	std::vector<std::string> tos,
	std::vector<std::string> ccs,
	std::vector<std::string> bccs,
	std::vector<std::string> tonames,
	std::vector<std::string> ccnames,
	std::vector<std::string> bccnames,
	std::vector<std::string> headers,
	std::vector<std::string> files,
	std::string subject, std::string body)
{
#ifndef SSL_SKIP_PEER_VERIFICATION
#define SSL_SKIP_PEER_VERIFICATION
#endif

#ifndef SSL_SKIP_HOSTNAME_VERIFICATION
#define SSL_SKIP_HOSTNAME_VERIFICATION
#endif
	CURL *curl = 0;
	CURLcode res = CURLE_OK;
	struct curl_slist *headerslist = 0;
	struct curl_slist *recipientslist = 0;
	struct curl_slist *slist = 0;
	curl_mime *mime = 0;
	curl_mime *alt = 0;
	curl_mimepart *part = 0;
	time_t tt = time(0);
	char cDate[MAXCHAR] = { 0 };
	std::string strDate = ("Date:");
	std::string strTo = ("To:");
	std::string strFrom = ("From:");
	std::string strCc = ("Cc:");
	std::string strBcc = ("Bcc:");
	std::string strMessageId = ("Message-ID:");
	std::string strSubject = ("Subject:");

	curl = curl_easy_init();
	if (curl)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////
		/* If you want to connect to a site who isn't using a certificate that is
		* signed by one of the certs in the CA bundle you have, you can skip the
		* verification of the server's certificate. This makes the connection
		* A LOT LESS SECURE.
		*
		* If you have a CA cert for the server stored someplace else than in the
		* default bundle, then the CURLOPT_CAPATH option might come handy for
		* you. */
#ifdef SSL_SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
#endif

		/* If the site you're connecting to uses a different host name that what
		* they have mentioned in their server certificate's commonName (or
		* subjectAltName) fields, libcurl will refuse to connect. You can skip
		* this check, but this will make the connection less secure. */
#ifdef SSL_SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
#endif

		////////////////////////////////////////////////////////////////////////////////////////////////

		/* This is the URL for your mailserver */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());

		/* Note that this option isn't strictly required, omitting it will result
		* in libcurl sending the MAIL FROM command with empty sender data. All
		* autoresponses should have an empty reverse-path, and should be directed
		* to the address in the reverse-path which triggered them. Otherwise,
		* they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
		* details.
		*/
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

		/* Add two recipients, in this particular case they correspond to the
		* To: and Cc: addressees in the header, but they could be any kind of
		* recipient. */

		strftime(cDate, sizeof(cDate) / sizeof(*cDate), ("%#c +0000"), gmtime(&tt));
		strDate.append(cDate);

		strFrom.append("<").append(from).append(">").append("(").append(fromname).append(")");
		strMessageId.append("<").append("dcd9cb36-11db-889a-9f3a-e652a9658efd").append(">");
		strSubject.append(subject);

		for each (auto v in tos)
		{
			strTo.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in tonames)
		{
			strTo.append("(").append(v).append(")");
		}
		for each (auto v in ccs)
		{
			strCc.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in ccnames)
		{
			strCc.append("(").append(v).append(")");
		}
		for each (auto v in bccs)
		{
			strBcc.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in bccnames)
		{
			strBcc.append("(").append(v).append(")");
		}
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipientslist);

		/* Build and set the message header list. */
		headerslist = curl_slist_append(headerslist, strDate.c_str());
		headerslist = curl_slist_append(headerslist, strTo.c_str());
		headerslist = curl_slist_append(headerslist, strFrom.c_str());
		headerslist = curl_slist_append(headerslist, strCc.c_str());
		headerslist = curl_slist_append(headerslist, strBcc.c_str());
		headerslist = curl_slist_append(headerslist, strMessageId.c_str());
		headerslist = curl_slist_append(headerslist, strSubject.c_str());
		for each (auto v in headers)
		{
			headerslist = curl_slist_append(headerslist, v.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerslist);

		/* Build the mime message. */
		mime = curl_mime_init(curl);

		/* The inline part is an alternative proposing the html and the text
		versions of the e-mail. */
		alt = curl_mime_init(curl);

		/* HTML message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/html;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);

		/* Text message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/plain;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, ("<html><body>" + body + "</body></html>\r\n").c_str(), CURL_ZERO_TERMINATED);

		/* Create the inline part. */
		part = curl_mime_addpart(mime);
		curl_mime_subparts(part, alt);
		curl_mime_type(part, "multipart/alternative");
		slist = curl_slist_append(NULL, "Content-Disposition:inline");
		curl_mime_headers(part, slist, 1);

		for each (auto v in files)
		{
			/* Add the current source program as an attachment. */
			part = curl_mime_addpart(mime);
			curl_mime_filedata(part, v.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Send the message */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		//fprintf(stdout, "%s\n", pWriterByteData->p);
		//pWriterByteData->cleanup();

		/* Free lists. */
		curl_slist_free_all(recipientslist);
		curl_slist_free_all(headerslist);

		/* curl won't send the QUIT command until you call cleanup, so you should
		* be able to re-use this connection for additional messages (setting
		* CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
		* curl_easy_perform() again. It may not be a good idea to keep the
		* connection open for a very long time though (more than a few minutes
		* may result in the server timing out the connection), and you do want to
		* clean up in the end.
		*/
		curl_easy_cleanup(curl);

		/* Free multipart message. */
		curl_mime_free(alt);
		curl_mime_free(mime);
	}

	return (int)res;
}
//smtp-tls/starttls发送邮件
__inline static int curl_smtp_tls(void)
{
#ifdef TLS_SKIP_PEER_VERIFICATION
#undef TLS_SKIP_PEER_VERIFICATION
#endif

#ifdef TLS_SKIP_HOSTNAME_VERIFICATION
#undef TLS_SKIP_HOSTNAME_VERIFICATION
#endif
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *headers = NULL;
	struct curl_slist *recipients = NULL;
	struct curl_slist *slist = NULL;
	curl_mime *mime;
	curl_mime *alt;
	curl_mimepart *part;
	const char **cpp;

	curl = curl_easy_init();
	if (curl)
	{
		/* This is the URL for your mailserver */
		curl_easy_setopt(curl, CURLOPT_URL, SMTP_URL);
		//curl_easy_setopt(curl, CURLOPT_URL, SMTPS_URL);
		curl_easy_setopt(curl, CURLOPT_USERNAME, USER_NAME);
		curl_easy_setopt(curl, CURLOPT_PASSWORD, PASS_WORD);

		////////////////////////////////////////////////////////////////////////////////////////////////
		/* In this example, we'll start with a plain text connection, and upgrade
		* to Transport Layer Security (TLS) using the STARTTLS command. Be careful
		* of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
		* will continue anyway - see the security discussion in the libcurl
		* tutorial for more details. */
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

		/* If you want to connect to a site who isn't using a certificate that is
		* signed by one of the certs in the CA bundle you have, you can skip the
		* verification of the server's certificate. This makes the connection
		* A LOT LESS SECURE.
		*
		* If you have a CA cert for the server stored someplace else than in the
		* default bundle, then the CURLOPT_CAPATH option might come handy for
		* you. */
#ifdef TLS_SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
#endif

		/* If the site you're connecting to uses a different host name that what
		* they have mentioned in their server certificate's commonName (or
		* subjectAltName) fields, libcurl will refuse to connect. You can skip
		* this check, but this will make the connection less secure. */
#ifdef TLS_SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
#endif

		/* If your server doesn't have a valid certificate, then you can disable
		* part of the Transport Layer Security protection by setting the
		* CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST options to 0 (false).
		*   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		*   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		* That is, in general, a bad idea. It is still better than sending your
		* authentication details in plain text though.  Instead, you should get
		* the issuer certificate (or the host certificate if the certificate is
		* self-signed) and add it to the set of certificates that are known to
		* libcurl using CURLOPT_CAINFO and/or CURLOPT_CAPATH. See docs/SSLCERTS
		* for more information. */
		curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
		////////////////////////////////////////////////////////////////////////////////////////////////

		/* Note that this option isn't strictly required, omitting it will result
		* in libcurl sending the MAIL FROM command with empty sender data. All
		* autoresponses should have an empty reverse-path, and should be directed
		* to the address in the reverse-path which triggered them. Otherwise,
		* they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
		* details.
		*/
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);

		/* Add two recipients, in this particular case they correspond to the
		* To: and Cc: addressees in the header, but they could be any kind of
		* recipient. */
		recipients = curl_slist_append(recipients, TO);
		recipients = curl_slist_append(recipients, TO_CC);
		recipients = curl_slist_append(recipients, TO_CC2);
		recipients = curl_slist_append(recipients, TO_BCC);
		recipients = curl_slist_append(recipients, TO_BCC2);
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

		/* Build and set the message header list. */
		for (cpp = headers_text; *cpp; cpp++)
		{
			headers = curl_slist_append(headers, *cpp);
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		/* Build the mime message. */
		mime = curl_mime_init(curl);

		/* The inline part is an alternative proposing the html and the text
		versions of the e-mail. */
		alt = curl_mime_init(curl);

		/* HTML message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/html;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, inline_html, CURL_ZERO_TERMINATED);

		/* Text message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/plain;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, inline_text, CURL_ZERO_TERMINATED);

		/* Create the inline part. */
		part = curl_mime_addpart(mime);
		curl_mime_subparts(part, alt);
		curl_mime_type(part, "multipart/alternative");
		slist = curl_slist_append(NULL, "Content-Disposition:inline");
		curl_mime_headers(part, slist, 1);

		/* Add the current source program as an attachment. */
		part = curl_mime_addpart(mime);
		curl_mime_filedata(part, "curl_smtp.h");

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		//BYTEDATA * pReaderByteData = (BYTEDATA *)BYTEDATA::startup();
		//curl_easy_setopt(curl, CURLOPT_READDATA, pReaderByteData);
		//curl_easy_setopt(curl, CURLOPT_READFUNCTION, &reader_callback);
		//BYTEDATA * pWriterByteData = (BYTEDATA *)BYTEDATA::startup();
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, pWriterByteData);
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writer_callback);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Send the message */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		//fprintf(stdout, "%s\n", pWriterByteData->p);
		//pWriterByteData->cleanup();

		/* Free lists. */
		curl_slist_free_all(recipients);
		curl_slist_free_all(headers);

		/* curl won't send the QUIT command until you call cleanup, so you should
		* be able to re-use this connection for additional messages (setting
		* CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
		* curl_easy_perform() again. It may not be a good idea to keep the
		* connection open for a very long time though (more than a few minutes
		* may result in the server timing out the connection), and you do want to
		* clean up in the end.
		*/
		curl_easy_cleanup(curl);

		/* Free multipart message. */
		curl_mime_free(alt);
		curl_mime_free(mime);
	}

	return (int)res;
}
//smtp-tls/starttls发送邮件
__inline static int curl_smtp_tls(
	std::string url,
	std::string username,
	std::string password,
	std::string from,
	std::string fromname,
	std::vector<std::string> tos,
	std::vector<std::string> ccs,
	std::vector<std::string> bccs,
	std::vector<std::string> tonames,
	std::vector<std::string> ccnames,
	std::vector<std::string> bccnames,
	std::vector<std::string> headers,
	std::vector<std::string> files,
	std::string subject, std::string body)
{
#ifdef TLS_SKIP_PEER_VERIFICATION
#undef TLS_SKIP_PEER_VERIFICATION
#endif

#ifdef TLS_SKIP_HOSTNAME_VERIFICATION
#undef TLS_SKIP_HOSTNAME_VERIFICATION
#endif
	CURL *curl = 0;
	CURLcode res = CURLE_OK;
	struct curl_slist *headerslist = 0;
	struct curl_slist *recipientslist = 0;
	struct curl_slist *slist = 0;
	curl_mime *mime = 0;
	curl_mime *alt = 0;
	curl_mimepart *part = 0;
	time_t tt = time(0);
	char cDate[MAXCHAR] = { 0 };
	std::string strDate = ("Date:");
	std::string strTo = ("To:");
	std::string strFrom = ("From:");
	std::string strCc = ("Cc:");
	std::string strBcc = ("Bcc:");
	std::string strMessageId = ("Message-ID:");
	std::string strSubject = ("Subject:");

	curl = curl_easy_init();
	if (curl)
	{
		////////////////////////////////////////////////////////////////////////////////////////////////
		/* In this example, we'll start with a plain text connection, and upgrade
		* to Transport Layer Security (TLS) using the STARTTLS command. Be careful
		* of using CURLUSESSL_TRY here, because if TLS upgrade fails, the transfer
		* will continue anyway - see the security discussion in the libcurl
		* tutorial for more details. */
		curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

		/* If you want to connect to a site who isn't using a certificate that is
		* signed by one of the certs in the CA bundle you have, you can skip the
		* verification of the server's certificate. This makes the connection
		* A LOT LESS SECURE.
		*
		* If you have a CA cert for the server stored someplace else than in the
		* default bundle, then the CURLOPT_CAPATH option might come handy for
		* you. */
#ifdef TLS_SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
#endif

		/* If the site you're connecting to uses a different host name that what
		* they have mentioned in their server certificate's commonName (or
		* subjectAltName) fields, libcurl will refuse to connect. You can skip
		* this check, but this will make the connection less secure. */
#ifdef TLS_SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#else
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);
#endif

		/* If your server doesn't have a valid certificate, then you can disable
		* part of the Transport Layer Security protection by setting the
		* CURLOPT_SSL_VERIFYPEER and CURLOPT_SSL_VERIFYHOST options to 0 (false).
		*   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		*   curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		* That is, in general, a bad idea. It is still better than sending your
		* authentication details in plain text though.  Instead, you should get
		* the issuer certificate (or the host certificate if the certificate is
		* self-signed) and add it to the set of certificates that are known to
		* libcurl using CURLOPT_CAINFO and/or CURLOPT_CAPATH. See docs/SSLCERTS
		* for more information. */
		curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
		////////////////////////////////////////////////////////////////////////////////////////////////

		/* This is the URL for your mailserver */
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
		curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());

		/* Note that this option isn't strictly required, omitting it will result
		* in libcurl sending the MAIL FROM command with empty sender data. All
		* autoresponses should have an empty reverse-path, and should be directed
		* to the address in the reverse-path which triggered them. Otherwise,
		* they could cause an endless loop. See RFC 5321 Section 4.5.5 for more
		* details.
		*/
		curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from.c_str());

		/* Add two recipients, in this particular case they correspond to the
		* To: and Cc: addressees in the header, but they could be any kind of
		* recipient. */

		strftime(cDate, sizeof(cDate) / sizeof(*cDate), ("%#c +0000"), gmtime(&tt));
		strDate.append(cDate);

		strFrom.append("<").append(from).append(">").append("(").append(fromname).append(")");
		strMessageId.append("<").append("dcd9cb36-11db-889a-9f3a-e652a9658efd").append(">");
		strSubject.append(subject);

		for each (auto v in tos)
		{
			strTo.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in tonames)
		{
			strTo.append("(").append(v).append(")");
		}
		for each (auto v in ccs)
		{
			strCc.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in ccnames)
		{
			strCc.append("(").append(v).append(")");
		}
		for each (auto v in bccs)
		{
			strBcc.append("<").append(v).append(">");
			recipientslist = curl_slist_append(recipientslist, v.c_str());
		}
		for each (auto v in bccnames)
		{
			strBcc.append("(").append(v).append(")");
		}
		curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipientslist);

		/* Build and set the message header list. */
		headerslist = curl_slist_append(headerslist, strDate.c_str());
		headerslist = curl_slist_append(headerslist, strTo.c_str());
		headerslist = curl_slist_append(headerslist, strFrom.c_str());
		headerslist = curl_slist_append(headerslist, strCc.c_str());
		headerslist = curl_slist_append(headerslist, strBcc.c_str());
		headerslist = curl_slist_append(headerslist, strMessageId.c_str());
		headerslist = curl_slist_append(headerslist, strSubject.c_str());
		for each (auto v in headers)
		{
			headerslist = curl_slist_append(headerslist, v.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerslist);

		/* Build the mime message. */
		mime = curl_mime_init(curl);

		/* The inline part is an alternative proposing the html and the text
		versions of the e-mail. */
		alt = curl_mime_init(curl);

		/* HTML message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/html;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, body.c_str(), CURL_ZERO_TERMINATED);

		/* Text message. */
		part = curl_mime_addpart(alt);
		curl_mime_type(part, "text/plain;charset=gb2312;");
		curl_mime_encoder(part, "7bit");
		curl_mime_data(part, ("<html><body>" + body + "</body></html>\r\n").c_str(), CURL_ZERO_TERMINATED);

		/* Create the inline part. */
		part = curl_mime_addpart(mime);
		curl_mime_subparts(part, alt);
		curl_mime_type(part, "multipart/alternative");
		slist = curl_slist_append(NULL, "Content-Disposition:inline");
		curl_mime_headers(part, slist, 1);

		for each (auto v in files)
		{
			/* Add the current source program as an attachment. */
			part = curl_mime_addpart(mime);
			curl_mime_filedata(part, v.c_str());
		}

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* Send the message */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		//fprintf(stdout, "%s\n", pWriterByteData->p);
		//pWriterByteData->cleanup();

		/* Free lists. */
		curl_slist_free_all(recipientslist);
		curl_slist_free_all(headerslist);

		/* curl won't send the QUIT command until you call cleanup, so you should
		* be able to re-use this connection for additional messages (setting
		* CURLOPT_MAIL_FROM and CURLOPT_MAIL_RCPT as required, and calling
		* curl_easy_perform() again. It may not be a good idea to keep the
		* connection open for a very long time though (more than a few minutes
		* may result in the server timing out the connection), and you do want to
		* clean up in the end.
		*/
		curl_easy_cleanup(curl);

		/* Free multipart message. */
		curl_mime_free(alt);
		curl_mime_free(mime);
	}

	return (int)res;
}
