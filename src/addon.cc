#include <iostream>

#include <nan.h>
#include <windows.h>

#include <psapi.h>
#pragma comment(lib, "psapi.lib")

using namespace v8;
using namespace std;

vector<string> window_titles;
vector<string> process_paths;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam) {
  DWORD process_id;
  HANDLE process_handle;
  char window_title[512];
  char process_path[512];

  // Get window title
  GetWindowText(hwnd, window_title, sizeof(window_title));

  // Get process id from window handle
  GetWindowThreadProcessId(hwnd, &process_id);
  process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, process_id);

  // Get process name from process handle
  GetProcessImageFileName(process_handle, process_path, 4 * 1024);

  // Push values to vectors
  window_titles.push_back(string(window_title));
  process_paths.push_back(string(process_path));

  // Release handle
  CloseHandle(process_handle);

  return true;
}

void GetProcessList(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  window_titles = vector<string>();
  process_paths = vector<string>();

  EnumWindows(EnumWindowsProc, (LPARAM) 0);

  Handle<Array> results = Array::New(isolate);
  for (size_t i = 0; i < window_titles.size(); i++) {
    Local<Object> result = Object::New(isolate);
    result->Set(
      String::NewFromUtf8(isolate, "window_title"),
      String::NewFromUtf8(isolate, window_titles[i].c_str())
    );
    result->Set(
      String::NewFromUtf8(isolate, "process_path"),
      String::NewFromUtf8(isolate, process_paths[i].c_str())
    );
    results->Set(i, result);
  }
  args.GetReturnValue().Set(results);
}

void Init(Local<Object> exports, Local<Object> module)
{
  NODE_SET_METHOD(exports, "parseSync", GetProcessList);
}

NODE_MODULE(addon, Init);
