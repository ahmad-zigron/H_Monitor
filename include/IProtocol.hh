/*
 * IProtocol.hh
 *  Created on: Oct 24, 2017
 *      Author: ahmad
 */

#ifndef IPROTOCOL_HH_
#define IPROTOCOL_HH_
#include <string>

/**
 * IProtocol is an pure abstract class to be inherit
 */
class IProtocol{
public:
	virtual bool sendEmail()=0;
	virtual void mailMessage(std::string msg)=0;
	virtual std::string getStatus(std::string ip,std::string port)=0;
	virtual bool sendSlackMessage(std::string msg)=0;
	virtual std::string getPDNSStatus(std::string ip)=0;

	IProtocol(){}
	virtual ~IProtocol(){}
};



#endif /* IPROTOCOL_HH_ */
