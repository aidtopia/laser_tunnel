# Laser Tunnel

Adrian McCarthy (a.k.a. Hayward Haunter)<br>
January 2022

INCOMPLETE ROUGH DRAFT

## Description

By aiming a laser at a spinning mirror in a foggy or hazy environment, you can create a tunnel of light to transport your haunt visitors to an other worldly place.  We’re going to take it a step farther than other laser tunnels you may have seen.  By carefully timing the laser pulses to the motor, we can make the tunnel as disorienting as a physical rotating tunnel.  Furthermore, this design should be safer than other DIY laser tunnels you may have seen online.

http://www.ecfr.gov/graphics/er01fe93.031.gif

## Safety

* Never look directly into a laser, regardless of power or color.  Most consumer laser pointers put out enough power to seriously and permanently damage your vision if you look directly into the beam and suppress your reflexes to blink or look away.
* Never point a laser at someone else’s eyes.
* Never point a laser into the sky.  Doing so can endanger aircraft.  It’s against the law.
* Never point a laser at a vehicle in motion.
* Don’t omit safety features.  Several aspects of our design exist to mitigate certain safety risks.
* The build will include small parts that could be a choking hazard for small children or pets.
* Use a quality, UL-listed or -recognized power adapter.
* Exercise caution against burns when using a fog machine, smoke machine, hazer, etc.
* Warn your guests about the use of fog/smoke/haze, lasers, and stroboscopic effects.

**Disclaimer**:  I’m not a laser safety officer nor do I have any other specific qualifications to advise you about safety.  I have done some online research in order to design this laser tunnel to be safer than others I’ve seen online, but I cannot guarantee that it’s consistent with any specific regulations or best practices.  Build and use this laser tunnel at your own risk.

## Materials
* A 5-volt DC dot laser module that produces 5 mW or less.  I’m using 650nm (red) laser, because they seem to be much cheaper than other colors.  Do not use an infrared laser--those are invisible.  I would stay away from blue or purple lasers because they tend to be much more powerful.  You do not want to use a laser pointer.  A laser module is one component in a laser pointer and they are also used CD and DVD players, so they are generally easy to find online and quite affordable.  Laser modules often come with a lens that transforms the beam into a line, fan, or cross shape.  For this effect, you want a dot laser.

* A 4-pin computer fan (often called a PC case fan).  The case of the fan should be 80 mm in width and height..  A 4-pin fan has extra wires that will allow us to control the speed and timing of the effect.

* A 3D-printed bracket to hold the fan and laser.  An OpenSCAD file is included in this package designed for an 80 mm fan and a 6 mm diameter laser module.  I printed mine with PLA in an Ultimaker 2 with default slicer settings.

* Two M4 machine screws at least 35mm long, with nuts and washers to connect the fan to the bracket.  If the metric size is hard to find, you can probably substitute 1.5-inch long #6 or #8 machine screws.

* A small round mirror, 1 inch in diameter, plastic or glass.  Sometimes these are called “lipstick” mirrors because they are often glued to the end of a lipstick cap.  We’ll only need one, but you usually have to buy a quantity of them.

* A 3D-printed angled mirror holder.  An OpenSCAD file is included in this package that will let you produce the design using the angle of your choice.  I printed mine with PLA in an Ultimaker 2 with the default slicer settings.

* A couple drops of super glue (cyanoacrylate) to adhere the mirror to its holder.

* A self adhesive pair of hook and loop (e.g., Velcro) dots, approximately the same diameter as the mirror.  This will be used to attach the mirror holder to the spinning hub of the fan.

* A 16 MHz, 5V Arduino.  I’m using a 5V Pro Mini.  Though officially “retired”, Arduino Pro Minis are still widely available and inexpensive.  They are perfect for this project.  An Arduino Uno (or clone) should be fine as well.  Note that a 3V Pro Mini will _not_ work with this design.

* A 12 volt DC power adapter.  This project uses very little current, so probably any one you find will be sufficient.  Cheap knock-offs can be quite dangerous.  Please use a UL-listed or UL-recognized one.

* A plastic box to contain the device.  I’m using a 6 quart Sterilite storage bin.  These are roughly the size of a shoe box.

* TBD

# Failure Modes

These potential failures have been considered and the design attempts to mitigate the safety problems.

**Failure:** The laser module is out of spec and outputs more than the rated 5 mW of power.

**Mitigations:** An additional current-limiting resistor in series with the laser should bring down the total power of lasers at or slightly above 5 mW down below 5 mW.  The laser will be modulated with a duty cycle to bring down the output power.  The laser dot is reflected off a rapidly spinning mirror, greatly decreasing the time the beam might cross paths with an eyeball.

**Failure:** The fan motor becomes obstructed so that it stops or spins slowly.

**Mitigations:** A watchdog timer in the software will turn off the laser whenever the fan motor falls below a threshold speed (as measured by the tachometer pin).  The fan itself is designed to stop when obstructed to avoid overloading the motor.  The fan will attempt a restart after a few seconds.  If the problem is resolved, the fan will return to normal speed and the software will restart the laser.

**Failure:** The mirror becomes detached as is thrown by the centrifugal force.

**Mitigations:** The device enclosure should contain the mirror.  The centrifugal force should be relatively low, since the mirror will be approximately centered on the hub.  In absence of the mirror, the laser beam will terminate on the hub of the fan, which is a diffusely reflecting surface, thus no significant laser radiation should escape the enclosure.

The device will have an emergency stop button.

The device will have provisions for sensors to trigger the laser effect and to suppress it.  For example, a pressure map could disable the laser when someone steps too close to the beat.

The device will have appropriate warning labels inside and out.
