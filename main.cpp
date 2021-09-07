#include "XTcp.cpp"

#include<string>
#include<regex>
using namespace std;

class HttpThread {
public:
	void Main() {
		char buf[10000] = { 0 };

		int recvlen = client.Recv(buf, sizeof(buf) - 1);
		if (recvlen <= 0)
		{
			return Close();
		}
		printf("-------------recv----------------------\n%s---------------recv----------------\n\n", buf);

		string src = buf;

		string pattern = "^([A-Z]+) /([a-zA-Z0-9]*([.][a-zA-Z0-9]*)?)[?]?(.*) HTTP/1";
		regex r(pattern);
		smatch mas;
		regex_search(src, mas, r);
		if (mas.size() == 0) {
			printf("%s failed\n", pattern.c_str());
			return Close();
		}

		string type = mas[1];
		string path = "/";
		path += mas[2];

		string filetype = mas[3];

		string query = mas[4];


		if (filetype.size() > 0)
			filetype = filetype.substr(1, filetype.size() - 1);

		printf("type: [%s]\npath: [%s]\nfiletype: [%s]\nquery: [%s]\n", \
			type.c_str(), path.c_str(), filetype.c_str(), query.c_str());

		if (type != "GET") {
			printf("not GET\n");
			return Close();
		}
		string filename = path;
		if (path == "/") {
			filename = "/index.html";
		}

		string filepath = "www";
		filepath += filename;

		string cmd;
		if (filetype == "php") {
			cmd = "proxychains python3 twitter-dl.py";
			cmd += (" " + query);
			printf("downloading  %s\n", query.c_str());
			printf("exec %s\n", cmd.c_str());
		}



		//回应http GET请求
		string rmsg = "HTTP/1.1 200 OK\r\n";
		rmsg += "Server: XHttp\r\n";
		rmsg += "Content-Type: text/html\r\n";
		//rmsg+="Content-Type: image/gif\r\n";
		rmsg += "Content-Length: ";

		//char bsize[128]={0};
		//sprintf(bsize,"%d",filesize);


	//rmsg+="10\r\n";
		rmsg += "11";
		rmsg += "\r\n\r\n";
		rmsg += "received ok";

		//发送消息头
		int sendedsize = client.Send(rmsg.c_str(), rmsg.size());
		printf("sended size = %d\n", sendedsize);



		buf[recvlen] = '\0';
		printf("------------------send----------------%s\n----------------send---------------\n\n", rmsg.c_str());
		Close();
		//int id = fork();
		//if (id == 0) {
		//	//execl("proxychains","python3","twitter-dl.py",query.c_str());
		//	//execl("/usr/bin/proxychains","proxychains","python3","twitter-dl.py","-w","500",query.c_str(),NULL);
		//	if (execl("/usr/bin/proxychains", "proxychains", "you-get", query.c_str(), NULL) == -1)
		//		printf("wrong===============================\n");
		//}
		//system(cmd.c_str());
	}
	void Close() {
		client.Close();
		delete this;
	}
	XTcp client;
};

int main(int argc, char* argv[]) {

	unsigned short port = 8080;
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	XTcp server;
	server.CreateSocket();
	server.Bind(port);

	while (true) {
		XTcp client = server.Accept();
		HttpThread* th = new HttpThread();
		th->client = client;

		//???
		thread sth(&HttpThread::Main, th);
		sth.detach();
	}

	server.Close();
	return 0;
}
