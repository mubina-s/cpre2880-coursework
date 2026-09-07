<h1 align="center">🤖 CPRE 2880 Coursework</h1>

<h3 align="center">Embedded Systems • Sensors • UART • Interrupts • Robotics</h3>

<p align="center">
  <strong>Iowa State University</strong>
</p>

<p align="center">
  A curated collection of selected CPRE 2880 coursework documenting my progression
  through embedded C programming, robot movement, communication interfaces,
  sensor integration, interrupts, and object detection.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/EMBEDDED_C-111111?style=for-the-badge&logo=c&logoColor=FF2E93">
  <img src="https://img.shields.io/badge/ROBOTICS-FF2E93?style=for-the-badge&logoColor=white">
  <img src="https://img.shields.io/badge/EMBEDDED_SYSTEMS-111111?style=for-the-badge&logoColor=FF2E93">
  <img src="https://img.shields.io/badge/IOWA_STATE-FF2E93?style=for-the-badge&logoColor=white">
</p>

---

## 🔎 About This Repository

This repository contains **selected coursework from CPRE 2880 at Iowa State University**.

The course used an embedded robotic platform to build skills progressively across movement control, UART communication, interrupts, sensing, timing, object detection, and servo control.

This repository is intentionally **curated rather than a complete copy of the course workspace**. It includes selected source files that best show my learning progression while excluding build artifacts, IDE metadata, duplicate archives, instructor libraries, and unnecessary course files.

This is **coursework**, not a single standalone project.

---

## 🧭 Coursework Progression

### 🚗 Lab 02 — Robot Movement

Worked with movement-control functions and robot sensor data to practice forward motion, turning, and basic navigation logic.

**Selected concepts:**

- Embedded C functions
- Robot wheel control
- Distance tracking
- Sensor-driven movement
- Modular source/header files

---

### 📡 Lab 03 — Sensor Scanning

Expanded the robot program to work with CyBot scanning and sensor data.

**Selected concepts:**

- Sensor scans
- Servo-based scanning
- Distance measurements
- Structured object data
- UART-based communication

---

### 💬 Lab 05 — UART Communication

Implemented UART configuration and communication using the TM4C123 microcontroller.

**Selected concepts:**

- UART initialization
- GPIO configuration
- Serial communication
- Register-level embedded programming
- Sending and receiving data

---

### ⚡ Lab 06 — UART Interrupts

Built on serial communication by adding interrupt-driven UART behavior.

**Selected concepts:**

- Hardware interrupts
- Interrupt service routines
- Shared volatile state
- UART receive events
- Event-driven embedded programming

---

### 🔍 Lab 07 — Object Detection

Combined movement, scanning, and sensor data to identify objects and reason about their position and size.

**Selected concepts:**

- Object detection
- Sensor fusion concepts
- Scan-angle processing
- Width estimation
- Structured data
- Robot navigation logic

---

### 📏 Lab 08 — Infrared Sensor & ADC

Worked with analog-to-digital conversion and infrared distance sensing.

**Selected concepts:**

- ADC configuration
- Raw sensor readings
- Calibration data
- Distance estimation
- Median filtering
- Embedded sensor processing

---

### 📶 Lab 09 — PING Sensor

Implemented ultrasonic distance measurement using timer capture behavior.

**Selected concepts:**

- Ultrasonic sensing
- Timer capture
- Rising/falling edges
- Pulse-width measurement
- Distance calculation
- Interrupt-based timing

---

### 🎯 Lab 10 — Servo Control

Implemented servo positioning using timer/PWM-style timing calculations.

**Selected concepts:**

- Servo control
- Timer registers
- Pulse-width calculations
- Angle-to-timing conversion
- Embedded actuator control

---

## 🛠️ Technologies & Concepts

`C` • `Embedded Systems` • `TM4C123` • `UART` • `Interrupts` • `ADC` • `PING Sensor` • `Infrared Sensor` • `Servo Control` • `Robot Navigation` • `Sensor Processing`

---

## 📁 Repository Structure

```text
cpre2880-coursework/
│
├── README.md
├── .gitignore
│
├── labs/
│   ├── lab02-movement/
│   ├── lab03-sensor-scanning/
│   ├── lab05-uart/
│   ├── lab06-uart-interrupts/
│   ├── lab07-object-detection/
│   ├── lab08-ir-adc/
│   ├── lab09-ping-sensor/
│   └── lab10-servo/
│
└── docs/
    └── ATTRIBUTION.md
```

---

## 🏺 Final Project

My CPRE 2880 final project is documented separately as a standalone portfolio project:

### [CyBot Archaeology Survey Rover](https://github.com/mubina-s/cybot-archaeology-survey-rover)

That repository highlights the larger team project, including the GUI, system design, testing, documentation, and integrated rover behavior.

Keeping the final project separate allows this repository to focus on the **coursework progression that led up to it**.

---

## 💡 What I Learned

CPRE 2880 helped me connect software with physical hardware.

Through the labs, I gained experience with:

- Writing embedded C
- Reading microcontroller registers
- Working with sensors and actuators
- Debugging hardware/software interactions
- Building reusable movement and communication functions
- Using interrupts instead of relying only on polling
- Calibrating sensor data
- Processing real-world measurements
- Combining multiple subsystems into larger robotic behaviors

The course gave me a stronger understanding of how software interacts with hardware in real time.

---

## 🎓 Academic Context

**University:** Iowa State University  
**Course:** CPRE 2880  
**Repository Type:** Curated Coursework Portfolio

The source files are preserved as examples of my learning at the time of the course and are not presented as production-ready embedded software.

---

## 🔐 Academic Integrity

This public repository intentionally excludes:

- Full course workspaces
- Instructor-provided libraries when not needed for portfolio review
- Assignment documents and answer sheets
- Build artifacts
- Compiled binaries
- IDE metadata
- Target/debug files
- Duplicate project copies
- ZIP archives
- Course infrastructure details
- Materials that could unnecessarily reproduce the course

Some source files were built from instructor-provided starter templates. Original template comments and attribution are retained where present.

See [`docs/ATTRIBUTION.md`](docs/ATTRIBUTION.md).

---

## 🔗 Connect With Me

<p align="center">

<a href="https://github.com/mubina-s">
  <img src="https://img.shields.io/badge/GITHUB-111111?style=for-the-badge&logo=github&logoColor=FF2E93">
</a>

<a href="https://www.linkedin.com/in/mubina-sadriddinova-bb889a363/">
  <img src="https://img.shields.io/badge/LINKEDIN-FF2E93?style=for-the-badge&logo=linkedin&logoColor=white">
</a>

<a href="mailto:mubish@iastate.edu">
  <img src="https://img.shields.io/badge/EMAIL-111111?style=for-the-badge&logo=gmail&logoColor=FF2E93">
</a>

</p>

---

<p align="center">
  <i>From register-level programming to integrated robotic systems.</i>
</p>
