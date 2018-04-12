/*
 * response.h
 *
 *  Created on: Apr 1, 2018
 *      Author: cis505
 */

#ifndef RESPONSE_H_
#define RESPONSE_H_

#include <string>
#include "request.h"

using namespace std;

extern int fd_usb;


class response {
	public:
		string status;
		string version;

		response(int client_fd, request req);
		void reply(string address, string type, int client_fd);
		void replace_reply(string address, string type, int client_fd,
				string target, string replacement);
		void handle_get(string path, int client_fd);
		void handle_post(string path, int client_fd, vector<char> message);
		void handle_signin(string path, int client_fd, vector<char> message);
		void handle_upload(string path, int client_fd, vector<char> message);
};


#endif /* RESPONSE_H_ */
