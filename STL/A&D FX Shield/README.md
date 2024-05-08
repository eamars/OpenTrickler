# A&D FX Shield Print and Assembly Guide

The OpenTrickler is designed to be mounted to the A&D FX Shield if you're using the A&D fx-120/300i series of precision scales. 

![fx_shield_overview.png](resources/fx_shield_overview.png)

## Print Guide

| Filename                                                             | Quantity | Remarks        |
| -------------------------------------------------------------------- | -------- | -------------- |
| [cup_base_7mm.stl](cup_base_7mm.stl)                                 | 1        |                |
| [front_discharger_mount.stl](front_discharger_mount.stl)             | 1        |                |
| [pan_cover.stl](pan_cover.stl)                                       | 1        |                |
| [pan_cover_lid.stl](pan_cover_lid.stl)                               | 1        |                |
| [powder_cup_body.stl](powder_cup_body.stl)                           | 1        |                |
| [powder_cup_handle.stl](powder_cup_handle.stl)                       | 1        |                |
| [rear_discharge_cup_ring.stl](rear_discharge_cup_ring.stl)           | 1        |                |
| [rear_discharge_mount.stl](rear_discharge_mount.stl)                 | 1        |                |
| [rear_discharger_cup.stl](rear_discharger_cup.stl)                   | 1        |                |
| [rear_discharger_sliding_door.stl](rear_discharger_sliding_door.stl) | 1        |                |
| [scale_base_adapter_ring.stl](scale_base_adapter_ring.stl)           | 1        |                |
| [scale_pan_cover.stl](scale_pan_cover.stl)                           | 1        |                |
| [scale_shield.stl](scale_shield.stl)                                 | 1        |                |
| [scale_weighting_pan_adapter.stl](scale_weighting_pan_adapter.stl)   | 1        |                |
| [trickler_adapter_plate.stl](trickler_adapter_plate.stl)             | 1        |                |
| [weighting_pan_27mm.stl](weighting_pan_27mm.stl)                     | 1        | See note below |

### How to Print Weiging Pan?

You **DO NOT** need to enable support to print the weighting pan. The slicer (using the OrcaSlicer as example) will automatically recognize the floating surface as bridge (or Overhang if you're using PrusaSlicer). The slicer will automatically reduce the speed and flow, and increase the fan speed to print mid air. 

![print_weighting_pan_with_bridge](resources/print_weighting_pan_with_bridge.png)

Using the [tutorial](https://help.prusa3d.com/article/poor-bridging_1802) to tune the slicer for better bridging performance. 
