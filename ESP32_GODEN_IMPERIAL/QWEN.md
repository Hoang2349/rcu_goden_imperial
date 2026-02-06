# ESP32 Golden Imperial Hotel Room Control System

## Project Overview

This is an ESP32-based firmware project for the Golden Imperial Hotel Room Control Unit (RCU-V2). The system manages hotel room automation including lighting control, occupancy detection, door sensors, motion sensors, and integration with PMS (Property Management System) via MQTT protocol.

The project is built using the ESP-IDF (Espressif IoT Development Framework) and implements a comprehensive hotel room automation system with features such as:
- Automatic occupancy detection and scene management
- Staff mode functionality
- Welcome scenes for guests
- Standby modes for energy efficiency
- Integration with external systems via Modbus and MQTT

## Architecture

The system consists of several key modules:

### Core Components
- **Main Application**: Located in `/main/main.c`, handles initialization of all subsystems
- **Golden Imperial Logic**: Custom hotel-specific logic in `/main/GodenImperial_Scen/`
- **MQTT Client**: Handles communication with external systems
- **Modbus Interface**: For device communication
- **GPIO Management**: Controls hardware interfaces
- **OTA Updates**: Over-the-air updates for both ESP32 and STM32 components
- **Web Server**: Built-in HTTP server with login interface

### Key Features
- **Occupancy Detection**: Uses door sensors and motion detectors to determine room status
- **Scene Management**: Different lighting scenes for welcome, standby, occupied, and unoccupied states
- **Staff Mode**: Special handling when staff enters an unrented room
- **Setback Control**: Energy-saving automation when rooms are unoccupied
- **PMS Integration**: Communicates room status via MQTT

## Building and Running

### Prerequisites
- ESP-IDF v4.x or later installed and configured
- ESP32 development board
- Appropriate USB-to-serial adapter

### Build Instructions
```bash
# Navigate to project directory
cd ESP32_GODEN_IMPERIAL

# Configure the project
idf.py menuconfig

# Build the project
idf.py build

# Flash to ESP32
idf.py flash monitor
```

### Configuration
- The project uses `partitions.csv` for flash partitioning
- WiFi and MQTT settings can be configured through the AP configuration module
- GPIO pin assignments are defined in the GPIO configuration module

## Development Conventions

### Code Style
- C/C++ code follows standard ESP-IDF conventions
- Formatting is enforced using clang-format (see `.clang-format`)
- CI pipeline enforces code formatting standards

### Project Structure
- `/main/` - Main application code
  - `/driver/` - Hardware abstraction layers
  - `/GodenImperial_Scen/` - Hotel-specific logic
  - `/RuleEngine/` - Scene and automation rules
  - `/ModuleOTAStm32/` - STM32 OTA update module
- `/html/` - Web interface files embedded in firmware
- `/docs/` - Documentation

### Key Data Structures
The system maintains several important state structures:
- `status_room_t` - Current room device status (lights, minibar, etc.)
- `pms_room_status_t` - Room booking and occupancy status
- `status_sensor_t` - Sensor inputs (door, motion)

## Testing and Debugging

### Serial Monitor
Monitor system behavior through serial output:
```bash
idf.py monitor
```

### Web Interface
The system hosts a web interface accessible via the configured IP address:
- Login page at `/login.html`
- Upload interface at `/upload_script.html`

### MQTT Communication
The system communicates via MQTT with topics for:
- Room status updates
- Sensor data
- Command reception
- Device birth/death notifications

## Version Information
- Current version: 0.0.1
- Project name: RCU-V2
- Target platform: ESP32

## Key Files and Directories

- `main/main.c` - Main application entry point
- `main/GodenImperial_Scen/` - Hotel-specific automation logic
- `main/CMakeLists.txt` - Component build configuration
- `partitions.csv` - Flash memory partition layout
- `CMakeLists.txt` - Project-level build configuration
- `html/` - Embedded web interface resources
- `.gitlab-ci.yaml` - CI/CD pipeline configuration

## Hotel Automation Logic

The system implements sophisticated automation rules:
1. **Unrented Rooms**: Staff mode activation when entering
2. **Rented Rooms**: 
   - Welcome scene on guest arrival
   - Occupancy detection via motion sensors
   - Standby mode when unoccupied
   - Timeout mechanisms for state transitions

The system tracks room states including:
- `ROOM_UNRENT` / `ROOM_RENTED`
- `OCCUPIED` / `UNOCCUPIED`
- Various lighting and appliance states