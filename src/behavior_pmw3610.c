/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_pmw3610

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <dt-bindings/zmk/modifiers.h>
#include <dt-bindings/zmk/pmw3610.h>
#include <zmk/behavior.h>
#include <zmk/hid.h>

#include "pmw3610.h"

LOG_MODULE_REGISTER(pmw3610_behavior, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
static const struct behavior_parameter_value_metadata command_values[] = {
    {.display_name = "Increase CPI", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = PMW_CPI_INC},
    {.display_name = "Decrease CPI", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = PMW_CPI_DEC},
    {.display_name = "Increase Sniping CPI", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = PMW_SNIPE_CPI_INC},
    {.display_name = "Decrease Sniping CPI", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = PMW_SNIPE_CPI_DEC},
    {.display_name = "Toggle Sniping", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = PMW_SNIPE_TOGGLE},
    {.display_name = "Suppress Sniping", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = PMW_SNIPE_SUPPRESS},
    {.display_name = "Drag Scroll", .type = BEHAVIOR_PARAMETER_VALUE_TYPE_VALUE,
     .value = PMW_DRAG_SCROLL},
};

static const struct behavior_parameter_metadata_set command_metadata_set[] = {{
    .param1_values = command_values,
    .param1_values_len = ARRAY_SIZE(command_values),
}};

static const struct behavior_parameter_metadata command_metadata = {
    .sets = command_metadata_set,
    .sets_len = ARRAY_SIZE(command_metadata_set),
};
#endif

static int invoke_runtime_command(struct zmk_behavior_binding *binding, bool pressed) {
#if DT_HAS_COMPAT_STATUS_OKAY(pixart_pmw3610)
    const struct device *sensor = DEVICE_DT_GET(DT_COMPAT_GET_ANY_STATUS_OKAY(pixart_pmw3610));
    const bool shifted =
        (zmk_hid_get_explicit_mods() & (MOD_LSFT | MOD_RSFT)) != 0;

    return pmw3610_runtime_command(sensor, binding->param1, pressed, shifted);
#else
    LOG_ERR("No enabled PMW3610 sensor is available");
    return -ENODEV;
#endif
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    return invoke_runtime_command(binding, true);
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    switch (binding->param1) {
    case PMW_SNIPE_SUPPRESS:
    case PMW_DRAG_SCROLL:
        return invoke_runtime_command(binding, false);
    default:
        return ZMK_BEHAVIOR_OPAQUE;
    }
}

static const struct behavior_driver_api behavior_pmw3610_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &command_metadata,
#endif
};

#define PMW3610_BEHAVIOR_DEFINE(n)                                                               \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_pmw3610_driver_api);

DT_INST_FOREACH_STATUS_OKAY(PMW3610_BEHAVIOR_DEFINE)

#endif
