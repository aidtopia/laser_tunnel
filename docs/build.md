# Laser Tunnel

Adrian McCarthy (a.k.a. Hayward Haunter)<br>
January-March 2022

INCOMPLETE ROUGH DRAFT

## Description

By aiming a laser at a spinning mirror in a foggy or hazy environment, you can create a tunnel of light to transport your haunt visitors to an other worldly place.  We’re going to take it a step further than other laser tunnels you may have seen.  By carefully timing the laser pulses to the motor, we can make the tunnel as disorienting as a physical rotating tunnel.  Furthermore, this design should be safer than other DIY laser tunnels you may have seen online.

## Safety

* Never look directly into a laser, regardless of power or color.  Most consumer laser pointers put out enough power to seriously and permanently damage your vision if you look directly into the beam and suppress your reflexes to blink or look away.
* Never point a laser at someone else’s eyes.
* Never point a laser into the sky.  Doing so can endanger aircraft.  It’s against the law.
* Never point a laser at a vehicle in motion.
* Don’t omit safety features.  Several aspects of the design exist to reduce risks.
* The build will include small parts that could be a choking hazard for small children or pets.
* Wash your hands after handling the electronics, especially before eating.  The PCB and solder used contains lead.  Some of the electronics components might contain small amounts of other harmful substances.
* Use a quality, UL-listed or -recognized power adapter.
* Exercise caution against burns when using a fog machine, smoke machine, hazer, soldering iron, etc.
* Warn your guests about the use of fog/smoke/haze, lasers, and stroboscopic effects.

**Disclaimer**:  I’m not a laser safety officer nor do I have any other specific qualifications to advise you about safety.  I have done some online research in order to design this laser tunnel to be safer than others I’ve seen online, but I cannot guarantee that it’s consistent with any specific regulations or best practices.  Build and use this laser tunnel at your own risk.

## Materials

* A 5-volt DC dot laser module that produces 5 mW or less.  I’m using 650nm (red) laser, because they seem to be much cheaper than other colors.  Do not use an infrared laser--those are invisible.  I would stay away from blue or purple lasers because they tend to be much more powerful.  You do not want to use a laser pointer.  A laser module is one component in a laser pointer and they are also used CD and DVD players, so they are generally easy to find online and quite affordable.  Laser modules often come with a lens that transforms the beam into a line, fan, or cross shape.  For this effect, you want a dot laser.

* A 4-pin computer fan (often called a PC case fan).  The case of the fan should be 80 mm in width and height..  A 4-pin fan has extra wires that will allow us to control the speed and timing of the effect.

* A 3D-printed chassis to hold the fan and laser.  An OpenSCAD file is included in this package.  It has customizable parameters so that you can adjust it to use different sizes of fans, laser modules, mounting screws, etc.  If you're using an 80 mm fan and a 6 mm diameter laser module, the defaults should be appropriate for you.  Note the OpenSCAD's console window will give you information about the sizes of the various machine screws and nuts you will need to put it together.  I printed mine with PLA in an Ultimaker 2 with default slicer settings.

* Two M4 machine screws 35 mm long, with nuts to connect the fan to the chassis.  If the metric size is hard to find, you can substitute #6-32 machine screws if you select that size when you use OpenSCAD to create .STL file.  Note that the OpenSCAD output will tell you the correct lengths for those screws.

* A small round mirror, 1 inch in diameter, plastic or glass.  Sometimes these are called “lipstick” mirrors because they are often glued to the end of a lipstick cap.  We’ll only need one, but you usually have to buy a quantity of them.

