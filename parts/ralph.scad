// Model of a possible laser tunnel setup for Ralph.

use <laser_tunnel_3d_kit.scad>

function inch(x) = 25.4 * x;

// Ralph has an oval frame with an opening of 16"x24".
module target(w=inch(16), h=inch(24), rim=inch(2), th=inch(0.75)) {
    backstop_w = w + 4*rim;
    backstop_h = h + 4*rim;
    module oval() { scale([w, h]) circle(d=1, $fn=64); }

    translate([0, 0, th]) difference() {
        union() {
            translate([-backstop_w/2, -backstop_h/2, -th])
                cube([backstop_w, backstop_h, th]);
            color("white") linear_extrude(th)
                offset(r=rim) oval();
        }
        linear_extrude(3*th, center=true) oval();
    }
}

module projector(beams=false) {
    laser_tunnel(
        mirror_angles=[8],
        assembly=(beams ? "tunnel" : "fan, pcb, mirror")
    );
}

module beams() { projector(beams=true); }

rotate([-22.5, 0, 0]) translate([0, inch(26), 0]) { projector(); beams(); }
translate([0, 0, inch(14)]) rotate([90, 0, 0]) target();

