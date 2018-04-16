/*
 * response.h
 *
 *  Created on: Apr 1, 2018
 *      Author: cis505
 */

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include <string>
#include <unordered_map>
#include "request.h"

using namespace std;

extern int fd_usb;
extern char temperature[100];
extern string avg_temperature_C;
extern string avg_temperature_F;
extern string min_temperature_C;
extern string min_temperature_F;
extern string max_temperature_C;
extern string max_temperature_F;



class response {
	public:
		string status;
		string version;

		response(int client_fd, request req);
		void reply(string address, string type, int client_fd);
		void replace_reply(string address, string type, int client_fd,
				unordered_map<string, string> replacement);
		void reply_file(string address, string type, int client_fd);
		void handle_get(string path, int client_fd);
		void handle_post(string path, int client_fd, vector<char> message);
		void handle_signin(string path, int client_fd, vector<char> message);
		void handle_upload(string path, int client_fd, vector<char> message);
};


#endif /* RESPONSE_H_ */
