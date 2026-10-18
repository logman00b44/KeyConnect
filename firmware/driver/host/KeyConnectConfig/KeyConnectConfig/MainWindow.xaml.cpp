#include "pch.h"
#include "MainWindow.h"
#include "KeyPickerDialog.h"
#include <sstream>
#include <iomanip>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media;
using namespace Microsoft::UI::Dispatching;

static const char *KEY_LABELS[] = {
    "A","B","C","D","E","F","G","H",
    "I","J","K","L","M","N","O","P"
};
static const char *KEY_PINS[] = {
    "PA0","PA1","PA2","PA3","PA4","PA5","PA6","PA7",
    "PB0","PB1","PB2","PB3","PB4","PB6","PB8","PB10"
};

static hstring HIDModToString(uint8_t mod)
{
    std::wstring s;
    if (mod & 0x01) s += L"Ctrl+";
    if (mod & 0x02) s += L"Shift+";
    if (mod & 0x04) s += L"Alt+";
    if (mod & 0x08) s += L"Win+";
    return s.empty() ? hstring(L"") : hstring(s);
}

static hstring HIDKeyToString(uint8_t key)
{
    if (key >= 0x04 && key <= 0x1D) {
        wchar_t c = L'A' + (key - 0x04);
        return { &c, 1 };
    }
    if (key >= 0x1E && key <= 0x26) {
        wchar_t buf[4];
        swprintf(buf, 4, L"F%d", key - 0x1E + 1);
        return buf;
    }
    if (key == 0x00) return L"---";
    if (key == 0x2B) return L"Tab";
    if (key == 0x2C) return L"Space";
    if (key == 0x28) return L"Enter";
    if (key == 0x29) return L"Esc";
    if (key == 0x2A) return L"Bspc";
    if (key == 0x46) return L"PrntScr";
    if (key == 0x3E) return L"F5";
    wchar_t buf[16];
    swprintf(buf, 16, L"0x%02X", key);
    return buf;
}

static hstring FormatMapping(uint8_t mod, uint8_t key)
{
    hstring modStr = HIDModToString(mod);
    hstring keyStr = HIDKeyToString(key);
    if (modStr.empty()) return keyStr;
    return modStr + keyStr;
}

namespace winrt::KeyConnectConfig::implementation
{
    MainWindow::MainWindow()
    {
        m_hid = std::make_unique<HidDevice>();

        // Build cell data
        for (int i = 0; i < 16; i++) {
            m_cells.push_back({
                .index = i,
                .label = winrt::to_hstring(KEY_LABELS[i]),
                .pin = winrt::to_hstring(KEY_PINS[i]),
                .mod = 0,
                .key = 0,
                .pressed = false
            });
        }

        // === Build UI programmatically ===

        // Row 0 toolbar
        m_connectButton.Content(box_value(L"Connect"));
        m_connectButton.Click({ this, &MainWindow::OnConnectClick });

        m_disconnectButton.Content(box_value(L"Disconnect"));
        m_disconnectButton.IsEnabled(false);
        m_disconnectButton.Click({ this, &MainWindow::OnDisconnectClick });

        m_deviceStatus.Text(L"Disconnected");
        m_deviceStatus.VerticalAlignment(VerticalAlignment::Center);
        m_deviceStatus.Margin({ 8, 0, 0, 0 });

        auto toolbar = StackPanel();
        toolbar.Orientation(Orientation::Horizontal);
        toolbar.Spacing(8);
        toolbar.Margin({ 0, 0, 0, 8 });
        toolbar.Children().Append(m_connectButton);
        toolbar.Children().Append(m_disconnectButton);
        toolbar.Children().Append(m_deviceStatus);

        // Row 1 layer selector
        auto layerLabel = TextBlock();
        layerLabel.Text(L"Layer:");
        layerLabel.VerticalAlignment(VerticalAlignment::Center);

        m_layerCombo.SelectedIndex(0);
        m_layerCombo.IsEnabled(false);
        m_layerCombo.MinWidth(80);
        for (int i = 0; i <= 15; i++)
            m_layerCombo.Items().Append(winrt::box_value(i));

        m_readAllButton.Content(box_value(L"Read All from Device"));
        m_readAllButton.IsEnabled(false);
        m_readAllButton.Click({ this, &MainWindow::OnReadAllClick });

        m_saveButton.Content(box_value(L"Save to Flash"));
        m_saveButton.IsEnabled(false);
        m_saveButton.Click({ this, &MainWindow::OnSaveClick });

        auto layerRow = StackPanel();
        layerRow.Orientation(Orientation::Horizontal);
        layerRow.Spacing(8);
        layerRow.Margin({ 0, 0, 0, 8 });
        layerRow.Children().Append(layerLabel);
        layerRow.Children().Append(m_layerCombo);
        layerRow.Children().Append(m_readAllButton);
        layerRow.Children().Append(m_saveButton);

        // Row 2 info
        m_infoText.Text(L"Click a key cell to edit its mapping");
        m_infoText.Margin({ 0, 0, 0, 4 });
        m_infoText.Opacity(0.6);

        // Row 3 key grid (inside a border)
        m_keyGrid.HorizontalAlignment(HorizontalAlignment::Stretch);
        m_keyGrid.VerticalAlignment(VerticalAlignment::Stretch);

        auto gridBorder = Border();
        gridBorder.BorderThickness({ 1, 1, 1, 1 });
        gridBorder.Padding({ 4, 4, 4, 4 });
        gridBorder.Child(m_keyGrid);

        // Row 4 status bar
        m_statusText.Text(L"Ready");
        m_statusText.Margin({ 0, 8, 0, 0 });
        m_statusText.Opacity(0.5);

        // Root grid
        auto root = Grid();
        root.Margin({ 12, 12, 12, 12 });

        auto rowDefs = root.RowDefinitions();
        rowDefs.Append(RowDefinition());
        rowDefs.Append(RowDefinition());
        rowDefs.Append(RowDefinition());
        rowDefs.Append(RowDefinition());
        rowDefs.Append(RowDefinition());

        Grid::SetRow(toolbar, 0);
        root.Children().Append(toolbar);
        Grid::SetRow(layerRow, 1);
        root.Children().Append(layerRow);
        Grid::SetRow(m_infoText, 2);
        root.Children().Append(m_infoText);
        Grid::SetRow(gridBorder, 3);
        root.Children().Append(gridBorder);
        Grid::SetRow(m_statusText, 4);
        root.Children().Append(m_statusText);

        Content(root);

        BuildKeyGrid();

        EnableControls(false);
    }

