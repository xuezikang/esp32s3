# ESP32-S3 Health Monitor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build an ESP-IDF project that recreates the ESP32-S3 portable health monitor resume project for GitHub.

**Architecture:** The project is split into focused ESP-IDF components for health data, MAX30102 access, NV3030B display setup, LVGL UI, and MQTT upload. The default runtime uses simulated samples so the full task and data path is demonstrable without hardware.

**Tech Stack:** ESP32-S3, ESP-IDF, CMake, FreeRTOS, LVGL, MQTT, I2C, SPI, MAX30102, NV3030B.

---

### Task 1: Add Repository Contract Tests

**Files:**
- Create: `tests/test_project_contract.py`

- [x] **Step 1: Write the failing tests**

The tests assert that the ESP-IDF project files, module boundaries, Kconfig options, and README sections exist.

- [x] **Step 2: Run tests and verify they fail**

Run: `python -m pytest tests -q`
Expected: FAIL because the ESP-IDF source files do not exist yet.

### Task 2: Create ESP-IDF Project Skeleton

**Files:**
- Create: `CMakeLists.txt`
- Create: `main/CMakeLists.txt`
- Create: `main/Kconfig.projbuild`
- Create: `main/app_config.h`
- Create: `main/app_main.c`
- Create component source and header files under `components/`

- [ ] **Step 1: Implement minimal source files**

Create focused modules for health samples, sensor boundary, display boundary, UI boundary, and MQTT boundary.

- [ ] **Step 2: Run contract tests**

Run: `python -m pytest tests -q`
Expected: PASS.

### Task 3: Add Open-Source Project Documentation

**Files:**
- Create: `README.md`
- Create: `.gitignore`
- Create: `LICENSE`

- [ ] **Step 1: Document overview, hardware, build, configuration, and publish notes**

The README must describe the resume-aligned functionality and how to open the project in Espressif's VS Code ESP-IDF extension.

- [ ] **Step 2: Run contract tests**

Run: `python -m pytest tests -q`
Expected: PASS.

### Task 4: Prepare GitHub Release State

**Files:**
- Modify: repository git metadata

- [ ] **Step 1: Initialize git if needed**

Run: `git init`

- [ ] **Step 2: Verify status**

Run: `git status --short`
Expected: project files appear as untracked or staged changes.

- [ ] **Step 3: Commit project**

Run: `git add . && git commit -m "feat: add esp32-s3 health monitor demo"`
Expected: commit succeeds when Git user identity is configured.

## Self-Review

- The plan covers the spec's project skeleton, simulated default data, hardware boundaries, MQTT upload, LVGL UI, README, and verification path.
- No implementation placeholders are left in the expected source plan.
- File names and component names are consistent with the design.
