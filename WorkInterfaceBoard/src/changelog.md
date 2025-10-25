# Changelog


## 08-10-2025
### Added
### Fixed
		-1. Can Receive Error fixed
		-2. Can Digipot tested
		-3. Load Cell Value changing by changing Digipot load cell value.


## 06-10-2025
### Added
### Fixed
	- CAN Bus working at 500 K 
	Following Keys are available to read periodically:	
	Sl.no	CAN_ID TYPE	           Frame ID	     Data Len	Period
	1		CAN_ID_LOADCELL			0x120u	        2
	2		CAN_ID_ACCELEROMETER	0x121u			3		
	3		CAN_ID_TEMPERATURE		0x122u			2	
	4       CAN_ID_TOOLTYPE         0x123u          1
	5       CAN_ID_STATUS           0x200u          
	6       CAN_ID_POT_COMMAND      0x220u


### Changed

## 09-16-2025

### Added	
	- All AD5252 related Methods Included and tested with Load Cell Data in Load cell task .
	- Files Impacted:
	-- tasks.c

## 09-15-2025

### Fixed
	- I2C addresses are now properly fetched bbased on the respective device APIs
	
### Added	
	- All AD5252 related Methods written , debugged and tested .
### Changed
	- Delay between two AD5252 operation set to 10 mSec
	
## 09-10-2025

### Changed
	- Removed Registers regarding Temperature from SPI- ADS
	- Added registers and Methods to read temperature from LIS2 [I2C]
	- BDU disabled for Accelerometer
	- BDU Enabled for Temperature


## 09-09-2025

### Added
- I2C debug logging and bus scanning for LIS2 accelerometer
- Debug variables in TIB_Init and lis2_i2c drivers for troubleshooting
- I2C readiness check and timeout mechanisms
- I2C bus recovery and reinitialization for LIS2 communication
- Comprehensive I2C debug guide documentation

### Changed
- Refactored I2C and CAN initialization to occur in main function
- Adjusted FreeRTOS task stack sizes for better memory management
- Updated compiler path and build configuration
- Improved SPI0 configuration for ADS1120 with clearer pin and mode settings

### Fixed
- Compilation errors in CAN application
- I2C communication hangs and timeout issues
- Undefined reference to vApplicationMallocFailedHook
- I2C data reception issues with LIS2 accelerometer



### Added
- Initial project structure for Torque Interface Board
- SAM4E8C microcontroller support with Atmel Software Framework (ASF) 3.52.0
- FreeRTOS 7.3.0 integration for real-time task management
- CAN bus communication support
- I2C communication for LIS2 accelerometer
- SPI communication for ADS1120 ADC
- Hardware initialization framework (TIB_Init)
- Tool sensing capabilities
- AD5252 digital potentiometer support

### Technical Details
- **Target MCU**: ATSAM4E8C (ARM Cortex-M4F)
- **Development Environment**: Atmel Studio 7.0
- **Toolchain**: ARM GCC
- **RTOS**: FreeRTOS 7.3.0
- **Framework**: Atmel Software Framework (ASF) 3.52.0
- **Debug Interface**: SWD (Atmel-ICE)

### Hardware Features
- **I2C Pins**: PA3 (SDA), PA4 (SCL)
- **LIS2 Accelerometer Support**: 
  - Addresses: 0x18 (SA0=0) or 0x19 (SA0=1)
  - WHO_AM_I values: 0x33 (LIS2DH12/LIS2DE12), 0x44 (LIS2DW12), 0x43 (LIS2DS12)
- **SPI Configuration**: SPI0 for ADS1120 ADC communication
- **CAN Bus**: Full CAN controller support
- **Tool Types**: Low torque and high torque tool detection

### Project Structure
```
src/TorqueInterfaceBoard/
├── src/                    # Application source code
│   ├── main.c             # Main application entry point
│   ├── TIB_Init.c/h       # Hardware initialization
│   ├── can_app.c/h        # CAN bus application
│   ├── lis2_i2c.c/h       # LIS2 accelerometer I2C driver
│   ├── spi0.c/h           # SPI0 driver
│   ├── i2c0.c/h           # I2C0 driver
│   ├── tasks.c/h          # FreeRTOS task definitions
│   ├── toolsense.c/h      # Tool sensing functionality
│   └── ad5252.c/h         # Digital potentiometer driver
├── ASF/                   # Atmel Software Framework
├── config/                # Configuration files
└── Device_Startup/        # MCU startup code
```

### Debug Features
- Comprehensive I2C debug variables for troubleshooting
- Bus scanning capabilities
- Clock configuration monitoring
- TWI status register tracking
- WHO_AM_I register validation
- Communication timeout handling

---

## Development Notes

### Recent Development Focus
The project has been actively developed with a focus on:
1. **I2C Communication Reliability**: Extensive debugging and recovery mechanisms
2. **Real-time Performance**: FreeRTOS task optimization
3. **Hardware Integration**: Multiple sensor and communication interfaces
4. **Debug Capabilities**: Comprehensive logging and diagnostic tools

### Known Issues
- I2C communication may require specific timing and pull-up resistor values
- LIS2 accelerometer detection depends on correct I2C address configuration
- CAN bus initialization must complete successfully for proper operation

### Future Enhancements
- Enhanced error handling and recovery mechanisms
- Additional sensor support
- Improved real-time performance optimization
- Extended debugging and monitoring capabilities

---

*This changelog is automatically maintained based on git commit history and project development progress.*