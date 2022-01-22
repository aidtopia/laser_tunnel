// Laser Tunnel -- Fan/Laser bracket
// Adrian McCarthy 2022

// https://github.com/aidtopia/PrintableParts/tree/main/libraries
use <aidbolt.scad>
use <aidutil.scad>

// Key dimensions for standard PC case fans.
fan_table = [
    //width height  dia     screw_sep
    [ 60,    60,     57.0,   50.0   ],
    [ 80,    80,     77.0,   71.5   ],
    [ 92,    92,     89.0,   82.5   ],
    [120,   120,    117.0,  104.5   ]
];

function find_fan_params(size) =
    let (candidate=find_params(size, table=fan_table))
    assert(!is_undef(candidate),
           str("fan size ", size, " mm not found in table"))
    candidate;

module fan_laser_bracket(fan_size=80, fan_depth=25.4, laser_dia=6, laser_l=6, distance=95, angle=15, thickness=2, nozzle_d=0.4) {
    fan_params = find_fan_params(fan_size);
    fan_w = fan_params[0];
    fan_h = fan_params[1];
    fan_dia = fan_params[2];
    screw_sep = fan_params[3];
    fan_d = fan_depth;
    hub_h = thickness + fan_h/2;

    module fan_envelope() {
        cube([fan_w, fan_d, fan_h], center=true);
        rotate([90, 0, 0]) {
            difference() {
                cylinder(h=fan_d+10, d=fan_dia, center=true);
                hub_dia = 22;
                cylinder(h=fan_d+12, d=hub_dia, center=true);
            }
            offset = screw_sep/2;
            translate([0, -offset, 0]) {
                translate([-offset, 0, 0])
                    cylinder(h=fan_d+10, d=4.3, center=true);
                translate([offset, 0, 0])
                    cylinder(h=fan_d+10, d=4.3, center=true);
            }
            translate([0, offset, 0]) {
                translate([-offset, 0, 0])
                    cylinder(h=fan_d+10, d=4.3, center=true);
                translate([offset, 0, 0])
                    cylinder(h=fan_d+10, d=4.3, center=true);
            }
        }
    }

    module fan_base() {
        base_h = 10;
        w = fan_w + 2*thickness;
        d = fan_d + 2*thickness;
        h = base_h + thickness;
        offset = screw_sep/2;
        screw_l = d + 3.5;  // +3.5 to approximate the nut
        translate([0, d/2, 0]) {
            if ($preview) { #fan_envelope(); }
            difference() {
                translate([-w/2, -d/2, -fan_h/2-thickness]) difference() {
                    cube([w, d, h]);
                    nudge = thickness - nozzle_d;
                    translate([nudge, nudge, nudge])
                        cube([fan_w+2*nozzle_d, fan_d+2*nozzle_d, h]);
                }
                rotate([90, 0, 0]) cylinder(h=d+2, d=fan_dia, center=true, $fn=64);
                translate([0, d/2, -offset]) {
                    translate([-offset, 0, 0]) rotate([-90, 90, 0]) bolt_hole("M4", screw_l);
                    translate([ offset, 0, 0]) rotate([-90, 90, 0]) bolt_hole("M4", screw_l);
                }
            }
        }
    }
    
    module laser_beam() { cylinder(h=distance, d=laser_dia/2); }
    
    module laser_mount(support_h=40) {
        inner_d = laser_dia + nozzle_d/2;
        outer_d = laser_dia + 2*thickness;
        split = 2*nozzle_d;
        translate([0, -laser_l/2, 0]) {
            rotate([-90, 0, 0]) {
                if ($preview) { #laser_beam(); }
                difference() {
                    union() {
                        cylinder(h=laser_l, d=outer_d, center=true, $fn=8);
                        translate([0, support_h/2, 0]) cube([thickness, support_h, laser_l], center=true);
                    }
                    cylinder(h=laser_l+2, d=inner_d, center=true, $fs=nozzle_d/2);
                    translate([0, -laser_dia/2, 0])
                        cube([split, inner_d, laser_l+2], center=true);
                }
            }
        }
    }
    
    module base_plate() {
        translate([-5, -distance, 0])
            cube([10, distance, thickness]);
    }

    intersection() {
        union() {
            translate([0, 0, hub_h]) {
                fan_base();
                rotate([angle, 0, 0]) translate([0, -distance, 0])
                    laser_mount(support_h=hub_h);
            }
            base_plate();
        }
        translate([-500, -500, 0]) cube(1000);
    }
}

fan_laser_bracket(thickness=2);