* A 3D-printed angled mirror holder.  (I'm calling these deflectors.)  When you create your chassis with OpenSCAD, you can also create up to three deflectors of various angles.  I find that 5, 10, and 15 degrees make for a flexible set.

* A couple drops of super glue (cyanoacrylate) to adhere the mirror to its deflector.

* A pair of self-adhesive of hook-and-loop dots, approximately the same diameter as the mirror.  This will be used to attach the mirror holder to the spinning hub of the fan.

* A 12 volt DC power adapter.  This project uses very little current, so probably any one you find will be sufficient.  Cheap knock-offs can be quite dangerous.  Please use a UL-listed or UL-recognized one.

* A plastic box to contain the device.  I’m using a 6 quart Sterilite storage bin.  These are roughly the size of a shoe box.

* A circuit board to drive the prop.  A schematic in KiCAD 6 format is included in this package.  I ordered custom-fabricated PCBs that fit into the bracket, but you can probably replicate the schematic on a protoboard.  For the PCBs, you'll need three M3 x 10 mm screws to mount it to the bracket.  The OpenSCAD file that generates the bracket will let you substitute non-metric screws if necessary.

* A 16 MHz, 5V Arduino Pro Mini.  Though officially “retired”, Arduino Pro Minis are still widely available and inexpensive.  They are perfect for this project--even smaller than an Arduino Nano.  Note that a 3V Pro Mini will _not_ work with this design.  I recommend the original SparkFun version.  Some of the cheaper ones have unreliable voltage regulators that can fail.

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

### Install the fan in the chassis

The fan should fit snuggly into the rectangular recess at one end of the chassis.  The side with the mirror should face the opposite end of the chassis.  The cable from the fan should extend from one of the top corners.  Don't pinch it with the chassis.  I like to orient the fan so that the cord extends from the top corner on the side marked "Laser Tunnel".

With the fan in place, slip the M4 nuts into the hexagonal recesses on the front, and insert the M4 screws into the holes on the opposite side.  Hold a finger tip over the nut recess so that the nut doesn't slip out and tighten the screw manually from the back using a screwdriver.  The screw should engage with the nut and pull it all the way into the recess.

Avoid overtightening, which might crack the plastic.  If both nuts are fully seated and the heads of the screws are against the chassis, the fan should be securely connected tp the chassis.

### Install the circuit board

If your printed circuit board (PCB) is not already assembled, it's time to fire up the soldering iron.  Once you've soldered all of the components and sockets to it, return to this section to resume assembly of your Laser Tunnel.

The board has three mounting holes that correspond to the three posts in the middle part of the chassis.  Like the screws that hold the fan, there are recesses for the nuts on the bottom of the chassis.  Check the nut recesses for blobs of excess plastic from the 3D printing process and scrape them out if necessary.

Carefully attach the PCB using the three M3 x 10mm machine screws.  You will need a small screwdriver.  Do not overtighten.  The ends of the screws should not protrude beyond the flat bottom of the chassis.  Look at the side of the assembly to check that the edge of the chassis is parallel to the edge of the circuit board.  If they're not, you may have overtightened one or more of the PCB screws.

### Install the laser module

The laser module consists of a laser diode, a current-limiting resistor, and a lens.  Two wires are pre-soldered to the module.  Handle with care.  A strong tug could snap the wires off of the module, and re-soldering them could prove challenging.

Although an ideal laser would not need a lens for focus, laser diodes are not quite ideal.  The lens will focus the roughly rectangular cross section of the beam into a dot.  The focal distance can be  changed by twisting the front part of the module relative to the back part.  Initially, these can be sticky.  Do not use tools (pliers, wrenches, etc.) to try to adjust the focus, as you will likely damage the module.  With just your fingers try to turn the front of the barrel counterclockwise as though you're trying to loosen a tight screw.  You can always adjust the focus later, when the laser is actually on.

Slide the laser module into the laser mount so that the lens is facing the fan and about flush with the mount.  The back of the module will extend behind the mount.  The laser is held in the mount with friction.

If it seems too hard to slide the laser into the mount, check that the slot on top of the mount is completely clear.  Sometimes, when printing, a bit of plastic will remain to bridge the slot.  Use a knife to cut through any plastic that's briding the slot.

The wires from the laser should be connected to the screw terminal block marked `LASER`.  Do _not_ connect them to the screw terminal block marked `EMERGENCY STOP`.  I highly recommend crimping ferrules onto the ends of the laser wires.

Watch the polarity!  The red wire should be connected to the terminal marked `+`.  The blue (or possibly black) wire should be connected to the `-` terminal.  Getting that backwards could damage your laser module.

Secure the wires and then give them a gentle tug to make sure they're not likely to slip out.

### Connect the fan

The fan cable should fit over the four-pin header next to the power connector.  The header has a plastic tab to ensure the cable is oriented the correct way.  Don't force it.  When you have the connectors aligned correctly, they will slide together easily.

Make sure the excess cable is clear of the fan.  You don't want it where it could touch the spinning bits nor cast a shadow from the laser beam.

### Install the Arduino Pro Mini

If your Pro Mini can without headers pre-installed, you'll have to solder them on now.

#### Solder on the headers

You'll need two single-row 12-pin strips for the long edges.  You don't need to specifically buy 12-pin headers.  You can buy "breakaway" headers (that usually come in rows of 40 pins) and snap off the lengths you need.  The longer part of the header pins and the plastic that holds them together should be on the underside of the Pro Mini and you solder them to the pads on the top side.  It's important that the pins are straight and square to the Pro Mini module.  You can use a solderless breadboard or some perforated protoboard to hole the headers in alignment and then slide the module onto them and solder it together.

At one end of the Pro Mini is a spot for a 6-pin header used for programming the device.  Most other Arduinos use a USB connector for programming, but the Pro Mini uses FTDI.  Even if your Pro Mini has been pre-programmed, I recommend soldering on the programming header.  It'll be convenient for software updates and possibly for troubleshooting.

Conventionally, right-angle pins are used for the FTDI header, but you can use straight pins if you prefer.  Either way, that should be placed on the top side of the board and soldered underneath, they same way most other components are on a simple circuit board.

#### Insert the module into the socket

If you bend a header pin or don't get the Pro Mini aligned properly with the sockets, you're going to have a bad day.  So take your time.

I like place the Pro Mini on top of the socket and check to ensure that every pin is lined up with its corresponding hole in the socket before pressing the device down into the socket.  Also check that you have the Pro Mini the right way around.  The circuit board is marked with six lines that represent where the FTDI header pins would extend (assuming you used right-angle pins).

Now press the module down into the socket.  Apply pressure evenly so the all the pins go in straight.

If you experience more resistance than you expect, stop!  You might be bending a pin.  Check the alignment of the pins again.  A slightly bent pin can usually be straightened with needle nose pliers.  A more severely bent or broken pin means you'll have to replace the entire row of header pins.

When installed properly, you shouldn't see the metal pins anymore.  They should all be completely inside the socket, and the Pro Mini board should be level and square (relative to the main circuit board).

### Install the audio player

The audio player is optional.  If you don't plan to use the Laser Tunnel for sound, you can skip this step.

The audio modules come with the headers pre-installed, so all you have to do is align it correctly and press it into its socket.

The rectangular bit of metal on top is the slot for a memory card.  The opening is along one edge of the module, and that edge should be closest to the edge of the circuit board.

In a later section, we'll cover the details of how to format your memory card.

### Place the jumper for continous triggering

Whether you plan to use triggering sensors or not, I recommend using the jumper to requestion continuous animation for initial testing.

The jumper (sometimes called a "shunt jumper" or "shorting connector"), is a bit of metal inside a plastic sleeve.  You can slide it over two adjacent pins, and it creates an electrical connection between them.  If you place it on the GND and LO pins on the TRIGGER header, the effect will run continuously.  There are markings on the board to show where to connect it.

### Download the software to the Pro Mini.

If your Pro Mini hasn't yet been programmed, this is an ideal point to do so.

### Connect the power

With the Laser Tunnel set so that the mirror faces a wall or other suitable projection surface, connect your 12 VDC power supply to the power jack on the circuit board.

## Making an enclosure

When configuring the 3D-printed parts in OpenSCAD, you can choose whether the chassis has tabs for screwing it down to a base.  If you want the tabs, indicate your preferred screw size, #6 or #8.  The holes in the tabs will be shaped for flat-head wood screws of that size.

(The March 2022 Norcal Haunters kit has tabs with #6 holes.)

## Optional outputs

### Audio

Audio Info

You'll need a Micro SD card (sometimes called TF or TrueFlash).  It does not need to be high capacity, since you'll only need to put three audio files on it.  Most cards come preformatted, but your card stores more than 32 GB, you'll probably need to reformat it as FAT32.  The audio module doesn't understand the newer formats used on higher capacity cards.

You'll need a computer that can copy files to the card.  Most Micro SD cards come with an adapter that will let them fit into a regular SD slot, since SD slots are much more common on computers.

In the root directory of the card you want to place your audio files.  The first three characters in the file name should be a number, like 001, 002, or 003.  The file extension should be ".mp3" or ".wav" as appropriate.  After the number and before the extension, you can put anything else that you can normally put in a file name.  I name my files like this:

* `001startle.mp3`
* `002ambient.mp3`
* `003emergency.mp3`

You should not have other types of files on the card.  (I'm told Macs might create a hidden file, and that can sometimes confuse the audio player.  I'm not a Mac expert, so I can't give you specific directions for correcting that.  Ask your local genius to help you make sure the card has nothing but the audio files.)

Once you've copied the files to the card, you can insert the card into the audio module.  (Label up, pins down, and the edge with the pins should go in first.)  When you push it all the way in, there will be a click, and it will latch into place.  To remove it, push it again, and the latch will be released and a spring will eject it far enough for you to grasp it.

The audio jack next to the audio module takes a typical 1/8" stereo audio plug.  Yes, the audio module will output true stereo to the jack.  The output is "line level."  You can plug in powered speakers (like computer speakers) or wired headphones or ear buds.  If you want to output it through big speakers, you'll connect an audio amplifier to the jack and your speakers to the amplifier.

If there is just one file on the card, it will be used as the effect sound (played when an animation is running).

If there are two files on the card, the first is the effect, and the second is the "ambient" sound, which will loop until the effect is triggered.

If there are three or more files on the card, the first is the effect, the second is the ambient loop, and the third will be played if/when the emergency stop is pressed.  Extra audio are ignored.  Extra non-audio files can confuse the audio module.

If there's an effect sound, the animation will run for the duration of that sound, and the "Effect Time" adjustment on the board will control how long the fog signal stays high.  If there's no sound, the "Effect Time" sets the duration of the animation in 1-second steps from 3 to 30 seconds.

### Fog signal

### House lights signal

## Optional inputs

### Remote emergency stop switch

### Sensors for triggering

### Sensors for suppressing the laser

## Troubleshooting

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
