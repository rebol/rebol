Rebol [
	Title:   "Rebol3 DH test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %dh-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "DH"

===start-group=== "Diffie-Hellman key exchange"

--test-- "DH keys generation"

	p: #{
851F602DAF916C8518FAEDEA6474704DFC3843456824EBF814ADDEB7D1D0352B
D99A652ACDC1BFB77BF74CF0F7351B0B4E1EE1AEF41C58547B76C82A2C58EAAC
CBED85C457B0B60EC6C4FD1112F79CC3E5A0422D820446F6193747318561BF67
C79E915C3277361FBFA587C6DC06FEDE0B7E57FEC0B68F96B3AD651D54264357
0152F91D8F2E22B889B9BEBF7555580C73C062F4FBE1B0F8635387FD2D023292
263BFA7B09A5B80D765539E436FCFC0B5CDCF9CBD75CFFC578E69E3D500635B2
9A4CF92C310C2580A431C146C224E82FAFA36230241B51A953A26DE02E59D341
382C4472FE30A281E1A81FCC8D42F95D7AAC3995370DA5A596076F913AA523D3}
	g: #{02}

	;- Boban and Alice both have G and P values and generates DH keys...

	--assert handle? k-Alice: dh-init g p
	--assert "#(handle! dhm)" = mold k-Alice
	
	--assert handle? k-Boban: dh-init g p
	--assert "#(handle! dhm)" = mold k-Boban

--test-- "DH public key exportation"	

	;- They create their public keys...

	--assert binary? pub-Alice: dh/public k-Alice
	--assert binary? pub-Boban: dh/public k-Boban

	;- and exchange it somehow...

--test-- "DH secret computation"

	;- Now use these to compute shared secret...

	--assert binary? secret-Alice: dh/secret k-Alice pub-Boban
	--assert binary? secret-Boban: dh/secret k-Boban pub-Alice

	;- These keys should be same on both sides

	--assert secret-Alice = secret-Boban

	;- This secret can be used in symmetric encryption algorithms like AES

--test-- "DH keys release"

	;- Once done with the exchange, the DH key must be released!

	--assert true? release k-Alice
	--assert none? dh/public k-Alice
	
	--assert binary? secret-Boban-2: dh/secret k-Boban pub-Alice
	--assert secret-Boban = secret-Boban-2
	--assert true? release k-Boban
	
	;- once released, the secret and public will be unavailable

	--assert none? dh/secret k-Boban pub-Alice
	--assert none? dh/public k-Boban


===end-group===




===start-group=== "Elliptic-curve Diffie-Hellman key exchange"

foreach ecurve system/catalog/elliptic-curves [
	--test-- rejoin ["ECDH (" ecurve ") keys usage"]

		;- Boban and Alice both init key with same curve
		--assert handle? k-Alice: ecdh/init none ecurve
		--assert handle? k-Boban: ecdh/init none ecurve
		--assert ecurve = ecdh/curve k-Alice
		--assert ecurve = ecdh/curve k-Boban
		;- ... exports and exchange public parts of it
		--assert binary? pub-Alice: ecdh/public k-Alice
		--assert binary? pub-Boban: ecdh/public k-Boban
		;- ... and use them to compute shared secret.
		--assert binary? secret-Alice: ecdh/secret k-Alice pub-Boban
		--assert binary? secret-Boban: ecdh/secret k-Boban pub-Alice
		;- These keys should be same on both sides
		--assert secret-Alice = secret-Boban
		;- Once done with the exchange, the ECDH key must be released!
		--assert true? release k-Alice
		--assert none? ecdh/public k-Alice

		;- re-initialization...
		--assert handle? ecdh/init k-Alice ecurve ;- using existing key for a new init
		--assert binary? pub-Alice: ecdh/public k-Alice
		--assert binary? secret-Alice: ecdh/secret k-Alice pub-Boban
		--assert binary? secret-Boban: ecdh/secret k-Boban pub-Alice
		--assert secret-Alice = secret-Boban
		; It is safe not to release (it would be done on GC), but it's better to do it
		--assert true? release k-Alice
		--assert true? release k-Boban

	--test-- rejoin ["ECDSA (" ecurve ") signing"]

		;- Alice generates her key-pair
		k-Alice: ecdh/init none ecurve
		;- wants to sign some data:
		data: {Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.}
		;- computes its hash:
		hash: checksum data 'sha256
		;- and use ECDSA to sign it using her private key
		signature: ecdsa/sign k-Alice hash
		if find [curve25519 curve448] ecurve [
			; these curves are not for signing
			--assert none? signature
			continue
		]
		--assert binary? signature
		; signature is encoded as ASN1
		--assert block? try [decode 'der signature]
		;- can verify if it really works:
		--assert ecdsa/verify k-Alice hash signature

		;- than Alice may send the data with signature, used curve and public key to Boban
		pub-Alice: ecdh/public k-Alice
		;- Boban can verify, that the data are really from Alice
		--assert ecdsa/verify/curve pub-Alice hash signature ecurve

		;@@ https://github.com/Oldes/Rebol-issues/issues/2642
		--assert all [
			binary? sig: ecdsa k-Alice hash ;; /sign refine is mandatory...
			signature != sig                ;; signatures are unique!
			ecdsa/verify k-Alice hash sig
		]

]




===end-group===

~~~end-file~~~