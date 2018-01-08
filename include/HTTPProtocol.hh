/*
 * HTTPProtocol.hh
 *  Created on: Oct 24, 2017
 *      Author: ahmad
 */

#ifndef HTTPPROTOCOL_HH_
#define HTTPPROTOCOL_HH_
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>
#include <iostream>
#include "IProtocol.hh"
#include<arpa/inet.h>
#include <sstream>
#include "ConfigUtil.hh"

/**
 *Here you have to define sender and receiver 's email addresses
 *after enable the option of less secure access in sender's email
 *account.
 */
#define FROM    "<testahmad41@gmail.com>"
#define USERNAME "testahmad41@gmail.com"
#define PASSWORD "1234567ABCabc"


class HTTPProtocol :public IProtocol{
	CURL *curl;
	//std::string FROM;
	std::string TO;
	int SLACK_CHANNELS;
	std::vector<std::string> hookurl;
	std::vector<std::string>emailVec;
public:
	struct curl_slist *recipients;

	bool sendSlackMessage(std::string msg);

	/**
	 * sendEmail method will be called after calling mailMessage
	 * to send an email.
	 */
	bool sendEmail();

	/**
	 * we will send our email message as a string to mailMessage
	 * method to prepare email.
	 */
	void mailMessage(std::string msg);

	/**
	 * we  will send server url string(ip and port) to getStatus
	 * method  and will get result as string as 'up' or 'down'.
	 */
	std::string getStatus(std::string ip,std::string port);

	std::string getPDNSStatus(std::string ip);

	/**
	 * Here in constructor  we will  perform curl init operations
	 */
	HTTPProtocol();

	/**
	 * Here in destructor we will perform curl cleanup operations
	 */
	virtual ~HTTPProtocol();
};

#endif /* HTTPPROTOCOL_HH_ */
