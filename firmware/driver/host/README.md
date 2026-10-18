# KeyConnect R1 – Host Tools

## CLI Tool (`keyconnect-config`)

Command-line tool for reading/writing keymap via HID feature reports.

### Build

```console
cl /Fe:keyconnect-config.exe keyconnect-config.c /I path\to\hidapi hidapi.lib
```

### Usage

```
keyconnect-config list                          # list devices
keyconnect-config read  <layer> <key>           # read one key (two-step)
keyconnect-config write <layer> <key> <mod> <usage>  # write one key
keyconnect-config save                          # save to flash
```

### Protocol (see `driver/firmware/config_protocol.md`)

Read uses a two-step exchange:
1. `SET_REPORT` with Report ID 1 (selects layer+key cursor)
2. `GET_REPORT` with Report ID 1 (reads mapping at cursor)

---

## WinUI 3 GUI App (`KeyConnectConfig/`)

Desktop app with visual 4×4 key grid, real-time matrix state indicator,
and graphical key picker dialog.

### Prerequisites

- Windows 10 1809+
- Visual Studio 2022 with **Windows App SDK** workload installed
- hidapi.dll (64-bit) — included in the project directory

### Build

1. Open `KeyConnectConfig/KeyConnectConfig.sln` in Visual Studio 2022
2. Restore NuGet packages (Windows App SDK 1.6.x)
3. Build for x64 (Debug or Release)
4. `hidapi.dll` is copied to the output directory automatically

Alternatively, from command line:
```console
msbuild KeyConnectConfig.sln /t:restore;Build /p:Configuration=Debug /p:Platform=x64
```

### Features

| Feature | Description |
|---------|-------------|
| Connect/Disconnect | HID enumeration by VID 0x2709 / PID 0x0001 |
| Read All | Fetches all 16 key mappings from device via feature reports |
| Key Grid | 4×4 layout showing label (A–P), pin name (PA0…PB10), and current mapping |
| Edit Key | Click a cell → ContentDialog with modifier checkboxes and keycode picker |
| Save to Flash | Commits current RAM config to flash (last page) |
| Matrix Tester | Polls Report ID 4 at 50ms intervals → highlights pressed keys green |

### Dependencies

- [Microsoft.WindowsAppSDK](https://www.nuget.org/packages/Microsoft.WindowsAppSDK/) (NuGet)
- [hidapi](https://github.com/libusb/hidapi) (MIT) — loaded at runtime via `hidapi.dll`
