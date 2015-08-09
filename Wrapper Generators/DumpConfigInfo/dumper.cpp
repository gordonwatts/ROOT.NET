///
/// Dump out some config info so others can see what we did (and didn't) do. Goes to stdout.
///

#include "WrapperConfigurationInfo.hpp"
#include <iostream>

using std::endl;
using std::cout;
using std::vector;
using std::string;

void dumpList (const vector<string> &list, const string &title);

int main(int argc, char* argv[])
{
	auto badGlobals = WrapperConfigurationInfo::GetListOfBadGlobalVariables();

	auto badMethodSet = WrapperConfigurationInfo::GetListOfBadMethods();
	vector<string> badMethods;
	badMethods.assign(badMethodSet.begin(), badMethodSet.end());

	auto badLibs = WrapperConfigurationInfo::GetListOfBadLibraries();

	dumpList (badGlobals, "Bad Globals");
	dumpList (badMethods, "Bad Methods");
	dumpList (badLibs, "Bad Libraries");
}

void dumpList (const vector<string> &list, const string &title)
{
	if (list.size() == 0)
		return;

	cout << endl << endl;
	cout << title << endl;
	for (int i = 0; i < title.size(); i++) {
		cout << "-";
	}
	cout << endl;

	for each (auto &item in list)
	{
		cout << item << endl;
	}
}
