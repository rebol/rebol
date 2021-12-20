Rebol [
	Title: "Test SQLite extension"
	Date: 10-Nov-2021
	Author: "Oldes"
	File: %test-extension-sqlite.r3
	Version: 1.0.0
]

unless find system/modules 'sqlite [
	unless exists? %sqlite.rebx [
		;@@ this part should be part of the built-in system!
		use [url bin compressed?][
			;-- download extension for correct platform...      
			url: rejoin [
				https://github.com/Siskin-framework/Rebol-SQLite/releases/latest/download/
				%sqlite- system/build/os #"-" system/build/arch %.rebx
			]
			if system/build/os <> 'windows [
				append url %.gz
				compressed?: true
			]

			print [as-green "Downloading:" as-yellow url]
			if "true" = get-env "CI" [
				;enhancing verbosity to try locate source of occasional read errors in CI
				system/schemes/tls/set-verbose 4
				codecs/der/verbose: 4
			]
			bin: read url
			if compressed? [ bin: decode 'GZIP bin ]
			write %sqlite.rebx bin
		]
	]
	sqlite: import %sqlite.rebx
]
? sqlite

print sqlite/info
with sqlite [
	db: open %test.db
	? db
	print info/of db
	trace db 3 ;= SQLITE_TRACE_STMT or SQLITE_TRACE_PROFILE

	exec db {
BEGIN TRANSACTION;
DROP TABLE IF EXISTS Cars;
CREATE TABLE Cars(Id INTEGER PRIMARY KEY, Name TEXT, Price INTEGER);
INSERT INTO "Cars" VALUES(1,'Audi',52642);
INSERT INTO "Cars" VALUES(2,'Mercedes',57127);
INSERT INTO "Cars" VALUES(3,'Skoda',9000);
INSERT INTO "Cars" VALUES(4,'Volvo',29000);
INSERT INTO "Cars" VALUES(5,'Bentley',350000);
INSERT INTO "Cars" VALUES(6,'Citroen',21000);
INSERT INTO "Cars" VALUES(7,'Hummer',41400);
COMMIT;}

	exec db {INSERT INTO "Cars" VALUES(null,'Hummer',null);}
	exec db "SELECT last_insert_rowid();"
	exec db "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"
	exec db "SELECT * FROM Cars ORDER BY name"
	exec db "SELECT hex(randomblob(16));"

	stmt: prepare db "SELECT * FROM Cars ORDER BY name"
	? stmt
	print info/of stmt
	probe step stmt
	finalize stmt
	print info/of stmt

	print "^/Random bin generator..."

	sb: prepare db {select randomblob(16)}
	loop 4 [
		probe step sb
		reset sb
	]
	finalize sb

	print "^/Using prepared statements and input values..."

	stmt: prepare db "SELECT * FROM Cars WHERE Price > ? ORDER BY name"
	probe step/with/rows stmt [20000] 100
	probe step/with/rows stmt [40000.0] 100
	finalize stmt

	stmt: prepare db "SELECT * FROM Cars WHERE Name LIKE :pattern ORDER BY name"
	probe step/with/rows stmt ["H%"] 100
	; testing an input where index is not at its head
	name: next "xA_di"
	probe step/with/rows stmt reduce [name] 100 
	finalize stmt
	
	print "Shutting down.."
	print info
	close db
	probe shutdown
	probe shutdown ; no-op
	probe initialize
	print info
	print "SQLite tests done."
]

delete %test.db
