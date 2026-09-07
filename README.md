# securitycam

Hobby project that combines a **Node.js web page**, an **Arduino ultrasonic sensor + buzzer**, and an **ESP32-CAM** stream.

The Node process (`app.js`) serves a static dashboard, talks to the Arduino over serial, and pushes distance readings to the browser with Socket.IO. The camera feed is not processed by Node; the page embeds the ESP32-CAM MJPEG stream in an iframe.

This is a student / lab prototype. It is not a finished security product.

## 한국어 요약

Arduino(초음파+부저)와 ESP32-CAM을 Node.js(Express + Socket.IO) 웹 페이지에 연결한 프로토타입입니다. `npm install` 후 `npm start`로 서버를 띄우고 `http://localhost:3000`을 엽니다. 시리얼 포트(`COM4`)와 카메라 IP는 코드에 하드코딩되어 있어 환경에 맞게 바꿔야 합니다. Express generator용 `routes/`·`views/`는 현재 서버에 연결되어 있지 않습니다.

## How it works

```
Browser  (public/index.html)
   |  GET /  (static files from Express)
   |  Socket.IO event "dist"
   |  GET /security_on  or  /security_off
   v
Node     (app.js, port 3000)
   |  serial 9600, Windows port COM4
   v
Arduino  (security/security.ino)
   HC-SR04 distance  +  buzzer when "security" is on

ESP32-CAM  (ESP32_CameraWebServer/)
   Wi-Fi HTTP server :80   +  stream :81/stream
   (iframe URL is hardcoded in public/index.html)
```

1. Arduino measures distance with an HC-SR04 and prints `dist` plus a number over serial.
2. `app.js` reads those lines and emits a Socket.IO event named `dist`.
3. The dashboard shows that value and can send `sec1` / `sec0` to turn the Arduino buzzer mode on or off.
4. The ESP32-CAM runs a separate Camera Web Server. The dashboard only iframes its `/stream` URL.

## Requirements

- Node.js (the lockfile was generated with npm; `serialport` 9.x needs a working native build)
- A board that can run `security/security.ino` (Arduino-style `tone()` / `serialEvent`)
- HC-SR04 ultrasonic sensor and a buzzer
- ESP32-CAM (sketch selects **AI-Thinker**)
- Arduino IDE (or equivalent) to flash the two sketches

The serial path in `app.js` is the Windows device `COM4`. On macOS/Linux you must change that string (for example `/dev/ttyUSB0`) before the server will stay up.

## Run the Node server

```bash
npm install
npm start
```

`npm start` runs `node app.js`. The process listens on **port 3000** and serves files from `public/`.

Open [http://localhost:3000](http://localhost:3000) (`public/index.html`).

This repo currently commits a `node_modules` tree. `serialport` 9.x includes a native addon, so those binaries are OS-specific (the checked-in build looks like a Windows addon). If `npm start` dies with a missing or invalid `.node` binding, delete `node_modules` and run `npm install` on the machine you will use. An older Node (the stack is Express 4.16 / serialport 9) is more likely to compile that addon than current Node 22.

Without a board on `COM4`, SerialPort will fail to open. The HTTP server may still print `listening on *:3000`, but `/security_on`, `/security_off`, and live distance updates will not work.

### Node HTTP and Socket.IO

| Path / event | Role |
| --- | --- |
| `GET /` and other static files | `express.static` from `public/` |
| `GET /security_on` | writes `sec1\n\r` to the serial port |
| `GET /security_off` | writes `sec0\n\r` to the serial port |
| Socket.IO `dist` | payload is the serial line (the `dist` prefix is stripped when present) |

The dashboard calls the security URLs as `http://localhost:3000/security_on` and `.../security_off`.

## Hardware notes (from the sketches)

### Arduino — `security/security.ino`

| Pin | Use |
| --- | --- |
| 3 | buzzer (`tone`) |
| 4 | HC-SR04 `TRIG` |
| 5 | HC-SR04 `ECHO` |

- Serial **9600** baud.
- Each loop: trigger the sensor, print `dist` + `distance_mm / 10` (so the number is roughly centimeters), then wait 500 ms.
- Incoming lines that start with `sec` set a boolean: `sec1` → on, `sec0` → off.
- When that flag is on and the measured distance is **below 750 mm**, the buzzer beeps.

### ESP32-CAM — `ESP32_CameraWebServer/`

Adapted from the Espressif Camera Web Server example.

- Board define: `CAMERA_MODEL_AI_THINKER`.
- Connects to Wi-Fi using the `ssid` / `password` strings in `ESP32_CameraWebServer.ino`. Replace those with your own network before flashing.
- Starts an HTTP UI on port **80** (`/`, `/status`, `/control`, `/capture`) and MJPEG on port **81** (`/stream`).
- Face detect / enroll / recognize code is present in `app_httpd.cpp`. The Node dashboard does not use it.

After flash, the serial monitor prints the camera IP. Put that IP into the iframe in `public/index.html` (currently `http://192.168.231.68:81/stream`).

## Working vs incomplete

### Implemented and wired together

- `app.js`: Express static server, SerialPort readline parser, Socket.IO `dist`, security GET routes.
- `public/index.html`: iframe stream, ON/OFF control, live distance, simple clock.
- `security/security.ino`: distance printout and buzzer when security mode is on.
- `ESP32_CameraWebServer/`: standalone camera HTTP + stream server for AI-Thinker ESP32-CAM.

### Present in the repo but not part of the running Node app

| Path | Why it is leftover |
| --- | --- |
| `bin/www` | Express generator entry. `app.js` does not `module.exports` an Express app, so this file is unused after the start-script fix. |
| `routes/index.js`, `routes/users.js` | Default Express routers; `app.js` never `require`s them. |
| `views/*.jade` | Jade templates for a stock Express page; no view engine is configured in `app.js`. |
| `public/index__.html` | Older draft (broken `javscripts` script path, different hardcoded camera IP). Not the file served as `/`. |
| `public/ESP32-CAM.html` | Stock OV2640 control page that talks to `document.location.origin`. It is meant to be served by the ESP32, not by this Node app. |
| `public/javascripts/clock.js`, `openweather.current.js`, `annyang.min.js`, `responsivevoice.js` | Not referenced by `public/index.html`. |

### Gaps (do not expect these)

- No login, HTTPS, or access control.
- No video recording, cloud upload, motion clips, or notification service.
- Camera IP, serial port, Wi-Fi credentials, and `localhost` URLs are hardcoded.
- Distance display has no unit label in the UI (the Arduino number is mm/10).
- `package.json` still lists Express-generator packages (`jade`, `morgan`, `cookie-parser`, `http-errors`) that `app.js` does not load.

## Repository layout

```
app.js                      Node server (the process npm start runs)
package.json
public/index.html           Dashboard
public/ESP32-CAM.html       Unused copy of the ESP32 camera UI
public/index__.html         Unused draft page
routes/                     Unused Express generator routes
views/                      Unused Jade views
security/security.ino       Arduino distance + buzzer
ESP32_CameraWebServer/      ESP32-CAM firmware
```

## License / origin

`ESP32_CameraWebServer/app_httpd.cpp` carries the Espressif Apache-2.0 header. There is no project-level license file in this repository.
