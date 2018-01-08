/*ConfigUtil.hh
 *  Created on: Aug 16, 2017
 *      Author: ahmad
 */

#ifndef READCONFIG_HH_
#define READCONFIG_HH_
#include <libconfig.h>
#include <vector>
#include <string>

#define SD_CONF_FILE "/usr/local/etc/monitor.cfg"

class ConfigUtil{
public:
	/**
	 * @ Param 'key' will be matched in config  file .
	 * read_config will return concerned value of key.
	 */
	std::string read_config_file(std::string key);

	void readEmailAddr(std::string key,std::vector<std::string> &emailVec);

	ConfigUtil();
	~ConfigUtil();
};



#endif /* READCONFIG_HH_ */
