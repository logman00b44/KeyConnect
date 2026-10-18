#include "pch.h"
#include "KeyPickerDialog.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media;

static hstring HIDModToString(uint8_t mod)
{
    std::wstring s;
    if (mod & 0x01) s += L"Ctrl+";
    if (mod & 0x02) s += L"Shift+";
    if (mod & 0x04) s += L"Alt+";
    if (mod & 0x08) s += L"Win+";
    return s.empty() ? hstring(L"") : hstring(s);
}

static hstring HIDKeyToName(uint8_t key)
{
    if (key >= 0x04 && key <= 0x1D) { wchar_t c = L'A' + (key - 0x04); return { &c, 1 }; }
    if (key >= 0x1E && key <= 0x27) { wchar_t buf[8]; swprintf(buf, 8, L"F%d", key - 0x1E + 1); return buf; }
    if (key == 0x00) return L"None (disabled)";
    if (key == 0x28) return L"Enter";
    if (key == 0x29) return L"Escape";
    if (key == 0x2A) return L"Backspace";
    if (key == 0x2B) return L"Tab";
    if (key == 0x2C) return L"Space";
    if (key == 0x46) return L"Print Screen";
    if (key == 0x3E) return L"F5";
    if (key == 0x4C) return L"Delete";
    if (key == 0x4F) return L"Right";
    if (key == 0x50) return L"Left";
    if (key == 0x51) return L"Down";
    if (key == 0x52) return L"Up";
    if (key == 0xE0) return L"Left Ctrl";
    if (key == 0xE1) return L"Left Shift";
    if (key == 0xE2) return L"Left Alt";
    if (key == 0xE3) return L"Left Win";
    wchar_t buf[16]; swprintf(buf, 16, L"0x%02X", key); return buf;
}

namespace winrt::KeyConnectConfig::implementation
{
    KeyPickerDialog::KeyPickerDialog()
    {
        Title(box_value(L"Configure Key"));
        PrimaryButtonText(L"OK");
        SecondaryButtonText(L"Cancel");
        DefaultButton(ContentDialogButton::Primary);

        // Build content
        m_keyInfoText.FontWeight(Windows::UI::Text::FontWeights::Bold());
        m_keyInfoText.FontSize(16);

        auto modLabel = TextBlock();
        modLabel.Text(L"Modifiers:");
        modLabel.Margin({ 0, 4, 0, 0 });

        m_ctrlCheck.Content(box_value(L"Ctrl"));
        m_ctrlCheck.Checked({ this, &KeyPickerDialog::OnModChanged });
        m_ctrlCheck.Unchecked({ this, &KeyPickerDialog::OnModChanged });

        m_shiftCheck.Content(box_value(L"Shift"));
        m_shiftCheck.Checked({ this, &KeyPickerDialog::OnModChanged });
        m_shiftCheck.Unchecked({ this, &KeyPickerDialog::OnModChanged });

        m_altCheck.Content(box_value(L"Alt"));
        m_altCheck.Checked({ this, &KeyPickerDialog::OnModChanged });
        m_altCheck.Unchecked({ this, &KeyPickerDialog::OnModChanged });

        m_winCheck.Content(box_value(L"Win"));
        m_winCheck.Checked({ this, &KeyPickerDialog::OnModChanged });
        m_winCheck.Unchecked({ this, &KeyPickerDialog::OnModChanged });

        auto modRow = StackPanel();
        modRow.Orientation(Orientation::Horizontal);
        modRow.Spacing(16);
        modRow.Children().Append(m_ctrlCheck);
        modRow.Children().Append(m_shiftCheck);
        modRow.Children().Append(m_altCheck);
        modRow.Children().Append(m_winCheck);

        auto kcLabel = TextBlock();
        kcLabel.Text(L"Keycode:");
        kcLabel.Margin({ 0, 4, 0, 0 });

        m_keycodeCombo.MinWidth(200);
        m_keycodeCombo.SelectionChanged({ this, &KeyPickerDialog::OnKeycodeChanged });

        auto prevLabel = TextBlock();
        prevLabel.Text(L"Preview:");
        prevLabel.Margin({ 0, 8, 0, 0 });
        prevLabel.FontWeight(Windows::UI::Text::FontWeights::SemiBold());

        m_previewText.Text(L"(none)");
        m_previewText.FontSize(14);

        auto prevBorder = Border();
        prevBorder.BorderThickness({ 1, 1, 1, 1 });
        prevBorder.Padding({ 8, 8, 8, 8 });
        prevBorder.Child(m_previewText);

        auto content = StackPanel();
        content.Spacing(8);
        content.MinWidth(300);
        content.Children().Append(m_keyInfoText);
        content.Children().Append(modLabel);
        content.Children().Append(modRow);
        content.Children().Append(kcLabel);
        content.Children().Append(m_keycodeCombo);
        content.Children().Append(prevLabel);
        content.Children().Append(prevBorder);

        Content(content);

        PopulateKeycodes();
        m_keycodeCombo.SelectedIndex(0);
    }

