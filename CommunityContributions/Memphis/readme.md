# A&D FX Shield by Memphis

![Overview_1.jpg](resources/Overview_1.jpg)
![Overview_2.jpg](resources/Overview_2.jpg)
![Overview_3.jpg](resources/Overview_3.jpg)
![Overview_4.jpg](resources/Overview_4.jpg)



## BOM

- Heatset Inserts M3x5x4: 24
- Nut M3: 4
- M3x6 BHCS: 4
- M3x10 BHCS: 16
- M3x10 SHCS: 4 
- M3x12 SHCS: 4

- A shot glass, height: 50mm, diameter at top: 41.6mm
- An o-ring 35mm in diameter

- A WS2812B led with cable and connector



## Print and Assembly Guide

Install the Neopixel LED (with both in and out cables) to the pocket of [front_discharger_mount_with_led.stl](STL/front_discharger_mount_with_led.stl). Make sure both cables are tucked into the cable channel and stay in flush or below than the top surface. 

![install_neopixel_at_discharge_port.png](resources/install_neopixel_at_discharge_port.png)

Continue with front and rear body assembly. Once the OpenTrickler is mounted to the [trickler_adapter_plate.stl](STL/trickler_adapter_plate.stl), you need to run the cable with the Neopixel LED, from the front discharger mount, through the cut out behind the [front_body_with_shutter_with_led.stl](STL/front_body_with_shutter_with_led.stl), following the red line as shown below: The other cable with JST connector (blue line) can be left floating at this stage. 

![cable_layout_bottom.png](resources/cable_layout_bottom.png)

Install the Neopixel LED (with only one set of cable) to the pocket of [FrontVolumeReductionInsert_Bottom_with_LED.stl](STL/FrontVolumeReductionInsert_Bottom_with_LED.stl), and run the cable through the cable channel on [FrontVolumeReductionInsert_Top_with_LED.stl.stl](STL/FrontVolumeReductionInsert_Top_with_LED.stl.stl) and [front_body_with_shutter_with_led.stl](STL/front_body_with_shutter_with_led.stl)

![cable_layout_top.png](resources/cable_layout_top.png)

Side the front body cover and the OpenTrickler assembly is complete. The last step is to ensure both cables are sitting nicely in the cut out on [scale_shield_with_led.stl](STL/scale_shield_with_led.stl). Pluging the cable to the PWNOUT port on the controller board and your assembly is now complete. 

![cable_layout_outside.png](resources/cable_layout_outside.png)

## Powder cup


## Options:
### Powder hopper:
Clear acrylic tube, outer diameter: 60mm and inner diameter: 56mm
![Hopper.jpg](resources/Hopper.jpg)

### Rear body:
Rear body without holes on the sides ([rear_body_without_holes.stl](STL/rear_body_without_holes.stl))
![Rear_body_memphis.jpg](resources/Rear_body_memphis.jpg)









![software_config.png](resources/software_config.png)
