REBOL [
    version: 0.12.0
    title: "TLS Protocol"
    name: 'tls
    date: 30-Nov-2025
    file: %tls.reb
    author: "Oldes"
    Yype: 'module
    License: MIT
    Home: https://github.com/Oldes/Rebol-TLS
]
try [do "_: #(none)"]
comment "## Include: %tls-context.reb"
comment {## Title:   "TLS Context Object Definition"}
TLS-context: context [
    in: binary 16104
    out: binary 16104
    bin: binary 64
    tcp-port:
    tls-port:
    encrypt-port:
    decrypt-port:
    sha256-port:
    sha384-port:
    sha-port:
    md5-port: none
    legacy-version: 771
    version: none
    TLS13?: none
    handshake?: true
    port-data: make binary! 32000
    rest: make binary! 8
    reading?: false
    server?: false
    protocol: none
    state: 'lookup
    state-prev: none
    error:
    critical-error:
    cipher-suite: none
    cipher-spec-set: 0
    ecdh-group: none
    key-method:
    hash-type:
    crypt-method: none
    is-aead?: false
    aad-length:
    tag-length:
    IV-size:
    IV-size-dynamic:
    mac-size:
    crypt-size:
    block-size: 0
    locale-hs-IV:
    locale-ap-IV:
    locale-hs-key:
    locale-ap-key:
    locale-mac:
    locale-random:
    locale-hs-secret:
    locale-ap-secret:
    remote-hs-IV:
    remote-ap-IV:
    remote-hs-key:
    remote-ap-key:
    remote-mac:
    remote-random:
    remote-hs-secret:
    remote-ap-secret:
    finished-hash:
    handshake-secret:
    verify-data:
    client-verify-data:
    dh-key:
    aead: none
    session-id: none
    server-certs: copy []
    extensions: copy []
    context-messages: []
    seq-read: 0
    seq-write: 0
    pre-secret:
    master-secret:
    certificate:
    pub-key:
    pub-exp:
    key-data:
    hello-retry-request:
    none
]
derived-secrets: make map! []
zero-keys: make map! []
empty-hash: make map! []
TLS-init-context: func [
    ctx [object!]
] [
    ctx/seq-read: ctx/seq-write: 0
    ctx/protocol: ctx/state: ctx/state-prev: none
    ctx/cipher-spec-set: 0
    clear ctx/server-certs
]
TLS-init-cipher-suite: func [
    ctx [object!]
    /local suite key-method cipher
] [
    cipher: ctx/cipher-suite
    suite: *Cipher-suite/name :cipher
    unless suite [
        log-error ["Unknown cipher suite:" enbase suite 16]
        return false
    ]
    unless find suported-cipher-suites suite [
        unless ctx/server? [log-error ["Server requests" suite "cipher suite!"]]
        return false
    ]
    log-info ["Init TLS Cipher-suite:^[[35m" suite "^[[22m" skip to binary! cipher 6]
    parse form suite [
        opt "TLS_"
        opt [copy key-method to "_WITH_" 6 skip (ctx/key-method: to word! key-method)]
        copy cipher [
            "CHACHA20-POLY1305" (ctx/crypt-size: 32 ctx/IV-size: 12 ctx/block-size: 16)
            | "AES-256-GCM" (ctx/crypt-size: 32 ctx/IV-size: 4 ctx/IV-size-dynamic: 8 ctx/tag-length: ctx/block-size: 16 ctx/aad-length: 13)
            | "AES-128-GCM" (ctx/crypt-size: 16 ctx/IV-size: 4 ctx/IV-size-dynamic: 8 ctx/tag-length: ctx/block-size: 16 ctx/aad-length: 13)
            | "AES-128-CBC" (ctx/crypt-size: 16 ctx/IV-size: 16 ctx/block-size: 16)
            | "AES-256-CBC" (ctx/crypt-size: 32 ctx/IV-size: 16 ctx/block-size: 16)
            | "RC4-128" (ctx/crypt-size: 16 ctx/IV-size: 0 ctx/block-size: none)
            | "NULL" (ctx/crypt-size: 0 ctx/IV-size: 0 ctx/block-size: none)
        ] #"_" [
            "SHA384" end (ctx/hash-type: 'SHA384 ctx/mac-size: 48)
            | "SHA256" end (ctx/hash-type: 'SHA256 ctx/mac-size: 32)
            | "SHA" end (ctx/hash-type: 'SHA1 ctx/mac-size: 20)
            | "SHA512" end (ctx/hash-type: 'SHA512 ctx/mac-size: 64)
            | "MD5" end (ctx/hash-type: 'MD5 ctx/mac-size: 16)
            | "NULL" end (ctx/hash-type: none ctx/mac-size: 0)
        ]
        (
            ctx/crypt-method: to word! cipher
            ctx/is-aead?: to logic! find [AES-128-GCM AES-256-GCM CHACHA20-POLY1305] ctx/crypt-method
            ctx/sha-port: open join checksum:// ctx/hash-type
            log-more [
                "Key:^[[1m" ctx/key-method
                "^[[22mcrypt:^[[1m" ctx/crypt-method
                "^[[22msize:^[[1m" ctx/crypt-size
                "^[[22mIV:^[[1m" ctx/IV-size
            ]
        )
    ]
]
comment "-- End of:  %tls-context.reb"
comment "## Include: %tls-constants.reb"
comment {## Title:   "TLS Protocol Constants and Enumerations"}
*Protocol-type: enum [
    CHANGE_CIPHER_SPEC: 20
    ALERT: 21
    HANDSHAKE: 22
    APPLICATION: 23
] 'TLS-protocol-type
*Protocol-version: enum [
    SSLv3: 768
    TLS1.0: 769
    TLS1.1: 770
    TLS1.2: 771
    TLS1.3: 772
] 'TLS-Protocol-version
*Handshake: enum [
    HELLO_REQUEST: 0
    CLIENT_HELLO: 1
    SERVER_HELLO: 2
    NEW_SESSION_TICKET: 4
    ENCRYPTED_EXTENSIONS: 8
    CERTIFICATE: 11
    SERVER_KEY_EXCHANGE: 12
    CERTIFICATE_REQUEST: 13
    SERVER_HELLO_DONE: 14
    CERTIFICATE_VERIFY: 15
    CLIENT_KEY_EXCHANGE: 16
    FINISHED: 20
    KEY_UPDATE: 24
] 'TLS-Handshake-type
*Cipher-suite: enum [
    TLS_AES-128-GCM_SHA256: 4865
    TLS_AES-256-GCM_SHA384: 4866
    TLS_CHACHA20-POLY1305_SHA256: 4867
    TLS_AES-128-CCM_SHA256: 4868
    TLS_AES-128-CCM_8_SHA256: 4869
    TLS_ECDHE_RSA_WITH_CHACHA20-POLY1305_SHA256: 52392
    TLS_ECDHE_ECDSA_WITH_CHACHA20-POLY1305_SHA256: 52393
    TLS_ECDHE_RSA_WITH_AES-256-CBC_SHA384: 49192
    TLS_ECDHE_RSA_WITH_AES-128-GCM_SHA256: 49199
    TLS_ECDHE_RSA_WITH_AES-256-GCM_SHA384: 49200
    TLS_ECDHE_ECDSA_WITH_AES-128-GCM_SHA256: 49195
    TLS_ECDHE_ECDSA_WITH_AES-256-GCM_SHA384: 49196
    TLS_ECDHE_RSA_WITH_AES-128-CBC_SHA256: 49191
    TLS_ECDHE_ECDSA_WITH_AES-256-CBC_SHA384: 49188
    TLS_ECDHE_ECDSA_WITH_AES-128-CBC_SHA256: 49187
    TLS_ECDHE_RSA_WITH_AES-128-CBC_SHA: 49171
    TLS_ECDHE_ECDSA_WITH_AES-128-CBC_SHA: 49161
    TLS_ECDHE_RSA_WITH_AES-256-CBC_SHA: 49172
    TLS_ECDHE_ECDSA_WITH_AES-256-CBC_SHA: 49162
    TLS_ECDH_ECDSA_WITH_AES-256-GCM_SHA384: 49198
    TLS_DHE_RSA_WITH_AES-128-CCM: 49310
    TLS_ECDHE_ECDSA_WITH_AES_256_CCM: 49325
    TLS_RSA_WITH_AES-128-GCM_SHA256: 156
    TLS_RSA_WITH_NULL_MD5: 1
    TLS_RSA_WITH_NULL_SHA: 2
    TLS_RSA_WITH_NULL_SHA256: 59
    TLS_RSA_WITH_RC4-128_MD5: 4
    TLS_RSA_WITH_RC4-128_SHA: 5
    TLS_RSA_WITH_3DES-EDE-CBC_SHA: 10
    TLS_RSA_WITH_AES-128-CBC_SHA: 47
    TLS_RSA_WITH_AES-256-CBC_SHA: 53
    TLS_RSA_WITH_AES-128-CBC_SHA256: 60
    TLS_RSA_WITH_AES-256-CBC_SHA256: 61
    TLS_DH_DSS_WITH_3DES-EDE-CBC_SHA: 13
    TLS_DH_RSA_WITH_3DES-EDE-CBC_SHA: 16
    TLS_DHE_DSS_WITH_3DES-EDE-CBC_SHA: 19
    TLS_DHE_RSA_WITH_3DES-EDE-CBC_SHA: 22
    TLS_DH_DSS_WITH_AES-128-CBC_SHA: 48
    TLS_DH_RSA_WITH_AES-128-CBC_SHA: 49
    TLS_DHE_DSS_WITH_AES-128-CBC_SHA: 50
    TLS_DHE_RSA_WITH_AES-128-CBC_SHA: 51
    TLS_DH_DSS_WITH_AES-256-CBC_SHA: 54
    TLS_DH_RSA_WITH_AES-256-CBC_SHA: 55
    TLS_DHE_DSS_WITH_AES-256-CBC_SHA: 56
    TLS_DHE_RSA_WITH_AES-256-CBC_SHA: 57
    TLS_DH_DSS_WITH_AES-128-CBC_SHA256: 62
    TLS_DH_RSA_WITH_AES-128-CBC_SHA256: 63
    TLS_DHE_DSS_WITH_AES-128-CBC_SHA256: 64
    TLS_DHE_RSA_WITH_AES-128-CBC_SHA256: 103
    TLS_DH_DSS_WITH_AES-256-CBC_SHA256: 104
    TLS_DH_RSA_WITH_AES-256-CBC_SHA256: 105
    TLS_DHE_DSS_WITH_AES-256-CBC_SHA256: 106
    TLS_DHE_RSA_WITH_AES-256-CBC_SHA256: 107
    TLS_DH_anon_WITH_RC4-128_MD5: 24
    TLS_DH_anon_WITH_3DES-EDE-CBC_SHA: 27
    TLS_DH_anon_WITH_AES-128-CBC_SHA: 52
    TLS_DH_anon_WITH_AES-256-CBC_SHA: 58
    TLS_DH_anon_WITH_AES-128-CBC_SHA256: 108
    TLS_DH_anon_WITH_AES-256-CBC_SHA256: 109
    PSUEDO-CIPHER-SUITE: 255
] 'TLS-Cipher-suite
*EllipticCurves: enum [
    secp192r1: 19
    secp224k1: 20
    secp224r1: 21
    secp256k1: 22
    secp256r1: 23
    secp384r1: 24
    secp521r1: 25
    bp256r1: 26
    bp384r1: 27
    bp512r1: 28
    curve25519: 29
    curve448: 30
] 'EllipticCurves
*HashAlgorithm: enum [
    none: 0
    md5: 1
    sha1: 2
    sha224: 3
    sha256: 4
    sha384: 5
    sha512: 6
    md5_sha1: 255
] 'TLSHashAlgorithm
*SignatureScheme: enum [
    rsa_pkcs1_sha256: 1025
    rsa_pkcs1_sha384: 1281
    rsa_pkcs1_sha512: 1537
    ecdsa_secp256r1_sha256: 1027
    ecdsa_secp384r1_sha384: 1283
    ecdsa_secp521r1_sha512: 1539
    rsa_pss_rsae_sha256: 2052
    rsa_pss_rsae_sha384: 2053
    rsa_pss_rsae_sha512: 2054
    ed25519: 2055
    ed448: 2056
    rsa_pss_pss_sha256: 2057
    rsa_pss_pss_sha384: 2058
    rsa_pss_pss_sha512: 2059
    rsa_pkcs1_sha1: 513
    ecdsa_sha1: 515
] 'TLSSignatureScheme
*ClientCertificateType: enum [
    rsa_sign: 1
    dss_sign: 2
    rsa_fixed_dh: 3
    dss_fixed_dh: 4
    rsa_ephemeral_dh_RESERVED: 5
    dss_ephemeral_dh_RESERVED: 6
    fortezza_dms_RESERVED: 20
    ecdsa_sign: 64
    rsa_fixed_ecdh: 65
    ecdsa_fixed_ecdh: 66
] 'TLSClientCertificateType
*Alert-level: enum [
    WARNING: 1
    FATAL: 2
] 'TLS-Alert-level
*Alert: enum [
    Close_notify: 0
    Unexpected_message: 10
    Bad_record_MAC: 20
    Decryption_failed: 21
    Record_overflow: 22
    Decompression_failure: 30
    Handshake_failure: 40
    No_certificate: 41
    Bad_certificate: 42
    Unsupported_certificate: 43
    Certificate_revoked: 44
    Certificate_expired: 45
    Certificate_unknown: 46
    Illegal_parameter: 47
    Unknown_CA: 48
    Access_denied: 49
    Decode_error: 50
    Decrypt_error: 51
    Export_restriction: 60
    Protocol_version: 70
    Insufficient_security: 71
    Internal_error: 80
    User_cancelled: 90
    No_renegotiation: 100
    Unsupported_extension: 110
] 'TLS-Alert
*TLS-Extension: enum [
    server_name: 0
    max_fragment_length: 1
    status_request: 5
    supported_groups: 10
    supported_point_formats: 11
    signature_algorithms: 13
    use_srtp: 14
    heartbeat: 15
    application_layer_protocol_negotiation: 16
    signed_certificate_timestamp: 18
    client_certificate_type: 19
    server_certificate_type: 20
    padding: 21
    encrypt_then_MAC: 22
    extended_master_secret: 23
    compress_certificate: 27
    session_ticket: 35
    pre_shared_key: 41
    early_data: 42
    supported_versions: 43
    cookie: 44
    psk_key_exchange_modes: 45
    certificate_authorities: 47
    oid_filters: 48
    post_handshake_auth: 49
    signature_algorithms_cert: 50
    key_share: 51
    encrypted_client_hello: 65037
    renegotiation_info: 65281
] 'TLS-Extension
*TLS-CertCompression: enum [
    zlib: 1
    brotli: 2
] 'TLS-CertCompression
hash-len: make map! [sha384: 48 sha256: 32]
signature-hash-methods: make map! [
    ecdsa_secp256r1_sha256: sha256
    ecdsa_secp384r1_sha384: sha384
    ecdsa_secp521r1_sha512: sha512
    ecdsa_brainpoolP256r1tls13_sha256: sha256
    ecdsa_brainpoolP384r1tls13_sha384: sha384
    ecdsa_brainpoolP512r1tls13_sha512: sha512
    rsa_pss_pss_sha256: sha256
    rsa_pss_pss_sha384: sha384
    rsa_pss_pss_sha512: sha512
    rsa_pss_rsae_sha256: sha256
    rsa_pss_rsae_sha384: sha384
    rsa_pss_rsae_sha512: sha512
    rsa_pkcs1_sha256: sha256
    rsa_pkcs1_sha384: sha384
    rsa_pkcs1_sha512: sha512
]
HRR-magic: #{CF21AD74E59A6111BE1D8C021E65B891C2A211167ABB8C5E079E09E2C8A8339C}
server-certificate-verify-context: rejoin [
    #{
2020202020202020202020202020202020202020202020202020202020202020
2020202020202020202020202020202020202020202020202020202020202020
}
    "TLS 1.3, server CertificateVerify^@"
]
comment "-- End of:  %tls-constants.reb"
comment "## Include: %tls-utils.reb"
comment {## Title:   "TLS Utility Functions"}
cause-TLS-error: func [
    name [word!]
    /local message
] [
    message: replace/all form name #"_" #" "
    log-error message
    do make error! [type: 'Access id: 'Protocol arg1: message]
]
change-state: function [
    ctx [object!]
    new-state [word!]
] [
    ctx/state-prev: ctx/state
    if ctx/state <> new-state [
        log-more ["New state:^[[33m" new-state "^[[22mfrom:" ctx/state]
        ctx/state: new-state
    ]
]
assert-prev-state: function [
    ctx [object!]
    legal-states [block!]
] [
    unless find legal-states ctx/state-prev [
        log-error ["State" ctx/state "is not expected after" ctx/state-prev]
        cause-TLS-error 'Internal_error
    ]
]
dispatch-event: function [
    event [word!]
    target [port!]
] [
    log-debug ["Send-event:^[[1m" pad event 8 "^[[m->" target/spec/ref]
    either all [
        port? target/parent
        function? :target/parent/awake
    ] [
        target/parent/awake make event! [type: event port: target]
    ] [
        insert system/ports/system make event! [type: event port: target]
    ]
]
_log-error: func [msg] [
    sys/log/error 'TLS msg
]
_log-info: func [msg] [
    if block? msg [msg: reform msg]
    print rejoin [" ^[[1;33m[TLS] ^[[36m" msg "^[[0m"]
]
_log-more: func [msg] [
    if block? msg [msg: reform msg]
    print rejoin [" ^[[33m[TLS] ^[[0;36m" msg "^[[0m"]
]
_log-debug: func [msg] [
    if block? msg [msg: reform msg]
    print rejoin [" ^[[33m[TLS] ^[[0;32m" msg "^[[0m"]
]
_log-----: :print-hline
log-error: log-info: log-more: log-debug: log-----: none
tls-verbosity: func [
    {Sets the log verbosity level for TLS-related messages and debug output.}
    verbose [integer!] "Verbosity level"
] [
    log-error: log-info: log-more: log-debug: log-----: none
    case/all [
        verbose >= 0 [log-error: :_log-error]
        verbose >= 1 [log-info: :_log-info]
        verbose >= 2 [log-more: :_log-more
        log-----: :_log-----]
        verbose >= 3 [log-debug: :_log-debug]
    ]
]
log-error: :_log-error
comment "-- End of:  %tls-utils.reb"
comment "## Include: %tls-crypto.reb"
comment {## Title:   "TLS Cryptographic Functions"}
HKDF-Extract: func [
    hash [word!]
    salt [binary!]
    ikm [binary!]
    return: [binary!]
] [
    checksum/with ikm hash salt
]
HKDF-Expand: func [
    hash [word!]
    prk [binary!]
    data [binary!]
    length [integer!]
    /label
    context [string!]
    return: [binary!]
    /local tmp i out
] [
    if label [
        label: ajoin ["tls13 " context]
        tmp: make binary! 64
        binary/write tmp [
            UI16 :length
            UI8BYTES :label
            UI8BYTES :data
        ]
        data: tmp
    ]
    out: make binary! length
    tmp: #{} i: 0
    while [length > length? out] [
        ++ i
        tmp: checksum/with rejoin [tmp data i] hash prk
        append out tmp
    ]
    head clear atz out length
]
prf: function [
    hash [word!]
    label [string! binary!]
    seed [binary!]
    secret [binary!]
    output-length [integer!]
] [
    log-more ["PRF" hash mold label "len:" output-length]
    seed: join to binary! label seed
    p-sha256: make binary! output-length
    a: seed
    while [output-length >= length? p-sha256] [
        a: checksum/with a :hash :secret
        append p-sha256 checksum/with append copy :a :seed :hash :secret
    ]
    clear at p-sha256 (1 + output-length)
    p-sha256
]
TLS-key-expansion: func [
    ctx [object!]
    /local rnd1 rnd2 key-expansion sha
    derived_secret empty_hash hello_hash early_secret
    handshake_secret client_secret server_secret
] [
    with ctx [
        sha: ctx/hash-type
        log-debug ["===================TLS-key-expansion" sha]
        either TLS13? [
            unless derived_secret: derived-secrets/:sha [
                empty-hash/:sha: checksum #{} :sha
                zero-keys/:sha: append/dup copy #{} 0 :mac-size
                early_secret: HKDF-Extract :sha #{} zero-keys/:sha
                derived-secrets/:sha:
                derived_secret: HKDF-Expand/label :sha early_secret empty-hash/:sha mac-size "derived"
            ]
            hello_hash: get-transcript-hash ctx _
            handshake-secret: HKDF-Extract :sha derived_secret :pre-secret
            either server? [
                locale-hs-secret: HKDF-Expand/label :sha handshake-secret hello_hash mac-size "s hs traffic"
                remote-hs-secret: HKDF-Expand/label :sha handshake-secret hello_hash mac-size "c hs traffic"
            ] [
                locale-hs-secret: HKDF-Expand/label :sha handshake-secret hello_hash mac-size "c hs traffic"
                remote-hs-secret: HKDF-Expand/label :sha handshake-secret hello_hash mac-size "s hs traffic"
            ]
            locale-hs-key: HKDF-Expand/label :sha locale-hs-secret #{} crypt-size "key"
            remote-hs-key: HKDF-Expand/label :sha remote-hs-secret #{} crypt-size "key"
            locale-hs-IV: HKDF-Expand/label :sha locale-hs-secret #{} IV-size + IV-size-dynamic "iv"
            remote-hs-IV: HKDF-Expand/label :sha remote-hs-secret #{} IV-size + IV-size-dynamic "iv"
            cipher-spec-set: 2
            aad-length: 5
        ] [
            either server? [
                rnd1: append copy ctx/remote-random ctx/locale-random
                rnd2: append copy ctx/locale-random ctx/remote-random
            ] [
                rnd2: append copy ctx/remote-random ctx/locale-random
                rnd1: append copy ctx/locale-random ctx/remote-random
            ]
            master-secret: prf :sha "master secret" rnd1 pre-secret 48
            key-expansion: prf :sha "key expansion" rnd2 master-secret (mac-size + crypt-size + iv-size) * 2
            either server? [
                unless is-aead? [
                    remote-mac: take/part key-expansion mac-size
                    locale-mac: take/part key-expansion mac-size
                ]
                remote-hs-key: take/part key-expansion crypt-size
                locale-hs-key: take/part key-expansion crypt-size
                remote-hs-IV: take/part key-expansion iv-size
                locale-hs-IV: take/part key-expansion iv-size
            ] [
                unless is-aead? [
                    locale-mac: take/part key-expansion mac-size
                    remote-mac: take/part key-expansion mac-size
                ]
                locale-hs-key: take/part key-expansion crypt-size
                remote-hs-key: take/part key-expansion crypt-size
                locale-hs-IV: take/part key-expansion iv-size
                remote-hs-IV: take/part key-expansion iv-size
            ]
            if IV-size-dynamic > 0 [
                append/dup locale-hs-IV 0 IV-size-dynamic
                append/dup remote-hs-IV 0 IV-size-dynamic
            ]
        ]
        log-more ["locale-IV: ^[[32m" locale-hs-IV]
        log-more ["locale-mac:^[[32m" locale-mac]
        log-more ["locale-key:^[[32m" locale-hs-key]
        log-more ["remote-IV: ^[[32m" remote-hs-IV]
        log-more ["remote-mac:^[[32m" remote-mac]
        log-more ["remote-key:^[[32m" remote-hs-key]
        encrypt-port: open [
            scheme: 'crypt
            algorithm: :crypt-method
            init-vector: :locale-hs-IV
            key: :locale-hs-key
        ]
        decrypt-port: open [
            scheme: 'crypt
            direction: 'decrypt
            algorithm: :crypt-method
            init-vector: :remote-hs-IV
            key: :remote-hs-key
        ]
        modify encrypt-port 'aad-length :aad-length
        modify decrypt-port 'aad-length :aad-length
        if tag-length > 0 [
            modify decrypt-port 'tag-length :tag-length
            modify encrypt-port 'tag-length :tag-length
        ]
        pre-secret: locale-hs-key: remote-hs-key: none
        seq-write: seq-read: 0
    ]
]
wrap-record: func [
    ctx [object!]
    plaintext [binary!]
    type [integer!]
    /locale
    length
    nonce
    seq-bytes
    aad
] [with ctx [
    plaintext: append copy plaintext type
    length: tag-length + length? plaintext
    binary/write aad: clear #{} [
        UI8 23
        UI16 :legacy-version
        UI16 :length
    ]
    if crypt-method != 'CHACHA20-POLY1305 [
        nonce: append clear #{} either type = 23 [locale-ap-IV] [locale-hs-IV]
        seq-bytes: #{000000000000000000000000}
        binary/write seq-bytes [ATz 4 UI64BE :seq-write]
        nonce: nonce xor seq-bytes
        modify encrypt-port 'iv nonce
        write encrypt-port aad
    ]
    ++ seq-write
    read update write encrypt-port :plaintext
]]
encrypt-tls-record: function [
    ctx [object!]
    content [binary!]
    /type
    msg-type [integer!] "application data is default"
] [
    log-debug ["--encrypt-tls-record--" as-red ctx/seq-write]
    msg-type: any [msg-type 23]
    with ctx [
        length: length? content
        binary/write bin [
            UI64 :seq-write
            UI8 :msg-type
            UI16 :legacy-version
            UI16 :length
        ]
        either is-aead? [
            aad: bin/buffer
            either crypt-method = 'CHACHA20-POLY1305 [
                write encrypt-port :aad
                encrypted: read update write encrypt-port content
            ] [
                binary/write locale-hs-IV [ATz :IV-size UI64be :seq-write]
                log-more ["locale-IV:   ^[[32m" locale-hs-IV]
                log-more ["AAD:        ^[[32m" bin/buffer]
                modify encrypt-port 'iv locale-hs-IV
                write encrypt-port :aad
                encrypted: read update write encrypt-port content
                if IV-size-dynamic > 0 [
                    insert encrypted copy/part skip locale-hs-IV :IV-size :IV-size-dynamic
                ]
            ]
        ] [
            binary/write clear locale-hs-IV [RANDOM-BYTES :block-size]
            modify encrypt-port 'init-vector locale-hs-IV
            log-more ["locale-IV: ^[[32m" locale-hs-IV]
            log-more ["locale-mac:^[[32m" locale-mac]
            log-more ["hash-type:^[[32m" hash-type]
            binary/write bin content
            MAC: checksum/with bin/buffer ctx/hash-type ctx/locale-mac
            len: length? append content MAC
            if block-size [
                padding: block-size - ((len + 1) % block-size)
                insert/dup tail content padding padding + 1
            ]
            encrypted: read update write encrypt-port content
            insert encrypted locale-hs-IV
        ]
        binary/init bin 0
        ++ seq-write
    ]
    encrypted
]
decrypt-tls-record: func [
    ctx [object!]
    data [binary!]
    type [integer!]
    /local
    length
    nonce
    seq-bytes
    mac
    tag
    aad
] [
    log-more ["Decrypt record of type:^[[1m" type]
    aad: clear #{}
    with ctx [
        either TLS13? [
            length: length? data
            binary/write aad [
                UI8 :type
                UI16 :legacy-version
                UI16 :length
            ]
            nonce: append clear #{} any [remote-ap-IV remote-hs-IV]
            seq-bytes: #{000000000000000000000000}
            binary/write seq-bytes [atz 4 ui64be :seq-read]
            nonce: nonce xor seq-bytes
            modify decrypt-port 'iv nonce
            write decrypt-port :aad
            mac: take/last/part data 16
            data: read write decrypt-port data
            unless equal? mac take decrypt-port [
                log-error "Failed to validate MAC after decryption!"
                cause-TLS-error 'Bad_record_MAC
            ]
            trim/tail data
        ] [
            binary/write aad [
                UI64 :seq-read
                UI8 :type
                UI16 :legacy-version
            ]
            either is-aead? [
                if crypt-method <> 'CHACHA20-POLY1305 [
                    change/part skip remote-hs-IV 4 take/part data 8 8
                    modify decrypt-port 'iv remote-hs-IV
                    log-more ["Remote IV:^[[32m" remote-hs-IV]
                ]
                binary/write tail aad reduce ['UI16 (length? data) - 16]
                write decrypt-port aad
                log-more ["AAD:      ^[[32m" aad]
                mac: take/last/part data 16
                data: read write decrypt-port data
                unless equal? mac tag: take decrypt-port [
                    log-debug "Failed to validate MAC after decryption!"
                    log-debug ["Expected:" mac]
                    log-debug ["Counted: " tag]
                    critical-error: 'Bad_record_MAC
                ]
            ] [
                if block-size [
                    remote-hs-IV: take/part data block-size
                ]
                modify decrypt-port 'init-vector remote-hs-IV
                data: read update write decrypt-port :data
                if block-size [
                    clear skip tail data (-1 - (to integer! last data))
                    mac: take/last/part data mac-size
                    binary/write tail aad [UI16BYTES :data]
                    if mac <> checksum/with aad hash-type remote-mac [
                        critical-error: 'Bad_record_MAC
                    ]
                    unset 'remote-hs-IV
                ]
            ]
            binary/init bin 0
        ]
        ++ seq-read
    ]
    unless data [critical-error: 'Bad_record_MAC]
    data
]
derive-application-traffic-secrets: func [
    ctx [object!]
    /local
    derived-secret
    finished-hash
    finished-key
] [with ctx [
    either TLS13? [
        log-info "Derive application traffic secrets"
        finished-hash: get-transcript-hash ctx _
        finished-key: HKDF-Expand/label hash-type either server? [remote-hs-secret] [locale-hs-secret] #{} mac-size "finished"
        verify-data: checksum/with finished-hash hash-type finished-key
        derived-secret: HKDF-Expand/label hash-type handshake-secret empty-hash/:hash-type mac-size "derived"
        master-secret: HKDF-Extract hash-type :derived-secret zero-keys/:hash-type
        log-more ["Master Secret:^[[2m" master-secret]
        log-more ["Local  Handshake Secret:^[[2m" locale-hs-secret]
        log-more ["Remote Handshake Secret:^[[2m" remote-hs-secret]
        either server? [
            locale-ap-secret: HKDF-Expand/label hash-type master-secret :finished-hash mac-size "s ap traffic"
            remote-ap-secret: HKDF-Expand/label hash-type master-secret :finished-hash mac-size "c ap traffic"
        ] [
            locale-ap-secret: HKDF-Expand/label hash-type master-secret :finished-hash mac-size "c ap traffic"
            remote-ap-secret: HKDF-Expand/label hash-type master-secret :finished-hash mac-size "s ap traffic"
        ]
        log-more ["Local  Traffic   Secret:^[[2m" locale-ap-secret]
        log-more ["Remote Traffic   Secret:^[[2m" remote-ap-secret]
        locale-ap-key: HKDF-Expand/label hash-type locale-ap-secret #{} crypt-size "key"
        remote-ap-key: HKDF-Expand/label hash-type remote-ap-secret #{} crypt-size "key"
        locale-ap-IV: HKDF-Expand/label hash-type locale-ap-secret #{} IV-size + IV-size-dynamic "iv"
        remote-ap-IV: HKDF-Expand/label hash-type remote-ap-secret #{} IV-size + IV-size-dynamic "iv"
        log-more ["Local  App IV :^[[2m" locale-ap-IV]
        log-more ["Remote App IV :^[[2m" remote-ap-IV]
        log-more ["Local  App Key:^[[2m" locale-ap-key]
        log-more ["Remote App Key:^[[2m" remote-ap-key]
        either server? [
            switch-to-app-encrypt ctx
        ] [switch-to-app-decrypt ctx]
        reading?: server?
    ] [
        verify-data: prf hash-type either server? ["client finished"] ["server finished"] :finished-hash master-secret 12
    ]
]]
switch-to-app-encrypt: func [
    ctx [object!]
] [
    log-info "Switch to application encrypt for traffic"
    with ctx [
        close encrypt-port
        encrypt-port: open [
            scheme: 'crypt
            algorithm: :crypt-method
            init-vector: :locale-ap-IV
            key: :locale-ap-key
        ]
        modify encrypt-port 'aad-length :aad-length
        if tag-length > 0 [
            modify encrypt-port 'tag-length :tag-length
        ]
        seq-write: 0
    ]
]
switch-to-app-decrypt: func [
    ctx [object!]
] [
    log-info "Switch to application decrypt for traffic"
    with ctx [
        close decrypt-port
        decrypt-port: open [
            scheme: 'crypt
            direction: 'decrypt
            algorithm: :crypt-method
            init-vector: :remote-ap-IV
            key: :remote-ap-key
        ]
        modify decrypt-port 'aad-length :aad-length
        if tag-length > 0 [
            modify decrypt-port 'tag-length :tag-length
        ]
        seq-read: 0
    ]
]
comment "-- End of:  %tls-crypto.reb"
comment "## Include: %tls-certificate.reb"
comment {## Title:   "TLS Certificate Functions"}
decode-certificates: function [
    ctx [object!]
    msg [binary!]
] [
    assert-prev-state ctx [SERVER_HELLO CLIENT_HELLO ENCRYPTED_EXTENSIONS]
    msg: binary msg
    if ctx/TLS13? [
        cert-context: binary/read msg 'UI8BYTES
    ]
    len: binary/read msg 'UI24
    if len != length? msg/buffer [
        log-error ["Improper certificate list end?" len "<>" length? msg/buffer]
        cause-TLS-error 'Handshake_failure
    ]
    while [3 < length? msg/buffer] [
        cert: binary/read msg 'UI24BYTES
        if ctx/TLS13? [
            cert-extensions: binary/read msg 'UI16BYTES
        ]
        append ctx/server-certs cert: attempt [decode 'CRT cert]
        log-more ["Certificate subject:^[[1m" mold/only/flat cert/subject]
    ]
    try/with [
        key: ctx/server-certs/1/public-key
        switch key/1 [
            ecPublicKey [
                ctx/pub-key: key/3
                ctx/pub-exp: key/2
                if 0 == ctx/pub-key/1 [remove ctx/pub-key]
            ]
            rsaEncryption [
                ctx/pub-key: key/2/1
                ctx/pub-exp: key/2/2
            ]
        ]
    ] [
        log-error "Missing public key in certifiate"
        cause-TLS-error 'Bad_certificate
    ]
]
decode-certificate-verify: function [
    ctx [object!]
    msg [object! binary!]
] [
    binary/read msg [
        signature-type: UI16
        signature: UI16BYTES
    ]
    log-debug ["Verify certificate using type:^[[1m" *SignatureScheme/name signature-type]
    if signature-type == 2052 [
        either system/version < 3.19.7 [
            log-error {Current Rebol version is not able to validate this certificate!}
        ] [
            to-sign: rejoin [
                server-certificate-verify-context
                get-transcript-hash ctx 'CERTIFICATE
            ]
            key: rsa-init ctx/pub-key ctx/pub-exp
            unless rsa/verify/pss :key :to-sign :signature [
                log-error "Certificate validation failed!"
            ]
        ]
    ]
]
decode-certificate-request: function [
    ctx [object!]
    message [binary!]
] [
    either ctx/TLS13? [] [
        assert-prev-state ctx [SERVER_HELLO SERVER_KEY_EXCHANGE CERTIFICATE]
        binary/read message [
            certificate_types: UI8BYTES
            supported_signature_algorithms: UI16BYTES
            certificate_authorities: BYTES
        ]
    ]
    log-more ["R[" ctx/seq-read "] certificate_types:   " certificate_types]
    log-more ["R[" ctx/seq-read "] signature_algorithms:" supported_signature_algorithms]
    log-more ["R[" ctx/seq-read "] certifi_authorities: " certificate_authorities]
]
comment "-- End of:  %tls-certificate.reb"
comment "## Include: %tls-protocol.reb"
comment {## Title:   "TLS Handshake messages and parsing"}
TLS-update-messages-hash: func [
    ctx [object!]
    msg [binary!]
    /part
    len [integer!]
] [
    unless ctx/handshake? [exit]
    len: any [len length? msg]
    repend ctx/context-messages [ctx/state copy/part msg len]
]
get-transcript-hash: function [
    ctx [object!]
    stop-state [word! none!]
] [
    sha: open any [
        ctx/sha-port
        ctx/sha-port: open join checksum:// ctx/hash-type
    ]
    foreach [state bin] ctx/context-messages [
        write sha bin
        if state = stop-state [break]
    ]
    read sha
]
TLS-parse-handshake-records: function [
    ctx [object!]
] [
    bin: binary ctx/port-data
    while [4 <= length? bin/buffer] [
        start: bin/buffer
        binary/read bin [type: UI8 len: UI24]
        if len > length? bin/buffer [
            bin/buffer: start
            break
        ]
        message: binary/read bin len
        log-debug ["R[" ctx/seq-read "] length:" length? message "type:" type]
        change-state ctx *Handshake/name type
        TLS-update-messages-hash/part ctx start 4 + length? message
        switch/default ctx/state [
            CLIENT_HELLO [decode-client-hello :ctx :message]
            SERVER_HELLO [decode-server-hello :ctx :message]
            CERTIFICATE [decode-certificates :ctx :message]
            CERTIFICATE_VERIFY [
                decode-certificate-verify :ctx :message
                if ctx/TLS13? [with ctx [
                    finished-key: HKDF-Expand/label hash-type remote-hs-secret #{} mac-size "finished"
                    finished-hash: get-transcript-hash ctx _
                    verify-data: checksum/with finished-hash hash-type finished-key
                ]]
            ]
            FINISHED [
                log-more "Verify handshake data..."
                if ctx/version < 772 [
                    seed: get-transcript-hash ctx _
                    ctx/verify-data: prf :ctx/sha-port/spec/method either ctx/server? ["client finished"] ["server finished"] seed ctx/master-secret 12
                ]
                if ctx/verify-data <> message [
                    return 'Handshake_failure
                ]
                either ctx/server? [
                    switch-to-app-decrypt ctx
                    change-state ctx 'APPLICATION
                ] [
                    if ctx/TLS13? [derive-application-traffic-secrets ctx]
                    ctx/reading?: false
                ]
            ]
            ENCRYPTED_EXTENSIONS [
                assert-prev-state ctx [SERVER_HELLO]
                log-more ["R[" ctx/seq-read "] encrypted-extensions:" message]
            ]
            NEW_SESSION_TICKET [
                assert-prev-state ctx [FINISHED APPLICATION]
                session-ticket: binary/read message [
                    UI32
                    UI32
                    UI8BYTES
                    UI16BYTES
                    UI16BYTES
                ]
                log-more ["Session ticket:" mold/flat session-ticket]
                ctx/protocol: 'APPLICATION
                ctx/state: ctx/state-prev
            ]
            SERVER_KEY_EXCHANGE [decode-server-key-exchange :ctx :message]
            CLIENT_KEY_EXCHANGE [decode-client-key-exchange :ctx :message]
            CERTIFICATE_REQUEST [decode-certificate-request :ctx :message]
            SERVER_HELLO_DONE [ctx/reading?: false]
        ] [
            log-error ["Unknown state: " ctx/state "-" type]
            cause-TLS-error 'Unexpected_message
        ]
    ]
    log-more ["DONE: handshake^[[1m" ctx/state] log-----
    ctx/port-data: truncate bin/buffer
    false
]
prepare-change-cipher-spec: function [
    ctx [object!]
] [
    change-state ctx 'CHANGE_CIPHER_SPEC
    with ctx [
        binary/write out [
            UI8 20
            UI16 :legacy-version
            UI16 1
            UI8 1
        ]
    ]
    ctx/cipher-spec-set: 1
]
prepare-wrapped-record: function [
    ctx [object!]
    plain [binary!]
    type [integer!]
] [
    encrypted: wrap-record ctx plain type
    log-more ["W[" ctx/seq-write "] wrapped-record type:" type "bytes:" length? encrypted]
    binary/write ctx/out [
        UI8 23
        UI16 :ctx/legacy-version
        UI16BYTES :encrypted
    ]
]
encrypt-handshake-msg: function [
    ctx [object!]
    unencrypted [binary!]
] [
    log-more ["W[" ctx/seq-write "] encrypting-handshake-msg"]
    encrypted: encrypt-tls-record/type ctx unencrypted 22
    with ctx [
        binary/write out [
            UI8 22
            UI16 :legacy-version
            UI16BYTES :encrypted
        ]
    ]
]
decode-cipher-suites: function [
    bin [binary!]
] [
    num: (length? bin) >> 1
    out: make block! num
    bin: binary bin
    loop num [
        if cipher: *Cipher-suite/name binary/read bin 'UI16 [
            append out cipher
            log-debug ["Cipher-suite:" cipher]
        ]
    ]
    out
]
decode-list: function [
    *group [object!]
    bin [object! binary!]
    len [word! none!]
] [
    either object? bin [
        bytes: binary/read bin len
        if bytes != length? bin/buffer [
            log-error ["Invalid length of the" *group/title* "extension!"]
            cause-TLS-error 'Decode_error
        ]
    ] [
        bytes: length? bin
        bin: binary bin
    ]
    num: bytes >> 1
    out: make block! num
    loop num [
        append out *group/name binary/read bin 'UI16
    ]
    trim/all out
    out
]
decode-extensions: function [
    ctx [object!]
    bin [binary!]
] [
    bin: binary bin
    out: make map! 4
    while [not empty? bin/buffer] [
        binary/read bin [
            ext-type: UI16
            ext-data: UI16BYTES
        ]
        decoded: ext-data
        ext-type: any [*TLS-Extension/name ext-type ext-type]
        unless empty? ext-data [
            ext-data: binary ext-data
            switch ext-type [
                supported_groups [
                    decoded: decode-list *EllipticCurves ext-data 'UI16
                ]
                supported_versions [
                    either ctx/server? [
                        num: (binary/read ext-data 'UI8) >> 1
                        decoded: make block! num
                        loop num [
                            append decoded binary/read ext-data 'UI16
                        ]
                    ] [
                        either 2 != length? ext-data/buffer [
                            log-error {Invalid length of the supported_versions extension!}
                        ] [decoded: binary/read ext-data 'UI16]
                    ]
                ]
                key_share [
                    bytes: either ctx/server? [
                        binary/read ext-data 'UI16
                    ] [length? ext-data/buffer]
                    decoded: copy []
                    either bytes == 2 [
                        decoded: binary/read ext-data 'UI16
                    ] [
                        while [bytes >= 8] [
                            binary/read ext-data [curve: UI16 len: UI16]
                            bytes: bytes - len - 4
                            tmp: binary/read ext-data :len
                            if curve: *EllipticCurves/name curve [
                                repend decoded [curve tmp]
                            ]
                        ]
                    ]
                ]
                server_name [
                    bytes: binary/read ext-data 'UI16
                    case [
                        bytes != length? ext-data/buffer [
                            log-error "Invalid length of the server_name extension!"
                        ]
                        0 != binary/read ext-data 'UI8 [
                            log-error "Unknown server_name type!"
                        ]
                        'else [
                            decoded: to string! binary/read ext-data 'UI16BYTES
                            log-info ["Requested server name:^[[1m" decoded]
                        ]
                    ]
                ]
                signature_algorithms [
                    decoded: decode-list *SignatureScheme ext-data 'UI16
                ]
                compress_certificate [
                    decoded: decode-list *TLS-CertCompression ext-data 'UI8
                ]
            ]
        ]
        out/:ext-type: decoded
        log-more ["Extension:^[[1m" ext-type "^[[2m" mold decoded]
    ]
    out
]
encode-extension: function [
    ext [binary!]
    id [integer!]
    data [binary!]
    /length
] [
    either length [
        length: 2 + length? data
        binary/write tail ext [
            UI16 :id
            UI16 :length
            UI16BYTES :data
        ]
    ] [
        binary/write tail ext [
            UI16 :id
            UI16BYTES :data
        ]
    ]
]
encode-handshake-record: function [
    ctx [object!]
    record [binary!]
] [
    with ctx [
        TLS-update-messages-hash ctx record
        if TLS13? [
            record: wrap-record ctx record 22
        ]
        binary/write out [
            UI8 23
            UI16 :legacy-version
            UI16BYTES :record
        ]
    ]
]
comment "-- End of:  %tls-protocol.reb"
comment "## Include: %tls-client.reb"
comment {## Title:   "TLS Client Implementation"}
TLS-client-awake: function [
    event [event!]
] [
    log-debug ["AWAKE Client:^[[1m" event/type]
    TCP-port: event/port
    ctx: TCP-port/extra
    TLS-port: ctx/TLS-port
    if all [
        ctx/protocol = 'APPLICATION
        not TCP-port/data
    ] [
        TLS-port/data: none
    ]
    switch/default event/type [
        lookup [
            open TCP-port
            TLS-init-context ctx
            return false
        ]
        connect [
            if none? ctx [return true]
            return TLS-init-connection ctx
        ]
        wrote [
            switch ctx/protocol [
                CLOSE-NOTIFY [
                    return true
                ]
                APPLICATION [
                    if ctx/state = 'FINISHED [
                        change-state ctx 'APPLICATION
                        handshake-finished ctx
                        return false
                    ]
                    dispatch-event 'wrote TLS-port
                    return false
                ]
            ]
            read TCP-port
            return false
        ]
        read [
            error: try [
                log-debug ["READ TCP" length? TCP-port/data "bytes proto-state:" ctx/protocol]
                complete?: TLS-read-data ctx TCP-port/data
                if ctx/critical-error [cause-TLS-error ctx/critical-error]
                log-debug ["Read complete?" complete? "protocol:" ctx/protocol "state:" ctx/state]
                unless complete? [
                    read TCP-port
                    return false
                ]
                TLS-port/data: ctx/port-data
                binary/init ctx/in none
                switch ctx/protocol [
                    APPLICATION [
                        if all [
                            ctx/state = 'FINISHED
                            ctx/version == 772
                        ] [
                            prepare-finished-message ctx
                            do-TCP-write ctx
                            return false
                        ]
                        dispatch-event 'read TLS-port
                        return true
                    ]
                    HANDSHAKE [
                        switch ctx/state [
                            SERVER_HELLO_DONE [
                                binary/init ctx/out none
                                prepare-client-key-exchange ctx
                                prepare-change-cipher-spec ctx
                                prepare-finished-message ctx
                                do-TCP-write ctx
                                return false
                            ]
                            FINISHED [
                                either ctx/server? [
                                    handshake-finished ctx
                                    return true
                                ] [
                                    either ctx/TLS13? [
                                        prepare-finished-message ctx
                                        do-TCP-write ctx
                                        return false
                                    ] [
                                        change-state ctx ctx/protocol: 'APPLICATION
                                        dispatch-event 'connect ctx/TLS-port
                                        return true
                                    ]
                                ]
                            ]
                        ]
                    ]
                ]
                read TCP-port
                return false
            ]
            if ctx [log-error ctx/error: error]
            dispatch-event 'error TLS-port
            return true
        ]
        close [
            dispatch-event 'close TLS-port
            return true
        ]
        error [
            unless ctx/error [
                ctx/error: case [
                    ctx/state = 'lookup [
                        make error! [
                            code: 500 type: 'access id: 'cannot-open
                            arg1: TCP-port/spec/ref
                        ]
                    ]
                    'else [
                        make error! [
                            code: 500 type: 'access id: 'protocol
                            arg1: TCP-port/spec/ref
                        ]
                    ]
                ]
            ]
            dispatch-event 'error TLS-port
            return true
        ]
    ] [
        close TCP-port
        do make error! rejoin ["Unexpected TLS event: " event/type]
    ]
    false
]
TLS-init-connection: function [
    ctx [object!]
] [
    binary/init ctx/out none
    binary/init ctx/in none
    prepare-client-hello ctx
    do-TCP-write ctx
    false
]
TLS-read-data: function [
    ctx [object!]
    tcp-data [binary!]
] [
    log-debug ["read-data:^[[1m" length? tcp-data "^[[22mbytes previous rest:" length? ctx/rest]
    inp: ctx/in
    binary/write inp ctx/rest
    binary/write inp tcp-data
    clear tcp-data
    clear ctx/rest
    ctx/reading?: true
    while [ctx/reading? and ((available: length? inp/buffer) >= 5)] [
        binary/read inp [
            start: INDEX
            type: UI8
            server-version: UI16
            len: UI16
        ]
        available: available - 5
        log-debug ["Fragment type: ^[[1m" type "^[[22mver:^[[1m" server-version "/" ctx/version "^[[22mbytes:^[[1m" len "^[[22mbytes"]
        if ctx/legacy-version < server-version [
            ctx/critical-error: 'Internal_error
            return false
        ]
        if available < len [
            log-debug ["Incomplete fragment:^[[22m available^[[1m" available "^[[22mof^[[1m" len "^[[22mbytes"]
            binary/read inp [AT :start]
            log-debug ["Data starts: " copy/part inp/buffer 10]
            return false
        ]
        if type != 20 [
            binary/read inp [data: BYTES :len]
            if ctx/decrypt-port [
                data: decrypt-tls-record ctx data :type
                if ctx/TLS13? [
                    type: take/last data
                    log-debug ["Inner type:^[[1m" type]
                ]
            ]
            append ctx/port-data data
        ]
        *protocol-type/assert type
        *protocol-version/assert server-version
        protocol: *protocol-type/name type
        version: *protocol-version/name server-version
        end: start + len + 5
        log-more ["^[[22mR[" ctx/seq-read "] Protocol^[[1m" protocol "^[[22mbytes:^[[1m" len "^[[22mfrom^[[1m" start "^[[22mto^[[1m" end]
        ctx/protocol: protocol
        switch protocol [
            APPLICATION [
                assert-prev-state ctx [APPLICATION ALERT FINISHED NEW_SESSION_TICKET]
            ]
            HANDSHAKE [
                ctx/critical-error: TLS-parse-handshake-records ctx
                ctx/reading?: any [ctx/server? not empty? inp/buffer]
            ]
            CHANGE_CIPHER_SPEC [
                value: binary/read inp 'UI8
                if value != 1 [
                    log-error ["*** CHANGE_CIPHER_SPEC value should be 1 but is:" value]
                    ctx/critical-error: 'Handshake_failure
                    return false
                ]
                either ctx/TLS13? [
                    log-debug "Ignoring TLS 1.3 compatibility ChangeCipherSpec"
                ] [
                    unless integer? ctx/extensions/key_share [
                        ctx/handshake?: false
                        ctx/cipher-spec-set: 2
                    ]
                ]
                if integer? ctx/extensions/key_share [
                    ctx/reading?: false
                ]
            ]
            ALERT [
                log-debug ["ALERT len:" :len "ctx/cipher-spec-set:" ctx/cipher-spec-set]
                unless data [
                    log-error "Failed to decode ALERT message!"
                    ctx/critical-error: none
                    ctx/protocol: 'APPLICATION
                    continue
                ]
                level: data/1
                id: data/2
                level: any [*Alert-level/name level join "Alert-" level]
                name: any [*Alert/name id 'Unknown]
                ctx/critical-error: either level = 'WARNING [false] [name]
                either id = 0 [
                    ctx/reading?: false
                    ctx/protocol: 'APPLICATION
                    log-info "Server done"
                ] [
                    log-more ["ALERT:" level "-" replace/all form name #"_" #" "]
                ]
            ]
        ]
        if ctx/critical-error [return false]
        if end <> index? inp/buffer [
            log-error ["Record end mismatch:^[[22m" end "<>" index? inp/buffer]
            ctx/critical-error: 'Record_overflow
            return false
        ]
        unless ctx/reading? [
            log-debug ["Reading finished!"]
            if all [
                not ctx/server?
                integer? ctx/extensions/key_share
            ] [
                if ctx/hello-retry-request [
                    log-error {Only one HelloRetryRequest is permitted per handshake!}
                    cause-TLS-error 'Unexpected_message
                ]
                log-info "Retry Hello..."
                ctx/state: 'HELLO_RETRY
                ctx/ecdh-group: *EllipticCurves/name ctx/extensions/key_share
                ctx/hello-retry-request: true
                prepare-client-hello ctx
                do-TCP-write ctx
                ctx/reading?: false
                return false
            ]
            return true
        ]
    ]
    log-debug "continue reading..."
    unless empty? ctx/in/buffer [
        ctx/rest: copy ctx/in/buffer
    ]
    return true
]
prepare-client-hello: function [
    ctx [object!]
] [
    change-state ctx 'CLIENT_HELLO
    with ctx [
        extensions: make binary! 100
        if all [
            ctx/tcp-port
            host-name: ctx/tcp-port/spec/host
        ] [
            host-name: to binary! host-name
            length-name: length? host-name
            binary/write tail extensions compose [
                UI16 0
                UI16 (5 + length-name)
                UI16 (3 + length-name)
                UI8 0
                UI16BYTES :host-name
            ]
        ]
        encode-extension/length extensions 10 supported-elliptic-curves
        encode-extension/length extensions 13 supported-signature-algorithms
        append extensions #{002B00050403040303}
        curve: first supported-groups
        dh-key: ecdh/init none ecdh-group: any [ecdh-group curve]
        pub-key: ecdh/public dh-key
        curve: *EllipticCurves/:ecdh-group
        key-share: clear #{}
        binary/write key-share [
            UI16 :curve
            UI16BYTES :pub-key
        ]
        encode-extension/length extensions 51 key-share
        log-debug ["Client key_share:" *EllipticCurves/name curve "public:" pub-key]
        append extensions #{000B000403000102FF01000100002D00020101}
        append extensions #{00120000}
        length-extensions: length? extensions
        length-message: 73 + length-extensions + length? suported-cipher-suites-binary
        length-record: 4 + length-message
        unless session-id [
            binary/write session-id: make binary! 32 [RANDOM-BYTES 32]
        ]
        binary/write out [
            UI8 22
            UI16 769
            UI16 :length-record
            UI8 1
            UI24 :length-message
            UI16 :legacy-version
            RANDOM-BYTES 32
            UI8BYTES :session-id
            UI16BYTES :suported-cipher-suites-binary
            UI8 1
            UI8 0
            UI16BYTES :extensions
        ]
        out/buffer: head out/buffer
        locale-random: copy/part (at out/buffer 12) 32
        TLS-update-messages-hash ctx (at out/buffer 6) (4 + length-message)
        log-more [
            "W[" ctx/seq-write "] Bytes:" length? out/buffer "=>"
            "record:" length-record
            "message:" length-message
            "extensions:" length-extensions
            "signatures:" length? supported-signature-algorithms
        ]
        log-more ["W[" ctx/seq-write "] CRandom:^[[32m" locale-random]
    ]
]
prepare-finished-message: function [
    ctx [object!]
] [
    either ctx/TLS13? [
        with ctx [
            log-debug {Send CHANGE_CIPHER_SPEC record (middlebox compatibility mode)}
            binary/write out [
                UI8 20
                UI16 :legacy-version
                UI16 1
                UI8 1
            ]
            log-debug "Send Client FINISHED"
            binary/write plain: copy #{} [
                UI8 20
                UI24BYTES :verify-data
            ]
            prepare-wrapped-record ctx plain 22
            switch-to-app-encrypt ctx
            protocol: 'APPLICATION
        ]
    ] [
        change-state ctx 'FINISHED
        seed: get-transcript-hash ctx _
        unencrypted: rejoin [
            #{14}
            #{00000C}
            prf :ctx/sha-port/spec/method either ctx/server? ["server finished"] ["client finished"] seed ctx/master-secret 12
        ]
        TLS-update-messages-hash ctx unencrypted
        encrypt-handshake-msg ctx unencrypted
    ]
]
decode-server-hello: function [
    ctx [object!]
    message [binary!]
] [
    assert-prev-state ctx [CLIENT_HELLO]
    with ctx [
        if any [
            error? try [
                binary/read message [
                    server-version: UI16
                    remote-random: BYTES 32
                    session-id: UI8BYTES
                    cipher-suite: UI16
                    compressions: UI8BYTES
                    extensions: UI16BYTES
                    pos: INDEX
                ]
            ]
            32 < length? session-id
        ] [
            log-error "Failed to read server hello."
            cause-TLS-error 'Handshake_failure
        ]
        log-more ["R[" seq-read "] Version:" *Protocol-version/name server-version "cipher-suite:" *Cipher-suite/name cipher-suite]
        log-more ["R[" seq-read "] SRandom:^[[32m" remote-random]
        log-more ["R[" seq-read "] Session:^[[32m" session-id]
        if server-version <> version [
            log-error [
                "Version required by server:" server-version
                "is not same like clients:" version
            ]
            if server-version <> version [
                cause-TLS-error 'Protocol_version
            ]
            version: server-version
        ]
        unless empty? compressions [
            log-more ["R[" seq-read "] Compressions:^[[1m" compressions]
            log-error "Compression flag must be 0!"
            cause-TLS-error 'Illegal_parameter
        ]
        unless TLS-init-cipher-suite ctx [
            log-error "Unsupported cipher suite!"
            cause-TLS-error 'Illegal_parameter
        ]
        extensions: decode-extensions ctx :extensions
        case/all [
            integer? extensions/key_share [
                log-info ["Hello Retry Request with key_share type:" *EllipticCurves/name extensions/key_share]
                hash: checksum ctx/context-messages/2 ctx/hash-type
                binary/write clear ctx/context-messages/2 [
                    UI8 254
                    UI16 0
                    UI8BYTES :hash
                ]
            ]
            all [
                extensions/supported_versions == 772
                handle? dh-key
                block? extensions/key_share
            ] [
                log-info "Using TLS v1.3"
                version: 772
                TLS13?: true
                pre-secret: ecdh/secret dh-key extensions/key_share/2
                log-more ["Elyptic curve^[[1m" extensions/key_share/1 "^[[22mdata (pre-secret):" pre-secret]
                TLS-key-expansion ctx
            ]
        ]
        false
    ]
]
comment "-- End of:  %tls-client.reb"
comment "## Include: %tls12-client.reb"
comment {## Title:   "TLS v1.2 Client Functions"}
decode-server-key-exchange: function [
    ctx [object!]
    message [binary!]
] [
    assert-prev-state ctx [CERTIFICATE SERVER_HELLO]
    msg: binary message
    log-more ["R[" ctx/seq-read "] Using key method:^[[1m" ctx/key-method]
    switch ctx/key-method [
        ECDHE_RSA
        ECDHE_ECDSA [
            try/with [
                binary/read msg [
                    ECCurveType: UI8
                    ECCurve: UI16
                    pub_key: UI8BYTES
                    message-len: INDEXz
                ]
            ] [
                log-error "Error reading elyptic curve"
                return 'User_cancelled
            ]
            if any [
                3 <> ECCurveType
                none? curve: *EllipticCurves/name ECCurve
            ] [
                log-error ["Unsupported ECurve type:" ECCurveType ECCurve]
                cause-TLS-error critical-error: 'User_cancelled
            ]
            log-more ["R[" ctx/seq-read "] Elyptic curve type:" ECCurve "=>" curve]
            log-more ["R[" ctx/seq-read "] Elyptic curve data:" mold pub_key]
        ]
        DHE_DSS
        DHE_RSA [
            binary/read msg [
                dh_p: UI16BYTES
                dh_g: UI16BYTES
                pub_key: UI16BYTES
                message-len: INDEXz
            ]
        ]
    ]
    verify-data: rejoin [
        ctx/locale-random
        ctx/remote-random
        copy/part message message-len
    ]
    binary/read msg [
        hash-algorithm: UI8
        sign-algorithm: UI8
        signature: UI16BYTES
    ]
    either hash-algorithm == 8 [
        switch sign-algorithm [
            4 [sign-algorithm: 'rsa_pss hash-algorithm: 'sha256]
            5 [sign-algorithm: 'rsa_pss hash-algorithm: 'sha384]
            6 [sign-algorithm: 'rsa_pss hash-algorithm: 'sha512]
        ]
    ] [
        hash-algorithm: *HashAlgorithm/name :hash-algorithm
        sign-algorithm: *ClientCertificateType/name :sign-algorithm
    ]
    log-more ["R[" ctx/seq-read "] Using algorithm:" hash-algorithm "with" sign-algorithm]
    key: ctx/server-certs/1/public-key
    switch sign-algorithm [
        ecdsa_sign [
            log-more "Checking signature using ECDSA"
            message-hash: checksum verify-data hash-algorithm
            ecdsa/verify/curve ctx/pub-key message-hash signature ctx/pub-exp
        ]
        rsa_sign [
            log-more "Checking signature using RSA"
            rsa-key: apply :rsa-init ctx/server-certs/1/public-key/rsaEncryption
            valid?: rsa/verify/hash rsa-key verify-data signature hash-algorithm
        ]
        rsa_pss [
            log-more "Checking signature using RSA_PSS"
            rsa-key: apply :rsa-init ctx/server-certs/1/public-key/rsaEncryption
            valid?: rsa/verify/pss/hash rsa-key verify-data signature hash-algorithm
        ]
    ]
    unless valid? [
        log-error "Failed to validate signature"
        cause-TLS-error 'Decode_error
    ]
    log-more "Signature valid!"
    unless tail? msg/buffer [
        len: ends - pos
        binary/read msg [extra: BYTES :len]
        log-error ["Extra" len "bytes at the end of message:" ellipsize form extra 40]
        cause-TLS-error 'Decode_error
    ]
    if dh_p [
        dh-key: dh-init dh_g dh_p
        ctx/pre-secret: dh/secret dh-key pub_key
        log-more ["DH common secret:" mold ctx/pre-secret]
        ctx/key-data: dh/public :dh-key
        release :dh-key dh-key: none
    ]
    if curve [
        dh-key: ecdh/init none curve
        ctx/pre-secret: ecdh/secret dh-key pub_key
        log-more ["ECDH common secret:^[[32m" mold ctx/pre-secret]
        ctx/key-data: ecdh/public :dh-key
        release :dh-key dh-key: none
    ]
]
decode-client-key-exchange: function [
    ctx [object!]
    message [binary!]
] [
    assert-prev-state ctx [CLIENT_CERTIFICATE SERVER_HELLO_DONE SERVER_HELLO]
    unless ctx/server? [
        log-error "This message is expected on server!"
        cause-TLS-error 'Decode_error
    ]
    switch ctx/key-method [
        ECDHE_RSA
        ECDHE_ECDSA [
            key-data: binary/read msg 'UI8BYTES
            ctx/pre-secret: ecdh/secret ctx/dh-key key-data
            log-more ["ECDH common secret:^[[32m" ctx/pre-secret]
        ]
        DHE_DSS
        DHE_RSA [
            key-data: binary/read msg 'UI8BYTES
        ]
        RSA [
            key-data: binary/read msg 'UI16BYTES
        ]
    ]
    TLS-key-expansion ctx
]
prepare-client-key-exchange: function [
    ctx [object!]
] [
    log-debug ["client-key-exchange -> method:" ctx/key-method "key-data:" mold ctx/key-data]
    change-state ctx 'CLIENT_KEY_EXCHANGE
    assert-prev-state ctx [CLIENT_CERTIFICATE SERVER_HELLO_DONE SERVER_HELLO]
    with ctx [
        binary/write out [
            UI8 22
            UI16 :version
            pos-record-len:
            UI16 0
            pos-record:
            UI8 16
            pos-message:
            UI24 0
            pos-key:
        ]
        switch key-method [
            ECDHE_ECDSA
            ECDHE_RSA [
                log-more ["W[" seq-write "] Using ECDH key-method"]
                key-data-len-bytes: 1
            ]
            RSA [
                log-more ["W[" seq-write "] Using RSA key-method"]
                binary/write bin [
                    UI16 :version RANDOM-BYTES 46
                ]
                binary/read bin [pre-secret: BYTES 48]
                binary/init bin 0
                log-more ["W[" seq-write "] pre-secret:" mold pre-secret]
                rsa-key: rsa-init pub-key pub-exp
                key-data: rsa/encrypt rsa-key pre-secret
                key-data-len-bytes: 2
                log-more ["W[" seq-write "] key-data:" mold key-data]
                release :rsa-key
            ]
            DHE_DSS
            DHE_RSA [
                log-more ["W[" seq-write "] Using DH key-method"]
                key-data-len-bytes: 2
            ]
        ]
        length-message: key-data-len-bytes + length? key-data
        length-record: 4 + length-message
        binary/write out compose [
            AT :pos-record-len UI16 :length-record
            AT :pos-message UI24 :length-message
            AT :pos-key (pick [UI8BYTES UI16BYTES] key-data-len-bytes) :key-data
        ]
        TLS-key-expansion ctx
        TLS-update-messages-hash/part ctx (at head out/buffer pos-record) length-record
    ]
]
comment "-- End of:  %tls12-client.reb"
comment "## Include: %tls-server.reb"
comment {## Title:   "TLS Server Implementation"}
TLS-server-awake: func [
    event [event!]
    /local port info serv
] [
    log-more ["AWAKE Server:^[[1m" event/type]
    switch event/type [
        accept [
            port: first serv: event/port
            info: query port [remote-ip remote-port]
            port/extra: make TLS-context [
                tcp-port: port
                tls-port: serv/parent
                server?: true
                state: 'CLIENT_HELLO
                version: serv/extra/version
            ]
            port/spec/title: "TLS Server's client"
            port/spec/ref: rejoin [tcp:// info/remote-ip #":" info/remote-port]
            port/awake: :TLS-server-client-awake
            read port
        ]
    ]
    false
]
TLS-server-client-awake: function [
    event [event!]
] [
    TCP-port: event/port
    ctx: TCP-port/extra
    log-debug ["Server's client awake event:" event/type "state:" ctx/state ctx/server?]
    switch event/type [
        read [
            error: try [
                complete?: TLS-read-data ctx TCP-port/data
                if ctx/critical-error [cause-TLS-error ctx/critical-error]
                log-debug ["==============Read complete?" complete? "state:" ctx/state]
                either complete? [
                    switch ctx/state [
                        CLIENT_HELLO [
                            prepare-server-hello ctx
                            unless ctx/hello-retry-request [
                                TLS-key-expansion ctx
                                prepare-change-cipher-spec ctx
                                prepare-server-encrypted-extensions ctx
                                prepare-server-certificate ctx
                                either ctx/TLS13? [
                                    prepare-server-handshake-finish ctx
                                ] [
                                    prepare-server-hello-done ctx
                                ]
                            ]
                            write TCP-port head ctx/out/buffer
                        ]
                        FINISHED [
                            ctx/cipher-spec-set: 2
                            log-more "FINISHED"
                            change-state ctx 'APPLICATION
                            log-more "Start reading real data..."
                            read TCP-port
                        ]
                        APPLICATION [
                            TCP-port/parent/actor/On-Read TCP-port
                        ]
                    ]
                ] [
                    read TCP-port
                ]
                return false
            ]
            if ctx [log-error ctx/error: error]
            do-TLS-close TCP-port
            return true
        ]
        wrote [
            either ctx/protocol = 'APPLICATION [
                TCP-port/parent/actor/On-Wrote TCP-port
            ] [
                read TCP-port
            ]
            return false
        ]
        close [
            do-TLS-close TCP-port
            return true
        ]
    ]
    false
]
prepare-server-hello: function [
    ctx [object!]
] [
    change-state ctx 'SERVER_HELLO
    with ctx [
        key_share: none
        if all [
            block? extensions/supported_versions
            block? extensions/key_share
            find extensions/supported_versions 772
        ] [
            log-info "Using TLS v1.3"
            version: 772
            TLS13?: true
            key_share: make binary! 32
            curve: extensions/key_share/1
            either find supported-groups curve [
                dh-key: ecdh/init none curve
                pub-key: ecdh/public dh-key
                curve: *EllipticCurves/:curve
                binary/write key_share [
                    UI16 :curve
                    UI16BYTES :pub-key
                ]
                pre-secret: ecdh/secret dh-key extensions/key_share/2
                log-more ["Elyptic curve^[[1m" extensions/key_share/1 "^[[22mdata (pre-secret):" pre-secret]
                ctx/hello-retry-request: none
            ] [
                change-state ctx 'SERVER_HELLO_RETRY
                ?? supported-groups
                ?? ctx/extensions/supported_groups
                hello-retry-request: true
                ecdh-group: attempt [
                    curve: first union supported-groups ctx/extensions/supported_groups
                    *EllipticCurves/:curve
                ]
                unless ecdh-group [
                    cause-TLS-error 'Insufficient_security
                ]
                log-info ["Server requests HelloRetry with elliptic group:" ecdh-group]
                binary/write key_share [
                    UI16 :ecdh-group
                ]
            ]
        ]
        binary/init out none
        server-extensions: #{}
        if find extensions 'ec_point_formats [
            append server-extensions #{000B000403000102}
        ]
        if find extensions 'renegotiation_info [
            append server-extensions #{FF01000100}
        ]
        if TLS13? [
            append server-extensions #{002B00020304}
            if key_share [
                encode-extension server-extensions 51 key_share
            ]
        ]
        binary/write out [
            pos-start:
            UI8 22
            UI16 769
            pos-record-len:
            UI16 0
            pos-record:
            UI8 2
            pos-message-len:
            UI24 0
            UI16 771
        ]
        binary/write out either/only hello-retry-request [
            :HRR-magic
        ] [
            UNIXTIME-NOW RANDOM-BYTES 28
        ]
        binary/write out [
            UI8BYTES :session-id
            UI16 :cipher-suite
            UI8 0
            UI16BYTES :server-extensions
            pos-end:
        ]
        locale-random: copy/part (at out/buffer 12) 32
        log-more ["W[" ctx/seq-write "] SRandom:^[[32m" locale-random]
        log-more ["W[" ctx/seq-write "] Session:^[[32m" session-id]
        binary/write out compose [
            AT :pos-record-len UI16 (length-record: pos-end - pos-record)
            AT :pos-message-len UI24 (length-message: length-record - 4)
            AT :pos-end
        ]
        TLS-update-messages-hash/part ctx (at head out/buffer :pos-record) :length-record
        log-more [
            "W[" ctx/seq-write "] Bytes:" pos-end - pos-start "=>"
            "record:" length-record
            "message:" length-message
        ]
    ]
]
prepare-server-certificate: function [
    ctx [object!]
] [
    change-state ctx 'CERTIFICATE
    with ctx [
        certificates: tls-port/state/certificates
        length: 4 + length? certificates
        record: clear #{}
        binary/write record [
            UI8 11
            UI24 :length
            UI8 0
            UI24BYTES :certificates
        ]
        encode-handshake-record ctx record
        if TLS13? [
            change-state ctx 'CERTIFICATE_VERIFY
            to-sign: rejoin [
                server-certificate-verify-context
                get-transcript-hash ctx 'CERTIFICATE
            ]
            signature: rsa/sign/pss tls-port/state/private-key :to-sign
            length: 4 + length? signature
            binary/write clear record [
                UI8 15
                UI24 :length
                UI16 2052
                UI16BYTES :signature
            ]
            encode-handshake-record ctx record
        ]
        if find [ECDHE_RSA ECDHE_ECDSA DHE_RSA] key-method [
            change-state ctx 'SERVER_KEY_EXCHANGE
            binary/write clear record [
                UI8 12
                UI24 0
            ]
            switch key-method [
                ECDHE_RSA [
                    spec: TCP-port/parent/state
                    curve: first spec/elliptic-curves
                    dh-key: ecdh/init none curve
                    pub-key: ecdh/public dh-key
                    curve: *EllipticCurves/:curve
                    sign-algorithm: *ClientCertificateType/rsa_sign
                    hash-method-int: *HashAlgorithm/:hash-method
                    binary/write message: clear #{} [
                        BYTES :remote-random
                        BYTES :locale-random
                        pos-msg:
                        UI8 3
                        UI16 :curve
                        UI8BYTES :pub-key
                    ]
                    signature: rsa/sign/hash spec/private-key :message :hash-method
                    remove/part message (pos-msg - 1)
                    binary/write record [
                        BYTES :message
                        UI8 :hash-method-int
                        UI8 :sign-algorithm
                        UI16BYTES :signature
                    ]
                ]
            ]
            length: (length? record) - 4
            binary/write next record [UI24 :length]
            encode-handshake-record ctx record
        ]
    ]
]
prepare-server-hello-done: function [
    ctx [object!]
] [
    change-state ctx 'SERVER_HELLO_DONE
    encode-handshake-record ctx #{0E000000}
]
prepare-server-encrypted-extensions: function [
    ctx [object!]
] [
    change-state ctx 'ENCRYPTED_EXTENSIONS
    encode-handshake-record ctx #{080000020000}
]
prepare-server-handshake-finish: function [
    ctx [object!]
] [
    change-state ctx 'FINISHED
    with ctx [
        finished-hash: get-transcript-hash ctx _
        finished-key: HKDF-Expand/label hash-type locale-hs-secret #{} mac-size "finished"
        verify-data: checksum/with finished-hash hash-type finished-key
        binary/write record: clear #{} [
            UI8 20
            UI24BYTES :verify-data
        ]
        encode-handshake-record ctx record
        derive-application-traffic-secrets ctx
    ]
]
decode-client-hello: function [
    ctx [object!]
    message [binary!]
] [
    binary/read message [
        client-version: UI16
        remote-random: BYTES 32
        session-id: UI8BYTES
        cipher-suites: UI16BYTES
        compressions: UI8BYTES
        extensions: UI16BYTES
    ]
    log-debug ["Client requests:" *Protocol-version/name :client-version]
    log-debug ["Client random: ^[[1m" remote-random]
    ctx/remote-random: remote-random
    ctx/session-id: session-id
    unless empty? session-id [
        log-debug ["Client session:" session-id]
    ]
    client-cipher-suites: decode-list *Cipher-suite :cipher-suites _
    ?? client-cipher-suites
    foreach cipher client-cipher-suites [
        if find suported-cipher-suites cipher [
            ?? cipher
            ctx/cipher-suite: *Cipher-suite/:cipher
            log-info ["Server choose cipher:" as-yellow ctx/cipher-suite]
            TLS-init-cipher-suite ctx
            break
        ]
    ]
    unless ctx/crypt-method [
        log-error "No supported cipher-suite!"
        cause-TLS-error 'Handshake_failure
    ]
    if #{00} <> compressions [
        log-error ["Client requests compression:" compressions]
        cause-TLS-error 'Unexpected_message
    ]
    ctx/extensions: decode-extensions ctx :extensions
    if all [
        block? ctx/extensions/supported_groups
        block? ctx/extensions/key_share
        none? find ctx/extensions/supported_groups ctx/extensions/key_share/1
    ] [
        log-error ["Key_share type^[[22m" ctx/extensions/key_share/1 "^[[1mthat is not listed in supported_groups!"]
        cause-TLS-error 'Illegal_parameter
    ]
    ctx/reading?: false
]
comment "-- End of:  %tls-server.reb"
comment "## Include: %tls-scheme.reb"
comment {## Title:   "TLS Scheme Implementation"}
do-TLS-open: func [
    port [port!]
    /local spec conn config certs bin der key
] [
    log-debug "OPEN"
    if port/state [return port]
    spec: port/spec
    either port? conn: select spec 'conn [
        spec/host: conn/spec/host
        spec/port: conn/spec/port
        if block? spec/ref [
            spec/ref: rejoin [tls:// any [spec/host ""] ":" spec/port]
        ]
    ] [
        conn: make port! [
            scheme: 'tcp
            host: spec/host
            port: spec/port
            ref: rejoin [tcp:// any [host ""] ":" port]
        ]
        if port/parent [
            conn/state: port/parent/state
        ]
        conn/parent: port
    ]
    either spec/host [
        port/extra: conn/extra: make TLS-context [
            tcp-port: conn
            tls-port: port
            version: *Protocol-version/TLS1.2
        ]
        port/data: conn/extra/port-data
        conn/awake: :TLS-client-awake
    ] [
        spec/ref: rejoin [tls://: spec/port]
        port/spec/title: "TLS Server"
        conn/spec/title: "TLS Server (internal)"
        port/state: conn/extra: object [
            TCP-port: conn
            certificates: none
            private-key: none
            elliptic-curves: decode-list *EllipticCurves :supported-elliptic-curves _
            version: *Protocol-version/TLS1.2
        ]
        if config: select spec 'config [
            certs: any [select config 'certificates []]
            unless block? certs [certs: to block! certs]
            bin: binary 4000
            foreach file certs [
                try/with [
                    der: select decode 'pkix read file 'binary
                    binary/write bin [UI24BYTES :der]
                ] [
                    log-error ["Failed to import certificate:" file]
                ]
            ]
            binary/write bin [UI16 0]
            port/state/certificates: bin/buffer
            if key: select config 'private-key [
                if file? key [try [key: load key]]
                either handle? key [
                    port/state/private-key: key
                ] [log-error ["Failed to import private key:" key]]
            ]
        ]
        port/actor: context [
            On-Read: func [port [port!] /local data] [
                log-debug "TLS On-Read"
                probe to string! data: port/extra/port-data
                either empty? data [
                    do-TLS-read port
                ] [
                    do-TLS-write port {HTTP/1.1 200 OK^M
Content-type: text/plain^M
^M
Hello from Rebol using TLS v1.3}
                ]
            ]
            On-Wrote: func [port [port!]] [
                dispatch-event 'close port
            ]
        ]
        conn/parent: port
        conn/awake: :TLS-server-awake
    ]
    either open? conn [
        TLS-init-context conn/extra
        TLS-init-connection conn/extra
    ] [
        open conn
    ]
    port
]
do-TLS-close: func [
    port [port!] /local ctx parent
] [
    log-debug "CLOSE"
    unless ctx: port/extra [return port]
    parent: port/parent
    log-debug "Closing port/extra/tcp-port"
    close ctx/tcp-port
    if port? ctx/encrypt-port [close ctx/encrypt-port]
    if port? ctx/decrypt-port [close ctx/decrypt-port]
    ctx/encrypt-port: none
    ctx/decrypt-port: none
    ctx/tcp-port/awake: none
    ctx/tcp-port: none
    ctx/tls-port: none
    port/extra: none
    log-more "Port closed"
    if parent [
        insert system/ports/system make event! [type: 'close port: parent]
    ]
    port
]
do-TLS-read: func [
    port [port!]
] [
    log-debug "READ"
    read port/extra/tcp-port
    port
]
do-TLS-write: func [
    port [port!]
    value [any-string! binary!]
    /local ctx
] [
    log-debug "WRITE"
    ctx: port/extra
    if ctx/protocol = 'APPLICATION [
        binary/init ctx/out none
        while [not tail? value] [
            prepare-application-data ctx copy/part :value 16384
            value: skip value 16384
        ]
        do-TCP-write ctx
        return port
    ]
]
do-TCP-write: func [
    ctx [object!]
] [
    log-debug ["Writing bytes:" length? ctx/out/buffer]
    clear ctx/port-data
    write ctx/tcp-port ctx/out/buffer
    binary/init ctx/out none
    ctx/reading?: true
]
prepare-application-data: func [
    ctx [object!]
    message [binary! string!]
] [
    log-more ["W[" ctx/seq-write "] application data:" length? message "bytes"]
    either ctx/TLS13? [
        prepare-wrapped-record ctx to binary! message 23
    ] [
        message: encrypt-tls-record ctx to binary! message
        with ctx [
            binary/write out [
                UI8 23
                UI16 :legacy-version
                UI16BYTES :message
            ]
            ++ seq-write
        ]
    ]
]
prepare-alert-close-notify: func [
    ctx [object!]
] [
    log-more "alert-close-notify"
    message: encrypt-tls-record ctx #{0100}
    with ctx [
        binary/write out [
            UI8 21
            UI16 :legacy-version
            UI16BYTES :message
        ]
    ]
]
handshake-finished: func [
    ctx [object!]
] [
    log-----
    log-info "Handshake finished"
    ctx/handshake?: false
    dispatch-event 'connect ctx/TLS-port
]
tls-config: func [
    spec
] [
    foreach [key value] spec [
        switch :key [
            groups
            supported-groups
            [
                if block? :value [
                    clear supported-elliptic-curves
                    clear supported-groups
                    foreach curve :value [
                        if find system/catalog/elliptic-curves curve [
                            append supported-groups curve
                            binary/write tail supported-elliptic-curves [UI16BE :*EllipticCurves/:curve]
                        ]
                    ]
                ]
            ]
            verbose
            verbosity
            [
                tls-verbosity :value
            ]
        ]
    ]
]
sys/make-scheme [
    name: 'tls
    title: "TLS protocol v1.3"
    spec: make system/standard/port-spec-net [
        supported-groups: [
            curve25519
            curve448
            secp521r1
            secp384r1
            secp256r1
            bp512r1
            bp384r1
            bp256r1
            secp256k1
            secp224r1
            secp224k1
        ]
    ]
    actor: reduce/no-set [
        read: :do-TLS-read
        write: :do-TLS-write
        open: :do-TLS-open
        close: :do-TLS-close
        query: func [port [port!]] [all [port/extra query port/extra/tcp-port]]
        open?: func [port [port!]] [all [port/extra open? port/extra/tcp-port]]
        copy: func [port [port!]] [if port/data [copy port/data]]
        length?: func [port [port!]] [either port/data [length? port/data] [0]]
    ]
    set-verbose: :tls-verbosity
    config: :tls-config
]
comment "-- End of:  %tls-scheme.reb"
comment "## Include: %tls-cipher-suites.reb"
comment {## Title:   "TLS Cipher Suite Configuration"}
TLS13-cipher-suites: make binary! 60
TLS12-cipher-suites: make binary! 60
if find system/catalog/ciphers 'chacha20-poly1305 [
    append TLS13-cipher-suites #{1303}
    append TLS12-cipher-suites #{CCA9CCA8}
]
if find system/catalog/ciphers 'aes-128-gcm [
    append TLS13-cipher-suites #{130213011304}
    append TLS12-cipher-suites #{C02BC02CC02F009C}
]
if find system/catalog/ciphers 'aes-128-cbc [
    append TLS12-cipher-suites #{C028C024C027C023C014C013C00AC009006B0067003D003C0035002F00390033}
]
suported-cipher-suites-binary: rejoin [
    #{
13021301C02CC030009FCCA9CCA8CCAAC02BC02F009EC024C028006BC023C027
0067C00AC0140039C009C0130033009D009C003D003C0035002F}
    #{00FF}
]
suported-cipher-suites: decode-list *Cipher-suite :suported-cipher-suites-binary _
supported-signature-algorithms: #{0403050306030807080408050806040105010601}
supported-elliptic-curves: make binary! 22
supported-groups: make block! 12
foreach curve system/schemes/tls/spec/supported-groups [
    if find system/catalog/elliptic-curves curve [
        append supported-groups curve
        binary/write tail supported-elliptic-curves [UI16BE :*EllipticCurves/:curve]
    ]
]
supported-versions: #{0403040303}
comment "-- End of:  %tls-cipher-suites.reb"
