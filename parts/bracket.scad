// Laser Tunnel -- Fan/Laser bracket
// Adrian McCarthy 2022

// Width of the fan. (mm)
Fan_Size = 80; // [60, 80, 92, 120, 140, 180]

// Depth of the fan. (mm)
Fan_Depth = 25.4; // [15:0.2:35]

// Screws used to mount the fan are typically M4, but you can choose #6-32 if metric screws aren't available.
Fan_Screws = "M4"; // [M4, #6-32]

module __Customizer_Limit__ () {}

// https://github.com/aidtopia/PrintableParts/tree/main/libraries
use <aidbolt.scad>
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

module fan_laser_bracket(fan_size=80, fan_depth=25.4, fan_screw="M4", laser_dia=6, laser_l=6, distance=95, angle=15, thickness=2, nozzle_d=0.4) {
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
        base_h = max(10, fan_h/8);
        w = fan_w + 2*thickness;
        d = fan_d + 2*thickness;
        h = base_h + thickness;
        offset = screw_sep/2;

        // Add some extra depth to the back for pocket nuts.
        screw_l = round_up(d + 5, 5);
        full_d = screw_l;
        extra_d = full_d - d;
        echo(str("Fan screws should be ", screw_l, " mm long"));

        translate([0, d/2, 0]) {
            //if ($preview) { #fan_envelope(); }
            difference() {
                translate([-w/2, -d/2, -fan_h/2-thickness]) difference() {
                    union() {
                        cube([w, d, h]);
                        translate([0, d, 0]) cube([w, extra_d, h]);
                    }
                    nudge = thickness - nozzle_d;
                    translate([nudge, nudge, nudge])
                        cube([fan_w+2*nozzle_d, fan_d+2*nozzle_d, h]);
                }
                rotate([90, 0, 0]) cylinder(h=2*full_d, d=fan_dia, center=true, $fn=64);
                translate([0, -d/2, -offset]) {
                    translate([-offset, 0, 0]) rotate([90, 90, 0]) bolt_hole(fan_screw, screw_l, "recessed nut");
                    translate([ offset, 0, 0]) rotate([90, 90, 0]) bolt_hole(fan_screw, screw_l, "recessed nut");
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

fan_laser_bracket(fan_size=Fan_Size, fan_depth=Fan_Depth, fan_screw=Fan_Screws, thickness=2);
