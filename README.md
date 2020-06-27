# litterbox-ventilation
Automatic litterbox ventilation system for cats with asthma and dust sensitivity

For asthmatic cats, very small environmental factors can trigger flare ups of asthma, causing an undesirable reaction of increased stress levels, and ultimately resulting in a potentially harmful asthma attack. I have found that the leading factor for triggering attacks is air quality and humidity. Litterbox usage tends to create a lot of dry dust particulate in an enclosed area, irritating the throat and causing attacks. This project aims to mitigate this vector of attacks by detecting litter box usage, and venting the area (by way of a mounted fan) for the duration of usage.

# Components

* 1x microcontroller - anything works, I played with this on some Particle Xenons and Trinket Pros I had laying around
* 1x IR break beam sensors - I used 5mm (not 3mm) as the range is wide enough to comfortably span the large litterbox I have
* 1x 5v PWM Fan - 120mm works for me. Optimizing for low noise, and PWM control
* 1x RGB Indicator LED - Useful to see status from afar: In Use, Venting, Power Low (recharge needed)
* USB Battery pack with 2 plugs (one for microcontroller, one for fan power)

# Design

xx todo

# Software

xx todo
