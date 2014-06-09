#include <string>
#include <sstream>
#include "connect.h"

using namespace std;

string Connect::message(string cmd, string content) {
	ostringstream ret;
	ret << cmd.size() + 1 + content.size();
	ret << " ";
	ret << cmd;
	ret << " ";
	ret << content;
	return ret.str();
}