    void KeyPickerDialog::PopulateKeycodes()
    {
        m_keycodes.clear();
        m_keycodes.push_back({ L"None (disabled)", 0x00 });

        for (uint8_t c = 0x04; c <= 0x1D; c++) {
            wchar_t name[2] = { static_cast<wchar_t>(L'A' + (c - 0x04)), 0 };
            m_keycodes.push_back({ name, c });
        }

        for (uint8_t f = 0x1E; f <= 0x27; f++) {
            wchar_t buf[8]; swprintf(buf, 8, L"F%d", f - 0x1E + 1);
            m_keycodes.push_back({ buf, f });
        }

        struct { const wchar_t* name; uint8_t code; } extras[] = {
            { L"Enter", 0x28 }, { L"Escape", 0x29 }, { L"Backspace", 0x2A },
            { L"Tab", 0x2B }, { L"Space", 0x2C }, { L"Print Screen", 0x46 },
            { L"Delete", 0x4C }, { L"Left", 0x50 }, { L"Right", 0x4F },
            { L"Up", 0x52 }, { L"Down", 0x51 },
        };
        for (auto &e : extras)
            m_keycodes.push_back({ e.name, e.code });

        for (auto &item : m_keycodes)
            m_keycodeCombo.Items().Append(winrt::box_value(item.name));
    }

    int32_t KeyPickerDialog::KeyIndex() { return m_keyIndex; }
    void KeyPickerDialog::KeyIndex(int32_t val) { m_keyIndex = val; }

    hstring KeyPickerDialog::KeyLabel() { return m_keyLabel; }
    void KeyPickerDialog::KeyLabel(hstring const &val) { m_keyLabel = val; }

    hstring KeyPickerDialog::PinName() { return m_pinName; }
    void KeyPickerDialog::PinName(hstring const &val) { m_pinName = val; }

    uint8_t KeyPickerDialog::CurrentMod() { return m_currentMod; }
    void KeyPickerDialog::CurrentMod(uint8_t val) { m_currentMod = val; }

    uint8_t KeyPickerDialog::CurrentKey() { return m_currentKey; }
    void KeyPickerDialog::CurrentKey(uint8_t val) { m_currentKey = val; }

    uint8_t KeyPickerDialog::ResultMod() { return m_resultMod; }
    uint8_t KeyPickerDialog::ResultKey() { return m_resultKey; }

    void KeyPickerDialog::OnModChanged(IInspectable const &, RoutedEventArgs const &)
    {
        UpdatePreview();
    }

    void KeyPickerDialog::OnKeycodeChanged(IInspectable const &, SelectionChangedEventArgs const &)
    {
        UpdatePreview();
    }

    void KeyPickerDialog::UpdatePreview()
    {
        if (m_updating) return;
        m_updating = true;

        m_keyInfoText.Text(m_keyLabel + L" (" + m_pinName + L")");

        uint8_t mod = 0;
        if (m_ctrlCheck.IsChecked().GetBoolean()) mod |= 0x01;
        if (m_shiftCheck.IsChecked().GetBoolean()) mod |= 0x02;
        if (m_altCheck.IsChecked().GetBoolean()) mod |= 0x04;
        if (m_winCheck.IsChecked().GetBoolean()) mod |= 0x08;

        int selIdx = m_keycodeCombo.SelectedIndex();
        uint8_t key = (selIdx >= 0 && selIdx < (int)m_keycodes.size())
            ? m_keycodes[selIdx].code : 0x00;

        m_resultMod = mod;
        m_resultKey = key;

        hstring modStr = HIDModToString(mod);
        hstring keyStr = HIDKeyToName(key);
        hstring preview = modStr.empty() ? keyStr : modStr + keyStr;
        m_previewText.Text(preview);

        m_updating = false;
    }
}
