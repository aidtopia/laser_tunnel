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
* Exercise caution against burns when using a fog machine, smoke machine, hazer, soldering iron, etc.
* Warn your guests about the use of fog/smoke/haze, lasers, and stroboscopic effects.

**Disclaimer**:  I’m not a laser safety officer nor do I have any other specific qualifications to advise you about safety.  I have done some online research in order to design this laser tunnel to be safer than others I’ve seen online, but I cannot guarantee that it’s consistent with any specific regulations or best practices.  Build and use this laser tunnel at your own risk.

## Materials
* A 5-volt DC dot laser module that produces 5 mW or less.  I’m using 650nm (red) laser, because they seem to be much cheaper than other colors.  Do not use an infrared laser--those are invisible.  I would stay away from blue or purple lasers because they tend to be much more powerful.  You do not want to use a laser pointer.  A laser module is one component in a laser pointer and they are also used CD and DVD players, so they are generally easy to find online and quite affordable.  Laser modules often come with a lens that transforms the beam into a line, fan, or cross shape.  For this effect, you want a dot laser.

* A 4-pin computer fan (often called a PC case fan).  The case of the fan should be 80 mm in width and height..  A 4-pin fan has extra wires that will allow us to control the speed and timing of the effect.

* A 3D-printed bracket to hold the fan and laser.  An OpenSCAD file is included in this package.  It has customizable parameters so that you can adjust it to use different sizes of fans, laser modules, mounting screws, etc.  If you're using an 80 mm fan and a 6 mm diameter laser module, the defaults should be appropriate for you.  Note the OpenSCAD's console window will give you information about the sizes of the various machine screws and nuts you will need to put it together.  I printed mine with PLA in an Ultimaker 2 with default slicer settings.

* Two M4 machine screws 35 mm long, with nuts to connect the fan to the bracket.  If the metric size is hard to find, you can substitute #6-32 machine screws if you select that size when you use OpenSCAD to create .STL file.  Note that the OpenSCAD output will tell you the correct lengths for those screws.

* A small round mirror, 1 inch in diameter, plastic or glass.  Sometimes these are called “lipstick” mirrors because they are often glued to the end of a lipstick cap.  We’ll only need one, but you usually have to buy a quantity of them.

