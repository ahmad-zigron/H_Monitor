/*
 * ServiceDiscovery.cc
 *  Created on: Oct 24, 2017
 *      Author: ahmad
 */

#include "../include/ServiceDiscovery.hh"
#include "../include/ConfigUtil.hh"
using namespace std;

ServiceDiscovery *ServiceDiscovery::instance = 0;
ServiceDiscovery::ServiceDiscovery() {
	init();
}
void ServiceDiscovery::init(){
	char hostname[HOST_NAME_MAX];
	gethostname(hostname, HOST_NAME_MAX);
	HOSTNAME=std::string(hostname);
	lmIP=getLocalMachineIp();
	strMail="",strText="";
	ConfigUtil config;
	pdnsIp=config.read_config_file("PDNS-IP");
	pdnsPort=config.read_config_file("PDNS-PORT");
	mongoIp=config.read_config_file("MONGODB-IP");
	mongoPort=config.read_config_file("MONGODB-PORT");
	for(int i=0; i<CYREN_INSTANCES; i++){
		std::stringstream ss;
		ss << i+1;
		cyrenServerIps[i]=config.read_config_file("CYREN-SERVER-IP"+ss.str());
		cyrenServerPorts[i]=config.read_config_file("CYREN-SERVER-PORT"+ss.str());
		cyrenStatusArr[i]="down";
	}
	for(int j=0; j<ADBLOCKER_INSTANCES; j++){
		std::stringstream ss;
		ss << j+1;
		adBlockerIps[j]=config.read_config_file("ADBLOCKER-IP"+ss.str());
		adBlockerPorts[j]=config.read_config_file("ADBLOCKER-PORT"+ss.str());
		adblockStatusArr[j]="down";
	}
	
	urlServerIp=config.read_config_file("URL-SERVER-IP");
	urlServerPort=config.read_config_file("URL-SERVER-PORT");
	filePath=config.read_config_file("SD-LOG-FILE");
	std::string s=config.read_config_file("SD-LOG-FILE-SIZE");
	stringstream convert(s);
	convert>>fileSize;
}
ServiceDiscovery::~ServiceDiscovery() {
}

ServiceDiscovery* ServiceDiscovery::getInstance(){
	if (!instance){
		instance = new ServiceDiscovery();
	}
	return instance;
}

