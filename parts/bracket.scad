// Laser Tunnel -- Fan/Laser bracket
// Adrian McCarthy 2022

module fan_laser_bracket(thickness=3, nozzle_d=0.4) {
    // Plate that mounts to a standard 80mm PC case cooling fan
    module fan_plate(thickness) {
        difference() {
            cube([80, 80, thickness], center=true);
            
            // Cutout for air flow
            cylinder(h=thickness+2, d=77.5, center=true, $fn=92);

            // Holes for mounting screws
            offset = 71.5/2;
            h = thickness + 2;
            translate([-offset, -offset, 0]) cylinder(h=h, d=4.7, center=true);
            translate([ offset, -offset, 0]) cylinder(h=h, d=4.7, center=true);
            translate([-offset,  offset, 0]) cylinder(h=h, d=4.7, center=true);
            translate([ offset,  offset, 0]) cylinder(h=h, d=4.7, center=true);
        }
    }

    intersection() {
        translate([0, 0, 40]) rotate([90, 0, 0]) fan_plate(thickness=5, $fs=nozzle_d/2);
        cube([80, 5, 16], center=true);
    }
    translate([-7.5, -90, 0]) cube([15, 90, thickness]);
    translate([0, 0, 40]) rotate([15, 0, 0]) translate([0, -90, -40]) {
        translate([0, 0, 40]) {
            rotate([90, 0, 0]) difference() {
                rotate([0, 0, 45/2]) cylinder(h=6, d=9, center=true, $fn=8);
                cylinder(h=9, d=6.2, center=true, $fs=nozzle_d/2);
                translate([0, 3, 0]) cube([0.5, 6, 8], center=true);
            }
        }
        translate([0, 0, (40-6/2)/2]) cube([thickness, 6, 40-6/2], center=true);
    }
}

intersection() {
    fan_laser_bracket(thickness=2);
    translate([-50, -100, 0]) cube(110);
}
