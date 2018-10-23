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
	--assert "#[handle! dh]" = mold k-Alice
	
	--assert handle? k-Boban: dh-init g p
	--assert "#[handle! dh]" = mold k-Boban

--test-- "DH public key exportion"	

	;- They exchange their public keys...

	--assert binary? pub-Alice: dh/public k-Alice
	--assert binary? pub-Boban: dh/public k-Boban

--test-- "DH secret computation"

	;- And use them to compute shared secret...

	--assert binary? secret-Alice: dh/secret k-Alice pub-Boban
	--assert binary? secret-Boban: dh/secret k-Boban pub-Alice

	;- These keys should be same on both sides

	--assert secret-Alice = secret-Boban

	;- This secret can be used in symmetric encryption algorithms like AES

--test-- "DH keys release"

	;- Once done with the exchange, the DH key must be released!

	--assert handle? dh/release k-Alice
	--assert none? dh/public k-Alice
	
	;- the release may be also used while getting the secret
	
	--assert binary? secret-Boban-2: dh/secret/release k-Boban pub-Alice
	--assert secret-Boban = secret-Boban-2
	
	;- once released, the secret and public will be unavailable

	--assert none? dh/secret k-Boban pub-Alice
	--assert none? dh/public k-Boban

--test-- "DH handle re-initialization"

	;- released handle can be reused with new params

	--assert handle? dh-init/into g p k-Alice
	--assert binary? dh/public k-Alice

===end-group===

~~~end-file~~~