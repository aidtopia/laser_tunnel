# Laser Tunnel

Adrian McCarthy (a.k.a. Hayward Haunter)<br>
January 2022

INCOMPLETE ROUGH DRAFT

## Description

By aiming a laser at a spinning mirror in a foggy or hazy environment, you can create a tunnel of light to transport your haunt visitors to an other worldly place.  We’re going to take it a step farther than other laser tunnels you may have seen.  By carefully timing the laser pulses to the motor, we can make the tunnel as disorienting as a traditional vortex tunnel.  Furthermore, this design should be safer than other DIY laser tunnels you may have seen online.

## Safety

* Never look directly into a laser, regardless of power or color.  Most consumer laser pointers put out enough power to seriously and permanently damage your vision.
* Never point a laser at someone else’s eyes.
* Never point a laser into the sky.  Doing so can endanger aircraft.  It’s against the law.
* Don’t omit safety features.  Several aspects of our design are to mitigate certain safety risks.
* The build will include small parts that could be a choking hazard for small children or pets.
* Use a quality, UL-listed power adapter.
* Exercise caution against burns when using a fog machine, smoke machine, hazer, etc.
* Warn your guests about the use of fog/smoke/haze, lasers, and stroboscopic effects.

**Disclaimer**:  I’m not a laser safety officer nor do I have any other specific qualifications to advise you about safety.  I have done some online research in order to design this laser tunnel to be safer than others I’ve seen online, but I cannot guarantee that it’s consistent with any specific regulations or best practices.  Build and use this laser tunnel at your own risk.

## Materials
* A 5-volt DC dot laser module that produces 5 mW or less.  I’m using 650nm (red) laser, because they seem to be much cheaper than the more popular green ones.  Do not use an infrared laser—those are invisible.  I would stay away from blue or purple lasers because they tend to be much more powerful.  You do not want to use a laser pointer.  A laser module is one component in a laser pointer and they are also used CD and DVD players, so they are generally easy to find online and quite affordable.  Laser modules often come with a lens that transforms the beam into a line, fan, or cross shape.  For this effect, you want a dot laser.
* A 4-pin computer fan (often called a PC case fan).  I recommend an 80 mm one, because those will be cheaper than a 120 mm one.  A 4-pin fan has extra wires that will allow us to control the speed and timing of the effect.
* A small round mirror about 1 inch in diameter, plastic or glass.  Sometimes these are called “lipstick” mirrors because they are often glued to the end of a lipstick cap.  We’ll only need one, but you usually have to buy a quantity of them.
* A plastic box to contain the device.  I’m using a 6 quart Sterilite storage bin.  These are roughly the size of a shoe box.
* A 16 MHz, 5V Arduino.  I’m using a 5V Pro Mini.  Though officially “retired”, Arduino Pro Minis are still widely available and inexpensive.  They are perfect for this project.  An Arduino Uno (or clone) is fine too.
* A 12 volt DC power adapter.  This project uses very little current, so probably any one you find will be sufficient.  Cheap knock-offs can be quite dangerous.  Please use a UL-listed or UL-recognized one.
