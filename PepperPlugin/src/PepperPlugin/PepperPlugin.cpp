// PepperPlugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "PepperPlugin.h"


#include <stdio.h>
#include <stdlib.h>

#include "all_ppapi_c_includes.h"
#include "all_ppapi_cpp_includes.h"

#include <mono/metadata/mono-config.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/environment.h>
#include <mono/metadata/class.h>
#include <mono/metadata/metadata.h>
#include <mono/jit/jit.h>
#include <mono/metadata/debug-helpers.h>

#ifdef WIN32
#undef PostMessage
// Allow 'this' in initializer list
#pragma warning(disable : 4355)

// used for obtaining file information
#include <strsafe.h>
#include <codecvt>
#endif

using namespace std;
using namespace pepper;
using namespace pp;
namespace pepper {

	bool initialised = false;
	MonoDomain* monoDomain = NULL;

	const char* getEnvVar(const char *key)
	{

		char* val = 0;
		size_t sz = 0;
		if (_dupenv_s(&val, &sz, key) == 0)
		{
		}
		return val == NULL ? "" : val;
	}

	void split_namespace_class(const char* strc, string &nameSpace, string &className)
	{
		if (!strc)
			return;

		auto str = string(strc);
		int found = str.find_last_of(".");
		
		if (found == -1)
		{
			className = str.substr(0, str.length());
		}
		else
		{
			nameSpace = str.substr(0, found);
			className = str.substr(found + 1, str.length());
		}
	}

	MonoDomain* load_domain()
	{
		MonoDomain* newDomain = mono_domain_create_appdomain("PepperPlugin App Domain", NULL);
		if (!newDomain) {
			fprintf(stderr, "load_domain: Error creating domain\n");
			return nullptr;
		}

		if (!mono_domain_set(newDomain, false)) {
			fprintf(stderr, "load_domain: Error setting domain\n");
			return nullptr;
		}

		return mono_domain_get();
	}

	/*
		Recursively lookup a method signature on a class using the method description
		The signature format is 
			":methodname(argtype1,argtype2,...)" 

		http://mono.1490590.n4.nabble.com/Invoking-method-from-base-class-in-embedded-environment-td1525339.html
	*/
	MonoMethod *
		mono_class_get_method_from_desc_recursive(
			MonoClass *clazz, char *signature)
	{
		MonoMethodDesc *desc;
		MonoMethod *method = NULL;

		desc = mono_method_desc_new(signature, TRUE);

		while (clazz != NULL && method == NULL) {
			method = mono_method_desc_search_in_class(desc, clazz);
			if (method == NULL)
				clazz = mono_class_get_parent(clazz);
		}
		mono_method_desc_free(desc);

		return method;
	}

	MonoClass* find_class(const char* namspace, const char* className, vector<MonoImage*> images)
	{
		MonoClass *klass = NULL;
		for (auto& image : images) {
			klass = mono_class_from_name(image, namspace, className);
			if (klass) {
				return klass;
			}
		}
		return klass;
	}

	void InitMono() {

		if (initialised)
			return;

		// point to the relevant directories of the Mono installation
		auto mono_root = string(getEnvVar("MONO_ROOT"));
		if (mono_root.length() == 0)
		{
#ifdef WIN32
			mono_root = "C:\\Program Files (x86)\\Mono"; // default install location on windows
#endif
		}
		auto mono_lib = mono_root + "/lib";
		auto mono_etc = mono_root + "/etc";
		
		mono_set_dirs(mono_lib.c_str(), mono_etc.c_str());

		//mono_set_dirs("C:\\Program Files (x86)\\Mono\\lib",
		//	"C:\\Program Files (x86)\\Mono\\etc");

		////// load the default Mono configuration file in 'etc/mono/config'
		mono_config_parse(nullptr);

		MonoDomain* domain = mono_jit_init_version("PepperPlugin Domain", "v4.0.30319");

		initialised = true;

		if (!monoDomain)
		{
			monoDomain = load_domain();
		}
	}

	bool mono_invoke_with_desc(char* desc, void *args[], MonoClass* klass, 
		MonoObject* instance, MonoObject **result)
	{
		auto method = mono_class_get_method_from_desc_recursive(klass, desc);

		if (!method)
		{
			fprintf(stderr, "Method description not found: `%s` \n", desc);
			return false;
		}

		// finally, invoke the constructor
		MonoObject* exception = NULL;
		*result = mono_runtime_invoke(method, instance, args, &exception);
		if (exception != nullptr)
		{
			MonoObject *other_exc = NULL;
			auto str = mono_object_to_string(exception, &other_exc);
			char *mess = mono_string_to_utf8(str);
			printf("Exception has been thrown calling: `%s` - exception is: %s \n", desc, mess);
			mono_free(mess);
			return false;
		}

		return true;

	}

