#pragma once
#include "KeyPickerDialog.g.h"

namespace winrt::KeyConnectConfig::implementation
{
    struct KeyPickerDialog : KeyPickerDialogT<KeyPickerDialog>
    {
        KeyPickerDialog();

        int32_t KeyIndex();
        void KeyIndex(int32_t val);
        winrt::hstring KeyLabel();
        void KeyLabel(winrt::hstring const& val);
        winrt::hstring PinName();
        void PinName(winrt::hstring const& val);
        uint8_t CurrentMod();
        void CurrentMod(uint8_t val);
        uint8_t CurrentKey();
        void CurrentKey(uint8_t val);

        uint8_t ResultMod();
        uint8_t ResultKey();

        void OnModChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnKeycodeChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const&);

    private:
        void UpdatePreview();
        void PopulateKeycodes();

        int32_t m_keyIndex = 0;
        winrt::hstring m_keyLabel;
        winrt::hstring m_pinName;
        uint8_t m_currentMod = 0;
        uint8_t m_currentKey = 0;
        uint8_t m_resultMod = 0;
        uint8_t m_resultKey = 0;
        bool m_updating = false;

        struct KeycodeItem
        {
            winrt::hstring name;
            uint8_t code;
        };
        std::vector<KeycodeItem> m_keycodes;

        // XAML named elements (built in code)
        Microsoft::UI::Xaml::Controls::TextBlock m_keyInfoText;
        Microsoft::UI::Xaml::Controls::CheckBox m_ctrlCheck;
        Microsoft::UI::Xaml::Controls::CheckBox m_shiftCheck;
        Microsoft::UI::Xaml::Controls::CheckBox m_altCheck;
        Microsoft::UI::Xaml::Controls::CheckBox m_winCheck;
        Microsoft::UI::Xaml::Controls::ComboBox m_keycodeCombo;
        Microsoft::UI::Xaml::Controls::TextBlock m_previewText;
    };
}
namespace winrt::KeyConnectConfig::factory_implementation
{
    struct KeyPickerDialog : KeyPickerDialogT<KeyPickerDialog, implementation::KeyPickerDialog>
    {
    };
}
