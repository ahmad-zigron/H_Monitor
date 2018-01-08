/*
 * HTTPProtocol.cc
 *  Created on: Oct 24, 2017
 *      Author: ahmad
 */

#include "../include/HTTPProtocol.hh"
using namespace std;
#define T_A 1 //Ipv4 address

std::vector<std::string>payload_text;

struct upload_status {
  int lines_read;
};
HTTPProtocol::HTTPProtocol() {
	recipients = NULL;
	curl_global_init(CURL_GLOBAL_ALL);
	this->curl = curl_easy_init();
	//-----------------<>
	ConfigUtil *config=new ConfigUtil();
	TO=config->read_config_file("TO");
	stringstream slackchannels(config->read_config_file("SLACK_CHANNELS"));
    slackchannels >> SLACK_CHANNELS;
	for (int i=1; i <= SLACK_CHANNELS; i++){
		auto s = to_string(i);
		hookurl.push_back(config->read_config_file("HOOKURL"+s));		
	}
	config->readEmailAddr("EMAILS",emailVec);
}

HTTPProtocol::~HTTPProtocol() {
	curl_slist_free_all(recipients);
	curl_easy_cleanup(this->curl);
	curl_global_cleanup();
}

size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  std::string data;
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
  data = payload_text[upload_ctx->lines_read];
  if(data!="\0") {
	  size_t len = data.length();
	  memcpy(ptr, &data[0], len);
	  upload_ctx->lines_read++;
	  return len;
  }
  return 0;
}
char dns_servers[10][100];
struct DNS_HEADER
{
    unsigned short id; // identification number

    unsigned char rd :1; // recursion desired
    unsigned char tc :1; // truncated message
    unsigned char aa :1; // authoritive answer
    unsigned char opcode :4; // purpose of message
    unsigned char qr :1; // query/response flag

    unsigned char rcode :4; // response code
    unsigned char cd :1; // checking disabled
    unsigned char ad :1; // authenticated data
    unsigned char z :1; // its z! reserved
    unsigned char ra :1; // recursion available

    unsigned short q_count; // number of question entries
    unsigned short ans_count; // number of answer entries
    unsigned short auth_count; // number of authority entries
    unsigned short add_count; // number of resource entries
};
struct QUESTION
{
    unsigned short qtype;
    unsigned short qclass;
};
struct R_DATA
{
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};
struct RES_RECORD
{
    unsigned char *name;
    struct R_DATA *resource;
    unsigned char *rdata;
};
typedef struct
{
    unsigned char *name;
    struct QUESTION *ques;
} QUERY;
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;

    *count = 1;
    name = (unsigned char*)malloc(256);

    name[0]='\0';

    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }

        reader = reader+1;

        if(jumped==0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }

    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }

    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++)
    {
        p=name[i];
        for(j=0;j<(int)p;j++)
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}
void ChangetoDnsNameFormat(unsigned char* dns,unsigned char* host)
{
    int lock = 0 , i;
    strcat((char*)host,".");

    for(i = 0 ; i < strlen((char*)host) ; i++)
    {
        if(host[i]=='.')
        {
            *dns++ = i-lock;
            for(;lock<i;lock++)
            {
                *dns++=host[lock];
            }
            lock++; //or lock=i+1;
        }
    }
    *dns++='\0';
}
bool ngethostbyname(unsigned char *host , int query_type)
{
    unsigned char buf[65536],*qname,*reader;
    int i , j , stop , s;

    struct sockaddr_in a;

    struct RES_RECORD answers[20],auth[20],addit[20]; //the replies from the DNS server
    struct sockaddr_in dest;

    struct DNS_HEADER *dns = NULL;
    struct QUESTION *qinfo = NULL;

    printf("Resolving %s" , host);

    s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP); //UDP packet for DNS queries

    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(dns_servers[0]); //dns servers

    //Set the DNS structure to standard queries
    dns = (struct DNS_HEADER *)&buf;

    dns->id = (unsigned short) htons(getpid());
    dns->qr = 0; //This is a query
    dns->opcode = 0; //This is a standard query
    dns->aa = 0; //Not Authoritative
    dns->tc = 0; //This message is not truncated
    dns->rd = 1; //Recursion Desired
    dns->ra = 0; //Recursion not available! hey we dont have it (lol)
    dns->z = 0;
    dns->ad = 0;
    dns->cd = 0;
    dns->rcode = 0;
    dns->q_count = htons(1); //we have only 1 question
    dns->ans_count = 0;
    dns->auth_count = 0;
    dns->add_count = 0;

    //point to the query portion
    qname =(unsigned char*)&buf[sizeof(struct DNS_HEADER)];

    ChangetoDnsNameFormat(qname , host);
    qinfo =(struct QUESTION*)&buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname) + 1)]; //fill it

    qinfo->qtype = htons( query_type ); //type of the query , A , MX , CNAME , NS etc
    qinfo->qclass = htons(1); //its internet (lol)

    printf("\nSending Packet...");
    if( sendto(s,(char*)buf,sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION),0,(struct sockaddr*)&dest,sizeof(dest)) < 0)
    {
        perror("sendto failed");
    }
    printf("Done");

    //Receive the answer
    i = sizeof dest;
    printf("\nReceiving answer...");
    struct timeval tv;//ahmad's change
    tv.tv_sec = 1;//ahmad's change
    tv.tv_usec = 100000;//ahmad's change
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {//ahmad's change
        perror("Error");//ahmad's change
    }//ahmad's change
    bool status=true;//ahmad's change
    if(recvfrom (s,(char*)buf , 65536 , 0 , (struct sockaddr*)&dest , (socklen_t*)&i ) < 0)
    {
        perror("recvfrom failed");
        status=false;//ahmad's change
    }
    printf("Done");

    dns = (struct DNS_HEADER*) buf;

    //move ahead of the dns header and the query field
    reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char*)qname)+1) + sizeof(struct QUESTION)];

