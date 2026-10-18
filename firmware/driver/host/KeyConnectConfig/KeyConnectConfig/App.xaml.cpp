#include "pch.h"
#include "App.h"
#include "MainWindow.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

int32_t WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int32_t)
{
    winrt::init_apartment();
    Application::Start([](auto&&) {
        winrt::make<winrt::KeyConnectConfig::implementation::App>();
    });
    return 0;
}

namespace
{
    winrt::Microsoft::UI::Xaml::Window g_mainWindow{ nullptr };
}

namespace winrt::KeyConnectConfig::implementation
{
    App::App()
    {
    }

    void App::OnLaunched(LaunchActivatedEventArgs const &)
    {
        g_mainWindow = winrt::make<MainWindow>();
        g_mainWindow.Activate();
    }
}
