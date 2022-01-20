// Breadboard experiment
// Adrian McCarthy 2022

// Can we print a piece that will fit in a solderless breadboard
// or piece of protoboard?  My guess is that the pins will be too
// thin and fragile.

// Conclusion:  Nope.  Even after tweaking slicer parameters, the
// pins were too thick and too fragile.

// Can we print a plate with holes that can be "staked" to a
// breadboard with generic headers?

// Conclusion:  Nope.  The holes are too small to print reliably.

// Dimensions based off of the dimensions of a DIP package that
// was in inches.

function thou(x) = x*25.4/1000;

pin_spacing  = thou(100);
pin_diameter = thou( 18);
pin_height   = thou(140);
base_width   = thou(400);
base_depth   = thou(410);
base_height  = 1;
x_offset     = thou( 45);
y_offset     = x_offset;

difference() {
    cube([base_width, base_depth, base_height]);
    for (y = [y_offset:3*pin_spacing:base_depth]) {
        for (x = [x_offset:pin_spacing:base_width]) {
            translate([x, y, -1])
              cylinder(h=base_height+2, d=pin_diameter + 0.5, $fn=10);
        }
    }
}
