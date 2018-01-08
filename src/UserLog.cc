/*
 * UserLog.cc
 *  Created on: Sep 26, 2017
 *      Author: ahmad
 */

#include "../include/UserLog.hh"
using namespace std;

UserLog *UserLog::instance = 0;

UserLog::~UserLog() {
	this->myfile.close();
}

UserLog::UserLog() {
	resetFlag=false;
	init();

}
void UserLog::init(){
	ConfigUtil config;
	path=config.read_config_file("SD-LOG-FILE");
	std::string s=config.read_config_file("SD-LOG-FILE-SIZE");
	stringstream convert(s);
	convert>>fileSize;
}

void UserLog::writeStatFile(std::string record){
	std::ifstream in(path.c_str(), std::ifstream::ate | std::ifstream::binary);
	int size=in.tellg();// will tell the size of file in bytes

	if(size>=fileSize){
		resetFlag=true;
	}
	if(resetFlag==true){
		myfile.open(path.c_str(), std::ios::out | std::ios::trunc );
		if (myfile.is_open())
			myfile.close();
		resetFlag=false;
	}

	myfile.open (path.c_str(),std::ios::out | std::ios::app);
	if (myfile.is_open()){
		myfile <<record<<std::endl;
		myfile.close();
	}
}

UserLog* UserLog::getInstance(){
	if (!instance){
		instance = new UserLog();
	}
	return instance;
}

