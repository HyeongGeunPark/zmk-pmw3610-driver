PMW3610 driver implementation for ZMK with at least Zephyr 3.5

This work is based on [ufan's implementation](https://github.com/ufan/zmk/tree/support-trackpad) of the driver.

## Installation

Only GitHub actions builds are covered here. Local builds are different for each user, therefore it's not possible to cover all cases.

Include this project on your ZMK's west manifest in `config/west.yml`:

```yml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/petejohanson
    - name: inorichi
      url-base: https://github.com/inorichi
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: feat/pointers-move-scroll
      import: app/west.yml
    - name: zmk-pmw3610-driver
      remote: inorichi
      revision: main
  self:
    path: config
```

Then, edit your `build.yml` to look like this, 3.5 is now on main:

```yml
on: [workflow_dispatch]

jobs:
  build:
    uses: zmkfirmware/zmk/.github/workflows/build-user-config.yml@main
```

Now, update your `board.overlay` adding the necessary bits (update the pins for your board accordingly):

```dts
&pinctrl {
    spi0_default: spi0_default {
        group1 {
            psels = <NRF_PSEL(SPIM_SCK, 0, 8)>,
                <NRF_PSEL(SPIM_MOSI, 0, 17)>,
                <NRF_PSEL(SPIM_MISO, 0, 17)>;
        };
    };

    spi0_sleep: spi0_sleep {
        group1 {
            psels = <NRF_PSEL(SPIM_SCK, 0, 8)>,
                <NRF_PSEL(SPIM_MOSI, 0, 17)>,
                <NRF_PSEL(SPIM_MISO, 0, 17)>;
            low-power-enable;
        };
    };
};

&spi0 {
    status = "okay";
    compatible = "nordic,nrf-spim";
    pinctrl-0 = <&spi0_default>;
    pinctrl-1 = <&spi0_sleep>;
    pinctrl-names = "default", "sleep";
    cs-gpios = <&gpio0 20 GPIO_ACTIVE_LOW>;

    trackball: trackball@0 {
        status = "okay";
        compatible = "pixart,pmw3610";
        reg = <0>;
        spi-max-frequency = <2000000>;
        irq-gpios = <&gpio0 6 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;

        /*   optional features   */
        // snipe-layers = <1>;
        // scroll-layers = <2 3>;
        // automouse-layer = <4>;
    };
};

/ {
  trackball_listener {
    compatible = "zmk,input-listener";
    device = <&trackball>;

  };
};
```

Now enable the driver config in your `board.config` file (read the Kconfig file to find out all possible options):

```conf
CONFIG_SPI=y
CONFIG_INPUT=y
CONFIG_ZMK_MOUSE=y
CONFIG_PMW3610=y
```

## Runtime pointer controls

Enable the behavior on every split half whose keymap references it:

```conf
CONFIG_PMW3610_RUNTIME_CONTROLS=y
```

Add the behavior node and command header to the keymap:

```dts
#include <dt-bindings/zmk/pmw3610.h>

/ {
    behaviors {
        pmw: pmw3610_runtime {
            compatible = "zmk,behavior-pmw3610";
            #binding-cells = <1>;
            display-name = "PMW3610 Runtime";
        };
    };
};
```

The central behavior supports normal CPI increase/decrease, sniping CPI
increase/decrease, sniping toggle/suppression, and momentary drag-scroll. Normal
CPI cycles from 400 through 3200 in 200 CPI steps. Sniping CPI cycles through
200, 400, 600, and 800. Holding either Shift key reverses CPI step direction.
The initial values can be selected with the sensor node's
`runtime-default-cpi` and `runtime-default-snipe-cpi` properties. Drag-scroll
uses 200 CPI and `CONFIG_PMW3610_RUNTIME_SCROLL_TICK` as its wheel threshold.

```dts
&pmw PMW_CPI_INC
&pmw PMW_CPI_DEC
&pmw PMW_SNIPE_CPI_INC
&pmw PMW_SNIPE_CPI_DEC
&pmw PMW_SNIPE_TOGGLE
&pmw PMW_SNIPE_SUPPRESS
&pmw PMW_DRAG_SCROLL
```

When Zephyr settings are enabled, the normal and sniping CPI selections are
saved after `CONFIG_PMW3610_SETTINGS_SAVE_DEBOUNCE_MS`. Mode state itself is not
persisted.
