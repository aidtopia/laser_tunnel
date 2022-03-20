// Model of a PC case fan.
// Adrian McCarthy 2022
//
// Useful for visualizations and for creating mounts (by subtracting the
// fan's envelope).

use <aidutil.scad>

// Key dimensions for standard PC case fans.
fan_table = [
    //width height  dia     screw_sep
    [ 60,    60,     57.0,   50.0   ],
    [ 70,    70,     67.0,   61.5   ],
    [ 80,    80,     77.0,   71.5   ],
    [ 92,    92,     89.0,   82.5   ],
    [120,   120,    117.0,  104.5   ],
    [140,   140,    127.0,  124.5   ],
    [180,   180,    177.0,  165.0   ]
];

function find_fan_params(size) =
    let (candidate=find_params(size, table=fan_table))
    assert(!is_undef(candidate),
           str("fan size ", size, " mm not found in table"))
    candidate;

// A simple model of a PC case fan, centered at the origin,
// with the axis of rotation along the z-axis.
module fan_model(fan_size=80, fan_depth=25.4, envelope=false, rotation_angle=0, nozzle_d=0.4) {
    extra = envelope ? nozzle_d : 0;
    fan_params = find_fan_params(fan_size);
    fan_w = fan_params[0] + extra;
    fan_h = fan_params[1] + extra;
    fan_dia = fan_params[2];
    screw_sep = fan_params[3];
    fan_d = fan_depth + extra;
    hub_dia = max(30, fan_size/4);
    
    module hub() {
        cylinder(h=fan_d, d=hub_dia, center=true);
    }
    
    module blades() {
        count = floor(fan_size*7/80);
        d = fan_d - 1;
        l = fan_dia/2 - 2;
        rotate([90, 0, 0])
        for (i=[1:count])
          rotate([0, i*360/count, 0])
            linear_extrude(l, twist=50) square([1, d], center=true);
    }

    module body() {
        udi(envelope ? "u" : "d") {
            cube([fan_w, fan_h, fan_d], center=true);
            cylinder(h=2*fan_d+10, d=fan_dia, center=true);
            if (!envelope) {
                screw_offset = screw_sep/2;
                translate([0, -screw_offset, 0]) {
                    translate([-screw_offset, 0, 0])
                        cylinder(h=fan_d+10, d=4.3, center=true);
                    translate([screw_offset, 0, 0])
                        cylinder(h=fan_d+10, d=4.3, center=true);
                }
                translate([0, screw_offset, 0]) {
                    translate([-screw_offset, 0, 0])
                        cylinder(h=fan_d+10, d=4.3, center=true);
                    translate([screw_offset, 0, 0])
                        cylinder(h=fan_d+10, d=4.3, center=true);
                }
            }
        }
    }

    color("gray") body();
    if (!envelope) {
        rotate([0, 0, rotation_angle]) {
            color("gray") {
                hub();
                blades();
            }
        }
    }
}
