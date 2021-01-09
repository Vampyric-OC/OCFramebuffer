#include <stdio.h>
#include <windows.h>

int main(int argc, char *argv[]) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_HIDE;
//  si.wShowWindow = SW_SHOWMINNOACTIVE;

  if (!CreateProcess(NULL, "C:\\Program Files (x86)\\DOSBox-0.74-3\\DOSBox.exe", NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
    printf("CreateProcess failed (%d).\n", (int)GetLastError());
  }

  char path[_MAX_PATH];
  GetFullPathName("D:\\Desktop\\OCFramebuffer\\boxlib.dll", _MAX_PATH, path, NULL);

  LPVOID path_addr = VirtualAllocEx(pi.hProcess, NULL, _MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  WriteProcessMemory(pi.hProcess, path_addr, path, strlen(path), NULL);

  LPVOID LoadLibraryA_addr = GetProcAddress(GetModuleHandle("Kernel32"), "LoadLibraryA");
  HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA_addr, path_addr, 0, NULL);

  WaitForSingleObject(hThread, INFINITE);

  DWORD exit_code;
  GetExitCodeThread(hThread, &exit_code);

  CloseHandle(hThread);
  VirtualFreeEx(pi.hProcess, path_addr, 0, MEM_RELEASE);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return 0;
}
