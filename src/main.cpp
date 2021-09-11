#include "XTcp.cpp"

#include<string>
#include <sys/types.h>
#include <sys/wait.h>

#include<regex>
using namespace std;

class HttpThread{
public:
	void Main(){
		char buf[10000]={0};

		
		int recvlen=client.Recv(buf,sizeof(buf)-1);
		if(recvlen<=0)
		{
			return Close();
		}
		printf("-------------recv----------------------\n%s---------------recv----------------\n\n",buf);

		string src=buf;


		/*
		   ?????????????????????????????????????????????src: GET /?nihaoa&aaaaa HTTP/1.1
		    Host: 8.210.70.64:34567
			Upgrade-Insecure-Requests: 1
			User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36
			Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,;q=0.8,application/signed-exchange;v=b3;q=0.9
			Accept-Encoding: gzip, deflate
			Accept-Language: zh-CN,zh;q=0.9
			Connection: close
			type: [GET]
			path: [/]
			filetype: []
			query: [nihaoa&aaaaa]
			sended size = 90
	    */



		
			// / index.html /ff
			//string pattern="^([A-Z]+) /([a-zA-Z0-9]*([.].*)?) HTTP/1";
		string pattern="^([A-Z]+) /([a-zA-Z0-9]*([.][a-zA-Z0-9]*)?)[?]?(.*) HTTP/1";
		regex r(pattern);
		smatch mas;
		regex_search(src,mas,r);
		if(mas.size()==0){
			printf("%s failed\n",pattern.c_str());
			return Close();
		}

		string type=mas[1];
		string path="/";
		path+=mas[2];

		string filetype = mas[3];

		string query=mas[4];


		if(filetype.size()>0)
			filetype=filetype.substr(1,filetype.size()-1);

		printf("type: [%s]\npath: [%s]\nfiletype: [%s]\nquery: [%s]\n",\
			type.c_str(),path.c_str(),filetype.c_str(),query.c_str());



		//download twitter   query
		if(query.size()>=8&&query.substr(0,8)=="twitter&"){

			//獲取輸入的twitter下載的url
			string url=query.substr(8);

			printf("start download  %s\n",query.c_str());

			int id=fork();
			if(id==0){
				//execl("proxychains","python3","twitter-dl.py",query.c_str());
				//execl("/usr/bin/proxychains","proxychains","python3","twitter-dl.py","-w","500",query.c_str(),NULL);

				int id2=fork();

				if(id2==0){
					if(execl("/usr/local/bin/you-get","you-get",url.c_str(),"-o", "twitter",NULL)==-1)
						{printf("download %s failed\n",url.c_str());}
				}
				else {
					int status;
					waitpid(id2, &status, 0);
					printf("download %s success\n",url.c_str());

					int id3=fork();
					int status2;
					if(id3==0){
						execl("/usr/bin/python3","python3","/home/wpengfei/downTwi/py/aliyunpan/main.py","upload","-p","/home/wpengfei/downTwi/twitter",NULL);
					}
					else{
						waitpid(id3, &status2, 0);
						execl("/bin/rm","rm","-rf","/home/wpengfei/downTwi/twitter",NULL);
					}
					//python3 home/wpengfei/downTwi/py/aliyunpan/main.py upload -p  
				}
				
			}


		}




		if(type != "GET"){
			printf("not GET\n");
			return Close();
		}
		string filename=path;
		if(path=="/"){
			filename="/index.html";
		}

		string filepath="www";
		filepath+=filename;





		//回应http GET请求
		string rmsg="HTTP/1.1 200 OK\r\n";
		rmsg+="Server: XHttp\r\n";
		rmsg+="Content-Type: text/html\r\n";
		//rmsg+="Content-Type: image/gif\r\n";
		rmsg+="Content-Length: ";

			//char bsize[128]={0};
			//sprintf(bsize,"%d",filesize);


		//rmsg+="10\r\n";
		rmsg+="11";
		rmsg+="\r\n\r\n";
		rmsg+="received ok";

		//发送消息头
		int sendedsize=client.Send(rmsg.c_str(),rmsg.size());
		printf("sended size = %d\n", sendedsize);


		
		buf[recvlen]='\0';
		printf("------------------send----------------%s\n----------------send---------------\n\n", rmsg.c_str());
		Close();
		
		//system(cmd.c_str());
	}
	void Close(){
		client.Close();
		delete this;
	}
	XTcp client;
};

int main(int argc, char* argv[]) {

	unsigned short port=8080;
	if(argc>1){
		port=atoi(argv[1]);
	}
	XTcp server;
	server.CreateSocket();
	server.Bind(port);

	while(true){
		XTcp client = server.Accept();
		HttpThread *th=new HttpThread();
		th->client=client;

		//???
		thread sth(&HttpThread::Main,th);
		sth.detach();
	}
	
	server.Close();
	return 0;
}
