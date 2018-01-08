all:
	g++ ./src/PdnsHm.cc ./src/ServiceDiscovery.cc ./src/UserLog.cc ./src/HTTPProtocol.cc ./src/ConfigUtil.cc -o pdns-hm -lcurl -lconfig -lrudeconfig -std=c++11

.PHONY: install	
install:
	cp monitor.cfg /usr/local/etc/
	cp pdns-hm /etc/cron.d/

.PHONY: clean
clean:
	rm -f pdns-hm
	rm -f /etc/cron.d/pdns-hm
	rm -f /usr/local/etc/monitor.cfg
