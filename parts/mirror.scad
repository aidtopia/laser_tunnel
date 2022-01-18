// Laser Tunnel -- Angled Mirror Holder
// Adrian McCarthy 2022

module angled_mirror_holder(angle=5, mirror_d=26, mirror_th=1.75) {
    h = mirror_d;
    outer_d = mirror_d + 2;
    box = sqrt(2)*outer_d;
    intersection() {
        translate([mirror_d/2, 0, 0]) rotate([0, angle, 0]) translate([-mirror_d/2, 0, 0])
        difference() {
            translate([0, 0, mirror_th-h]) cylinder(d=outer_d, h=h);
            translate([0, 0, 0.1]) cylinder(d=mirror_d, h=h);
        }

        translate([-box/2, -box/2, 0]) cube(box);
    }
}

angled_mirror_holder(5, $fn=92);
