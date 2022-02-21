// Laser Tunnel
// Adrian McCarthy 2022
//
// Generates the 3D parts necessary for the build.

// Width of the fan. (mm)
Fan_Size = 80; // [60, 80, 92, 120, 140, 180]

// Depth of the fan from front to back. (mm)
Fan_Depth = 25.4; // [15:0.2:35]

// Screws used to mount the fan are typically M4, but you can choose #6-32 if metric screws aren't available. Check OpenSCAD's console output for the ideal screw length.
Fan_Screws = "M4"; // [M4, #6-32]

// Diameter of the laser tube. (mm)
Laser_Diameter = 6; // [3:0.1:15]

// Length of the laser tube that can fit into the mount. (mm)
Laser_Length = 6; // [3:15]

// Distance between the mirror and the laser module. (mm)
Laser_Distance = 95; // [90:200]

// Angle by which the laser is offset from the center of the mirror to avoid casting a shadow. (degrees)
Laser_Angle = 15; // [0:15]

// Screws used to mount the printed circuit board are typically M3, but you can choose #4-40 if metric screws aren't available. Check OpenSCAD's console output for the proper screw length.
PCB_Screws = "M3"; // [M3, #4-40]

// Thickness of the bracket walls. (mm)
Thickness = 1.6; // [1.2:0.4:3.2]

// The mirror should not be wider than the hub of the fan. (mm)
Mirror_Diameter = 25.4; // [20:0.2:27]

// The mirror should be thin and light. (mm)
Mirror_Thickness = 1.75; // [1:0.25:2]

// A larger angle makes a wider cone of light. (degrees)
Mirror_Angle_1 = 5; // [0:1:15]

// To print a second deflector at the same time, set its angle. (degrees)
Mirror_Angle_2 = 0; // [0:1:15]

// To print a third deflector at the same time, set its angle. (degrees)
Mirror_Angle_3 = 0; // [0:1:15]

// Some dimensions will be optimized according to the diameter of the nozzle on your 3D printer. If unknown, the default (0.4) should be adequate. (mm)
Nozzle_Diameter = 0.4; // [0.1:0.1:1.0]

module __Customizer_Limit__ () {}

// https://github.com/aidtopia/PrintableParts/tree/main/libraries
use <aidbolt.scad>
use <aidutil.scad>

module udi(option="u") {
    if (option == "i" && $children > 1) {
        intersection() { children([0]); children([1:$children-1]); }
    } else if (option == "d" && $children > 1) {
        difference() { children([0]); children([1:$children-1]); }
    } else {
        union() children();
    }
}

module bosses(locations, boss_h) {
    for (l = locations) {
        translate([l.x, l.y, -boss_h]) boss(l[2], boss_h);
    }
}

module bores(locations, depth, threads="none") {
    for (l = locations) {
        translate([l.x, l.y, 0])
            bolt_hole(l[2], depth, threads=threads);
    }
}

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
module fan_model(fan_size=80, fan_depth=25.4, envelope=false, rpm=0, nozzle_d=0.4) {
    extra = envelope ? nozzle_d : 0;
    fan_params = find_fan_params(fan_size);
    fan_w = fan_params[0] + extra;
    fan_h = fan_params[1] + extra;
    fan_dia = fan_params[2];
    screw_sep = fan_params[3];
    fan_d = fan_depth + extra;
    hub_dia = max(22, fan_size/4);
    
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
        rotate([0, 0, rpm/60*$t/30*360]) {
            color("gray") {
                hub();
                blades();
            }
            if ($preview) {
                // Make the front (air intake) distinguishable from the back.
                color("white") translate([0, 0, fan_d/2+1.1]) sphere(d=hub_dia/4);
            }
        }
    }
}

function find_pcb_params() = [
    // width, length, thickness
    75, 75, 1.6,
    [  // mounting hole locations with sizes
        [ 4.5, 75 -  5.5, "M3"],
        [ 4.5, 75 - 50.0, "M3"],
        [70.5, 75 - 34.0, "M3"]
    ]
];

// A simple model of a printed circuit board for the project.
module pcb_model() {
    pcb_params = find_pcb_params();
    w = pcb_params[0];
    l = pcb_params[1];
    th = pcb_params[2];
    mounting_holes = pcb_params[3];

