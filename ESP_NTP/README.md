Beem Hub 
====================

# MQTT Topic structure

# Parameters
## SDKConfig
### MQTT Credentials
 Please set the parameters of `MQTT client`
### BeeM HUB Configuration
 `Serial Number`: this is the place to enter the serial number
	
__Note__: `Save Serial number to NVS` Check this option if the device is Funky Fresh or needs a Refresh of serial number 

# MUST DO

For correct uploading and use of the OTA change the `Partition Table` in **sdkconfig** to `Factory app, two OTA definitions` and
update partition table **.csv** file in `C:\Espressif\frameworks\esp-idf-v4.4.3\components\partition_table\partitions_two_ota.csv` as follow:

for 4MB

Name     |   | Type |   | SubType | Offset |   | Size   | Flags
---------|---|------|---|---------|--------|---|--------|------
nvs      |   | data |   | nvs     |        |   | 0x4000 |
otadata  |   | data |   | ota     |        |   | 0x2000 |
phy_init |   | data |   | phy     |        |   | 0x1000 |
ota_0    |   | app  |   | ota_0   |        |   | 1984K  |
ota_1    |   | app  |   | ota_1   |        |   | 1984K  |


for 16MB
Name     |   | Type |   | SubType | Offset |   | Size   | Flags
---------|---|------|---|---------|--------|---|--------|------
nvs      |   | data |   | nvs     |        |   | 0x4000 |
otadata  |   | data |   | ota     |        |   | 0x2000 |
phy_init |   | data |   | phy     |        |   | 0x1000 |
ota_0    |   | app  |   | ota_0   |        |   | 4M	|
ota_1    |   | app  |   | ota_1   |        |   | 4M	|


>Note: if you have increased the bootloader size	 make sure to update the offsets to avoid overlap   

# TOPICs publish
| topic                       | message(payload) | format |
|-----------------------------|------------------|--------|
| beem/H23001/Status          | 0-1              | bool   |
| beem/H23001/batt            | 0-100            | int    |
| beem/H23001/battAlm         |                  | int    |
| beem/H23001/RH              | 0-100            | float  |
| beem/H23001/temp            | -20 : 60         | float  |
| beem/H23001/RSSI            |                  | float  |
| beem/H23001/SSID            |                  | string |
| beem/H23001/LastResetReason |                  | string |
| beem/H23001/uptimeSec       |                  | int    |
|                             |                  |        |