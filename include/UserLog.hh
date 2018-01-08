/*
 * UserLog.hh
 *  Created on: Sep 26, 2017
 *      Author: ahmad
 */

#ifndef USERLOG_H_
#define USERLOG_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "ConfigUtil.hh"

/**
 * 'fileSize' is managing entry level of stats. if you set 1 level then near
 * about 300 to 400 record will be insert and after that log file will be
 * cleared automatically. if we set level 10 then 10 time record of this ratio
 * will be inserted and after that log file will be cleared.
 * you can set this level of user stats in purdns.conf file.
 */
class UserLog {
	std::ofstream myfile;
	int fileSize;
	bool resetFlag;
	std::string path;
	static UserLog *instance;
	void init();
	UserLog();
public:
	/**
	 * writeStats(std::vector<std::string>vec) will take a vector of string
	 * containing stats to write on file.
	 */
	void writeStatFile(std::string record);

	/**
	 * getInstance will return the same object of UserLog class on every
	 * call to getInstance method
	 */
	static UserLog* getInstance();

	~UserLog();
};

#endif /* USERLOG_H_ */
