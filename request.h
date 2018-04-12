/*
 * request.h
 *
 *  Created on: Apr 1, 2018
 *      Author: cis505
 */

#ifndef REQUEST_H_
#define REQUEST_H_

#include <string>
#include <vector>

using namespace std;

//extern bool debugInfo;

class request {
	public:
		string method;
		string path;
		string version;
		vector<char> message;

		request(int client_fd);
};


#endif /* REQUEST_H_ */
