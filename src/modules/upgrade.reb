Rebol [
	title: "Upgrade"
	purpose: "Keep track of possible Rebol upgrades"
	name:   upgrade
	type:   module
	options: [delay]
	version: 0.0.1
	exports: [upgrade]
	author: @Oldes
	file: %upgrade.reb
	home: https://src.rebol.tech/modules/upgrade.reb
]

system/options/log/upgrade: 4
upgrade: function [
	"Check if there are possible upgrades"
][
	sys/log/error 'UPGRADE "Not yet implemented!"
	;; remove itself...
	try [delete system/options/modules/upgrade.reb] 
	try [unset in system/contexts/user 'upgrade]
	try [unset in system/contexts/lib  'upgrade]
	system/modules/upgrade: https://src.rebol.tech/modules/upgrade.reb
]