	bool mono_invoke_with_method(MonoMethod* method, void *args[], MonoObject* instance, MonoObject **result)
	{

		// finally, invoke the constructor
		MonoObject* exception = NULL;
		*result = mono_runtime_invoke(method, instance, args, &exception);
		if (exception != nullptr)
		{
			MonoObject *other_exc = NULL;
			auto str = mono_object_to_string(exception, &other_exc);
			char *mess = mono_string_to_utf8(str);
			printf("Exception invoking method - exception is: %s \n", mess);
			mono_free(mess);
			return false;
		}

		return true;

	}


	MonoObject* create_managed_wrapper(intptr_t ptr, const char* nameSpace, const char* className, vector<MonoImage*> images)
	{

		MonoClass * wrapper = find_class(nameSpace, className, images);
		if (wrapper == nullptr) {
			return nullptr;
		}

		// create the class (doesn't run constructors)
		MonoObject* obj = mono_object_new(monoDomain, wrapper);
		if (obj == nullptr) {
			return nullptr;
		}

		void *args[1] = { 0 };
		args[0] = &ptr;

		MonoObject* result;
		if (!mono_invoke_with_desc(":.ctor(intptr)", args, wrapper, obj, &result))
		{
			fprintf(stderr, "Error creating managed wrapper for : %s.%s\n ", nameSpace, className);
			return false;
		}


		return obj;
	}

	// TODO: The following routines will probably only work on windows so will need to be 
	// looked at for other platforms.
	wstring toUTF16(const std::string& str)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.from_bytes(str);
	}

	string toUTF8(const std::wstring& wstr)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(wstr);
	}

	std::vector<std::string> get_assembly_list ( std::string path = ".", bool includePath = true)
	{

		WIN32_FIND_DATA ffd;
		TCHAR szDir[MAX_PATH];
		HANDLE hFind = INVALID_HANDLE_VALUE;
		DWORD dwError = 0;

		vector<std::string> fileList;

		// Check that the input path plus 7 is not longer than MAX_PATH.
		// Three characters are for the "\*.dll" plus NULL appended below.
		if (path.length() > (MAX_PATH - 7))
		{
			printf("\nDirectory path is too long.\n");
			return fileList;
		}

		auto wPath = toUTF16(path);

		// Prepare string for use with FindFile functions.  First, copy the
		// string to a buffer, then append '\*.dll' to the directory name.
		StringCchCopy(szDir, MAX_PATH, wPath.c_str());
		StringCchCat(szDir, MAX_PATH, TEXT("\\*.dll"));

		// Find the first file in the directory.
		hFind = FindFirstFile(szDir, &ffd);

		if (INVALID_HANDLE_VALUE == hFind)
		{
			return fileList;
		}

		// List all the files in the directory with some info about them.
		do
		{
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (includePath)
					fileList.push_back(path + '\\' + toUTF8(ffd.cFileName));
				else
					fileList.push_back(toUTF8(ffd.cFileName));
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		FindClose(hFind);

		return fileList;
	}

}  // namespace

class PluginInstance : public pp::Instance, public pp::MouseLock {
public:
	explicit PluginInstance(PP_Instance instance)
		: pp::Instance(instance),
		pp::MouseLock(this)
	{
		InitMono();
	}

	~PluginInstance() {  }

	virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]) {

		auto className = string();
		auto nameSpace = string();

		MonoArray *parmsArgN = mono_array_new(monoDomain, mono_get_string_class(), argc);
		MonoArray *parmsArgV = mono_array_new(monoDomain, mono_get_string_class(), argc);

		//printf("cwd: %s\n", GetCWD().c_str());

		auto src = string();
		auto path = string();

		for (unsigned int x = 0; x < argc; x++)
		{
			mono_array_set(parmsArgN, MonoString*, x, mono_string_new(monoDomain, argn[x]));
			mono_array_set(parmsArgV, MonoString*, x, mono_string_new(monoDomain, argv[x]));

			//printf("argn = %s argv = %s\n", argn[x], argv[x]);
			auto property = string(argn[x]);

			if (property == "path")
			{
				path += argv[x];
			}
			if (property == "src")
			{
				src += argv[x];
			}
		}

		if (path.length() >= 0)
		{
			auto fileList = get_assembly_list(path);
			for (string f : fileList)
			{
				printf("loading assembly: %s\n", f.c_str());
				MonoAssembly* assembly = mono_domain_assembly_open(monoDomain,
					f.c_str());
				if (assembly)
				{
					auto gotImage = mono_assembly_get_image(assembly);
					images.push_back(gotImage);
				}
				else
					fprintf(stderr, "Error loading assembly: %s\n", f.c_str());
			}

		}

		// If we have an error loading assemblies then return false
		if (images.empty())
			return false;

		if (src.length() >= 0)
		{
			printf("loading class: %s\n", src.c_str());
			split_namespace_class(src.c_str(), nameSpace, className);

			instanceClass = find_class(nameSpace.c_str(), className.c_str(), images);
			if (!instanceClass)
				fprintf(stderr, "Error loading: namespace %s / class %s\n", nameSpace.c_str(), className.c_str());
		}

		if (!instanceClass)
			return false;

		// Load up our methods to be called so we do not have look them up each time.
		did_change_view = mono_class_get_method_from_desc_recursive(instanceClass, ":DidChangeView(PepperSharp.PPResource)");
		did_change_focus = mono_class_get_method_from_desc_recursive(instanceClass, ":DidChangeFocus(bool)");
		handle_input_event = mono_class_get_method_from_desc_recursive(instanceClass, ":HandleInputEvent(PepperSharp.PPResource)");
		handle_document_load = mono_class_get_method_from_desc_recursive(instanceClass, ":HandleDocumentLoad(PepperSharp.PPResource)");
		handle_message = mono_class_get_method_from_desc_recursive(instanceClass, ":HandleMessage(PepperSharp.PPVar)");

		MonoObject *result = NULL;

		printf("Constructing an instance of: %s\n", className.c_str());
		auto instance = pp_instance();
		pluginInstance = create_managed_wrapper(instance, nameSpace.c_str(), className.c_str(), images);

		// Call Init method
		printf("Calling Init on instance of: %s\n", className.c_str());

		void *args[3] = { 0 };
		args[0] = &argc;
		args[1] = parmsArgN;
		args[2] = parmsArgV;
		
		if (mono_invoke_with_desc(":Init(int,string[],string[])", args, instanceClass, pluginInstance, &result))
			return *(bool *)mono_object_unbox(result);

		return true;
	}

	virtual void DidChangeView(const pp::View& view) {
		
		if (!did_change_view)
			return;

		void *args[1] = { 0 };
		intptr_t res = view.pp_resource();
		args[0] = &res;
		MonoObject *result = NULL;
		mono_invoke_with_method(did_change_view, args, pluginInstance, &result);
	}

	virtual void DidChangeFocus(bool has_focus)
	{
		if (!did_change_focus)
			return;

		void *args[1] = { 0 };
		args[0] = &has_focus;
		MonoObject *result = NULL;
		mono_invoke_with_method(did_change_focus, args, pluginInstance, &result);

	}

	virtual bool HandleDocumentLoad(const pp::URLLoader &url_loader)
	{
		if (!handle_document_load)
			return false;

		void *args[1] = { 0 };
		intptr_t res = url_loader.pp_resource();
		args[0] = &res;
		MonoObject *result = NULL;
		mono_invoke_with_method(handle_document_load, args, pluginInstance, &result);
		if (result)
			return *(bool *)mono_object_unbox(result);
		
		return false;
	}

	virtual bool HandleInputEvent(const pp::InputEvent& event) {
		
		if (!handle_input_event)
			return false;
		
		void *args[1] = { 0 };
		intptr_t res = event.pp_resource();
		args[0] = &res;
		MonoObject *result = NULL;
		mono_invoke_with_method(handle_input_event, args, pluginInstance, &result);
		if (result)
			return *(bool *)mono_object_unbox(result);

		return false;
	}

	virtual void HandleMessage(const Var& message)
	{
		if (!handle_message)
			return;

		void *args[1] = { 0 };
		auto var = message.pp_var();
		args[0] = &var;
		MonoObject *result = NULL;
		mono_invoke_with_method(handle_message, args, pluginInstance, &result);

	}

	void MouseLockLost()
	{
		auto mouseLockLost = mono_class_get_method_from_desc_recursive(instanceClass, ":MouseLockLost()");
		if (!mouseLockLost)
			return;
		void *args[1] = { 0 };
		MonoObject *result = NULL;
		mono_invoke_with_method(mouseLockLost, NULL, pluginInstance, &result);
	}

private:

	MonoClass *instanceClass;
	MonoObject *pluginInstance;
	std::vector<MonoImage*> images;

	// methods to be called on our instance
	MonoMethod* did_change_view = NULL;
	MonoMethod* did_change_focus = NULL; 
	MonoMethod* handle_input_event = NULL;
	MonoMethod* handle_document_load = NULL;
	MonoMethod* handle_message = NULL;

};

class PluginModule : public pp::Module {
public:
	PluginModule() : pp::Module() {}
	virtual ~PluginModule() {}

	virtual pp::Instance* CreateInstance(PP_Instance instance) {
		return new PluginInstance(instance);
	}
};



namespace pp {

	Module* CreateModule() { return new PluginModule(); }
}  // namespace pp
