Kuba's dirty 

TODO:
- Add backward support for CC1101
- Add support for SX1262 (with limited frame length)
- Add triggers:
  - Radio->on packet (allow to blink on frame/telegram)
  - Meter->on telegram (allow e.g. to send whole telegram to MQTT)
- Reimplement TCP and UCP senders. Should be classes with common interface to use as action under Radio->on packet trigger
- Reimplement HEX and RTLWMBUS formatter to use as parameter of TCP/UDP action
- ...
- Prepare packages for ready made boards (like UltimateReader) with displays, leds etc.

DONE:
- Reuse CRCs and frame parsers from wmbusmeters
- Refactor 3out6 decoder
- Migrate to esp-idf and drop Arduino!
- Add support for SX176
- Run receiver in separate task
- Drop all non wmbus related components from rf code part
- Allow to specify ASCII decription key
- Divide codebase to separate components (radio for radio communication, meter for meters (on which sensor may subscribe) and common for wmbusmeters code)

Usage example:

```yaml
esphome:
  name: wmbus
  friendly_name: WMBus
  platformio_options:
    upload_speed: 921600

esp32:
  board: heltec_wifi_lora_32_V2
  flash_size: 8MB
  framework:
    type: esp-idf
  
logger:
  id: component_logger
  level: DEBUG
  baud_rate: 115200

wifi:
  networks:
    - ssid: !secret wifi_ssid
      password: !secret wifi_password

api:

web_server:
  version: 3 

time:
  - platform: homeassistant

spi:
  clk_pin:
    number: GPIO5
    ignore_strapping_warning: true
  mosi_pin: GPIO27
  miso_pin: GPIO19

wmbus_radio:
  cs_pin: GPIO18
  reset_pin: GPIO14
  irq_pin: GPIO35

wmbus_meter:
  - id: electricity_meter
    meter_id: 0x0101010101
    type: amiplus
    key: SomeKey
  - id: heat_meter
    meter_id: 0x101010101
    type: hydrocalm3

output:
  - platform: gpio
    id: vext_output
    pin: GPIO21
  - platform: gpio
    id: oled_reset
    pin: GPIO16
    inverted: True

sensor:
  - platform: wmbus_meter
    parent_id: heat_meter
    field: total_heating_kwh
    device_class: energy
    name: Zużycie energii cieplnej
    accuracy_decimals: 4
    state_class: total_increasing

  - platform: wmbus_meter
    parent_id: electricity_meter
    field: current_power_consumption_kw
    name: Moc aktualna
    accuracy_decimals: 0
    device_class: power
    unit_of_measurement: W
    state_class: measurement
    filters:
      - multiply: 1000

  - platform: wmbus_meter
    parent_id: electricity_meter
    field: total_energy_consumption_kwh
    name: Zużycie energii
    accuracy_decimals: 3
    device_class: energy
    state_class: total_increasing

  - platform: wmbus_meter
    parent_id: electricity_meter
    field: rssi
    name: Electricity Meter RSSI
```

For SX1276 radio you need to configure SPI instance as usual in ESPHome and additionally specify reset pin and IRQ pin (as DIO1). Interrupts are triggered on non empty FIFO. 


===============================================================

# Szczepan's esphome custom components

