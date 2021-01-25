# Purpose

FreeRTOS and Finite State Machine (FSM) architecture used for real-time diagnostics and control for multiple devices. First application focuses on 

The project integrates:
- Battery BMS
- Motor controller
- (**FUTURE**) Qt-based dashboard (RestApi -> MQTT)
- (**FUTURE**) Battery Charger
- (**FUTURE**) Analog/Digital Inputs from onboard sensors

# Base

The code base inherits from TI Hercules porting found [here](http://loszi.hu/works/ti_launchpad_freertos_demo/).

Where http stack, basic software structure and platform porting were provided. 

# Features 

Extended:
- CLl commands
- FreeRTOS task structure

Added:
- FSM as a task
- RestApi

To do:
- Main FSM that controls the device on application layer (on/off/charging state, processing dashboard responses and additional signals)
- Add digital inputs 
- Charger Integration
