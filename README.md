ESP TIME NTP 
====================

# MQTT Topic structure

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
