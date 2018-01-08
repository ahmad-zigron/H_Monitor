/*
 * ServiceDiscovery.hh
 *  Created on: Oct 24, 2017
 *      Author: ahmad
 */

#ifndef SERVICEDISCOVERY_HH_
#define SERVICEDISCOVERY_HH_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include "HTTPProtocol.hh"
#include "UserLog.hh"
#include "IProtocol.hh"
#include <limits.h>
#include <unistd.h>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <rude/config.h>

#define CYREN_INSTANCES	3
#define ADBLOCKER_INSTANCES	3
#define ADBLOCKER "adblocker"
#define CYREN "ctwsd-ad"

class ServiceDiscovery {
	/**
	 * you have to define pdns,MongodbServer,UrlServer,CyrenServer's
	 * ip and port here
	 */
	std::string pdnsIp;
	std::string pdnsPort;
	std::string mongoIp;
	std::string mongoPort;
	std::string urlServerIp;
	std::string urlServerPort;
	std::string cyrenServerIps[CYREN_INSTANCES];
	std::string cyrenServerPorts[CYREN_INSTANCES];
	std::string cyrenStatusArr[CYREN_INSTANCES];
	std::string adBlockerIps[ADBLOCKER_INSTANCES];
	std::string adBlockerPorts[ADBLOCKER_INSTANCES];
	std::string adblockStatusArr[ADBLOCKER_INSTANCES];
	std::string HOSTNAME;
	std::string lmIP;
	int fileSize;
	std::string filePath;
	std::string strMail,strMail2,strText;

	void init();
	/**
	 *  This  data  member names  instance is to apply sigleton   pattern.
	 */
	static ServiceDiscovery *instance;

	ServiceDiscovery();
public:
	/**
	 * discoveryStart method will call serverStatus method to check status
	 * and   will call to mailMessage to prepare Email  and then will call
	 * sendEmail to send Email if any server is down.
	 * @Param: we  can set  interval  time  between each service those are
	 * monitored.If you have not sent any parameter then  defualt value is
	 * 5 seconds.
	 */
	void discoveryStart(int discInterval=5);

	/**
	 * getInstance will provide us a sigle object of this class.
	 */
	static ServiceDiscovery* getInstance();

	std::string getTimeString();
	std::string getLocalMachineIp();
	void reStartSetStrings(std::string serviceName,std::string ip,std::string port);
	bool setStrings(std::string serviceName,std::string ip,std::string port,std::string status);

	virtual ~ServiceDiscovery();
};

#endif /* SERVICEDISCOVERY_HH_ */
