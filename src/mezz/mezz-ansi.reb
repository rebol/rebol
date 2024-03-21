Rebol [
	Title:   "ANSI escape sequences support"
	File:    %mezz-ansi.reb
	Version: 1.0.0
	Date:    4-Mar-2021
	Purpose: "Decorate any value with bright ANSI color sequences"
]

;- using 2x append to avoid making a reduced block for the output
as-gray:   func[value][append append copy "^[[1;30m" value "^[[0m"]
as-red:    func[value][append append copy "^[[1;31m" value "^[[0m"]
as-green:  func[value][append append copy "^[[1;32m" value "^[[0m"]
as-yellow: func[value][append append copy "^[[1;33m" value "^[[0m"]
as-blue:   func[value][append append copy "^[[1;34m" value "^[[0m"]
as-purple: func[value][append append copy "^[[1;35m" value "^[[0m"]
as-cyan:   func[value][append append copy "^[[1;36m" value "^[[0m"]
as-white:  func[value][append append copy "^[[1;37m" value "^[[0m"]
