# Powder Hopper Print and Assembly Guide

![powder_hopper_overview.png](resources/powder_hopper_overview.png)

## Print Guide

| Filename                                           | Quantity     | Remarks                                                                                                                                                                                               |
| -------------------------------------------------- | ------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [hopper_base.stl](hopper_base.stl)                 | 1            | The design includes large overhang (45 deg). You may need to reduce the print speed to archieve better surface finish.                                                                                |
| [hopper_body_100mm.stl](hopper_body_100mm.stl)     | 1 (optional) | Print in vase mode (no Z seam) with 0.8mm single wall thickness. You can select the part between 100, 150 and 200mm height body. <br>The height can be customized by scaling the part in Z direction. |
| [hopper_body_150mm.stl](hopper_body_150mm.stl)     | 1 (optional) | Print in vase mode (no Z seam) with 0.8mm single wall thickness. You can select the part between 100, 150 and 200mm height body.                                                                      |
| [hopper_body_200mm.stl](hopper_body_200mm.stl)     | 1 (optional) | Print in vase mode (no Z seam) with 0.8mm single wall thickness. You can select the part between 100, 150 and 200mm height body.                                                                      |
| [hopper_cap.stl](hopper_cap.stl)                   | 1            |                                                                                                                                                                                                       |
| [rear_body_interface.stl](rear_body_interface.stl) | 1            |                                                                                                                                                                                                       |

## BOM

| Name        | Quantity | Remarks                     |
| ----------- | -------- | --------------------------- |
| M3x8 BHCS   | 1        |                             |
| Shim Washer | 2        | OD=6mm, ID=3mm, Thick=0.5mm |

## Tools

* 2.0 mm hex driver

### Assembly

Stack the hopper base to the rear body interface. Note the travel limiter should sit in the groove and the valve is in the "open" position. 

* [hopper_base.stl](hopper_base.stl)

* [rear_body_interface.stl](rear_body_interface.stl)

![powder_hopper_travel_limiter.png](resources/powder_hopper_travel_limiter.png)

Stack 2x M3 washer to the rear body adapter pocket, then thread the M3x8 BHCS directly to the hopper base. Don't over tighten the screw and make sure the hopper base can rotate (with some resistance) between ON and OFF position. 

![powder_hopper_install_screw.png](resources/powder_hopper_install_screw.png)

Lastly, slide the powder hopper to the top of the rear body. Make sure to push the powder hopper all the way in. The rear body adapter should sit flush with the rear body assembly.

* [hopper_body_100mm.stl](hopper_body_100mm.stl)

* [hopper_cap.stl](hopper_cap.stl)

![powder_hopper_install_to_rear_body.png](resources/powder_hopper_install_to_rear_body.png)