std::string ServiceDiscovery::getLocalMachineIp(){
	char number[100];
	std::string lmIP,interface;
	FILE *f = popen("ip route get  8.8.8.8 | awk '/dev/ {print $5}'", "r");
	while (fgets(number, 100, f) != NULL) {
		interface=number;
		interface[interface.length()-1]='\0';
	}
	pclose(f);
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ-1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	lmIP=inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	return lmIP;
}
std::string ServiceDiscovery::getTimeString(){
	time_t now;
	now = time(0);
	std::string dateStr;
	dateStr=ctime(&now);
	dateStr[dateStr.size()-1]=' ';
	return dateStr;
}
bool ServiceDiscovery::setStrings(std::string serviceName,std::string ip,std::string port,std::string status){
	bool mailFlag=false;
	if(status=="down"){
		mailFlag=true;
		strMail+=getTimeString()+" , "+HOSTNAME+" , "+lmIP+" , "+serviceName+" , "+ip+":"+port+" , "+status+" \r\n";
	}
	strText+=getTimeString()+" , "+HOSTNAME+" , "+lmIP+" , "+serviceName+" , "+ip+":"+port+" , "+status;
	return mailFlag;
}
void ServiceDiscovery::reStartSetStrings(std::string serviceName,std::string ip,std::string port){
	std::string cmd="docker restart "+ serviceName +" &";
	popen(cmd.c_str(), "r");
	HTTPProtocol http;
	IProtocol *httpPtr=&http;
	std::string status;
//	status=httpPtr->getStatus(ip,port);
//	if(status=="down")
//		popen(cmd.c_str(), "r");
	sleep(1);
	status=httpPtr->getStatus(ip,port);
	strMail2+=getTimeString()+" , "+HOSTNAME+" , "+lmIP+" , "+serviceName+" , "+ip+":"+port+" , "+status+" \r\n";
	strText+=" \n"+getTimeString()+" , "+HOSTNAME+" , "+lmIP+" , "+serviceName+" , "+ip+":"+port+" , "+status;
}
void ServiceDiscovery::discoveryStart(int discInterval){
	HTTPProtocol http;
	IProtocol *httpPtr=&http;
	bool mailFlag=false;
	std::string pdnsStatus="down",mongoStatus="down",urlServerStatus="down";
	//------------------------------------------------<>
	//pdnsStatus = httpPtr->getStatus(pdnsIp,pdnsPort);
	pdnsStatus = httpPtr->getPDNSStatus(pdnsIp);
	if(setStrings("PDNS",pdnsIp,pdnsPort,pdnsStatus))
		mailFlag=true;
	strText+=" \n";
	mongoStatus = httpPtr->getStatus(mongoIp,mongoPort);
	if(setStrings("mongodb",mongoIp,mongoPort,mongoStatus))
		mailFlag=true;
	strText+=" \n";
	urlServerStatus = httpPtr->getStatus(urlServerIp,urlServerPort);
	if(setStrings("urlserver",urlServerIp,urlServerPort,urlServerStatus))
		mailFlag=true;
	strText+=" \n";
	for(int i=0; i<CYREN_INSTANCES; i++){
		cyrenStatusArr[i] = httpPtr->getStatus(cyrenServerIps[i],cyrenServerPorts[i]);
		std::stringstream ss;
		ss << i;
		if(setStrings(CYREN+ss.str(),cyrenServerIps[i],cyrenServerPorts[i],cyrenStatusArr[i]))
			mailFlag=true;
		strText+=" \n";
	}
	for(int i=0; i<ADBLOCKER_INSTANCES; i++){
		adblockStatusArr[i] = httpPtr->getStatus(adBlockerIps[i],adBlockerPorts[i]);
		std::stringstream ss;
		ss << i;
		if(setStrings(ADBLOCKER+ss.str(),adBlockerIps[i],adBlockerPorts[i],adblockStatusArr[i]))
			mailFlag=true;
		if(i<ADBLOCKER_INSTANCES-1)
			strText+=" \n";
	}
	//-----------------------------------------<>
	if(mailFlag==true){
		httpPtr->sendSlackMessage(strMail);
		httpPtr->mailMessage(strMail);
		httpPtr->sendEmail();
		sleep(discInterval);
	}
	//-----------------------------------------<>
	bool reEmailFlag=false;
	if(pdnsStatus=="down"){
		popen("docker cp pdns:/usr/local/etc/puredns.conf /tmp/puredns.conf", "r");
		rude::Config config;
		if(config.load("/tmp/puredns.conf")){
			//bool pdns_safe_mode = config.getBoolValue("pdns-safe-mode");
			config.setBoolValue("pdns-safe-mode", true);
			config.save();
			popen("docker cp /tmp/puredns.conf pdns:/usr/local/etc/puredns.conf", "r");
		}
		else
			std::cout<<"error during updating safe-mode flag 'puredns.conf'..!"<<std::endl;
		reEmailFlag=true;
		reStartSetStrings("pdns",pdnsIp,pdnsPort);
	}
	for(int i=0; i<CYREN_INSTANCES; i++){
		std::stringstream ss;
		ss << i;
		if(cyrenStatusArr[i]=="down"){
			reEmailFlag=true;
			reStartSetStrings(CYREN+ss.str(),cyrenServerIps[i],cyrenServerPorts[i]);
		}
	}
	for(int i=0; i<ADBLOCKER_INSTANCES; i++){
		std::stringstream ss;
		ss << i;
		if(adblockStatusArr[i]=="down"){
			reEmailFlag=true;
			reStartSetStrings(ADBLOCKER+ss.str(),adBlockerIps[i],adBlockerPorts[i]);
		}
	}
	if(reEmailFlag==true){
		httpPtr->sendSlackMessage(strMail2);
		httpPtr->mailMessage(strMail2);
		httpPtr->sendEmail();
		sleep(discInterval);
	}
	UserLog *userLogOBJ=UserLog::getInstance();
	userLogOBJ->writeStatFile(strText);
}
