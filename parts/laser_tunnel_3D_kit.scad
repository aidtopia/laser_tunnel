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

// Optional flat-head screws used to anchor the chassis to a surface.
Anchor_Screws = "none"; // [none, #6-32, #8-32]

// Thickness of the chassis walls. (mm)
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
use <pc_case_fan.scad>

module bosses(locations, boss_h) {
    for (l = locations) {
        translate([l.x, l.y, -boss_h]) boss(l[2], boss_h);
    }
}

module bores(locations, depth, threads="none", head="pan") {
    for (l = locations) {
        translate([l.x, l.y, 0])
            bolt_hole(l[2], depth, threads=threads, head=head);
    }
}

function fan_rotation_angle(time=$t, rpm=1800, fps=30) =
    360 * rpm/60 * time/fps;

module laser_beam(distance, beam_dia=1.5) {
    translate([0, 0, -distance]) #cylinder(h=distance, d=beam_dia);
}
    
function find_pcb_params() = [
    // width, length, thickness
    75, 75, 1.6,
    [  // mounting hole locations with sizes
        [ 4.5, 75 -  5.5, "M3"],
        [ 4.5, 75 - 50.0, "M3"],
        [70.5, 75 - 34.0, "M3"]
    ],
    [  // additional points for support--no screws
        [45,   75 - 65.0, 5],
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

module chassis(
    fan_size=80, fan_d=25.4, fan_screw="M4",
    laser_dia=6, laser_l=6, distance=100, angle=15,
    pcb_screw="M3", anchor_screw="#6-32", thickness=2,
    nozzle_d=0.4, assembly=""
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
    pcb_support_points = pcb_params[4];
    pcb_screw_l = pcb_screw[0] == "M" ? 10 : 12.5;
    elevation = pcb_screw_l - pcb_th;
    echo(str("PCB screws should be ", screw_to_string(pcb_screw, pcb_screw_l)));
    boss_h = elevation - thickness;
    posts = set_radii(pcb_mounting_holes, pcb_screw);
    extra_posts = pcb_support_points;

    use_anchors = anchor_screw != "none";
    anchor_d = use_anchors ? boss_diameters(anchor_screw)[0] : 0;
    anchor_h = use_anchors ? bolt_head_height(anchor_screw, "flat") : 0;
    if (use_anchors) {
        echo(str("Anchor screws should be ", anchor_screw,
                 " with flat heads."));
    }

    x_support = support_w/2;
    y0_support = 0;
    y1_support = y0_support + support_full_d;
    x_fanchor = 0;
    y_fanchor = y1_support + anchor_d;
    
    x_pcb = pcb_w/2;
    y1_pcb = y0_support;
    y0_pcb = y1_pcb - thickness - pcb_l - thickness;

    x_lanchor = x_pcb/2 + thickness;
    y_lanchor = y0_pcb - 2*thickness - anchor_d/2;

    x_laser = 0;
    y_laser = y0_support - distance*cos(angle);

    anchors = use_anchors ?
        set_radii(
        [
            [ x_fanchor, y_fanchor - anchor_d/2],
            [ x_lanchor, y_lanchor + anchor_d/2],
            [-x_lanchor, y_lanchor + anchor_d/2]
        ], anchor_screw) :
        [];
    
    // Transforms its children just as the fan_model must be
    // transformed to put it into its position relative to
    // the chassis.
    module orient_fan() {
        translate([0, support_d/2+support_extra_d, fan_h/2+thickness])
        rotate([90, 0, 0]) children();
    }

    // Transforms its children just as the pcb_model must be
    // transformed to put it into its position relative to
    // the chassis.
    module orient_pcb() {
        translate([0, -thickness, elevation])
        rotate([0, 0, -90])
        translate([0, -pcb_l/2, 0])
        children();
    }

    module footprint() {
        core_points = [
            [ x_support,    y1_support                          ],
            [ x_support,    y0_support                          ],
            [ x_pcb,        y1_pcb,     (x_support - x_pcb)/2   ],
            [ x_pcb,        y0_pcb,     thickness               ],
        ];
        all_points = use_anchors ? [
            [ x_fanchor,    y_fanchor,  anchor_d/2              ],
            each core_points,
            [ x_lanchor + anchor_d/4,    y_lanchor,  anchor_d/2              ],
            [ x_laser,      y_laser,    laser_l+thickness       ]
        ] : [
            each core_points,
            [ x_laser,      y_laser,    laser_l+thickness       ]
        ];
        path = mirror_path(all_points);

        polygon(rounded_polygon(path));
    }

    module base_plate() {
        linear_extrude(thickness) footprint();

        // Add a rib to resist warping.
        translate([0, 0, thickness]) rotate([90, 0, 0])
            cylinder(h=abs(y0_pcb), r=thickness, $fn=10);
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

    module laser_mount() {
        inner_d = laser_dia + nozzle_d/2;
        outer_d = laser_dia + 2*thickness;
        mast_h = fan_h/2;
        mast_dia2 = max(laser_l, outer_d);
        mast_dia1 = min(laser_l, outer_d);
        split = 3*nozzle_d;
        translate([0, 0, laser_l/2]) {
            difference() {
                union() {
                    cylinder(h=laser_l, d=outer_d, center=true, $fn=8);
                    translate([0, -mast_h/2, 0]) rotate([90, 0, 0])
                        cylinder(h=mast_h, d1=mast_dia1, d2=mast_dia2,
                                 center=true);
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
    
    module orient_anchors() {
        translate([0, 0, anchor_h]) children();
    }
    
    intersection() {
        difference() {
            union() {
                base_plate();
                orient_fan() fan_support();
                orient_laser() laser_mount();
                orient_pcb() {
                    bosses(posts, boss_h);
                    bosses(extra_posts, boss_h);
                }
                orient_anchors() bosses(anchors, anchor_h);
            }

            orient_pcb() translate([0, 0, pcb_th])
                bores(posts, pcb_screw_l, threads="recessed hex nut");
            
            // The extra_posts are just for support and do not need to
            // be bored.

            orient_anchors()
                bores(anchors, 25.4, threads="none", head="flat");

            // Allow the QR code on the PCB to show through the bottom.
            orient_pcb() translate([0, 0, -(elevation+1)])
                linear_extrude(elevation+2)
                    polygon(rounded_polygon(set_radii([
                        [12.5, 3], [12.5, 21], [28.5, 21], [28.5, 3]
                    ], thickness)));
        }
        
        // The construction process leaves a few bits that need to be
        // scraped off the bottom.
        linear_extrude(100) footprint();
    }

    if (assembly != "") {
        // Show the kit in the context of some non-printed parts.
        parts = split(assembly, ",");
        rot = fan_rotation_angle(time=$t, rpm=1800, fps=30);
        if (search(["pcb"], parts) != [[]]) {
            orient_pcb() pcb_model();
        }
        if (search(["fan"], parts) != [[]]) {
            orient_fan() fan_model(fan_size, fan_d, rotation_angle=rot);
        }
        if (search(["beam"], parts) != [[]]) {
            orient_laser() laser_beam(distance);
        }
        if (search(["mirror"], parts) != [[]]) {
            orient_fan() translate([0, 0, fan_d/2]) rotate([0, 0, rot])
                deflector(Mirror_Angle_1, Mirror_Diameter, Mirror_Thickness, nozzle_d, show_mirror=true);
        }

        if (search(["beam"], parts) != [[]] && search(["mirror"], parts) != [[]]) {
            // Show the reflection of the beam.
            orient_fan() translate([0, 0, fan_d/2])  // origin of reflection
            rotate([180, 0, 0])  // reflect back toward the laser
            rotate([-angle, 0, 0])  // for offset angle of the laser
            rotate([0, 0, -rot])  // for deflector rotation
            rotate([0, -2*Mirror_Angle_1, 0])  // for deflector angle
            laser_beam(10*distance);
        }
    }
}

module deflector(angle=10, mirror_d=25.4, mirror_th=1.75, nozzle_d=0.4, show_mirror=false) {
    h = mirror_d;
    inner_d = mirror_d + nozzle_d;
    outer_d = inner_d + 2;
    pop_d = max(4, mirror_d/6);
    box = sqrt(2)*outer_d;
    intersection() {
        r = inner_d/2;
        translate([0, 0, nozzle_d/2])
        translate([r, 0, 0]) rotate([0, angle, 0]) translate([-r, 0, 0])
        union() {
            difference() {
                translate([0, 0, mirror_th-h]) cylinder(d=outer_d, h=h);
                translate([0, 0, 0.1]) cylinder(d=inner_d, h=h);
                translate([r/1.5, 0, -h+0.2]) cylinder(d=pop_d, h=h);
            }
            if (show_mirror) {
                color("white") cylinder(d=mirror_d, h=mirror_th);
            }
        }
        translate([-box/2, -box/2, 0]) cube(box);
    }
}

module deflectors(all_angles=[10], mirror_d=25.4, mirror_th=1.75, nozzle_d=0.4, show_mirror=false) {
    angles = [ for (a = all_angles) if (a != 0) a ];
    for (i = [0:len(angles)-1])
        translate([0, -i*(1.2 * mirror_d), 0])
            deflector(angles[i], mirror_d, mirror_th, nozzle_d, show_mirror);
}

module kit(assembly="") {
    chassis(
        fan_size=Fan_Size, fan_d=Fan_Depth, fan_screw=Fan_Screws, 
        laser_dia=Laser_Diameter, laser_l=Laser_Length,
        distance=Laser_Distance, angle=Laser_Angle,
        pcb_screw=PCB_Screws, anchor_screw=Anchor_Screws,
        thickness=Thickness, nozzle_d=Nozzle_Diameter, assembly=assembly);

    translate([Fan_Size/2 + 3*Thickness + Mirror_Diameter/2, 0, 0]) {
        all_angles = [Mirror_Angle_1, Mirror_Angle_2, Mirror_Angle_3];
        deflectors(all_angles, mirror_d=Mirror_Diameter, mirror_th=Mirror_Thickness, nozzle_d=Nozzle_Diameter, show_mirror=false);
    }
}

kit($preview ? "fan, beam, mirror" : "");
