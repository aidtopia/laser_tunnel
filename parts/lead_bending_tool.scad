module axial_lead_bender(l=6.2, d=2.5, pitch=7.62, lead_d=0.5, nozzle_d=0.4) {
    h = max(5, d/2 + 1.5);
    difference() {
        union() {
            cube([pitch, h, h], center=true);
            translate([-pitch/2, -25.4, -h/2]) cube([pitch, 25.4, 3]);
        }
        translate([0, 0, h/2]) rotate([0, 90, 0]) {
            cylinder(h=l + nozzle_d, d=d+nozzle_d, center=true, $fs=nozzle_d/2);
            cylinder(h=pitch+1, d=lead_d+nozzle_d, center=true, $fs=nozzle_d/2);
        }
        translate([-pitch/2, 0, 0])
            cylinder(h=h+1, d=lead_d+nozzle_d, center=true, $fs=nozzle_d/2);
        translate([ pitch/2, 0, 0])
            cylinder(h=h+1, d=lead_d+nozzle_d, center=true, $fs=nozzle_d/2);
        
    }
}

axial_lead_bender();
