# Humidifier scale bench

Two-channel humidifier scale using a Wemos D1 mini V4.0.0 (ESP8266), two HX711s, and two matched 20 kg full-bridge load cells in parallel per tank. ESPHome exposes both tank readings to Home Assistant. Calibration is pending installation of the tanks and measurements with known added weights.

See [ESPHome wiring and setup](esphome/README.md), the [wiring diagram](docs/two-channel-wiring.svg), the [ESPHome configuration](esphome/humidifier-scale.yaml), and the [Home Assistant dashboard](homeassistant/dashboard.yaml). Both HX711 channels have been verified live with ESPHome 2026.9.1 and Home Assistant.

The Arduino sketch and USB graph below are the original one-channel bench test. They require the Arduino firmware; ESPHome uses its own logging protocol and Home Assistant graphs.

## Use it

### USB graph on your laptop

```sh
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
.venv/bin/python scripts/graph.py --port /dev/cu.usbserial-10
```

Open **http://127.0.0.1:8000**. Use your actual USB port. This reads the real serial counts; it does not switch Wi-Fi. Close this process with Ctrl-C before flashing, or using Arduino Serial Monitor/Plotter. The local server retries if USB disconnects. In USB mode timestamps are host receipt times; the on-device Wi-Fi API uses device conversion timestamps. The browser's connection message refers to Wi-Fi; in USB mode check the cable and graph process instead.

### Wi-Fi graph on your phone or laptop

1. Flash `humidifier-scale.ino` with the board set to **LOLIN(WEMOS) D1 R2 & mini**.
2. On a phone or laptop, join **HX711-Scale**, password **hx711test**. Stay connected if it says there is no internet.
3. Open **http://192.168.4.1** (HTTP).
4. Press each cell gently. The signal should change, then return close to its baseline when released. A decreasing reading is fine; calibration can account for the sign.

The graph shows **raw ADC counts, not kg**. **Zero here** subtracts the average of the latest 20 available readings in that browser. **Show raw** removes that offset. Zero is temporary and resets on refresh or device restart. Pause freezes only the graph; acquisition continues. CSV exports the retained minute of raw and offset counts. No smoothing is applied to the plotted signal.

## Wiring

| HX711 | D1 mini |
| --- | --- |
| DOUT / DT | **D5 / GPIO14** |
| SCK / CLK | **D6 / GPIO12** |
| GND | GND |
| VCC / digital supply | 3V3, for a breakout that supports 3.3 V |

Both cells share E+, E−, A+, and A−, matching electrical function, not assumed wire colors. B is unused. This is one combined channel; software cannot read the two parallel cells separately. Calibrate the assembled pair later with known weights. Each cell still has its own 20 kg rating; load distribution matters.

## Build / flash

Dependencies: [ESP8266 Arduino core](https://github.com/esp8266/Arduino) **3.1.2** and **HX711 by Rob Tillaart 0.6.5**. Other HX711 libraries have different initialization APIs. This sketch enables the fast-processor timing option explicitly.

In Arduino IDE, add `https://arduino.esp8266.com/stable/package_esp8266com_index.json` to Additional Boards Manager URLs, install esp8266 3.1.2 and HX711 by Rob Tillaart 0.6.5, select the D1 mini board and USB port, then upload. Keep this folder named `humidifier-scale`.

With Arduino CLI:

```sh
arduino-cli core update-index --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli core install esp8266:esp8266@3.1.2 --additional-urls https://arduino.esp8266.com/stable/package_esp8266com_index.json
arduino-cli lib install HX711@0.6.5
bash scripts/build.sh
arduino-cli board list
bash scripts/flash.sh /dev/cu.usbserial-10
```

Use the actual port from `board list`. ESP8266 uploading also needs Python 3 with `pyserial` installed; if the core reports that it is missing, install it into your selected Python environment. Build output is in `build/`. GitHub Actions compiles the firmware on each push and attaches a binary artifact.

Arduino Serial Plotter also works at **115200 baud**: after the startup text, every line is a raw reading. Close serial monitors before uploading.

## How it works

The loop reads only when DOUT is ready, using channel A / gain 128. The server returns an 80-sample ring buffer from `/api/readings` every time the browser polls (250 ms between requests). Samples include a sequence and device timestamp, so polls do not duplicate points. The browser retains 60 seconds, marks missed samples, and draws gaps instead of joining missing data. Effective sample rate depends on the HX711 RATE setting and server load; most modules default to 10 SPS. The graph and API stay responsive if DOUT never becomes ready.

No new conversion for 2 seconds displays **HX711 not ready**. This checks conversion activity, not whether the bridges themselves are correctly wired. ADC rail readings trigger a wiring/excitation message. Noise span is the last five seconds' peak-to-peak count range.

## Troubleshooting

- **No Wi-Fi:** reset the D1 mini and check USB power. Startup serial output prints the AP address.
- **Not ready:** check common ground, supply, DT on D5, and CLK on D6.
- **At the ADC limit:** inspect A+/A−, E+/E− and bridge excitation voltage.
- **One cell cancels the other:** test each cell individually and check load direction/polarity before reconnecting the pair.
- **Numbers move with no load:** keep signal wiring short, secure the mounting, and check supply/excitation stability.

## Local UI preview

```sh
python3 scripts/graph.py --demo
```

Open http://127.0.0.1:8000. This serves the exact embedded dashboard with **explicitly labeled simulated data**. It does not test the physical HX711.
