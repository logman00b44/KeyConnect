#pragma once
#include "App.g.h"

namespace winrt::KeyConnectConfig::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
    };
}
namespace winrt::KeyConnectConfig::factory_implementation
{
    struct App : AppT<App, implementation::App>
    {
    };
}
