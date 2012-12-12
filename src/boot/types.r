REBOL [
	System: "REBOL [R3] Language Interpreter and Run-time Environment"
	Title: "Datatype definitions"
	Rights: {
		Copyright 2012 REBOL Technologies
		REBOL is a trademark of REBOL Technologies
	}
	License: {
		Licensed under the Apache License, Version 2.0
		See: http://www.apache.org/licenses/LICENSE-2.0
	}
	Purpose: {
		These words define the REBOL datatypes and their related attributes.
		This table generates a variety of C defines and intialization tables.
		During build, when this file is processed, this section is changed to
		hold just the datatype words - the initial entries the word table.
	}
]

;	Fields:
;		Datatype  - name of datatype (generates words)
;		Evaluator - how it is evaluated (by DO)
;		Typeclass - how type actions are dispatched (T_type)
;		Mold      - mold format: - self, + type, * typeclass
;		Form      - form format: above, and f* for special form functions
;		Path      - it supports various path forms (* for same as typeclass)
;		Make      - It can be made with #[datatype] method

;   Datatype    Evaluator   Typeclass   Mold     Form    Path   Make Typesets
;------------------------------------------------------------------------------
	end         end         0           -        -       -      -   -
	unset       self        none        -        -       -      *   -

	;Scalars
	none        self        none        +        +       -      *   -
	logic       self        logic       *        *       -      *   -
	integer     self        integer     *        *       -      -   [number scalar]
	decimal     self        decimal     *        *       -      *   [number scalar]
	percent     self        decimal     *        *       -      *   [number scalar]
	money       self        money       *        *       -      -   scalar
	char        self        char        *        f*      -      -   scalar
	pair        self        pair        *        *       *      *   scalar
	tuple       self        tuple       *        *       *      *   scalar
	time        self        time        *        *       *      *   scalar
	date        self        date        *        *       *      *   -

	;Series
	binary      self        string      +        +       *      *   [series]
	string      self        string      +        f*      *      *   [series string]
	file        self        string      +        f*      file   *   [series string]
	email       self        string      +        f*      *      *   [series string]  
	url         self        string      +        f*      file   *   [series string]
	tag         self        string      +        +       *      *   [series string]  

	bitset      self        bitset      *        *       *      *   -
	image       self        image       +        +       *      *   series
	vector      self        vector      -        -       *      *   series  

	block       self        block       *        f*      *      *   [series block]  
	paren       paren       block       *        f*      *      *   [series block]

	path        path        block       *        *       *      *   [series block path]
	set-path    path        block       *        *       *      *   [series block path]    
	get-path    path        block       *        *       *      *   [series block path]  
	lit-path    lit-path    block       *        *       *      *   [series block path]  

	map         self        map         +        f*      *      *   -  

	datatype    self        datatype    +        f*      -      *   -  
	typeset     self        typeset     +        f*      -      *   -  

	; Order dependent: next few words
	;symbol      invalid     word       *        *       -      -   word  
	word        word        word        +        *       -      -   word  
	set-word    set-word    word        +        *       -      -   word  
	get-word    get-word    word        +        *       -      -   word  
	lit-word    lit-word    word        +        *       -      -   word  
	refinement  self        word        +        *       -      -   word  
	issue       self        word        +        *       -      -   word

	native      function    function    *        -       -      *   function  
	action      function    function    *        -       -      *   function  
;	routine     function    routine     -        -       -      *   function  
	rebcode     function    0           -        -       -      *   function  
	command     function    function    -        -       -      *   function  
;	macro       function    0           -        -       -      -   function  
	op          operator    function    -        -       -      *   function  
	closure     function    function    *        -       -      *   function  
	function    function    function    *        -       -      *   function  

	frame       invalid     frame       -        -       *      -   -  
	object      self        object      *        f*      *      *   object  
	module      self        object      *        f*      *      *   object  
	error       self        object      +        f+      *      *   object  
	task        self        object      +        +       *      *   object  
	port        self        port        object   object  object -	object

	gob         self        gob         *        *       *      *   -  
	event       self        event       *        *       *      *   -  
	handle      self        0           -        -       -      -   -  
	struct      invalid     0           -        -       -      -   -  
	library     invalid     0           -        -       -      -   -  
	utype       self        utype       -        -       -      -   -  