    color("green")
    difference() {
        linear_extrude(th) square(w, l);
        translate([0, 0, th]) bores(mounting_holes, th+1);
    }
}

module branding() {
    font = "Liberation Sans:style=Bold Italic";
    scale([1/130, 1/16, 1])
        linear_extrude(1)
            text("Laser Tunnel V1", size=12, font=font,
                 halign="center", valign="center");
}

module credits() {
    font = "Liberation Sans";
    scale([1/183, 1/32, 1])
        linear_extrude(1) {
            text("Designed for Norcal Haunters", size=10, font=font,
                 halign="center", valign="baseline");
            translate([0, -15, 0])
            text("by Adrian McCarthy 2022", size=10.5, font=font,
                 halign="center", valign="baseline");
        }
}

module bracket(
    fan_size=80, fan_d=25.4, fan_screw="M4",
    laser_dia=6, laser_l=6, distance=100, angle=15,
    pcb_screw="M3", thickness=2, nozzle_d=0.4
) {
    fan_params = find_fan_params(fan_size);
    fan_w = fan_params[0];
    fan_h = fan_params[1];
    fan_dia = fan_params[2];
    screw_sep = fan_params[3];

    support_h = max(10, fan_h/8);
    support_w = fan_w + 2*thickness;
    support_d = fan_d + 2*thickness;
    // Add some extra depth for pocket nuts.
    fan_screw_l = round_up(support_d + 5, 5);
    echo(str("Fan screws should be ", screw_to_string(fan_screw, fan_screw_l)));
    support_full_d = fan_screw_l;
    support_extra_d = support_full_d - support_d;

    pcb_params = find_pcb_params();
    pcb_w = pcb_params[0];
    pcb_l = pcb_params[1];
    pcb_th = pcb_params[2];
    pcb_mounting_holes = pcb_params[3];
    pcb_screw_l = pcb_screw[0] == "M" ? 10 : 12.5;
    elevation = pcb_screw_l - pcb_th;
    echo(str("PCB screws should be ", screw_to_string(pcb_screw, pcb_screw_l)));
    boss_h = elevation - thickness;
    
    posts = set_radii(pcb_mounting_holes, pcb_screw);

    fan_plate = [
        [ support_w/2, support_full_d],
        [ support_w/2, 0],
    ];

    pcb_plate = [
        [ pcb_w/2, 0, (fan_plate[0].x - pcb_w/2)/2],
        [ pcb_w/2, -(pcb_l+2*thickness)],
    ];
    
    mast_point = [0, -distance*cos(angle), laser_l+thickness];

    // Transforms its children just as the fan_model must be
    // transformed to put it into its position relative to
    // the bracket.
    module orient_fan() {
        translate([0, support_d/2+support_extra_d, fan_h/2+thickness])
        rotate([90, 0, 0]) children();
    }

    // Transforms its children just as the pcb_model must be
    // transformed to put it into its position relative to
    // the bracket.
    module orient_pcb() {
        translate([0, -thickness, elevation])
        rotate([0, 0, -90])
        translate([0, -pcb_l/2, 0])
        children();
    }

    module base_plate() {
        corners = rounded_polygon(mirror_path([
            each fan_plate, each pcb_plate, mast_point
        ]));
        linear_extrude(thickness) polygon(corners);
    }
    
    module fan_support() {
        h = max(10, 0.15*fan_h);

        // Normally, we'd want the recesses for the nuts to have corners at
        // the top since a sloped peak prints better without support than a
        // flat span.  But the #6 nuts are wider than the M4 ones and their
        // bottom corners would cut into the base plate.  So we rotate
        // those to make the wide dimension horizontal, and hope for the
        // best printing the flat span.
        nutrot = fan_screw[0] == "#" ? 90 : 0;

        intersection() {
            difference() {
                cube([2*fan_w, 2*fan_h, 2*fan_d], center=true);
                fan_model(fan_size, fan_d, envelope=true, $fn=64);
                screw_offset = screw_sep/2;
                translate([0, -screw_offset, -(fan_screw_l - support_extra_d)/2]) {
                    translate([-screw_offset, 0, 0]) rotate([180, 0, nutrot])
                        bolt_hole(fan_screw, fan_screw_l, "recessed hex nut");
                    translate([screw_offset, 0, 0]) rotate([180, 0, nutrot])
                        bolt_hole(fan_screw, fan_screw_l, "recessed hex nut");
                }
                translate([-(fan_w+thickness)/2, (support_h-fan_h)/2, support_extra_d/2]) rotate([0, -90, 0]) scale([support_full_d, support_h, thickness]) branding();
                translate([(fan_w+thickness)/2, (support_h-fan_h)/2, support_extra_d/2]) rotate([0, 90, 0]) scale([support_full_d, support_h, thickness]) credits();
            }
            translate([0, (support_h-fan_h)/2, support_extra_d/2])
                cube([support_w, support_h, support_full_d], center=true);
        }
    }

    module laser_beam(distance) {
        translate([0, 0, -distance]) cylinder(h=distance, d=laser_dia/4);
    }
    
    module laser_mount() {
        inner_d = laser_dia + nozzle_d/2;
        outer_d = laser_dia + 2*thickness;
        mast_h = fan_h/2;
        split = 3*nozzle_d;
        translate([0, 0, laser_l/2]) {
            if ($preview) { #laser_beam(distance); }
            difference() {
                union() {
                    cylinder(h=laser_l, d=outer_d, center=true, $fn=8);
                    translate([0, -mast_h/2, 0]) cube([thickness, mast_h, laser_l], center=true);
                }
                cylinder(h=laser_l+2, d=inner_d, center=true, $fs=nozzle_d/2);
                translate([0, laser_dia/2, 0])
                    cube([split, inner_d, laser_l+2], center=true);
            }
        }
    }
    
    module orient_laser() {
        orient_fan()
            translate([0, 0, fan_d/2])
                rotate([angle, 0, 0])
                    translate([0, 0, distance])
                        children();
    }
    
    difference() {
        union() {
            base_plate();
            orient_fan() fan_support();
            orient_laser() laser_mount();
            orient_pcb() bosses(posts, boss_h);
        }
        orient_pcb() translate([0, 0, pcb_th])
            bores(posts, pcb_screw_l, threads="recessed hex nut");
        orient_fan() credits();

        // Clip the bottom of the laser mount mast left hanging below the
        // base plate.
        translate([0, 0, -500]) cube(1000, center=true);
        
        // Allow the QR code on the PCB to show through the bottom.
        orient_pcb() translate([0, 0, -(elevation+1)])
            linear_extrude(elevation+2)
                polygon(rounded_polygon(set_radii([
                    [12.5, 3], [12.5, 21], [28.5, 21], [28.5, 3]
                ], thickness)));
    }

    // Preview mode shows the kit in the context of some non-printed parts.
    if ($preview) {
        orient_pcb() pcb_model();
        orient_fan() fan_model(fan_size, fan_d, rpm=1800);
    }
}

module deflector(angle=10, mirror_d=25.4, mirror_th=1.75, nozzle_d=0.4) {
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

bracket(
    fan_size=Fan_Size, fan_d=Fan_Depth, fan_screw=Fan_Screws, 
    laser_dia=Laser_Diameter, laser_l=Laser_Length,
    distance=Laser_Distance, angle=Laser_Angle,
    pcb_screw=PCB_Screws, thickness=Thickness,
    nozzle_d=Nozzle_Diameter);


translate([-(Fan_Size/2 + 3*Thickness + Mirror_Diameter/2), 0, 0]) {
    if (Mirror_Angle_1 != 0) {
        deflector(Mirror_Angle_1, Mirror_Diameter, Mirror_Thickness, Nozzle_Diameter, $fn=92);
    }

    if (Mirror_Angle_2 != 0) {
        translate([0, -(1.2 * Mirror_Diameter), 0])
        deflector(Mirror_Angle_2, Mirror_Diameter, Mirror_Thickness, Nozzle_Diameter, $fn=92);
    }

    if (Mirror_Angle_3 != 0) {
        translate([0, -2*(1.2*Mirror_Diameter), 0])
        deflector(Mirror_Angle_3, Mirror_Diameter, Mirror_Thickness, Nozzle_Diameter, $fn=92);
    }
}