This repository contains a collection of my custom components
for [ESPHome](https://esphome.io/).

[!["Buy Me A Coffee"](https://github.com/SzczepanLeon/esphome-components/blob/main/docs/buy_me_coffe.png)](https://www.buymeacoffee.com/szczepan)
\
[!["Kup mi kawę"](https://github.com/SzczepanLeon/esphome-components/blob/main/docs/postaw_kawe.png)](https://buycoffee.to/szczepanleon)


## 1. Usage

Use latest [ESPHome](https://esphome.io/)
with external components and add this to your `.yaml` definition:

```yaml
external_components:
  - source: github://SzczepanLeon/esphome-components@main
```

## 2. Components

### 2.1. `wmbus`

Component to receive wMBus frame (via CC1101), create HA sensor and send decoded value.
You can also use this component with wmbusmeters HA addon:
https://github.com/SzczepanLeon/esphome-components/blob/main/docs/wmbus.md

[!["CC1101 to D1 mini PCB"](https://github.com/SzczepanLeon/esphome-components/blob/main/docs/pcb_v2.png)](https://www.pcbway.com/project/shareproject/CC1101_to_ESP_D1_mini_277f34e1.html)

> **_NOTE:_**  Configuration for version 3.x is described [here](https://github.com/SzczepanLeon/esphome-components/blob/main/docs/version_3.md)

#### 2.1.1. Fast start

- don't know meter type (driver) and don't know ID
```yaml
wmbus:
  all_drivers: True
  log_all: True
```
Then in logs you will have trace with driver name and ID:
```
[17:13:07][I][wmbus:085]: apator162 [0x00148686] RSSI: -44dBm T: 4e4401068686140005077a350040852f2f0f005B599600000010aa55000041545a42850Bd800437d037301c5500000564B00009e46 (79) T1 A
```


- meter type (driver) and ID are known, but you don't know how to add sensors (map decoded data to sensor)

```yaml
wmbus:
  all_drivers: False
  log_all: False

sensor:
  - platform: wmbus
    meter_id: 0x00148686
    type: apator162
    key: "00000000000000000000000000000000"
    sensors:
      - name: "my hot water RSSi"
        field: "rssi"
        accuracy_decimals: 0
        unit_of_measurement: "dBm"
        device_class: "signal_strength"
        state_class: "measurement"
        entity_category: "diagnostic"
```

Then in logs you will have trace with telegram:
```
[17:13:07][I][wmbus:085]: apator162 [0x00148686] RSSI: -44dBm T: 4e4401068686140005077a350040852f2f0f005B599600000010aa55000041545a42850Bd800437d037301c5500000564B00009e46 (79) T1 A
```
You can decode that telegram on [wmbusmeters](https://wmbusmeters.org/analyze/4E4401068686140005077A350040852F2F0F005B599600000010AA55000041545A42850BD800437D037301C5500000564B00009E4600006A410000A01778EC03FFFFFFFFFFFFFFFFFFFFFFFFFFE393) and create sensors.


- everything is known, let's find field and unit
From decoded JSON:
![decoded JSON](https://github.com/SzczepanLeon/esphome-components/blob/main/docs/decoded_telegram.png)

find interesting data (in that case total_m3), split it into field (total) and unit (m3) and create sensor in YAML. In YAML config please use units from HA (ie. "m³" not "m3", etc).

```yaml
wmbus:
  all_drivers: False
  log_all: False

sensor:
  - platform: wmbus
    meter_id: 0x00148686
    type: apator162
    key: "00000000000000000000000000000000"
    sensors:
      - name: "my hot water RSSi"
        field: "rssi"
        accuracy_decimals: 0
        unit_of_measurement: "dBm"
        device_class: "signal_strength"
        state_class: "measurement"
        entity_category: "diagnostic"
      - name: "my hot water"
        field: "total"
        accuracy_decimals: 3
        unit_of_measurement: "m³"
        device_class: "water"
        state_class: "total_increasing"
        icon: "mdi:water"
```

#### 2.1.2. Example

```yaml
time:
  - platform: sntp
    id: time_sntp

external_components:
  - source: github://SzczepanLeon/esphome-components@main
    refresh: 0d
    components: [ wmbus ]

wmbus:
  mosi_pin: GPIO13
  miso_pin: GPIO5
  clk_pin:  GPIO2
  cs_pin:   GPIO14
  gdo0_pin: GPIO15
  gdo2_pin: GPIO16

  led_pin: GPIO0
  led_blink_time: "1s"

  frequency: 868.950
  all_drivers: False
  sync_mode: True
  log_all: True

  mqtt:
    broker: 10.0.0.88
    username: mqttUser
    password: mqttPass

  clients:
    - name: "wmbusmeters"
      ip_address: "10.0.0.22"
      port: 7227

sensor:
# add driver to compile list (will be available for autodetect), don't create sensor
  - platform: wmbus
    type: itron

# add sensor with defined type (driver will be also added to compile list)
  - platform: wmbus
    meter_id: 0x12345678
    type: apator162
    key: "00000000000000000000000000000000"
    sensors:
      - name: "my hot water RSSi"
        field: "rssi"
        accuracy_decimals: 0
        unit_of_measurement: "dBm"
        device_class: "signal_strength"
        state_class: "measurement"
        entity_category: "diagnostic"
      - name: "my hot water"
        field: "total"
        accuracy_decimals: 3
        unit_of_measurement: "m³"
        device_class: "water"
        state_class: "total_increasing"
        icon: "mdi:water"

# add more sensors, one without field (name will be used)
  - platform: wmbus
    meter_id: 0xABCD4321
    type: amiplus
    sensors:
      - name: "my current power consumption in Watts"
        field: "current_power_consumption"
        accuracy_decimals: 1
        unit_of_measurement: "w"
        device_class: "power"
        state_class: "measurement"
        icon: "mdi:transmission-tower-import"
      - name: "total energy on T1"
        field: "total_energy_consumption_tariff_1"
        accuracy_decimals: 3
        unit_of_measurement: "kwh"
        device_class: "energy"
        state_class: "total_increasing"
        icon: "mdi:transmission-tower-import"
      - name: "voltage_at_phase_1"
        accuracy_decimals: 0
        unit_of_measurement: "v"
        device_class: "voltage"
        state_class: "measurement"
        icon: "mdi:sine-wave"

# sensor with offset, type should be autodetected
   - platform: wmbus
    meter_id: 0x11223344
    sensors:
      - name: "cold water from Apator NA-1"
        field: "total"
        accuracy_decimals: 3
        unit_of_measurement: "m³"
        device_class: "water"
        state_class: "total_increasing"
        icon: "mdi:water"
        filters:
          - offset: 123.0

# if mqtt defined, JSON with all decoded fields will be published to broker
  - platform: wmbus
    meter_id: 0x22113366
    type: vario411

text_sensor:
  - platform: wmbus
    meter_id: 0xABCD1122
    type: izar
    sensors:
      - name: "Izar current_alarms"
        field: "current_alarms"
```


Configuration variables:
------------------------

In wmbus platform:

- **mosi_pin** (*Optional*): CC1101 MOSI pin connection. Defaults to ``GPIO13``.
- **miso_pin** (*Optional*): CC1101 MISO pin connection. Defaults to ``GPIO12``.
- **clk_pin** (*Optional*): CC1101 CLK pin connection. Defaults to ``GPIO14``.
- **cs_pin** (*Optional*): CC1101 CS pin connection. Defaults to ``GPIO2``.
- **gdo0_pin** (*Optional*): CC1101 GDO0 pin connection. Defaults to ``GPIO5``.
- **gdo2_pin** (*Optional*): CC1101 GDO2 pin connection. Defaults to ``GPIO4``.
- **led_pin** (*Optional*): Pin where LED is connected. It will blink on each telegram. You can use all options from [Pin Schema](https://esphome.io/guides/configuration-types.html#config-pin-schema).
- **led_blink_time** (*Optional*): How long LED will stay ON. Defaults to ``300 ms``.
- **frequency** (*Optional*): Rx frequency in MHz. Defaults to ``868.950 MHz``.
- **sync_mode** (*Optional*): Receive telegram in one loop. Defaults to ``False``.
- **log_all** (*Optional*): Show all received telegrams in log. Defaults to ``False``.
- **all_drivers** (*Optional*): Compile with all drivers. Defaults to ``False``.
- **clients** (*Optional*):
  - **name** (**Required**): The name for this client.
  - **ip_address** (**Required**): IP address.
  - **port** (**Required**): Port number.
  - **format** (*Optional*): Telegram format to send. HEX or RTLWMBUS. Defaults to ``RTLWMBUS``.
  - **transport** (*Optional*): TCP or UDP. Defaults to ``TCP``.
- **mqtt** (*Optional*):
  - **broker** (**Required**): Broker IP address.
  - **username** (**Required**): User name.
  - **password** (**Required**): password.
  - **port** (*Optional*): Port number. Defaults to ``1883``.
  - **retain** (*Optional*): If the published message should have a retain flag on or not. Defaults to ``False``.

> **_NOTE:_**  MQTT can be defined in wmbus component or in [ESPHome level](https://esphome.io/components/mqtt.html).


sensor:

- **meter_id** (*Optional*, int): Meter ID. Can be specified as decimal or hex.
- **type** (*Optional*, string):  Meter type. When not defined, driver will be detected from telegram.
- **key** (*Optional*): Key for meter, used in payload decoding process. Defaults to ``""``.
- **sensors** (*Optional*):
  - **id** (*Optional*, string): Manually specify the ID for code generation. At least one of **id** and **name** must be specified.
  - **name** (*Optional*, string): The name for the sensor. At least one of **id** and **name** must be specified.
  - **field** (*Optional*): Field from decoded telegram (without unit). If **field** is not present then **name** is used.
  - **unit_of_measurement** (**Required**): Unit for field defined above.
  - All other options from [Sensor](https://esphome.io/components/sensor/).


text_sensor:

- **meter_id** (*Optional*, int): Meter ID. Can be specified as decimal or hex.
- **type** (*Optional*, string):  Meter type. When not defined, driver will be detected from telegram.
- **key** (*Optional*): Key for meter, used in payload decoding process. Defaults to ``""``.
- **sensors** (*Optional*):
  - **id** (*Optional*, string): Manually specify the ID for code generation. At least one of **id** and **name** must be specified.
  - **name** (*Optional*, string): The name for the sensor. At least one of **id** and **name** must be specified.
  - **field** (*Optional*): Text field from decoded telegram. If **field** is not present then **name** is used.
  - All other options from [Text Sensor](https://esphome.io/components/text_sensor/).

------------------------

Supported drivers: almost all from wmbusmeters version 1.17.1 (without drivers from [file](https://github.com/wmbusmeters/wmbusmeters/blob/1.17.1/src/generated_database.cc))


## 3. Author & License

Szczepan, GPL, 2022-2024
