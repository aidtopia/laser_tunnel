// Laser Tunnel -- Fan/Laser bracket
// Adrian McCarthy 2022

module fan_laser_bracket(thickness=3, nozzle_d=0.4) {
    difference() {
        // Plate that mounts to a standard 80mm PC case cooling fan
        cube([80, 80, thickness], center=true);
        
        // Cutout for air flow
        cylinder(h=4, d=77.5, center=true, $fn=92);

        // Holes for mounting screws
        offset = 71.5/2;
        h = thickness + 2;
        translate([-offset, -offset, 0]) cylinder(h=h, d=4.7, center=true, $fs=nozzle_d/2);
        translate([ offset, -offset, 0]) cylinder(h=h, d=4.7, center=true, $fs=nozzle_d/2);
        translate([-offset,  offset, 0]) cylinder(h=h, d=4.7, center=true, $fs=nozzle_d/2);
        translate([ offset,  offset, 0]) cylinder(h=h, d=4.7, center=true, $fs=nozzle_d/2);
    }
}

fan_laser_bracket(thickness=2);
