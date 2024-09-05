# Tuning Guide for Powder Profile

## Stop Thresholds

Stop thresholds are defined under Charge Mode -> Coarse/Fine Trickler Stop Threshold. The value are unitless. The unit should be tuned based on the unit reported by the scale. By default all values are designed for grain (gr). 

The coarse trickler stop threshold should be set around 3 to 5 gr, depending on the size of the kernel. 

The fine trickler stop threshold should be set to the accuracy of the scale. For example, the A&D FX-300i has accuracy of 0.001g or 0.02gr. In this case you should set to 0.02 gr stop threshold for best performance and accuracy. 

## Profile

The OpenTrickler controller is buit based on the [Proportional-Integral-Derivative (PID) Controller](https://en.wikipedia.org/wiki/Proportional%E2%80%93integral%E2%80%93derivative_controller). Thus the Tuning Methods for a PID applies for the OpenTrickler. Except, the overshoot is unacceptable for which translate to the overthrow. 

The Coarse Trickler should be tuned separately from the fine trickler. Below is the effect of Kp, Ki and Kd to the system. 

| Parameter | Raise Time    | Overshoot | Settling Time | Steady State Error                                  | Stability                    |
| --------- | ------------- | --------- | ------------- | --------------------------------------------------- | ---------------------------- |
| Kp        | Decrease      | Increase  | Small change  | Decrease                                            | Degrade                      |
| Ki        | KEEP AT 0     | KEEP AT 0 | KEEP AT 0     | KEEP AT 0                                           | KEEP AT 0                    |
| Kd        | Little change | Decrease  | Decrease      | Theoretically no change | Improve if Kd is already low |

### Tuning Coarse Trickler

1. Start with Kp, Ki and Kd at 0. 

2. Increase Kp until the Coarse Trickler will overthrow beyond the Coase Trickler stop threshold. 

3. Keep Ki at 0 at all time. 

4. Increase Kd to reduce the overthrow to an acceptable threshold. 

The tuning goal is to reduce the Coarse Trickler operating time below 10 seconds. 

### Tuning Fine Trickler

1. Start with Kp, Ki and Kd at 0. 

2. Increase Kp until the Fine Trickler will overthrow beyond the charge weight. 

3. Keep Ki at 0 at all time.

4. Increase Kd to eliminate the overthrow to an acceptable threshold (for example, 1/15 overthrow). 

The tuning goal is to reduce the overall operating time below 15 seconds.

Remember to save the powder profile once you've found the optimal values. 

# References

[Tuning Methods of A PID Controller | CTRL ALT FTC](https://www.ctrlaltftc.com/the-pid-controller/tuning-methods-of-a-pid-controller)
