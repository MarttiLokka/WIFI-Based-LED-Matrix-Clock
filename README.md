# WIFI-Based LED Matrix Clock

![Clock](clock.jpg)

A simple WiFi-connected LED matrix clock based on the **"Hello World"** and **Font** examples from the [MajicDesigns MD_Parola](https://github.com/MajicDesigns/MD_Parola) library.

Unlike the [RTC-Based LED Matrix Clock](https://github.com/MarttiLokka/RTC-Based-LED-Matrix-Clock), this version does **not** use an RTC module. The clock gets the current time from an NTP server over WiFi.

The project is intended for a **Raspberry Pi Pico 2W** and a MAX7219-based LED matrix.

## Features

- WiFi-based clock without an RTC module
- NTP time synchronization
- Automatic timezone and daylight-saving-time handling
- Supports up to **three known WiFi networks**
- Scans for available networks and connects to a matching saved network
- Useful when moving the clock between locations, for example home and summer cottage
- No need to manually reconfigure WiFi when a known network is available
- Custom fonts using `Font_Data.h`
- Easy font editing with the MDParola Font Editor
- Adjustable character spacing, alignment and brightness
- Blinking colon effect
- WiFi reconnect handling
- Diagnostic information through the Arduino Serial Monitor

## Hardware

### Controller

- Raspberry Pi Pico 2W

### LED matrix

The example configuration uses four MAX7219 8x8 LED matrix modules.

Hardware type:

```cpp
MD_MAX72XX::FC16_HW
```

### SPI pins

| Function | Pico 2W GPIO |
|---|---:|
| CLK / SCK | GPIO18 |
| DATA / MOSI | GPIO19 |
| CS | GPIO17 |

## Required Libraries

Install the following libraries through the Arduino IDE Library Manager or from their official repositories:

- [MD_Parola](https://github.com/MajicDesigns/MD_Parola)
- [MD_MAX72xx](https://github.com/MajicDesigns/MD_MAX72XX)

The Raspberry Pi Pico 2W WiFi support is provided by the board package and `WiFi.h`.

## WiFi configuration

The clock can store up to three known networks.

Edit this section in the `.ino` file:

```cpp
WiFiNetwork networks[] = {
  {"NETWORK1", "PASSWORD1"},
  {"NETWORK2", "PASSWORD2"},
  {"NETWORK3", "PASSWORD3"}
};
```

The clock first scans the surrounding WiFi networks. If one or more saved networks are found, it tries to connect to them.

This makes the clock easy to move between locations. For example:

- Home WiFi
- Summer cottage WiFi
- Another regularly used network

No additional login or configuration is required as long as one of the known networks is available.

### Security note

Do not commit real WiFi passwords to a public GitHub repository.

For a public repository, replace the example credentials with placeholders before committing the code.

## Time synchronization

The clock uses NTP to obtain the current time:

```text
pool.ntp.org
time.nist.gov
```

The timezone is configured separately:

```cpp
const char* LOCAL_TIMEZONE =
  "EET-2EEST,M3.5.0/3,M10.5.0/4";
```

This is the POSIX timezone rule for Finland:

- EET = UTC+2
- EEST = UTC+3
- Summer time starts on the last Sunday of March
- Winter time starts on the last Sunday of October

The timezone rules are applied by the Pico software, so the displayed time automatically follows Finnish daylight-saving time.

## Important: troubleshooting a black display

The display is intentionally kept **completely black until all required startup conditions are met**.

If the display stays black or suddenly stops displaying the time, open the **Arduino IDE Serial Monitor** at:

```text
115200 baud
```

The serial output tells you what happened.

Possible reasons include:

### No WiFi networks found

```text
No WiFi networks found.
No conditions to continue. Halting.
```

This means the Pico could not see any WiFi networks.

### No known network found

```text
No known WiFi networks found.
No conditions to continue. Halting.
```

WiFi networks were detected, but none of them matched the networks configured in the program.

### Connection failed

```text
Connection failed.
```

A known network was found, but the Pico could not establish a connection.

Possible causes include:

- Incorrect WiFi password
- Weak signal
- Access point temporarily unavailable
- Network authentication problem

### NTP time was not received

During startup the clock waits for a valid time from an NTP server.

If the clock cannot obtain the time, the display must not be assumed to be faulty. Check the Serial Monitor first.

The clock needs both:

1. A working WiFi connection
2. A valid NTP time

Only after these conditions are met is the LED matrix initialized.

## Font editing

One of the main ideas of this project is to keep the font easy to modify.

The font data is stored in:

```text
Font_Data.h
```

The font can be created and edited using the **MDParola Font Editor**:

https://pjrp.github.io/MDParolaFontEditor

This makes it easy to experiment with different digit shapes and create a font that fits the particular LED matrix.

The display formatting is also kept together in `setup()`:

```cpp
P.setFont(digit);
P.setCharSpacing(1);
P.setTextAlignment(PA_CENTER);
P.setIntensity(0);
```

This means the appearance can be changed without modifying the actual clock display logic.

## Display

The clock updates approximately twice per second and alternates between:

```text
12:34
```

and:

```text
12 34
```

This creates the blinking colon effect.

## How it works

The startup sequence is intentionally simple:

```text
Power on
   ↓
Scan WiFi networks
   ↓
Find a known network
   ↓
Connect to WiFi
   ↓
Configure timezone
   ↓
Request NTP time
   ↓
Receive valid time
   ↓
Initialize LED matrix
   ↓
Display clock
```

If a required step fails, the program stops rather than displaying potentially incorrect time.

## RTC vs WiFi version

| Feature | RTC version | WiFi version |
|---|---|---|
| Internet required | No | Yes |
| RTC module | DS3231 | Not required |
| NTP synchronization | No | Yes |
| Automatic time synchronization | No | Yes |
| Works without WiFi | Yes | No |
| Easy to move between networks | N/A | Yes |
| Battery-backed timekeeping | Yes | No |

See the RTC version here:

https://github.com/MarttiLokka/RTC-Based-LED-Matrix-Clock

## Design philosophy

This is intentionally a small and straightforward clock project.

The code started from the basic MD_Parola "Hello World" and font examples and was adapted to:

- connect to WiFi
- obtain time using NTP
- apply timezone and DST rules
- display the current time on a MAX7219 LED matrix

The project does not attempt to become a complete network clock framework. The goal is to keep the code easy to understand and easy to modify.

## License

This project is my own implementation based on examples and functionality provided by the libraries listed above.

Please refer to the individual library repositories for their respective licenses and terms.
