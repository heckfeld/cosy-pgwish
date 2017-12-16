background { color red 0.078 green 0.361 blue 0.753 }

camera {
    location <2.1, 1.3, 1.7>
    direction <-0.700389, -0.433574, -0.566982>
    right <-0.436046, 0.704383, 0>
    up <-0.399372, -0.24723, 0.682401>
} // camera

light_source {
    <4, 3, 2> color red 0.57735 green 0.57735 blue 0.57735
} // light

light_source {
    <1, -4, 4> color red 0.57735 green 0.57735 blue 0.57735
} // light

light_source {
    <-3, 1, 5> color red 0.57735 green 0.57735 blue 0.57735
} // light

#declare txt001 = texture {
    pigment {
        color red 1 green 0.75 blue 0.33
    } // pigment
    // normal { bumps, ripples, etc. }
    finish {
        ambient 0.2
        diffuse 0.8
    } // finish
} // texture txt001

object {
    triangle {
        <12, 12, -0.5>,
        <-12, -12, -0.5>,
        <12, -12, -0.5>
    } // tri
    texture { txt001 }
} // object

object {
    triangle {
        <12, 12, -0.5>,
        <-12, 12, -0.5>,
        <-12, -12, -0.5>
    } // tri
    texture { txt001 }
} // object

#declare txt002 = texture {
    pigment {
        color red 1 green 0.9 blue 0.7
    } // pigment
    // normal { bumps, ripples, etc. }
    finish {
        ambient 0
        diffuse 0.5
        phong 0.5  phong_size 45.2776
        reflection 0.5
    } // finish
} // texture txt002

sphere { <0, 0, 0>, 0.5  texture { txt002 } }

sphere { <0.272166, 0.272166, 0.544331>, 0.166667  texture { txt002 } }

sphere { <0.643951, 0.172546, 1.11022e-16>, 0.166667  texture { txt002 } }

sphere { <0.172546, 0.643951, 1.11022e-16>, 0.166667  texture { txt002 } }

sphere { <-0.371785, 0.0996195, 0.544331>, 0.166667  texture { txt002 } }

sphere { <-0.471405, 0.471405, 1.11022e-16>, 0.166667  texture { txt002 } }

sphere { <-0.643951, -0.172546, 1.11022e-16>, 0.166667  texture { txt002 } }

sphere { <0.0996195, -0.371785, 0.544331>, 0.166667  texture { txt002 } }

sphere { <-0.172546, -0.643951, 1.11022e-16>, 0.166667  texture { txt002 } }

sphere { <0.471405, -0.471405, 1.11022e-16>, 0.166667  texture { txt002 } }

