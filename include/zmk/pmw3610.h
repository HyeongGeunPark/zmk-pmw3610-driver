/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/device.h>

int pmw3610_runtime_command(const struct device *dev, uint32_t command, bool pressed, bool shifted);
