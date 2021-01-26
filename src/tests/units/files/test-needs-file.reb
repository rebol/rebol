Rebol [
	title: "Needs module test 2"
	name:  test-needs-file
	needs: %units/files/test-needs-file-value.reb
	;- at this moment files imported from other file
	;- does not use parent file's location!
]

export test-needs-file-result: (42 = test-needs-file-value)
