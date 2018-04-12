/*
 * response.cc
 *
 *  Created on: Apr 1, 2018
 *      Author: cis505
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sys/file.h>
#include <vector>
#include <cstdio>
#include <algorithm>
#include "request.h"
#include "response.h"

using namespace std;

//Macro
#define HOME "/"
#define STYLE_CSS "style.css"
#define SCRIPT_JS "script.js"
#define GRAPH_JS "graph.js"
#define TOGGLE_TEMP "/action?toggleTemp"
#define FILES "/file"
#define OK "200"
#define NOTFOUND "404"

//function
bool do_write(int socket_fd, char *buffer, int len);
string replace(string origin, string target, string replacement);
string replace_get_html(string filename, string target, string replacement);
string getHtml(string filename);
string find_filename(vector<char> message);
vector<char> find_file_content(vector<char> message);
void store_file(string dir, string filename, vector<char> content);
void change_temp();

/*
 * constructor of response and send back reponse to the client
 */
response::response(int client_fd, request req){
	printf("here!!!!\n");
	string path = req.path;
	printf("path : %s\n", path.c_str());
	this->version = req.version.substr(0,8); // skip \r\n
	if (strcasecmp(req.method.c_str(),"get") == 0) {
		// handle get request
		this->status = OK;
		handle_get(path, client_fd);
	} else if (strcasecmp(req.method.c_str(), "post") == 0) {
		// handle post request
		this->status = OK;
		handle_post(path, client_fd, req.message);
	}
}

/*
 * handle get request from client
 */
void response::handle_get(string path, int client_fd) {
	if (strcasecmp(path.c_str(), HOME) == 0) {
		printf("handle home!!!\n");
		// Home page
		string address("./html/index.html");
		string type("text/html");
		reply(address, type, client_fd);
	} else if (strcasecmp(path.c_str(), STYLE_CSS) == 0) {
		// pass css for the home page to the browser
		string address("./html/style.css");
		string type("text/css");
		reply(address, type, client_fd);
	} else if (strcasecmp(path.c_str(), SCRIPT_JS) == 0) {
		// email page
		string address("./html/script.js");
		string type("application/javascript");
		reply(address, type, client_fd);
	} else if (strcasecmp(path.c_str(), GRAPH_JS) == 0) {
		// file page
		string address("./html/graph.js");
		string type("application/javascript");
		reply(address, type, client_fd);
	}
	else if (strcasecmp(path.c_str(), TOGGLE_TEMP) == 0) {
		// file page
		string address("./html/index.html");
		string type("text/html");
		change_temp();
		reply(address, type, client_fd);
	}
}

/*
 * handle post request from client
 */
void response::handle_post(string path, int client_fd, vector<char> message) {
	if (strcasecmp(path.c_str(), HOME) == 0) {
		// handle sign in request
		handle_signin(path, client_fd, message);
	} else if (strcasecmp(path.c_str(), FILES) == 0) {
		// handle upload file
		handle_upload(path, client_fd, message);
	}
}

/*
 * handle upload request
 */
void response::handle_upload(string path, int client_fd, vector<char> message) {
	// find filename
	string filename = find_filename(message);
	printf("filename: %s\n", filename.c_str());

	// find content
	vector<char> content = find_file_content(message);

	// store the file
	string dir("./Test");
	store_file(dir, filename, content);
}

/*
 * handle sign in post request
 */
void response::handle_signin(string path, int client_fd, vector<char> message) {
	// parse the message to get username and password
	char copy[256];
	string info(message.begin(), message.end());
	printf("info: %s\n", info.c_str());
	strcpy(copy, info.c_str());
	char *token;
	token = strtok(copy, "&");

	// get username
	char *subToken;
	char copy2[256];
	strcpy(copy2, token);
	token = strtok(NULL, " &");
	subToken = strtok(copy2, "=");
	subToken = strtok(NULL, "=");
	string username(subToken);

	// get password
	char copy3[256];
	strcpy(copy3, token);
	char *subToken2;
	subToken2 = strtok(copy3, "=");
	subToken2 = strtok(NULL, "=");
	string password(subToken2);

	// reply to the client
	string address("./html/signin_successful.html");
	string type("text/html");
	replace_reply(address, type, client_fd, string("$user"), username);
}

