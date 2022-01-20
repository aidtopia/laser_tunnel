// Laser Tunnel -- Angled Mirror Holder
// Adrian McCarthy 2022

// A larger angle makes a wider cone of light. (degrees)
Angle = 5; // [3:1:20]

// The mirror should not be wider than the hub of the fan. (mm)
Mirror_Diameter = 25.4; // [20:0.2:27]

// The mirror should be thin and light. (mm)
Mirror_Thickness = 1.75; // [1:0.25:2]

// Some dimensions may be optimized for your printer's nozzle size, but the default value should work for most. (mm)
Nozzle_Diameter = 0.4;  // [0.1:0.1:0.8]

module angled_mirror_holder(angle=5, mirror_d=25.4, mirror_th=1.75, nozzle_d=0.4) {
    h = mirror_d;
    inner_d = mirror_d + nozzle_d;
    outer_d = inner_d + 2;
    box = sqrt(2)*outer_d;
    intersection() {
        r = inner_d/2;
        translate([r, 0, 0]) rotate([0, angle, 0]) translate([-r, 0, 0])
        difference() {
            translate([0, 0, mirror_th-h]) cylinder(d=outer_d, h=h);
            translate([0, 0, 0.1]) cylinder(d=inner_d, h=h);
        }

        translate([-box/2, -box/2, 0]) cube(box);
    }
}

angled_mirror_holder(Angle, Mirror_Diameter, Mirror_Thickness, Nozzle_Diameter, $fn=92);
