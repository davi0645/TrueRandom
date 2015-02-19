#if defined _WIN32 || defined _WIN64
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef _MSC
#define _CRT_SECURE_NO_WARNINGS
#endif

#if defined _WIN32 || defined _WIN64
#define EXPORT_FUNC __declspec(dllexport)
HMODULE module = NULL;
#else
#define EXPORT_FUNC
#define __stdcall
#endif

#include <stdint.h>
#include <stdio.h>
#include<iostream>
#include "RdoIntegers.hh"
#include "RdoStrings.hh"
#include "RdoRandom.hh"



/*********************************************************************************************************************/

typedef struct
{
	int32_t reference_count;
	int32_t array_length;
} PascalArray;


/**
Retrieves the pointer to the memory address
where a pascal array stores its reference count.
**/
int32_t* getReferencePointer(void* ps_array)
{
	return ((int*)ps_array) - (sizeof(int32_t) * 2);
}

/**
Retrieves the pointer to the memory address
where a pascal array stores its length.
**/
int32_t* getLengthPointer(void* ps_array)
{
	return ((int*)ps_array) - sizeof(int32_t);
}

/**
Allocates an array for use with Pascal.
Total size is the number of elements
you want to allocate.
Element size is the sizeof(an_element).
Ref Count is the specified custom reference count.
**/
void* AllocArrayWithRef(uint32_t total_size, uint32_t element_size, int32_t ref_count)
{
	PascalArray* arr = (PascalArray*)malloc(sizeof(PascalArray) + (total_size * element_size));
	if (arr)
	{
		arr->array_length = (total_size - 1);
		arr->reference_count = ref_count;
		return ++arr;
	}
	return NULL;
}

/**
Allocates an array for use with Pascal.
Total size is the number of elements
you want to allocate.
Element size is the sizeof(an_element).
**/
void* AllocArray(uint32_t total_size, uint32_t element_size)
{
	return AllocArrayWithRef(total_size, element_size, -1);
}

/**
Frees a PascalArray.
**/
void FreeArray(void* arr)
{
	if (arr)
	{
		PascalArray* ps_arr = (PascalArray*)arr;
		--ps_arr;
		free(ps_arr);
	}
}


/*********************************************************************************************************************/

static const int ABI_VERSION = 2;

static const char* PascalExports[] =
{
	"trueRandomInt", "Function trueRandomInt(min, max : Integer): Integer;",
	"trueRandomIntArray", "Function trueRandomIntArray(min, max, size : Integer): TIntegerArray;"
};

static const char* PascalTypes[] =
{
	"CppRecord", "record Arr: TIntegerArray; end;",  //exporting records..
	//"CppClass", "type TObject;"    //exporting classes..
};


/**
Called when your .dll or .so is loaded.
**/
void onLoad()
{
	//Do any initialisation here..
}


/**
Called when your .dll or .so is unloaded.
**/
void onUnload()
{
	//Do any de-initialisation here..
}


extern "C"
{
	EXPORT_FUNC void* trueRandomIntArray(int min, int max, int size)
	{
		int32_t* arr = (int32_t*)AllocArrayWithRef(size, sizeof(int32_t), 2);
		RdoIntegers rdo;
		rdo.setNum(size);
		rdo.setBase("10");
		rdo.setRange(min, max);
		rdo.setInMemory(true);

		bool failed = rdo.downloadData();
		if (failed){
			std::cerr << "Failed to download TrueRandom data" << std::endl;
			return NULL;
		}

		std::vector<long int> data = rdo.cache();
		unsigned int num = data.size();
		for (unsigned int i = 0; i < num; i++)
			arr[i] = data[i];
		return arr;
	}

	int EXPORT_FUNC trueRandomInt(int min, int max)
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

}








/**********************************************************************************************************/



#if defined _WIN32 || defined _WIN64

#if defined __cplusplus
extern "C"
{
#endif


	EXPORT_FUNC bool __stdcall DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		switch (fdwReason)
		{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hInstDLL);
			module = hInstDLL;
			onLoad();
			break;

		case DLL_PROCESS_DETACH:
			onUnload();
			break;
		}
		return true;
	}


#if defined __cplusplus
}
#endif


#else

void load() __attribute__((constructor))
{
	onLoad();
}

void unload() __attribute__((destructor))
{
	onUnload();
}

#endif




/********************************************************************************************************************/


#ifdef __cplusplus
extern "C"
{
#endif


	EXPORT_FUNC int GetPluginABIVersion();

	EXPORT_FUNC int GetTypeCount();
	EXPORT_FUNC int GetFunctionCount();

	EXPORT_FUNC int GetTypeInfo(int Index, char** Type, char** Definition);
	EXPORT_FUNC int GetFunctionInfo(int Index, void** Address, char** Definition);


#ifdef __cplusplus
}
#endif


int GetPluginABIVersion()
{
	return ABI_VERSION;
}

int GetTypeCount()
{
	return sizeof(PascalTypes) / (sizeof(PascalTypes[0]) * 2);
}

int GetFunctionCount()
{
	return sizeof(PascalExports) / (sizeof(PascalExports[0]) * 2);
}

int GetFunctionInfo(int Index, void** Address, char** Definition)
{
	if (Index < GetFunctionCount())
	{
#if defined _WIN32 || defined _WIN64
		*Address = (void*)GetProcAddress(module, PascalExports[Index * 2]);
#else
		*Address = (void*)dlsym(RTLD_DEFAULT, PascalExports[Index * 2]);
#endif
		strcpy(*Definition, PascalExports[Index * 2 + 1]);
		return Index;
	}
	return -1;
}

int GetTypeInfo(int Index, char** Type, char** Definition)
{
	if (Index < GetTypeCount())
	{
		strcpy(*Type, PascalTypes[Index * 2 + 0]);
		strcpy(*Definition, PascalTypes[Index * 2 + 1]);
		return Index;
	}
	return -1;
}