    void MainWindow::BuildKeyGrid()
    {
        m_keyGrid.RowDefinitions().Clear();
        m_keyGrid.ColumnDefinitions().Clear();

        for (int r = 0; r < 4; r++) {
            m_keyGrid.RowDefinitions().Append(RowDefinition());
            m_keyGrid.ColumnDefinitions().Append(ColumnDefinition());
        }

        m_buttons.clear();
        m_buttons.reserve(16);

        for (int i = 0; i < 16; i++) {
            int row = i / 4;
            int col = i % 4;

            auto btn = Button();
            btn.Tag(winrt::box_value(i));
            btn.Margin({ 2,2,2,2 });
            btn.Padding({ 4,4,4,4 });
            btn.HorizontalAlignment(HorizontalAlignment::Stretch);
            btn.VerticalAlignment(VerticalAlignment::Stretch);
            btn.Click({ this, &MainWindow::OnKeyCellClick });

            auto sp = StackPanel();
            sp.Orientation(Orientation::Vertical);
            sp.HorizontalAlignment(HorizontalAlignment::Center);

            auto tbLabel = TextBlock();
            tbLabel.FontWeight(Windows::UI::Text::FontWeights::Bold());
            tbLabel.TextAlignment(TextAlignment::Center);
            tbLabel.Text(m_cells[i].label + L" (" + m_cells[i].pin + L")");

            auto tbMap = TextBlock();
            tbMap.TextAlignment(TextAlignment::Center);
            tbMap.Text(L"---");
            tbMap.FontSize(11);
            tbMap.Opacity(0.7);

            sp.Children().Append(tbLabel);
            sp.Children().Append(tbMap);
            btn.Content(sp);

            Grid::SetRow(btn, row);
            Grid::SetColumn(btn, col);
            m_keyGrid.Children().Append(btn);
            m_buttons.push_back(btn);
        }
    }

    void MainWindow::UpdateCell(int idx)
    {
        auto &cell = m_cells[idx];
        auto btn = m_buttons[idx];
        auto sp = btn.Content().as<StackPanel>();
        auto tbMap = sp.Children().GetAt(1).as<TextBlock>();
        tbMap.Text(FormatMapping(cell.mod, cell.key));
    }

    void MainWindow::RefreshGrid()
    {
        for (int i = 0; i < 16; i++)
            UpdateCell(i);
    }

    void MainWindow::SetCellPressed(int idx, bool pressed)
    {
        auto &cell = m_cells[idx];
        if (cell.pressed == pressed) return;
        cell.pressed = pressed;
        auto brush = Media::SolidColorBrush(
            pressed ? PressedColor : IdleColor
        );
        m_buttons[idx].Background(brush);
    }

    void MainWindow::SetStatus(winrt::hstring const &msg)
    {
        m_statusText.Text(msg);
    }