/*
 * replace html file with message body and reply to the client
 */
void response::replace_reply(string address, string type, int client_fd,
							string target, string replacement) {
	string server_response(this->version);
	server_response += " " + this->status + " " + "OK" + "\r\n";
	server_response += string("Content-type: ") + string(type) + string("\r\n");
	server_response += "\n";
	server_response += replace_get_html(address, target, replacement);
	char res[10000];
	strcpy(res, server_response.c_str());
	do_write(client_fd, res, strlen(server_response.c_str()) + 1);
}

/*
 * reply the corresponding file to the client
 */
void response::reply(string address, string type, int client_fd) {
	string server_response(this->version);
	server_response += " " + this->status + " " + "OK" + "\r\n";
	server_response += string("Content-type: ") + string(type) + string("\r\n");
	server_response += "\r\n";
	//printf("-----------get!!\n");
	server_response += getHtml(address);
	//printf("-----------get!!\n");
	char res[10000];
	//printf("server_reponse: %s\n", server_response.c_str());
	strcpy(res, server_response.c_str());
	do_write(client_fd, res, strlen(server_response.c_str()) + 1);
}

/*
 * replace target substring in the original string with replacement
 */
string replace(string origin, string target, string replacement) {
	size_t index = 0;
	while (true) {
		// find the position of the substring
		index = origin.find(target, index);
		if (index == origin.npos) {
			// cannot find the target string
			break;
		}
		// replace the string
		origin.replace(index, target.length(), replacement);
		index += replacement.length(); // skip the new string
	}

	return origin;
}

/*
 * write response to client
 */
bool do_write(int socket_fd, char *buffer, int len) {
	int index = 0; // the starting index
	while (index < len) {
		int n = write(socket_fd, &buffer[index], len - index);
		if (n < 0) {
			return false;
		}
		index += n;
	}
	return true;
}

/*
 * get the html file and replace the target string in the file
 * with the replacement string
 */
string replace_get_html(string filename, string target, string replacement) {
	// open the file
	ifstream web(filename);

	if (!web) {
		fprintf(stderr, "cannot open given file!!!\n");
		exit(1);
	}

	string line;
	string content;

	while (getline(web, line)) {
		//replace line
		line = replace(line, target, replacement);
		cout<<line<<endl;
		// write line to content
		content += line;
	}

	web.close();

	return content;
}

/*
 * parse the html file into a string
 */
string getHtml(string filename) {
	// open the file
	ifstream web(filename);

	if (!web) {
		fprintf(stderr, "cannot open given file!!!\n");
		exit(1);
	}


	string line;
	string content;


	while (getline(web, line)) {
		// write line to content
		content += line;
	}

	web.close();
	return content;
}

/*
 * return the filename of the uploaded file
 */
string find_filename(vector<char> message) {
	string tmp(message.begin(), message.end());
	size_t index = tmp.find("filename=", 0);
	// skip the filename
	index += 10;
	// find position of second "
	size_t end_index = tmp.find("\"", index);
	return tmp.substr(index, end_index - index);
}

/*
 * return the content of the uploaded file
 */
vector<char> find_file_content(vector<char> message) {
	vector<char> res;
	vector<char> start_char = {'\r', '\n', '\r', '\n'};
	vector<char> end_char = {'-', '-', '-', '-', '-', '-'};
	// find the position of \r\n and skip them
	auto start = search(message.begin(), message.end(), start_char.begin(), start_char.end()) + 4;
	// find the position of ------
	auto end = search(start, message.end(), end_char.begin(), end_char.end()) - 2; // skip \r\n
	res.insert(res.end(), start, end);
	return res;
}

/*
 * store the file into the given directory
 */
void store_file(string dir, string filename, vector<char> content) {
	string address(dir);
	address += string("/") + filename;
	FILE* fp;
	fp = fopen(address.c_str(), "wb");
	fwrite(&content[0], 1, content.size(), fp);
	fclose(fp);
}

void change_temp(){
	int byte_written = write(fd_usb, "t", 1 * sizeof(char));
}