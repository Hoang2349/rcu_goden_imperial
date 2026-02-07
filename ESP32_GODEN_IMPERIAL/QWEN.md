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

## Detailed System Architecture

### Main Application Flow (`main/main.c`)
The main application initializes all subsystems in sequence:
1. System initialization (`espSystemInit()`)
2. WiFi AP configuration (`appWifiApInit()`)
3. SPIFFS file system (`appSpiffsInit()`)
4. Ethernet interface (`appEthernetInit()`)
5. GPIO setup (`appSetupGpio()`)
6. Modbus communication (`modbusInit()`)
7. MQTT client (`appMqttInit()`)
8. HTTPS OTA updates (`appHttpsOtaInit()`)
9. Golden Imperial logic (`init_goden_imperial()`)

### Golden Imperial Logic (`main/GodenImperial_Scen/`)
This module contains the core hotel automation logic with several key components:

#### Core State Management
- `status_room_t`: Tracks the current state of all room devices (master switches, lights, minibar, etc.)
- `pms_room_status_t`: Manages room booking and human occupancy status
- `status_sensor_t`: Monitors door and motion sensors
- `status_outdoor_t`: Controls outdoor indicators (DND, MUR, bell)

#### Scene Management Functions
- `handle_scene_unrented()`: Turns off all devices when room is unrented
- `handle_scene_staff_mode()`: Activates all lights when staff enters unrented room
- `handle_scene_welcome()`: Activates welcome scene for first-time guest check-in
- `handle_scene_standby()`: Applies energy-saving setback when unoccupied
- `handle_scene_occupied()`: Maintains occupied state with HVAC enabled

#### Input Handling (`app_handle_input.c`)
Processes events from STM32 microcontroller via queue system:
- Door sensor events (`EVENT_DOOR`)
- Motion sensor events (`EVENT_MOTION`)
- Button presses for various room controls
- DND, MUR, and bell functions
- Individual light controls (master switches, reading lights, ceiling lights, etc.)

#### Output Control (`app_control_output.c`)
Manages all device outputs via Modbus communication:
- Generates Modbus commands for relays and LEDs
- Handles all scene activation functions
- Manages DND/MUR mutual exclusivity
- Controls HVAC systems (ITC/IDU)

### Configuration Management (`app_nvs_config.c`)
- Stores persistent configuration in ESP32's NVS (Non-Volatile Storage)
- Configurable parameters include:
  - `time_crossing`: Timeout for occupancy detection (default: 30 minutes)
  - `time_door_ajar`: Duration before door ajar alert (default: 6 seconds)

### Rule Engine (`main/RuleEngine/`)
Manages automation rules with two operational modes:
- `RULE_LOCAL`: Executes rules stored locally on the device
- `RULE_MQTT`: Executes rules received from MQTT broker
- Automatically switches between modes based on connectivity

### Hardware Interfaces
- **Relay Control**: Up to 12 relays for controlling room devices
- **LED Indicators**: Up to 12 LEDs for status indication
- **RS485 Communication**: For connecting to STM32 microcontroller
- **Ethernet/WiFi**: For MQTT communication with PMS
- **Modbus RTU**: For communicating with connected devices

### Room Status Transitions
The system implements a comprehensive state machine for room management:

#### Unrented Room States:
- `UNRENTED`: Room is available, all devices off
- `STAFF`: Staff entered unrented room, all lights on temporarily

#### Rented Room States:
- `WELCOME`: Guest first entered after check-in, all lights on
- `OCCUPIED`: Guest present, normal operation
- `STANDBY`: Room empty but rented, setback mode active
- `SETBACK_ACTIVE`: Energy-saving mode when unoccupied

### MQTT Communication Protocol
The system publishes various status updates to MQTT broker:
- Room status changes (`ROOM_STATUS`)
- Welcome scene activation (`WELCOME_STATUS`)
- Setback activation (`SET_BACK_ACTIVE`)
- Door ajar alerts (`DOOR_AJAR`)
- Individual device states (switches, lights, etc.)

### Outdoor Indicator Management
- **DND (Do Not Disturb)**: Prevents housekeeping entry
- **MUR (Make Up Room)**: Requests room cleaning service
- **Bell**: Signals guest request for assistance
- Mutual exclusivity: DND disables MUR and vice versa

### Timeouts and Delays
- `TIMEOUT_AFTER_BELL_ACTIVE`: 3 seconds for bell activation
- `DEFAULT_TIME_CROSSING`: 30 minutes for occupancy detection
- `DEFAULT_TIME_DOOR_AJAR`: 6 seconds before door ajar alert
- Door sensor lockout: 500ms to prevent rapid state changes

### State Persistence Mechanism
The system implements a RAM-based state persistence mechanism that captures and maintains system states during runtime:

#### Input Events and State Changes
- Input events from STM32 microcontroller trigger state changes
- Events include: door sensor (`EVENT_DOOR`), motion sensor (`EVENT_MOTION`), button presses for lights and switches
- Each input event modifies persistent states in memory

#### Persistent States Tracked
- **Relay States** (`status_room_t`): All room devices (master switches, lights, minibar, etc.)
- **Sensor States** (`status_sensor_t`): Door and motion sensor status
- **Outdoor States** (`status_outdoor_t`): DND, MUR, and bell indicators
- **PMS States** (`pms_room_status_t`): Room booking and occupancy status

#### State Capture Process
1. Input events trigger `handle_event_*` functions via `check_active_scen`
2. These functions update persistent states and set `flag_syn_status_room = true`
3. The `syn_status_room` task detects the flag and saves current state to RAM
4. Periodic backups occur every minute to ensure state integrity

#### RAM-Based Storage
- States are stored in static RAM variable `g_stored_state`
- Volatile storage (lost on power cycle) as requested
- Fast access and minimal memory footprint
- Efficient state management during runtime

## Project Dependencies and Build System

The project uses ESP-IDF build system with the following components:
- Standard ESP-IDF libraries for WiFi, Ethernet, NVS, etc.
- Custom drivers for GPIO, Modbus, MQTT
- Rule engine for automation processing
- OTA update modules for both ESP32 and STM32
- Web server for configuration interface

## Configuration Options

The system supports runtime configuration via NVS storage:
- Occupancy timeout duration
- Door ajar alert timing
- Default scene settings

## Troubleshooting

Common issues and solutions:
- If room doesn't respond to occupancy: Check door and motion sensor connections
- If MQTT communication fails: Verify network configuration and broker settings
- If devices don't turn on/off: Check Modbus wiring and device addresses
- If timeouts seem incorrect: Verify NVS configuration values