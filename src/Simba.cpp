#if defined _WIN32 || defined _WIN64
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <cstring>
#include <iostream>
#include <string>
#include "RdoIntegers.hh"
#include "RdoStrings.hh"
#include "RdoRandom.hh"


#define NumExports 1
#if defined _WIN32 || defined _WIN64
HMODULE dllinst = NULL;
#endif

static char* exports[] =
{
	(char*)"trueRandomInt", (char*)"function trueRandomInt(min, max : integer) : integer;",
};

extern "C" int __declspec(dllexport) trueRandomInt(int min, int max)
{
	RdoIntegers rdo;
	rdo.setNum(1);
	rdo.setBase("10");
	rdo.setRange(min, max);
	rdo.setInMemory(true);

	bool failed = rdo.downloadData();
	if (failed){
		std::cerr << "Failed to download TrueRandom data" << std::endl;
		return -1;
	}

	std::vector<long int> data = rdo.cache();
	unsigned int num = data.size();

	return data[num - 1];
}

/*extern "C" double __declspec(dllexport) trueRandomFloat(float min, float max, int decimalPlaces)
{
	RdoRandom rdo;
	rdo.setNum(1);
	rdo.setDecimals(decimalPlaces);
	rdo.setInMemory(true);

	bool failed = rdo.downloadData();
	if (failed){
		std::cerr << "Failed to download TrueRandom data" << std::endl;
		return -1;
	}

	std::vector<double> data = rdo.cache();
	unsigned int num = data.size();

	return data[num - 1];
}

extern "C" char* __declspec(dllexport) trueRandomString(int length, bool upperAllowed)
{
	RdoStrings rdo;
	rdo.setNum(1);
	rdo.setLength(length);
	rdo.setUpper(upperAllowed);
	rdo.setLower(true);
	rdo.setDigits(false);
	rdo.setInMemory(true);

	bool failed = rdo.downloadData();
	if (failed){
		std::cerr << "Failed to download TrueRandom data" << std::endl;
		return "";
	}

	std::vector<std::string> data = rdo.cache();
	unsigned int num = data.size();
	const char* rtn = data[num - 1].c_str();

	return rtn;
}*/

extern "C" int __declspec(dllexport) GetPluginABIVersion()
{
	return 2;
}

extern "C" int __declspec(dllexport) GetFunctionCount()
{
	return NumExports;
}

extern "C" int __declspec(dllexport) GetFunctionInfo(int index, void* &address, char* &def)
{
	if (index < NumExports)
	{
#if defined _WIN32 || defined _WIN64
		address = (void*)GetProcAddress(dllinst, exports[index * 2]);
#else
		address = dlsym(RTLD_DEFAULT, exports[index * 2]);
#endif
		strcpy(def, exports[index * 2 + 1]);
		return index;
	}
	return -1;
}

#if defined _WIN32 || defined _WIN64
extern "C" __declspec(dllexport) bool __stdcall DllMain(HINSTANCE instance, unsigned int reason, void* checks)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		dllinst = instance;
		std::cout << "TrueRandom32 Plugin Loaded..\n";
	}
	break;

	case DLL_PROCESS_DETACH:
	{
		std::cout << "TrueRandom32 Plugin Freed..\n";
	}
	break;
	}
	return true;
}
#else
void OnProcessAttach() __attribute__((constructor))
{
	std::cout << "TrueRandom32 Plugin Loaded..\n";
}

void OnProcessDetach() __attribute__((destructor))
{
	std::cout << "TrueRandom32 Plugin UnLoaded..\n";
}
#endif