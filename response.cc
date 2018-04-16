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
#include <unordered_map>
#include "request.h"
#include "response.h"

using namespace std;

//Macro
#define HOME "/"
#define STYLE_CSS "/style.css"
#define SCRIPT_JS "/script.js"
#define GRAPH_JS "/graph.js"
#define TOGGLE_TEMP "/action?toggleTemp"
#define TOGGLE_LIGHT "/action?toggleLight"
#define CHANGE_COLOR "/action?changeLightColor"
#define TOGGLE_STANDBY "/action?toggleStandby"
#define OK "200"
#define NOTFOUND "404"

//function
bool do_write(int socket_fd, char *buffer, int len);
string replace(string origin, string target, string replacement);
string replace_get_html(string filename, unordered_map<string, string> replacement);
string getHtml(string filename);
string get_temperature_string();
unordered_map<string, string> build_map(string tmp);
void read_file_into_vector(vector<char>& res, string address);
void change_color();
void change_temp();
void toggle_light();
void toggle_standby();

// global variables
bool isF = false;
bool isFirstTime = true;
string standby_temp_C;
string standby_avg_C;
string standby_max_C;
string standby_min_C;
string standby_temp_F;
string standby_avg_F;
string standby_max_F;
string standby_min_F;


/*
 * constructor of response and send back reponse to the client
 */
response::response(int client_fd, request req){
	string path = req.path;
	this->version = req.version.substr(0,8); // skip \r\n
	if (strcasecmp(req.method.c_str(),"get") == 0) {
		// handle get request
		this->status = OK;
		handle_get(path, client_fd);
	} 
}

/*
 * handle get request from client
 */
void response::handle_get(string path, int client_fd) {
	if (strcasecmp(path.c_str(), HOME) == 0) {
		// Home page
		string address("./html/index-one-page.html");
		string type("text/html");
		string res = get_temperature_string();
		unordered_map<string, string> map = build_map(res);
		replace_reply(address, type, client_fd, map);
	} else if (strcasecmp(path.c_str(), STYLE_CSS) == 0) {
		// pass css for the home page to the browser
		string address("./html/style.css");
		string type("text/css");
		reply_file(address, type, client_fd);
	} else if (strcasecmp(path.c_str(), SCRIPT_JS) == 0) {
		// email page
		string address("./html/script.js");
		string type("application/javascript");
		reply_file(address, type, client_fd);
	} else if (strcasecmp(path.c_str(), GRAPH_JS) == 0) {
		// file page
		string address("./html/graph.js");
		string type("application/javascript");
		reply_file(address, type, client_fd);
	}
	else if (strcasecmp(path.c_str(), TOGGLE_TEMP) == 0) {
		// change temperature unit
		string address("./html/index-one-page.html");
		string type("text/html");
		change_temp();
		sleep(1);
		string res = get_temperature_string();
		unordered_map<string, string> map = build_map(res);
		replace_reply(address, type, client_fd, map);
	} else if (strcasecmp(path.c_str(), CHANGE_COLOR) == 0) {
		// change light color
		string address("./html/index-one-page.html");
		string type("text/html");
		change_color();
		string res = get_temperature_string();
		unordered_map<string, string> map = build_map(res);
		replace_reply(address, type, client_fd, map);
	} else if (strcasecmp(path.c_str(), TOGGLE_LIGHT) == 0) {
		// turn the light on or off
		string address("./html/index-one-page.html");
		string type("text/html");
		toggle_light();
		string res = get_temperature_string();
		unordered_map<string, string> map = build_map(res);
		replace_reply(address, type, client_fd, map);
	} else if (strcasecmp(path.c_str(), TOGGLE_STANDBY) == 0) {
		// stand by mode
		string address("./html/index-one-page.html");
		string type("text/html");
		toggle_standby();
		string res = get_temperature_string();
		unordered_map<string, string> map = build_map(res);
		replace_reply(address, type, client_fd, map);
	}
}

/*
 * reply the file to the client
 */
void response::reply_file(string address, string type, int client_fd) {
	string server_response(this->version);
	server_response += " " + this->status + " " + "OK" + "\r\n";
	server_response += string("Content-type: ") + string(type) + string("\r\n");
	server_response += "\r\n";

	//convert string to vector
	vector<char> res(server_response.begin(), server_response.end());

	// read character to the vector
	read_file_into_vector(res, address);
	do_write(client_fd, &res[0], res.size());
}



/*
 * replace html file with message body and reply to the client
 */
void response::replace_reply(string address, string type, int client_fd,
	unordered_map<string, string> replacement) {
	string server_response(this->version);
	server_response += " " + this->status + " " + "OK" + "\r\n";
	server_response += string("Content-type: ") + string(type) + string("\r\n");
	server_response += "\n";
	server_response += replace_get_html(address, replacement);
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
	server_response += getHtml(address);

	char res[10000];
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
string replace_get_html(string filename, unordered_map<string, string> replacement) {
	// open the file
	ifstream web(filename);

	if (!web) {
		fprintf(stderr, "cannot open given file!!!\n");
		exit(1);
	}

	string line;
	string content;

	while (getline(web, line)) {
		for (auto iter : replacement) {
			//replace every element in the map in line
			line = replace(line, iter.first, iter.second);
		}
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
 * send signal to change the unit of the temperature
 */
void change_temp(){
	int byte_written = write(fd_usb, "t", 1 * sizeof(char));
}

/*
 * read file into a vector of character
 */
void read_file_into_vector(vector<char>& res, string address) {
	ifstream file;
	file.open(address.c_str(), ios::binary);

	if (!file) {
		fprintf(stderr, "cannot open given file!!!\n");
		exit(1);
	}

	char data;
	while (file.get(data)) {
		// keep reading from the file
		res.push_back(data);
	}
}

/*
 * get the temperature string
 */
string get_temperature_string(){
	string res;
	char token_array[100];
	char token_array_2[100];
	printf("%s\n", temperature);
	
	char* token;

	token = strtok(temperature, " ");

	strcpy(token_array, token);
	res = string(token_array);
	res = res.substr(0, 4);
	token = strtok(NULL, " ");
	res += string("&deg;");

	strcpy(token_array_2, token);
	res += string(token_array_2);
	return res;
}

/*
 * send signal to change the color
 */
void change_color() {
	int byte_written = write(fd_usb, "c", 1 * sizeof(char));
}

/*
 * send signal to turn on or off the light
 */
void toggle_light() {
	int byte_written = write(fd_usb, "l", 1 * sizeof(char));
}

/*
 * send signal to enter into or exit standby mode
 */
void toggle_standby() {
	int byte_written = write(fd_usb, "s", 1 * sizeof(char));
}

/*
 * build the map from string in html to real temperature string
 */
unordered_map<string, string> build_map(string temp) {
	unordered_map<string, string> res;
	res.insert({string("$temp"), temp});
	if (temp.at(temp.length() - 2) == 'C') {
		// Celsius
		res.insert({string("$temp_avg"), avg_temperature_C + string("&deg;C")});
		res.insert({string("$temp_min"), min_temperature_C + string("&deg;C")});
		res.insert({string("$temp_max"), max_temperature_C + string("&deg;C")});
	} else {
		// Fahrenheit
		res.insert({string("$temp_avg"), avg_temperature_F + string("&deg;F")});
		res.insert({string("$temp_min"), min_temperature_F + string("&deg;F")});
		res.insert({string("$temp_max"), max_temperature_F + string("&deg;F")});
	}

	return res;
}

