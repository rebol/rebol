Rebol [
   title: "Triangulate extension test"
]

import 'triangulate

inp: object [
   points: #[f64! [
       0.0   0.0
       10.0  0.0
       10.0  10.0
       0.0   10.0
       2.5   2.5
   ]]
   attributes: #[f64! [
       100.0
       2.0
       3.0
       4.0
       5.0
   ]]
   markers: #[i64! [2 1 1 1 1]]
   regions: #[f64! [
      5.0
      5.0
      10.0 ; Regional attribute (for whole mesh).
      1.0  ; Area constraint that will not be used.
   ]]
   report: true
]

out: object [
	points:          none
	attributes:      none
	markers:         none
	segments:        none
	segment-markers: none
	edges:           none
	triangles:       none

	; voronoi output:
	v-points:        none
	v-attributes:    none
	v-edges:         none
	v-norms:         none  
]

? inp
triangulate inp out
? out