#pragma once
#include "MainWindow.g.h"
#include "HidDevice.h"
#include <vector>
#include <memory>

namespace winrt::KeyConnectConfig::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        ~MainWindow() { StopTimer(); }

        void OnConnectClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnDisconnectClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnReadAllClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnSaveClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnSizeChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::SizeChangedEventArgs const&);
        void OnKeyCellClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

    private:
        void BuildKeyGrid();
        void RefreshGrid();
        void UpdateCell(int idx);
        void SetCellPressed(int idx, bool pressed);
        void SetStatus(winrt::hstring const& msg);
        void EnableControls(bool connected);

        void StartTimer();
        void StopTimer();
        void OnTimerTick(Windows::Foundation::IInspectable const&, Windows::Foundation::IInspectable const&);

        struct KeyCellInfo
        {
            int index;
            winrt::hstring label;
            winrt::hstring pin;
            uint8_t mod;
            uint8_t key;
            bool pressed;
        };

        std::vector<KeyCellInfo> m_cells;
        std::vector<Microsoft::UI::Xaml::Controls::Button> m_buttons;
        std::unique_ptr<HidDevice> m_hid;
        Microsoft::UI::Dispatching::DispatcherQueueTimer m_timer{ nullptr };
        bool m_modified = false;
        inline static const Windows::UI::Color PressedColor = { 0xFF, 0x90, 0xEE, 0x90 };
        inline static const Windows::UI::Color IdleColor = { 0xFF, 0xF0, 0xF0, 0xF0 };

        // XAML named elements (built in code)
        Microsoft::UI::Xaml::Controls::Button m_connectButton;
        Microsoft::UI::Xaml::Controls::Button m_disconnectButton;
        Microsoft::UI::Xaml::Controls::TextBlock m_deviceStatus;
        Microsoft::UI::Xaml::Controls::ComboBox m_layerCombo;
        Microsoft::UI::Xaml::Controls::Button m_readAllButton;
        Microsoft::UI::Xaml::Controls::Button m_saveButton;
        Microsoft::UI::Xaml::Controls::TextBlock m_infoText;
        Microsoft::UI::Xaml::Controls::Grid m_keyGrid;
        Microsoft::UI::Xaml::Controls::TextBlock m_statusText;
    };
}
namespace winrt::KeyConnectConfig::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
