# OpenDash

OpenDash is a Qt-based infotainment center for your Linux OpenAuto installation!
The OpenDash project includes OpenAuto, AASDK, and Dash.

Main features of Dash include:

*	Embedded OpenAuto `Windowed/Fullscreen`
*	Wireless OpenAuto Capability
*	On-screen Volume, Brightness, & Theme Control
*	Responsive Scalable UI `Adjustable for screen size`
*	Bluetooth Media Control
*	Real-Time Vehicle OBD-II Data & SocketCAN Capabilities
*	Theming `Dark/Light mode` `Customizable RGB Accent Color`
*	True Raspberry Pi 7” Official Touchscreen Brightness Control
*	App-Launcher built in
*	Camera Access `Streaming/Local` `Backup` `Dash`
*	Keyboard Shortcuts `GPIO Triggerable`

![](docs/imgs/opendash-ui.gif)

# TODO V0.1:

- [x] Wire up Razor USB Audio
- [x] re-Implement Shutdown Button inside OpenDash // Done through disabling fullscreen
- [x] Wire up 12V from Climate Controls to board using shoes
- [x] Wire up Display power
- [x] Fix housing fitting in
- [x] Make housing clips

# TODO V1.0:

- [ ] Frontend: Fix lag caused by glow - find alternative or remove glow :(
- [ ] Backend: Connect media player controls to Android Auto
    - Note: It seems like it would only work over bluetooth. Test on RPI5
- [x] Frontend: Fix album cover size and track info styles
- [ ] Feature: Implement Steering Wheel buttons
    - [ ] Backend: Setup ESP32 Comms over UART/USB
    - [ ] Connect ESP32 Master device over USB
    - [ ] Connect Buttons line from steering wheel to ESP32 and parse to RPI5
- [ ] Feature: Automatic power on / off
    - [ ] Hardware: Get min. 50W power input from nearest always on source
        - [ ] Find nearest source
    - [ ] Hardware: Detect acc power on the ESP32
    - [ ] Hardware: Implement Relay switch for power control
    - [ ] Backend: Implement soft shutdown

- [ ] Backend: Analyze CAN receive functions
- [x] Frontend: Improve global theming for a single color config
- [x] Fix: Disable scan lines in the AA page when it's active
- [x] Fix: Disable cursor by default on RPI5
- [x] Fix: Fix Camera resolution
- [ ] Enhancement: Create Car driving animation
    - [ ] Get Car rear view SVG or png
    - [ ] Create while loop function, that moves the png or body element of the svg up and down
    - [ ] Alternativly: Move wheels up and down while moving entire image up and down in opposite directions => Car bumping
    - [ ] Create road side SVG
    - [ ] Recolor RPM and speed Boxes depending on the read or simulated speed and rpm data
    - [ ] Create PRNDS boxes with highlight function
    - [ ] Edit main speed Gauge's style
- [x] Enhancement: Create grid layout in Vehicle page
- [ ] Enhancement: Create new sub-pages for the main grid layout (Drive, AndroidAuto, Musicplayer, AC controls)
- [ ] Enhancement: 
- [ ] Hardware: Implement Wheel speed sensor reading circuit  
- [ ] Hardware: Add DS1820b Temp propes
- [ ] Hardware: Implement automatic startup and shutdown w/ ESP32 detection 
- [ ] Hardware: Add Relay for power 
- [ ] Hardware: Wire up USB sound to amplifier 
- [ ] Hardware: Wire up USB microphone 

## Cameras

A rear camera is necessary for parking especially in NRW. A PAL video camera is technically the most straight forward and cheapest approach. However the quality is quite ass and I have very limited access to camera image. However, the analog camera is easiest to install with the current long distance needed.

### Servaillance

#### General

A single reverse camera is not enough for a complete sentry mode. At least 4 camera should be positioned in all directions for a full coverage. A GoPro-like quality won't be nessisary, as it will increase the complexity, price and powerdraw significantly with very rare use cases. 
Best approach seems to be ESP EYE cameras, AKA OV5680 or whatever cameras with an ESP32 each. This will allow easy control and seperation, low cost, and the ESP32 nodes could serve other purposes as well having two cores and some pins.

This setup will allow a complete dash cam experience, as well as live outside monitoring in sentry mode. 

Only unanswered challange would be data transmission. WiFi should be tested more thoroughly, though it still should be better using a more rubost wired transmission. 

#### ESP CAM over WiFi

USB should be considered, with a theoretical speed of 12MBit/s.
UDP can theoretically reach up to 100Mbps bit realistically max out around 20Mbps, which is still better than USB, and requires no cables.

As planned before, ESP32 nodes in the car will communicate over ESP-NOW. So low power and instant low bit rate communication for commands and sensor readings. 

At the same time, a single ESP32 Cam could be able to run human detection and decide when to record or broadcast the live video to the interior, depending on wether I'm inside or not. 
Alternatively, a motion sensor can be paired with the camera module, allowing for more reliable, efficient, and easy to implement presence detection.

Note: Home Assistant could be used to receive video stream. 

#### OV5460 over usb 

Technically it should be possible to serialize OV5460 stream over USB 2.0 to achieve a much smoother video. However a ready to buy solution, while also being cheap and or easy to implement has no been found yet.

# Getting Started

## Video walk through
_steps may be slightly different such as ia (intelligent-auto) has been renamed to dash, the UI has changed, etc..._

https://youtu.be/CIdEN2JNAzw

## Install Script

Dash can be built automatically utilizing an included script.

The install script included in the dash repo will install all the required packages and compile all portions of the OpenDash project.

### 1. Clone the repo, Run the install script
```
git clone https://github.com/openDsh/dash

cd dash

./install.sh
```
