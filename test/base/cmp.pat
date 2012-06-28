(1 == 1): Ok
(1 != 1): Fail
(str_t::cmp<lower_t>(CSTR("/dOC"), CSTR("/doc"))): Ok
(str_t::cmp<lower_t>(CSTR("/doc"), CSTR("/doc"))): Ok
(!str_t::cmp<ident_t>(CSTR("/doc"), CSTR("/tmp"))): Ok
(!str_t::cmp<lower_t>(CSTR("/doc"), CSTR("/tmp"))): Ok
(str_t::cmp<ident_t>(CSTR("/doc1"), CSTR("/doc2")).is_less()): Ok
(str_t::cmp<lower_t>(CSTR("/doc1"), CSTR("/doc2")).is_less()): Ok
(str_t::cmp<ident_t>(CSTR("/doc"), CSTR("/doc2")).is_less()): Ok
(str_t::cmp<lower_t>(CSTR("/DOC"), CSTR("/doc2")).is_less()): Ok
(!str_t::cmp_eq<ident_t>(CSTR("/doc"), CSTR("/tmp"))): Ok
(!str_t::cmp_eq<lower_t>(CSTR("/doc"), CSTR("/tmp"))): Ok
(str_t::cmp_eq<ident_t>(CSTR("/doc"), CSTR("/doc"))): Ok
(str_t::cmp_eq<lower_t>(CSTR("/doc"), CSTR("/Doc"))): Ok