//    printf("\nThe response contains : ");//ahmad's change
//    printf("\n %d Questions.",ntohs(dns->q_count));//ahmad's change
//    printf("\n %d Answers.",ntohs(dns->ans_count));//ahmad's change
//    printf("\n %d Authoritative Servers.",ntohs(dns->auth_count));//ahmad's change
//    printf("\n %d Additional records.\n\n",ntohs(dns->add_count));//ahmad's change

    //Start reading answers
    stop=0;

    for(i=0;i<ntohs(dns->ans_count);i++)
    {
        answers[i].name=ReadName(reader,buf,&stop);
        reader = reader + stop;

        answers[i].resource = (struct R_DATA*)(reader);
        reader = reader + sizeof(struct R_DATA);

        if(ntohs(answers[i].resource->type) == 1) //if its an ipv4 address
        {
            answers[i].rdata = (unsigned char*)malloc(ntohs(answers[i].resource->data_len));

            for(j=0 ; j<ntohs(answers[i].resource->data_len) ; j++)
            {
                answers[i].rdata[j]=reader[j];
            }

            answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

            reader = reader + ntohs(answers[i].resource->data_len);
        }
        else
        {
            answers[i].rdata = ReadName(reader,buf,&stop);
            reader = reader + stop;
        }
    }

    //read authorities
    for(i=0;i<ntohs(dns->auth_count);i++)
    {
        auth[i].name=ReadName(reader,buf,&stop);
        reader+=stop;

        auth[i].resource=(struct R_DATA*)(reader);
        reader+=sizeof(struct R_DATA);

        auth[i].rdata=ReadName(reader,buf,&stop);
        reader+=stop;
    }

    //read additional
    for(i=0;i<ntohs(dns->add_count);i++)
    {
        addit[i].name=ReadName(reader,buf,&stop);
        reader+=stop;

        addit[i].resource=(struct R_DATA*)(reader);
        reader+=sizeof(struct R_DATA);

        if(ntohs(addit[i].resource->type)==1)
        {
            addit[i].rdata = (unsigned char*)malloc(ntohs(addit[i].resource->data_len));
            for(j=0;j<ntohs(addit[i].resource->data_len);j++)
            addit[i].rdata[j]=reader[j];

            addit[i].rdata[ntohs(addit[i].resource->data_len)]='\0';
            reader+=ntohs(addit[i].resource->data_len);
        }
        else
        {
            addit[i].rdata=ReadName(reader,buf,&stop);
            reader+=stop;
        }
    }

    //print answers
    //printf("\nAnswer Records : %d \n" , ntohs(dns->ans_count) );//ahmad's change
    for(i=0 ; i < ntohs(dns->ans_count) ; i++)
    {
        //printf("Name : %s ",answers[i].name);//ahmad's change

        if( ntohs(answers[i].resource->type) == T_A) //IPv4 address
        {
            long *p;
            p=(long*)answers[i].rdata;
            a.sin_addr.s_addr=(*p);
            //printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));//ahmad's change
        }

        if(ntohs(answers[i].resource->type)==5)
        {
            //Canonical name for an alias
            //printf("has alias name : %s",answers[i].rdata);//ahmad's change
        }

        printf("\n");
    }

    //print authorities
    //printf("\nAuthoritive Records : %d \n" , ntohs(dns->auth_count) );//ahmad's change
    for( i=0 ; i < ntohs(dns->auth_count) ; i++)
    {

        //printf("Name : %s ",auth[i].name);//ahmad's change
        if(ntohs(auth[i].resource->type)==2)
        {
            //printf("has nameserver : %s",auth[i].rdata);//ahmad's change
        }
        printf("\n");
    }

    //print additional resource records
    //printf("\nAdditional Records : %d \n" , ntohs(dns->add_count) );//ahmad's change
    for(i=0; i < ntohs(dns->add_count) ; i++)
    {
        //printf("Name : %s ",addit[i].name);//ahmad's change
        if(ntohs(addit[i].resource->type)==1)
        {
            long *p;
            p=(long*)addit[i].rdata;
            a.sin_addr.s_addr=(*p);
            //printf("has IPv4 address : %s",inet_ntoa(a.sin_addr));//ahmad's change
        }
        //printf("\n");//ahmad's change
    }
    return status;//ahmad's change
}
bool HTTPProtocol::sendSlackMessage(std::string msg){
	CURLcode res = CURLE_OK;
	struct curl_slist *list = NULL;
    list = curl_slist_append(list, "Content-type: application/json"); 
	string payloadstr = "{\"username\": \"H-M\", \"text\":\""+msg+"\", \"icon_emoji\": \":nerd_face:\"}";
    const char *payload = payloadstr.c_str();
	this->curl = curl_easy_init();
	if (this->curl){
		curl_easy_setopt(this->curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(this->curl, CURLOPT_POSTFIELDSIZE, (long) strlen(payload));
		curl_easy_setopt(this->curl, CURLOPT_POSTFIELDS, payload);
		for (const string& url : hookurl){
			curl_easy_setopt(this->curl, CURLOPT_URL, url.c_str());
			cout<<"performing curl on url: "<<url;
			res = curl_easy_perform(this->curl);
			if(res == CURLE_OK){
				continue;
			}
			else{
				return false;
			}
		}	
	}
	return true;
}

void HTTPProtocol::mailMessage(std::string msg){
	time_t now = time(0);
	std::string str(ctime(&now));
	std::string str1(str, 0, str.size()-1);
	payload_text.push_back("Date:"+ str1 +"\r\n");
	payload_text.push_back("To:"+ TO +"\r\n");
	payload_text.push_back("From: " FROM " (pdns-user)\r\n");
	//payload_text.push_back("Cc: " CC " (Another pdns User)\r\n");
	payload_text.push_back("Message-ID: <dcd7cb36-11db-487a-9f3a-e652a9458efd@rfcpedant.example.org>\r\n");
	payload_text.push_back("Subject: PDNS-Service alert notification \r\n");
	payload_text.push_back("\r\n");
	payload_text.push_back("\nDear subscriber,\n\nSERVICE ALERT: SERVICE LOG.\n");
	payload_text.push_back("Following server has service problems. Please login to server for complete logs.\n\n");
	payload_text.push_back(msg);
	payload_text.push_back("\n\nRegards, \nPDNS Health Monitor \nZigron TEAM  \n\n");
	payload_text.push_back("This is an automatically generated email – please do not reply to it.");
	payload_text.push_back("\0");
}

std::string HTTPProtocol::getStatus(std::string ip,std::string port){
	std::string status = "";
	int socket_desc;
	struct sockaddr_in server;
	char *message , server_reply[2000];
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1){
		printf("Could not create socket");
	}
	server.sin_addr.s_addr = inet_addr(ip.c_str());
	server.sin_family = AF_INET;
	stringstream convert(port);
	int p;
	convert>>p;
	server.sin_port = htons( p );
	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
		//puts("connect error");
		status="down";
	}
	else{
		//puts("Connected\n");
		status="up";
	}
	return status;
}
std::string HTTPProtocol::getPDNSStatus(std::string ip){
	unsigned char urlArray[4][100]={"google.com","youtube.com","facebook.com","twitter.com"};
	//dns_servers[0]="";
	strcpy(dns_servers[0] , ip.c_str());
	std::cout<<"dns_servers[0]: "<<dns_servers[0]<<std::endl;
	bool status=false;
	for(int i=0; i<4; i++){
		status=ngethostbyname(urlArray[i] , T_A);
		if(status==true)
			return "up";
	}
	//-------------------------------<>
	return "down";
}

