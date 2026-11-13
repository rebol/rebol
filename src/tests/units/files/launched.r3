Rebol [
	title: "Launched script test"
]
write/append %launched.txt ajoin ["launched started "  now/time/precise lf]
wait 2
write/append %launched.txt ajoin ["launched completed " now/time/precise lf]
