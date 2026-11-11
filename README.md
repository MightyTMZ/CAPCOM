# CAPCOM desktop telemetry API

The Windows program exposes CPU load, memory usage, and aggregate network receive/transmit rates through one read-only HTTP endpoint. The ESP8266 can request the current values with `GET /telemetry`.

## Build

Install CMake and Visual Studio Build Tools (the **Desktop development with C++** workload), then use a Developer PowerShell:

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run the API

```powershell
.\build\Release\capcom_sender.exe
```

The default port is `8080`. Use another port with `--port 9000`.

## Test the endpoint

```powershell
Invoke-RestMethod http://localhost:8080/telemetry
```

From the ESP8266, replace `localhost` with the desktop computer's LAN IP address. Windows Firewall may ask you to permit local-network access the first time the server runs.

Example payload:

```json
{"timestamp_ms":1794376800000,"hostname":"DESKTOP","cpu_percent":18.2,"memory_percent":47.0,"network_rx_bytes_per_sec":12500.4,"network_tx_bytes_per_sec":842.1,"mouse_clicks_last_2s":3,"keyboard_presses_last_2s":8}
```

`mouse_clicks_last_2s` counts left, right, and middle button presses in a rolling
two-second window. `keyboard_presses_last_2s` counts key presses in the same
window, including modifiers, excluding held-key repeats. Double clicks count as
two presses. Reads do not reset either counter; both return zero after two seconds
without input. Collection starts with the server and applies to its interactive
Windows desktop. Only timestamps and transient key-down state are retained, with
no typed text or key identities stored in telemetry. Synthetic input is counted too.
