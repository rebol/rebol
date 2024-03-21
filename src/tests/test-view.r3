Rebol [
	Title: "Test View"
	File:  %test-view.r3
	Purpose: "Minimal view (GUI) tests"
]

unless module? system/modules/window [
	print as-purple "View is not available on this Rebol version!"
	;; so far it is only on Windows (in a Bulk product)!
	quit/return 1
]

pos: (system/view/screen-gob/size - 600x600) / 2 ;= to center the windows

;- open window at offset 0x0                                      
view/as-is make gob! 600x600

;- open window with offset and title                              
view/as-is make gob! [size: 600x600 offset: :pos text: "My title"]

;- open window with a background color                            
;BUG: does not work!
;view/as-is w: make gob! [size: 600x600 color: 0.0.0]

;- using view/options                                             
view/options make gob! [size: 600x600] [title: "Resizable window" flags: [resize] offset: :pos]

print "Use ALT+F4 to close windows without title bar!"
view/options make gob! [size: 600x600] [title: "Window" flags: [no-title] offset: :pos]
view/options make gob! [size: 600x600] [title: "Window" flags: [no-title no-border] offset: :pos]
view/options/as-is make gob! [size: 600x600 offset: :pos] [title: "Window" flags: [no-title no-border]]

;BUG: fullscreen does not work:/
;view/options/as-is make gob! [size: 600x600] [title: "Fullscreen" flags: [fullscreen]]

;- native widgets                                                 
win: make gob! [size: 600x600 offset: :pos text: "Native widgets"]
fld: make gob! [size: 200x29  offset: 200x200 widget: [field "foooo"] ]
btn: make gob! [size: 200x29  offset: 200x250 widget: [button "hello" 255.0.0]] ;BUG: no color support!
pr1: make gob! [size: 200x9   offset: 200x290 widget: [progress 20 ]]
pr2: make gob! [size: 200x9   offset: 200x300 widget: [progress 50%]]     ;BUG: should accept percent!
txt: make gob! [size: 200x30  offset: 200x320 widget: [text "Some text"]] ;BUG: strange background when over an image
red-image: make image! [300x300 255.0.0.100]
img: make gob! [offset: 150x150 image: red-image]
? img/image ;BUG: shows `none`, but it should be the image!
sl1: make gob! [size: 300x29  offset: 150x450 widget: [slider 0.8]]
sl2: make gob! [size: 29x300  offset: 450x150 widget: [slider 0.2]]
are: make gob! [size: 300x99  offset: 150x30  widget: [area "bla bla bla"]]
dt:  make gob! [size: 200x29  offset: 200x350 widget: [date-time "list box"]]

append win img
append win fld
append win btn
append win [pr1 pr2]
append win txt
append win [sl1 sl2]
append win are
append win dt

; group-box widget...
gb:  make gob! [size: 300x100 offset: 150x480 widget: [group-box "My group"]]
;b3:  make gob! [size: 200x29  offset:  10x10  widget: [button "Inside group"]] ;Should the offset be relative to the parent?
;append gb b3

che: make gob! [size: 200x29  offset: 160x510  widget: [check "Ahoj světe"]]
ra1: make gob! [size: 100x29  offset: 160x540  widget: [radio "Radio 1"]]
ra2: make gob! [size: 100x29  offset: 260x540  widget: [radio "Radio 2"]]
;BUG: there cannot be more than 1 radio buttons group!

append win gb
append win [che ra1 ra2]
view/as-is win
