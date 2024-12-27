#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>
#include <windows.h>

#include "flutter_window.h"
#include "utils.h"

// [app_links]
#include "app_links/app_links_plugin_c_api.h"

// Constants for reusability
constexpr wchar_t kWindowTitle[] = L"Ente Auth";

bool SendAppLinkToInstance(const std::wstring &title)
{
  // Find our exact window
  HWND hwnd = ::FindWindow(L"FLUTTER_RUNNER_WIN32_WINDOW", title.c_str());

  if (hwnd)
  {
    // Dispatch new link to current window
    SendAppLink(hwnd);

    // Restore our window to front in the same state
    WINDOWPLACEMENT place = {sizeof(WINDOWPLACEMENT)};
    GetWindowPlacement(hwnd, &place);

    switch (place.showCmd)
    {
    case SW_SHOWMAXIMIZED:
      ShowWindow(hwnd, SW_SHOWMAXIMIZED);
      break;
    case SW_SHOWMINIMIZED:
      ShowWindow(hwnd, SW_RESTORE);
      break;
    default:
      ShowWindow(hwnd, SW_NORMAL);
      break;
    }

    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
    SetForegroundWindow(hwnd);

    // Window has been found, don't create another one
    return true;
  }

  // Log if the window is not found
  OutputDebugString(L"Failed to find the target window.\n");
  return false;
}

int APIENTRY wWinMain(_In_ HINSTANCE instance, _In_opt_ HINSTANCE prev,
                      _In_ wchar_t *command_line, _In_ int show_command)
{
  // [app_links]
  if (SendAppLinkToInstance(kWindowTitle))
  {
    OutputDebugString(L"Instance already running. Exiting.\n");
    return EXIT_SUCCESS;
  }

  // Attach to console when present (e.g., 'flutter run') or create a
  // new console when running with a debugger.
  if (!::AttachConsole(ATTACH_PARENT_PROCESS) && ::IsDebuggerPresent())
  {
    CreateAndAttachConsole();
  }

  // Initialize COM
  HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  if (FAILED(hr))
  {
    OutputDebugString(L"Failed to initialize COM.\n");
    return EXIT_FAILURE;
  }

  flutter::DartProject project(L"data");

  // Get command-line arguments
  std::vector<std::string> command_line_arguments = GetCommandLineArguments();
  project.set_dart_entrypoint_arguments(std::move(command_line_arguments));

  // Create the Flutter window
  FlutterWindow window(project);
  Win32Window::Point origin(70, 70);
  Win32Window::Size size(1280, 720);

  if (!window.Create(kWindowTitle, origin, size))
  {
    OutputDebugString(L"Failed to create Flutter window.\n");
    return EXIT_FAILURE;
  }

  window.SetQuitOnClose(true);

  // Message loop
  ::MSG msg;
  while (::GetMessage(&msg, nullptr, 0, 0))
  {
    if (msg.message == WM_QUIT)
    {
      break; // Exit loop on WM_QUIT
    }
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  // Clean up
  ::CoUninitialize();
  OutputDebugString(L"Application exited successfully.\n");
  return EXIT_SUCCESS;
}
