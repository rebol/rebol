Rebol [
	title: "Needs module test 3"
	name:  test-needs-url
	needs: https://github.com/Oldes/Rebol3/raw/master/src/tests/units/files/test-needs-url-value.reb
]

export test-needs-url-result: (42 = test-needs-url-value)
