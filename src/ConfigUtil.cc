/*ConfigUtil.cc
 *  Created on: Aug 16, 2017
 *      Author: ahmad
 */
#include <string>
#include <stdio.h>
#include "iostream"
#include <libconfig.h>
#include "../include/ConfigUtil.hh"
using namespace std;

ConfigUtil::ConfigUtil(){
}
ConfigUtil::~ConfigUtil(){
}
void ConfigUtil::readEmailAddr(std::string key,std::vector<std::string> &emailVec){
	config_t cfg;
	config_init(&cfg);
	const char *input_file = SD_CONF_FILE;
	const char *str;
	std::string value;
	if(! config_read_file(&cfg, input_file))
	{
		config_destroy(&cfg);
		return ;
	}

	if(key!="EMAILS"){
		if(config_lookup_string(&cfg, key.c_str(), &str))
		{
			value=str;
		}
	}
	else{
		config_setting_t *setting;
		setting = config_lookup(&cfg, "EMAILS");
		int n = 0;
		if(setting!=NULL){
			while (1) {
				const char *val1=config_setting_get_string_elem(setting, n);
				if (val1 == NULL) {
					break;
				}
				std::string email=val1;
				emailVec.push_back(email);
				n++;
			}
		}
	}
	config_destroy(&cfg);
}
std::string ConfigUtil::read_config_file(std::string key)
{
	config_t cfg;
	config_init(&cfg);
	const char *input_file = SD_CONF_FILE;
	const char *str;
	std::string value;
	if(! config_read_file(&cfg, input_file))
	{
		config_destroy(&cfg);
		return "";
	}
	if(config_lookup_string(&cfg, key.c_str(), &str))
	{
		value=str;
	}
	config_destroy(&cfg);
	return value;
}