* A 3D-printed angled mirror holder.  (I'm calling these deflectors.)  When you create your bracket with OpenSCAD, you can also create up to three deflectors of various angles.  I find that 5, 10, and 15 degrees make for a flexible set.

* A couple drops of super glue (cyanoacrylate) to adhere the mirror to its deflector.

* A self-adhesive pair of hook and loop (e.g., Velcro) dots, approximately the same diameter as the mirror.  This will be used to attach the mirror holder to the spinning hub of the fan.

* A 12 volt DC power adapter.  This project uses very little current, so probably any one you find will be sufficient.  Cheap knock-offs can be quite dangerous.  Please use a UL-listed or UL-recognized one.

* A plastic box to contain the device.  I’m using a 6 quart Sterilite storage bin.  These are roughly the size of a shoe box.

* A circuit board to drive the prop.  A schematic in KiCAD 6 format is included in this package.  I ordered custom-fabricated PCBs that fit into the bracket, but you can probably replicate the schematic on a protoboard.  For the PCBs, you'll need three M3 x 10 mm screws to mount it to the bracket.  The OpenSCAD file that generates the bracket will let you substitute non-metric screws if necessary.

* A 16 MHz, 5V Arduino Pro Mini.  Though officially “retired”, Arduino Pro Minis are still widely available and inexpensive.  They are perfect for this project--even smaller than an Arduino Nano.  Note that a 3V Pro Mini will _not_ work with this design.

* A DFPlayer Mini (or clone) audio player for the optional sound effects.

* Optional sensors to trigger or suppress the effect.

* A fog or haze machine.  There are inexpensive consumer level ones available at party stores, more durable ones marketed to DJs, and high end ones for concerts, theatrical presentations, etc.  You should have a way to run the machine on a timer or a way to trigger one from a 5V TTL-level signal.

## Assembly

### Glue the mirror into the deflector

Test fit the mirror into the recess in the angled deflector.  If everything is sized correctly, the mirror should fit snuggly inside the raised rim.  It may be necessary to remove imperfections in the 3D print that prevent the mirror from fitting inside.

Once you know it will fit correctly, apply a small amount of super glue (cyanoacrylate) to the recess in the deflector and then insert the mirror.  Give it a good squeeze so that both surfaces contact the glue.

Repeat if you're making more than one deflector.

### Apply a hook-and-loop dot to the fan's hub.

First, identify which side of the fan has the hub that spins.  With the fan disconnected, manually rotate the fan a bit.  You should see the hub turn with the blades.  If not, check the other end.  The spinning hub is typically on the intake side of the fan.  Some fans have hard-to-see arrow on them that indicate which way the air flows.

Apply a self-adhesive hook-and-loop dot to the hub that spins.  Take your time to center it as best you can.  Press firmly for several seconds to ensure the adhesive makes good contact.  (Don't press so hard that you damage the motor.)

Remove the backing from the other side of the hook-and-loop dot, and adhere that back of your mirror deflector to the dot.  Again, take your time to center it as best you can.

If you're making a set of deflectors, attach the appropriate half of the hook-and-loop dots to the others.  For the rest of this assembly, you can use any deflector angle you want, but you may find it most convenient to use one near the middle of the range, like 10 degrees.

When swapping deflectors, gently pull the deflector from one edge.  If the hook-and-loop dot lifts at all from either the fan or the deflector, you probably need to replace it.

### Install the fan in the bracket

The fan should fit snuggly into the rectangular recess at one end of the bracket.  The side with the mirror should face the opposite end of the bracket.  The cable from the fan should extend from one of the top corners.  Don't pinch it with the bracket.  I like to orient the fan so that the cord extends from the top corner on the side marked "Laser Tunnel".

With the fan in place, slip the M4 nuts into the hexagonal recesses on the front, and insert the M4 screws into the holes on the opposite side.  Hold a finger tip over the nut recess so that the nut doesn't slip out and tighten the screw manually from the back using a screwdriver.  The screw should engage with the nut and pull it all the way into the recess.

Avoid overtightening, which might crack the plastic.  If both nuts are fully seated and the heads of the screws are against the bracket, the fan should be securely connected ot the bracket.

### Install the circuit board

If your printed circuit board (PCB) is not already assembled, it's time to fire up the soldering iron.  Once you've soldered all of the components and sockets to it, return to this section to resume assembly of your Laser Tunnel.

The board has three mounting holes that correspond to the three posts in the middle part of the bracket.  Like the screws that hold the fan, there are recesses for the nuts on the bottom of the bracket.  Check the nut recesses for blobs of excess plastic from the 3D printing process and scrape them out if necessary.

Carefully attach the PCB using the three M3 x 10mm machine screws.  You will need a small screwdriver.  Do not overtighten.  The ends of the screws should not protrude beyond the flat bottom of the bracket.  Look at the side of the assembly to check that the edge of the bracket is parallel to the edge of the circuit board.  If they're not, you may have overtightened one or more of the PCB screws.

###

## Failure Modes

These potential failures have been considered and the design attempts to minimize the safety problems.

**Failure:** The laser module is out of spec and outputs more than the rated 5 mW of power.

**Mitigations:** The laser is controlled by an NPN transistor which has a forward voltage drop of about 0.2 V, so the supply to the laser will be slightly less than 5V.  Also, the beam moves rapidly, so it doesn't remain in any one place for very long.

**Failure:** The fan motor becomes obstructed so that it stops or spins slowly.

**Mitigations:** A watchdog timer in the software will turn off the laser whenever the fan motor falls below a threshold speed (as measured by the tachometer pin).  Most PC case fans (but not all) are designed to to stop when obstructed to avoid overloading the motor (rotor lock protection).  Most fans with rotor lock protection will automatically attempt a restart after a few seconds.  If the problem is resolved, the fan will return to normal speed and the software will re-enable the laser.

**Failure:** The mirror becomes detached as is thrown by the centrifugal force.

**Mitigations:** The device enclosure should contain the mirror.  The centrifugal force should be relatively low, since the mirror will be approximately centered on the hub.  In absence of the mirror, the laser beam will terminate on the hub of the fan itself, which is a diffusely reflecting surface, thus no significant laser radiation should escape the enclosure.

The device will have an emergency stop button.

The device will have provisions for a sensor to suppress the laser effect.  For example, a pressure map could disable the laser when someone steps too close to the projector.

The device will have appropriate warning labels inside and out.