bool HTTPProtocol::sendEmail(){
	CURLcode res = CURLE_OK;
	recipients = NULL;
	struct upload_status upload_ctx;
	upload_ctx.lines_read = 0;
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(this->curl, CURLOPT_USERNAME, USERNAME);
		curl_easy_setopt(this->curl, CURLOPT_PASSWORD, PASSWORD);
		//curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.googlemail.com");
		curl_easy_setopt(this->curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");//"smtps://plus.smtp.mail.yahoo.com"
#ifdef SKIP_PEER_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
		curl_easy_setopt(this->curl, CURLOPT_MAIL_FROM, FROM);
		recipients = curl_slist_append(recipients, TO.c_str());
		for(int i=0; i<emailVec.size(); i++){
			recipients = curl_slist_append(recipients, emailVec[i].c_str());
		}
		//recipients = curl_slist_append(recipients, CC);
		curl_easy_setopt(this->curl, CURLOPT_MAIL_RCPT, recipients);
		curl_easy_setopt(this->curl, CURLOPT_READFUNCTION, payload_source);
		curl_easy_setopt(this->curl, CURLOPT_READDATA, &upload_ctx);
		curl_easy_setopt(this->curl, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(this->curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(this->curl, CURLOPT_TIMEOUT, 15L);//request will wait the response for 15 seconds
		res = curl_easy_perform(this->curl);
		if(res != CURLE_OK){
			payload_text.clear();
			return false;
		}
		else{
			//std::cout<<"success curl: "<<res<<std::endl; success return 0
			payload_text.clear();
			return true;
		}
	}
	return false;
}