    void MainWindow::EnableControls(bool connected)
    {
        m_connectButton.IsEnabled(!connected);
        m_disconnectButton.IsEnabled(connected);
        m_layerCombo.IsEnabled(connected);
        m_readAllButton.IsEnabled(connected);
        m_saveButton.IsEnabled(connected);
        m_deviceStatus.Text(connected ? L"Connected" : L"Disconnected");

        if (!connected) {
            for (auto &cell : m_cells) {
                cell.mod = 0;
                cell.key = 0;
                cell.pressed = false;
            }
            RefreshGrid();
        for (auto &btn : m_buttons)
            btn.Background(Media::SolidColorBrush(Windows::UI::Colors::Transparent()));
            StopTimer();
        }
    }

    void MainWindow::OnConnectClick(IInspectable const &, RoutedEventArgs const &)
    {
        if (!m_hid->Open()) {
            SetStatus(L"Failed to connect: device not found");
            return;
        }
        EnableControls(true);
        SetStatus(L"Connected. Click 'Read All' to load keymap.");
        StartTimer();
    }

    void MainWindow::OnDisconnectClick(IInspectable const &, RoutedEventArgs const &)
    {
        m_hid->Close();
        EnableControls(false);
        SetStatus(L"Disconnected");
    }

    void MainWindow::OnReadAllClick(IInspectable const &, RoutedEventArgs const &)
    {
        int layer = 0;
        auto sel = m_layerCombo.SelectedItem();
        if (sel) layer = winrt::unbox_value<int>(sel);

        SetStatus(L"Reading " + winrt::to_hstring(layer) + L"...");

        for (int i = 0; i < 16; i++) {
            uint8_t mod, key;
            if (m_hid->ReadKeymapEntry(layer, i, &mod, &key)) {
                m_cells[i].mod = mod;
                m_cells[i].key = key;
                UpdateCell(i);
            }
        }
        m_modified = false;
        SetStatus(L"Read complete. Click a key to edit.");
    }

    void MainWindow::OnSaveClick(IInspectable const &, RoutedEventArgs const &)
    {
        if (m_hid->SaveToFlash()) {
            SetStatus(L"Configuration saved to flash.");
            m_modified = false;
        } else {
            SetStatus(L"Save failed.");
        }
    }

    void MainWindow::OnKeyCellClick(IInspectable const &sender, RoutedEventArgs const &)
    {
        auto btn = sender.as<Button>();
        int idx = winrt::unbox_value<int>(btn.Tag());

        auto dialog = winrt::make<KeyPickerDialog>();
        dialog.XamlRoot(this->Content().XamlRoot());
        dialog.KeyIndex(idx);
        dialog.KeyLabel(m_cells[idx].label);
        dialog.PinName(m_cells[idx].pin);
        dialog.CurrentMod(m_cells[idx].mod);
        dialog.CurrentKey(m_cells[idx].key);

        auto result = dialog.ShowAsync().get();
        if (result == Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary) {
            uint8_t newMod = dialog.ResultMod();
            uint8_t newKey = dialog.ResultKey();
            int layer = winrt::unbox_value<int>(m_layerCombo.SelectedItem());

            if (m_hid->WriteKeymapEntry(layer, idx, newMod, newKey)) {
                m_cells[idx].mod = newMod;
                m_cells[idx].key = newKey;
                UpdateCell(idx);
                m_modified = true;
                SetStatus(L"Key " + m_cells[idx].label + L" updated.");
            } else {
                SetStatus(L"Write failed.");
            }
        }
    }

    void MainWindow::OnSizeChanged(IInspectable const &, SizeChangedEventArgs const &)
    {
        double w = m_keyGrid.ActualWidth();
        double h = m_keyGrid.ActualHeight();
        double cellSize = (std::min)((w - 12) / 4, (h - 12) / 4);
        if (cellSize < 60) cellSize = 60;

        for (auto &btn : m_buttons) {
            btn.Width(cellSize);
            btn.Height(cellSize + 20);
        }
    }

    void MainWindow::StartTimer()
    {
        if (!m_timer) {
            auto queue = DispatcherQueue::GetForCurrentThread();
            m_timer = queue.CreateTimer();
            m_timer.Interval(std::chrono::milliseconds(50));
            m_timer.Tick({ this, &MainWindow::OnTimerTick });
        }
        m_timer.Start();
    }

    void MainWindow::StopTimer()
    {
        if (m_timer) m_timer.Stop();
    }

    void MainWindow::OnTimerTick(IInspectable const &, IInspectable const &)
    {
        if (!m_hid->IsOpen()) return;

        uint16_t state = m_hid->GetMatrixState();
        if (state == 0xFFFF) return;

        for (int i = 0; i < 16; i++)
            SetCellPressed(i, (state >> i) & 1);
    }
}
