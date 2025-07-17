# OpenTrickler Neopixel LED Mod

[![youtube_video](https://img.youtube.com/vi/Y1qmCO0_NH8/0.jpg)](https://www.youtube.com/watch?v=Y1qmCO0_NH8)

## Declaimer

- The mod is build based on [OpenTrickler v2.0.1 release](https://github.com/eamars/OpenTrickler/releases/tag/v2.0.1). The mod may not be compatible of future OpenTrickler releases. 
- The scale shield mod only works for A&D FX series scales only. 

## BOM

- 2x Neopixel RGBW Button PCB - [Link to Adafruit](https://www.adafruit.com/product/4776)
- 420 mm AWG24-26 three lead cable

## Preparations

1. Cut the three lead cable in two lengths: 220mm and 200mm. 
2. Solder one end of *220mm* cable to the **DIN** side of the Neopixel LED, pointing towards the outside **edge** of the PCB. 
3. Solder the other end of *220mm* cable to the **DIN** side of another Neopixel LED, pointing towards the **outside** edge of the PCB.
4. Crimp one end of *200mm* cable with 3pin JST (2.54pitch) connector. Check the order of each cable from the [board specification](https://github.com/eamars/RaspberryPi-Pico-Motor-Expansion-Board/blob/main/resources/opentrickler_pcb_v2_peripheral_annotation.drawio.png). 
5. Solder the other end of the *200mm* cable to the **DOUT** side of the Neopixel LED, pointing towards the **inside** of the PCB. 

![neopixel_cable_prep.png](resources/neopixel_cable_prep.png)

## Print and Assembly Guide

Install the Neopixel LED (with both in and out cables) to the pocket of [front_discharger_mount_with_led.stl](STL/front_discharger_mount_with_led.stl). Make sure both cables are tucked into the cable channel and stay in flush or below than the top surface. 

![install_neopixel_at_discharge_port.png](resources/install_neopixel_at_discharge_port.png)

Continue with front and rear body assembly. Once the OpenTrickler is mounted to the scale shield top assembly, you need to run the cable with the Neopixel LED, from the front discharger mount, through the cut out behind the [front_body_with_shutter_with_led.stl](STL/front_body_with_shutter_with_led.stl), following the red line as shown below: The other cable with JST connector (blue line) can be left floating at this stage. 

![cable_layout_bottom.png](resources/cable_layout_bottom.png)

Install the Neopixel LED (with only one set of cable) to the pocket of [FrontVolumeReductionInsert_Bottom_with_LED.stl](STL/FrontVolumeReductionInsert_Bottom_with_LED.stl), and run the cable through the cable channel on [FrontVolumeReductionInsert_Top_with_LED.stl.stl](STL/FrontVolumeReductionInsert_Top_with_LED.stl.stl) and [front_body_with_shutter_with_led.stl](STL/front_body_with_shutter_with_led.stl)

![cable_layout_top.png](resources/cable_layout_top.png)

Side the front body cover and the OpenTrickler assembly is complete. The last step is to ensure both cables are sitting nicely in the cut out on [scale_shield_with_led.stl](STL/scale_shield_with_led.stl). Pluging the cable to the PWNOUT port on the controller board and your assembly is now complete. 

![cable_layout_outside.png](resources/cable_layout_outside.png)
