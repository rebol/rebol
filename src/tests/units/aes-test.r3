Rebol [
	Title:   "Rebol3 AES test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %aes-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "AES"
if native? :aes [
;- this tests are only for the now deprecated `aes` native function!
===start-group=== "The Advanced Encryption Standard (AES) test vectors"

--test-- "AES-128"
	
	foreach [key iv plain cipher] [
		#{2b7e151628aed2a6abf7158809cf4f3c}
		#{000102030405060708090A0B0C0D0E0F}
		#{6bc1bee22e409f96e93d7e117393172a}
		#{7649abac8119b246cee98e9b12e9197d}

		#{2b7e151628aed2a6abf7158809cf4f3c}
		#{7649ABAC8119B246CEE98E9B12E9197D}
		#{ae2d8a571e03ac9c9eb76fac45af8e51}
		#{5086cb9b507219ee95db113a917678b2}


		#{2b7e151628aed2a6abf7158809cf4f3c}
		#{5086CB9B507219EE95DB113A917678B2}
		#{30c81c46a35ce411e5fbc1191a0a52ef}
		#{73bed6b8e3c1743b7116e69e22229516}

		#{2b7e151628aed2a6abf7158809cf4f3c}
		#{73BED6B8E3C1743B7116E69E22229516}
		#{f69f2445df4f9b17ad2b417be66c3710}
		#{3ff1caa1681fac09120eca307586e1a7}

;		#{8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b}
;		#{000102030405060708090A0B0C0D0E0F}
;		#{6bc1bee22e409f96e93d7e117393172a}
;		#{4f021db243bc633d7178183a9fa071e8}
;
;		#{8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b}
;		#{4F021DB243BC633D7178183A9FA071E8}
;		#{ae2d8a571e03ac9c9eb76fac45af8e51}
;		#{b4d9ada9ad7dedf4e5e738763f69145a}
;
;		#{8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b}
;		#{B4D9ADA9AD7DEDF4E5E738763F69145A}
;		#{30c81c46a35ce411e5fbc1191a0a52ef}
;		#{571b242012fb7ae07fa9baac3df102e0}
;
;		#{8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b}
;		#{571B242012FB7AE07FA9BAAC3DF102E0}
;		#{f69f2445df4f9b17ad2b417be66c3710}
;		#{08b0e27988598881d920a9e64f5615cd}

		#{603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4}
		#{000102030405060708090A0B0C0D0E0F}
		#{6bc1bee22e409f96e93d7e117393172a}
		#{f58c4c04d6e5f1ba779eabfb5f7bfbd6}

		#{603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4}
		#{F58C4C04D6E5F1BA779EABFB5F7BFBD6}
		#{ae2d8a571e03ac9c9eb76fac45af8e51}
		#{9cfc4e967edb808d679f777bc6702c7d}

		#{603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4}
		#{9CFC4E967EDB808D679F777BC6702C7D}
		#{30c81c46a35ce411e5fbc1191a0a52ef}
		#{39f23369a9d9bacfa530e26304231461}

		#{603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4}
		#{39F23369A9D9BACFA530E26304231461}
		#{f69f2445df4f9b17ad2b417be66c3710}
		#{b2eb05e2c39be9fcda6c19078c6a9d1b}
	][
		--assert handle? k1: aes/key key iv
		--assert cipher = aes/stream k1 plain
		
		--assert handle? k2: aes/key/decrypt key iv
		--assert plain = aes/stream k2 cipher
	]

===end-group===
] ; if ends

~~~end-file~~~