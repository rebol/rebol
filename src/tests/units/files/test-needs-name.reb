Rebol [
	title: "Needs module test 1"
	name:  test-needs-name
	needs: test-needs-name-value
]

export test-needs-name-result: (42 = test-needs-name-value)
