# Wireless Device Battery plugin for TrafficMonitor

Shows the battery level of connected wireless devices (Logitech, Razer, etc.) directly in TrafficMonitor's taskbar/main window, using the same translucent-graph style as the other plugins.

## Data source

The plugin calls the C ABI exported by [wireless-device-battery-monitor-rs](https://github.com/qqrm/wireless-device-battery-monitor-rs):

- `wb_poll_devices_json()` — polls devices once and returns a JSON array
- `wb_free_string()` — frees the returned buffer

The Rust engine is linked statically, so the resulting DLL has no external `wireless-battery.exe` dependency.

## Build

Requirements: Visual Studio 2022 with MFC (v143), Windows SDK 10, x64.

1. Build the Rust static library:

```powershell
git clone https://github.com/qqrm/wireless-device-battery-monitor-rs.git
cd wireless-device-battery-monitor-rs
cargo build --release
```

2. Copy `target\release\wireless_battery.lib` into `lib\x64\` of this repository.

3. Open `WirelessDeviceBattery.vcxproj` in Visual Studio and build `Release|x64`, or:

```powershell
msbuild WirelessDeviceBattery.vcxproj /p:Configuration=Release /p:Platform=x64
```

## Install

Copy `bin\x64\Release\WirelessDeviceBattery.dll` into the `plugins\` directory next to `TrafficMonitor.exe`, enable *Wireless Device Battery* in TrafficMonitor's display settings, and restart TrafficMonitor once.

## Status mapping

- `asleep` / `connected:false` — shown as `(last known)`
- `percent:null` — shown as `unknown` (never treated as 0%)
- `charging` — shown with a lightning mark

## License

GPL-3.0-or-later, because the Rust engine is statically linked. See [LICENSE](LICENSE) and [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
