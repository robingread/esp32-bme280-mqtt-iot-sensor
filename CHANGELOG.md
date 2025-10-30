# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v1.0.2] - 2025/10/30

### Added

- LED Lighting functionality
  - Power LED when the device is turned on.
  - Amber MQTT Error LED for when the MQTT connection is lost.
  - WiFi Error LED for when the WiFi connection is lost.
  
## [v1.0.1] - 2023/12/29

### Added

- Setting the hostname of the ESP32 via the WiFi library to match the `DEVICE_ID` specified in the `credentials.h` file.
- Added serial printing of the set IP address and hostname via the `Serial` device. This is useful when debugging.

## [v1.0.0] - 2023/11/3

### Added

- Initial working version of the code and project.
