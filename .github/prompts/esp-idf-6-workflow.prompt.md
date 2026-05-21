---
name: ESP-IDF 6 Workflow
description: Run structured ESP-IDF 6.0.x firmware workflows in PlantoSmart (feature, bugfix, bring-up) using project conventions and esp32c3 target defaults.
argument-hint: workflow=<feature|bugfix|bring-up>; module=<main/wifi.c|main/display.c|...>; goal=<expected behavior>; constraints=<optional>
agent: ESP-IDF 6 Firmware Engineer
---
Use this prompt to execute one focused firmware workflow in this repository.

Input format:
- workflow: feature | bugfix | bring-up
- module: primary file or subsystem to change
- goal: expected behavior and acceptance criteria
- constraints: optional limits (memory, task priority, timing, API compatibility)

If input is partial, infer missing values from repository context and proceed with the smallest safe change set.

Workflow playbooks:

1) Feature
- Map affected module boundaries (public header vs internal source).
- Implement minimal API or behavior changes in include/ and main/.
- Update CMake component dependencies only if required.
- Build for esp32c3 and report behavioral impact.

2) Bugfix
- Reproduce or isolate failure path from current code.
- Identify root cause and patch only the responsible module(s).
- Preserve current conventions for logging, error propagation, and FreeRTOS behavior.
- Build and validate with a short risk summary.

3) Bring-up
- Verify target assumptions (esp32c3, sdkconfig, peripherals used by module).
- Validate initialization order, event loop, and task startup paths.
- Add narrowly scoped diagnostics where needed and remove noisy debug output.
- Build and provide a clear pass/fail checklist for on-device verification.

Output contract:
- Summary: what changed and why.
- Files touched: grouped by module responsibility.
- Validation: build status and any runtime checks completed.
- Risks: unresolved issues, fallback path, and next best action.
