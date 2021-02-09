/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**  Copyright 2012 REBOL Technologies
**  REBOL is a trademark of REBOL Technologies
**  Licensed under the Apache License, Version 2.0
**  This is a code-generated file.
**
************************************************************************
**
**  Title: Function Prototypes
**  Build: A0
**  Date:  10-Feb-2021
**  File:  funcs.h
**
**  AUTO-GENERATED FILE - Do not modify. (From: make-headers)
**
***********************************************************************/


extern void *Extension_Lib(void); // a-lib.c
extern void Use_Natives(REBFUN *funcs, REBCNT limit); // b-init.c
extern REBVAL *Get_Action_Word(REBCNT action); // b-init.c
extern REBVAL *Get_Action_Value(REBCNT action); // b-init.c
extern void Init_UType_Proto(void); // b-init.c
extern void Set_Root_Series(REBVAL *value, REBSER *ser, const REBYTE *label); // b-init.c
extern REBINT Codec_Text(REBCDI *codi); // b-init.c
extern REBINT Codec_Markup(REBCDI *codi); // b-init.c
extern void Register_Codec(const char *name, codo dispatcher); // b-init.c
extern void Init_Task(void); // b-init.c
extern void Init_Year(void); // b-init.c
extern void Init_Core(REBARGS *rargs); // b-init.c
extern void Dispose_Core(void); // b-init.c
extern void Do_Op(REBVAL *func); // c-do.c
extern void Expand_Stack(REBCNT amount); // c-do.c
extern void DS_Ret_Int(REBINT n); // c-do.c
extern void DS_Ret_Val(REBVAL *value); // c-do.c
extern REBINT Eval_Depth(void); // c-do.c
extern REBVAL *Stack_Frame(REBCNT n); // c-do.c
extern void Trace_Value(REBINT n, REBVAL *value); // c-do.c
extern void Trace_String(REBINT n, REBYTE *str, REBINT limit); // c-do.c
extern void Trace_Error(REBVAL *value); // c-do.c
extern REBCNT Push_Func(REBFLG keep, REBSER *block, REBCNT index, REBCNT word, REBVAL *func); // c-do.c
extern void Next_Path(REBPVS *pvs); // c-do.c
extern REBVAL *Do_Path(REBVAL **path_val, REBVAL *val); // c-do.c
extern void Pick_Path(REBVAL *value, REBVAL *selector, REBVAL *val); // c-do.c
extern void Do_Signals(void); // c-do.c
extern REBCNT Do_Next(REBSER *block, REBCNT index, REBFLG op); // c-do.c
extern REBVAL *Do_Blk(REBSER *block, REBCNT index); // c-do.c
extern REBVAL *Do_Block_Value_Throw(REBVAL *block); // c-do.c
extern REBFLG Try_Block(REBSER *block, REBCNT index); // c-do.c
extern void Reduce_Block(REBSER *block, REBCNT index, REBVAL *into); // c-do.c
extern void Reduce_Only(REBSER *block, REBCNT index, REBVAL *words, REBVAL *into); // c-do.c
extern void Reduce_Block_No_Set(REBSER *block, REBCNT index, REBVAL *into); // c-do.c
extern void Reduce_Type_Stack(REBSER *block, REBCNT index, REBCNT type); // c-do.c
extern void Reduce_In_Frame(REBSER *frame, REBVAL *values); // c-do.c
extern void Compose_Block(REBVAL *block, REBFLG deep, REBFLG only, REBVAL *into); // c-do.c
extern void Apply_Block(REBVAL *func, REBVAL *args, REBFLG reduce); // c-do.c
extern REBVAL *Apply_Function(REBSER *wblk, REBCNT widx, REBVAL *func, va_list args); // c-do.c
extern REBVAL *Apply_Func(REBSER *where, REBVAL *func, ...); // c-do.c
extern REBVAL *Do_Sys_Func(REBCNT inum, ...); // c-do.c
extern void Do_Construct(REBVAL *value); // c-do.c
extern void Do_Min_Construct(REBVAL *value); // c-do.c
extern REBVAL *Do_Bind_Block(REBSER *frame, REBVAL *block); // c-do.c
extern void Reduce_Bind_Block(REBSER *frame, REBVAL *block, REBCNT binding); // c-do.c
extern REBOOL Try_Block_Halt(REBSER *block, REBCNT index); // c-do.c
extern REBVAL *Do_String(REBYTE *text, REBCNT flags); // c-do.c
extern void Halt_Code(REBINT kind, REBVAL *arg); // c-do.c
extern void Call_Func(REBVAL *func_val); // c-do.c
extern void Redo_Func(REBVAL *func_val); // c-do.c
extern REBVAL *Get_Simple_Value(REBVAL *val); // c-do.c
extern REBSER *Resolve_Path(REBVAL *path, REBCNT *index); // c-do.c
extern REBINT Init_Mezz(REBINT reserved); // c-do.c
extern void Check_Stack(void); // c-error.c
extern void Catch_Error(REBVAL *value); // c-error.c
extern void Throw_Error(REBSER *err); // c-error.c
extern void Throw_Break(REBVAL *val); // c-error.c
extern void Throw_Return_Series(REBCNT type, REBSER *series); // c-error.c
extern void Throw_Return_Value(REBVAL *value); // c-error.c
extern void Trap_Stack(void); // c-error.c
extern REBCNT Stack_Depth(void); // c-error.c
extern REBSER *Make_Backtrace(REBINT start); // c-error.c
extern void Set_Error_Type(ERROR_OBJ *error); // c-error.c
extern REBVAL *Find_Error_Info(ERROR_OBJ *error, REBINT *num); // c-error.c
extern void Make_Error_Object(REBVAL *arg, REBVAL *value); // c-error.c
extern REBSER *Make_Error(REBINT code, REBVAL *arg1, REBVAL *arg2, REBVAL *arg3); // c-error.c
extern void Trap0(REBCNT num); // c-error.c
extern void Trap1(REBCNT num, REBVAL *arg1); // c-error.c
extern void Trap2(REBCNT num, REBVAL *arg1, REBVAL *arg2); // c-error.c
extern void Trap3(REBCNT num, REBVAL *arg1, REBVAL *arg2, REBVAL *arg3); // c-error.c
extern void Trap_Arg(REBVAL *arg); // c-error.c
extern void Trap_Type(REBVAL *arg); // c-error.c
extern void Trap_Range(REBVAL *arg); // c-error.c
extern void Trap_Word(REBCNT num, REBCNT sym, REBVAL *arg); // c-error.c
extern void Trap_Action(REBCNT type, REBCNT action); // c-error.c
extern void Trap_Math_Args(REBCNT type, REBCNT action); // c-error.c
extern void Trap_Types(REBCNT errnum, REBCNT type1, REBCNT type2); // c-error.c
extern void Trap_Expect(REBVAL *object, REBCNT index, REBCNT type); // c-error.c
extern void Trap_Make(REBCNT type, REBVAL *spec); // c-error.c
extern void Trap_Num(REBCNT err, REBCNT num); // c-error.c
extern void Trap_Reflect(REBCNT type, REBVAL *arg); // c-error.c
extern void Trap_Port(REBCNT errnum, REBSER *port, REBINT err_code); // c-error.c
extern REBINT Check_Error(REBVAL *val); // c-error.c
extern void Init_Errors(REBVAL *errors); // c-error.c
extern REBYTE *Security_Policy(REBCNT sym, REBVAL *name); // c-error.c
extern void Trap_Security(REBCNT flag, REBCNT sym, REBVAL *value); // c-error.c
extern void Check_Security(REBCNT sym, REBCNT policy, REBVAL *value); // c-error.c
extern void Check_Bind_Table(void); // c-frame.c
extern REBSER *Make_Frame(REBINT len); // c-frame.c
extern void Expand_Frame(REBSER *frame, REBCNT delta, REBCNT copy); // c-frame.c
extern REBVAL *Append_Frame(REBSER *frame, REBVAL *word, REBCNT sym); // c-frame.c
extern void Collect_Start(REBCNT modes); // c-frame.c
extern REBSER *Collect_End(REBSER *prior); // c-frame.c
extern void Collect_Object(REBSER *prior); // c-frame.c
extern void Collect_Words(REBVAL *block, REBFLG modes); // c-frame.c
extern REBSER *Collect_Frame(REBFLG modes, REBSER *prior, REBVAL *block); // c-frame.c
extern void Collect_Simple_Words(REBVAL *block, REBCNT modes); // c-frame.c
extern REBSER *Collect_Block_Words(REBVAL *block, REBVAL *prior, REBCNT modes); // c-frame.c
extern REBSER *Create_Frame(REBSER *words, REBSER *spec); // c-frame.c
extern void Rebind_Frame(REBSER *src_frame, REBSER *dst_frame); // c-frame.c
extern REBSER *Make_Object(REBSER *parent, REBVAL *block); // c-frame.c
extern REBSER *Construct_Object(REBSER *parent, REBVAL *block, REBFLG asis); // c-frame.c
extern REBSER *Make_Object_Block(REBSER *frame, REBINT mode); // c-frame.c
extern void Assert_Public_Object(REBVAL *value); // c-frame.c
extern REBVAL *Make_Module(REBVAL *spec); // c-frame.c
extern REBSER *Make_Module_Spec(REBVAL *block); // c-frame.c
extern REBSER *Merge_Frames(REBSER *parent1, REBSER *parent2); // c-frame.c
extern void Resolve_Context(REBSER *target, REBSER *source, REBVAL *only_words, REBFLG all, REBFLG expand); // c-frame.c
extern void Bind_Block(REBSER *frame, REBVAL *block, REBCNT mode); // c-frame.c
extern void Unbind_Block(REBVAL *val, REBCNT deep); // c-frame.c
extern REBCNT Bind_Word(REBSER *frame, REBVAL *word); // c-frame.c
extern void Bind_Relative(REBSER *words, REBSER *frame, REBSER *block); // c-frame.c
extern void Bind_Stack_Block(REBSER *frame, REBSER *block); // c-frame.c
extern void Bind_Stack_Word(REBSER *frame, REBVAL *word); // c-frame.c
extern void Rebind_Block(REBSER *src_frame, REBSER *dst_frame, REBVAL *data, REBFLG modes); // c-frame.c
extern REBCNT Find_Arg_Index(REBSER *args, REBCNT sym); // c-frame.c
extern REBCNT Find_Word_Index(REBSER *frame, REBCNT sym, REBFLG always); // c-frame.c
extern REBVAL *Find_Word_Value(REBSER *frame, REBCNT sym); // c-frame.c
extern REBVAL *Find_In_Contexts(REBCNT sym, REBVAL *where); // c-frame.c
extern REBCNT Find_Word(REBSER *series, REBCNT index, REBCNT sym); // c-frame.c
extern REBSER* Get_Object_Words(REBVAL *object); // c-frame.c
extern REBVAL *Get_Var(REBVAL *word); // c-frame.c
extern REBVAL *Get_Var_Safe(REBVAL *word); // c-frame.c
extern REBVAL *Get_Var_No_Trap(REBVAL *word); // c-frame.c
extern REBVAL *Get_Any_Var(REBVAL *item); // c-frame.c
extern void Set_Var(REBVAL *word, REBVAL *value); // c-frame.c
extern void Set_Var_Series(REBVAL *var, REBCNT type, REBSER *series, REBCNT index); // c-frame.c
extern void Set_Var_Basic(REBVAL *var, REBCNT type, ...); // c-frame.c
extern REBVAL *Obj_Word(REBVAL *value, REBCNT index); // c-frame.c
extern REBVAL *Obj_Value(REBVAL *value, REBCNT index); // c-frame.c
extern void Init_Obj_Value(REBVAL *value, REBSER *frame); // c-frame.c
extern void Check_Frame(REBSER *frame); // c-frame.c
extern void Init_Frame(void); // c-frame.c
extern REBSER *List_Func_Words(REBVAL *func); // c-function.c
extern REBSER *List_Func_Types(REBVAL *func); // c-function.c
extern REBSER *Check_Func_Spec(REBSER *block); // c-function.c
extern void Make_Native(REBVAL *value, REBSER *spec, REBFUN func, REBINT type); // c-function.c
extern REBFLG Make_Function(REBCNT type, REBVAL *value, REBVAL *def); // c-function.c
extern REBFLG Copy_Function(REBVAL *value, REBVAL *args); // c-function.c
extern void Clone_Function(REBVAL *value, REBVAL *func); // c-function.c
extern void Do_Native(REBVAL *func); // c-function.c
extern void Do_Act(REBVAL *ds, REBCNT type, REBCNT act); // c-function.c
extern void Do_Action(REBVAL *func); // c-function.c
extern void Do_Function(REBVAL *func); // c-function.c
extern void Do_Closure(REBVAL *func); // c-function.c
extern REBCNT Register_Handle(REBCNT sym, REBCNT size, REB_HANDLE_FREE_FUNC free_func); // c-handle.c
extern REBHOB* Make_Handle_Context(REBCNT sym); // c-handle.c
extern REBCNT Find_Handle_Index(REBCNT sym); // c-handle.c
extern void Init_Handles(); // c-handle.c
extern REBVAL *Make_Port(REBVAL *spec); // c-port.c
extern REBFLG Is_Port_Open(REBSER *port); // c-port.c
extern void Set_Port_Open(REBSER *port, REBFLG flag); // c-port.c
extern void *Use_Port_State(REBSER *port, REBCNT device, REBCNT size); // c-port.c
extern REBFLG Pending_Port(REBVAL *port); // c-port.c
extern REBINT Awake_System(REBSER *ports, REBINT only); // c-port.c
extern REBINT Wait_Ports(REBSER *ports, REBCNT timeout, REBINT only); // c-port.c
extern void Sieve_Ports(REBSER *ports); // c-port.c
extern REBVAL *Form_Write(REBVAL *arg, REBYTE *newline); // c-port.c
extern REBCNT Find_Action(REBVAL *object, REBCNT action); // c-port.c
extern int Do_Port_Action(REBSER *port, REBCNT action); // c-port.c
extern void Secure_Port(REBCNT kind, REBREQ *req, REBVAL *name, REBSER *path); // c-port.c
extern void Validate_Port(REBSER *port, REBCNT action); // c-port.c
extern void Register_Scheme(REBCNT sym, const PORT_ACTION *map, REBPAF fun); // c-port.c
extern void Init_Ports(void); // c-port.c
extern void Dispose_Ports(void); // c-port.c
extern void Do_Task(REBVAL *task); // c-task.c
extern REBINT Get_Hash_Prime(REBCNT size); // c-word.c
extern void Expand_Hash(REBSER *ser); // c-word.c
extern REBCNT Make_Word(const REBYTE *str, REBCNT len); // c-word.c
extern REBCNT Last_Word_Num(void); // c-word.c
extern void Set_Word(REBVAL *value, REBINT sym, REBSER *frame, REBCNT index); // c-word.c
extern void Init_Word(REBVAL *value, REBCNT sym); // c-word.c
extern void Init_Frame_Word(REBVAL *value, REBCNT sym); // c-word.c
extern REBYTE *Get_Sym_Name(REBCNT num); // c-word.c
extern REBYTE *Get_Word_Name(REBVAL *value); // c-word.c
extern REBYTE *Get_Type_Name(REBVAL *value); // c-word.c
extern REBINT Compare_Word(REBVAL *s, REBVAL *t, REBFLG is_case); // c-word.c
extern void Init_Words(REBFLG only); // c-word.c
extern void Crash(REBINT id, ...); // d-crash.c
extern void NA(void); // d-crash.c
extern void Dump_Series_Fmt(REBSER *series, const char *memo); // d-dump.c
extern void Dump_Series(REBSER *series, const char *memo); // d-dump.c
extern void Dump_Bytes(REBYTE *bp, REBCNT limit); // d-dump.c
extern void Dump_Values(REBVAL *vp, REBCNT count); // d-dump.c
extern void Dump_Info(void); // d-dump.c
extern void Dump_Stack(REBINT dsf, REBINT dsp); // d-dump.c
extern void Init_StdIO(void); // d-print.c
extern void Dispose_StdIO(void); // d-print.c
extern void Out_Value(REBVAL *value, REBCNT limit, REBOOL mold, REBINT lines); // d-print.c
extern void Out_Str(const REBYTE *bp, REBINT lines); // d-print.c
extern void Enable_Backtrace(REBFLG on); // d-print.c
extern void Display_Backtrace(REBCNT lines); // d-print.c
extern void Debug_String(const REBYTE *bp, REBINT len, REBOOL uni, REBINT lines); // d-print.c
extern void Debug_Line(void); // d-print.c
extern void Debug_Str(const char *str); // d-print.c
extern void Debug_Uni(REBSER *ser); // d-print.c
extern void Debug_Series(REBSER *ser); // d-print.c
extern void Debug_Num(REBYTE *str, REBINT num); // d-print.c
extern void Debug_Chars(REBYTE chr, REBCNT num); // d-print.c
extern void Debug_Space(REBCNT num); // d-print.c
extern void Debug_Word(REBVAL *word); // d-print.c
extern void Debug_Type(REBVAL *value); // d-print.c
extern void Debug_Value(REBVAL *value, REBCNT limit, REBOOL mold); // d-print.c
extern void Debug_Values(REBVAL *value, REBCNT count, REBCNT limit); // d-print.c
extern void Debug_Buf(const REBYTE *fmt, va_list args); // d-print.c
extern void Debug_Fmt_(const REBYTE *fmt, ...); // d-print.c
extern void Debug_Fmt(const REBYTE *fmt, ...); // d-print.c
extern REBFLG Echo_File(REBCHR *file); // d-print.c
extern REBYTE *Input_Str(void); // d-print.c
extern REBYTE *Form_Hex_Pad(REBYTE *buf, REBU64 val, REBINT len); // d-print.c
extern REBYTE *Form_Hex2(REBYTE *bp, REBCNT val); // d-print.c
extern REBUNI *Form_Hex2_Uni(REBUNI *up, REBCNT val); // d-print.c
extern REBUNI *Form_Hex_Esc_Uni(REBUNI *up, REBUNI c); // d-print.c
extern REBUNI *Form_RGB_Uni(REBUNI *up, REBCNT val); // d-print.c
extern REBUNI *Form_Uni_Hex(REBUNI *out, REBCNT n); // d-print.c
extern REBYTE *Form_Var_Args(REBYTE *bp, REBCNT max, const REBYTE *fmt, va_list args); // d-print.c
extern void Prin_Value(REBVAL *value, REBCNT limit, REBOOL mold); // d-print.c
extern void Print_Value(REBVAL *value, REBCNT limit, REBOOL mold); // d-print.c
extern void Print_Mold_Block(REBVAL *block, REBCNT limit); // d-print.c
extern REBYTE *Form_Args(REBYTE *cp, REBCNT limit, REBYTE *fmt, ...); // d-print.c
extern void Init_Raw_Print(void); // d-print.c
extern REBSER *Make_Block(REBCNT length); // f-blocks.c
extern REBSER *Copy_Block(REBSER *block, REBCNT index); // f-blocks.c
extern REBSER *Copy_Block_Len(REBSER *block, REBCNT index, REBCNT len); // f-blocks.c
extern REBSER *Copy_Values(REBVAL *blk, REBCNT length); // f-blocks.c
extern void Copy_Deep_Values(REBSER *block, REBCNT index, REBCNT tail, REBU64 types); // f-blocks.c
extern REBSER *Copy_Block_Values(REBSER *block, REBCNT index, REBCNT tail, REBU64 types); // f-blocks.c
extern REBSER *Clone_Block(REBSER *block); // f-blocks.c
extern REBSER *Clone_Block_Value(REBVAL *code); // f-blocks.c
extern REBSER *Copy_Expand_Block(REBSER *block, REBCNT extra); // f-blocks.c
extern void Copy_Stack_Values(REBINT start, REBVAL *into); // f-blocks.c
extern REBVAL *Append_Value(REBSER *block); // f-blocks.c
extern void Append_Block(REBSER *block, REBSER *added); // f-blocks.c
extern void Append_Val(REBSER *block, REBVAL *val); // f-blocks.c
extern REBINT Find_Same_Block(REBSER *blk, REBVAL *val); // f-blocks.c
extern REBSER *Copy_Side_Series(REBSER *ser); // f-blocks.c
extern void Clear_Value_Opts(REBSER *ser); // f-blocks.c
extern void Unmark(REBVAL *val); // f-blocks.c
extern const REBYTE *Decode_Binary(REBVAL *value, const REBYTE *src, REBCNT len, REBINT base, REBYTE delim, REBOOL urlSafe); // f-enbase.c
extern REBSER *Encode_Base2(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk); // f-enbase.c
extern REBSER *Encode_Base16(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk); // f-enbase.c
extern REBSER *Encode_Base64(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk, REBOOL urlSafe); // f-enbase.c
extern REBSER *Encode_Base85(REBVAL *value, REBSER *series, REBCNT len, REBFLG brk); // f-enbase.c
extern void Make_Command(REBVAL *value, REBVAL *def); // f-extension.c
extern void Do_Command(REBVAL *value); // f-extension.c
extern void Do_Commands(REBSER *cmds, void *context); // f-extension.c
extern const REBYTE *Grab_Int(const REBYTE *cp, REBINT *val); // f-math.c
extern const REBYTE *Grab_Int_Scale(const REBYTE *cp, REBINT *val, REBCNT scale); // f-math.c
extern REBINT Form_Int_Len(REBYTE *buf, REBI64 val, REBINT maxl); // f-math.c
extern REBYTE *Form_Int_Pad(REBYTE *buf, REBI64 val, REBINT max, REBINT len, REBYTE pad); // f-math.c
extern REBYTE *Form_Int(REBYTE *buf, REBINT val); // f-math.c
extern REBYTE *Form_Integer(REBYTE *buf, REBI64 val); // f-math.c
extern REBINT Emit_Integer(REBYTE *buf, REBI64 val); // f-math.c
extern REBINT Emit_Decimal(REBYTE *cp, REBDEC d, REBFLG trim, REBYTE point, REBINT decimal_digits); // f-math.c
extern REBCNT Modify_Block(REBCNT action, REBSER *dst_ser, REBCNT dst_idx, REBVAL *src_val, REBCNT flags, REBINT dst_len, REBINT dups); // f-modify.c
extern REBCNT Modify_String(REBCNT action, REBSER *dst_ser, REBCNT dst_idx, REBVAL *src_val, REBCNT flags, REBINT dst_len, REBINT dups); // f-modify.c
extern void Set_Random(REBI64 seed); // f-random.c
extern REBI64 Random_Int(REBFLG secure); // f-random.c
extern REBI64 Random_Range(REBI64 r, REBFLG secure); // f-random.c
extern REBDEC Random_Dec(REBDEC r, REBFLG secure); // f-random.c
extern void Random_Bytes(REBYTE* dest, REBCNT length, REBOOL	no_zeros); // f-random.c
extern REBCNT Get_Round_Flags(REBVAL *ds); // f-round.c
extern REBDEC Round_Dec(REBDEC dec, REBCNT flags, REBDEC scale); // f-round.c
extern REBI64 Round_Int(REBI64 num, REBCNT flags, REBI64 scale); // f-round.c
extern REBDCI Round_Deci(REBDCI num, REBCNT flags, REBDCI scale); // f-round.c
extern REBINT Do_Series_Action(REBCNT action, REBVAL *value, REBVAL *arg); // f-series.c
extern REBINT Cmp_Block(REBVAL *sval, REBVAL *tval, REBFLG is_case); // f-series.c
extern REBINT Cmp_Value(REBVAL *s, REBVAL *t, REBFLG is_case); // f-series.c
extern REBCNT Find_Block_Simple(REBSER *series, REBCNT index, REBVAL *target); // f-series.c
extern void REBCNT_To_Bytes(REBYTE *out, REBCNT in); // f-stubs.c
extern REBCNT Bytes_To_REBCNT(REBYTE const *in); // f-stubs.c
extern REBCNT Find_Int(REBINT *array, REBINT num); // f-stubs.c
extern REBINT Get_Num_Arg(REBVAL *val); // f-stubs.c
extern REBINT Float_Int16(REBD32 f); // f-stubs.c
extern REBINT Int32(REBVAL *val); // f-stubs.c
extern REBINT Int32s(REBVAL *val, REBINT sign); // f-stubs.c
extern REBI64 Int64(REBVAL *val); // f-stubs.c
extern REBDEC Dec64(REBVAL *val); // f-stubs.c
extern REBI64 Int64s(REBVAL *val, REBINT sign); // f-stubs.c
extern REBINT Int8u(REBVAL *val); // f-stubs.c
extern REBCNT Find_Refines(REBVAL *ds, REBCNT mask); // f-stubs.c
extern void Assert_Max_Refines(REBVAL *ds, REBCNT limit); // f-stubs.c
extern void Set_Datatype(REBVAL *value, REBINT n); // f-stubs.c
extern REBVAL *Get_Type(REBCNT index); // f-stubs.c
extern REBVAL *Of_Type(REBVAL *value); // f-stubs.c
extern REBINT Get_Type_Sym(REBCNT type); // f-stubs.c
extern REBVAL *Get_Type_Word(REBCNT type); // f-stubs.c
extern REBYTE *Get_Field_Name(REBSER *obj, REBCNT index); // f-stubs.c
extern REBVAL *Get_Field(REBSER *obj, REBCNT index); // f-stubs.c
extern REBVAL *Get_Object(REBVAL *objval, REBCNT index); // f-stubs.c
extern REBVAL *In_Object(REBSER *base, ...); // f-stubs.c
extern REBVAL *Get_System(REBCNT i1, REBCNT i2); // f-stubs.c
extern REBINT Get_System_Int(REBCNT i1, REBCNT i2, REBINT default_int); // f-stubs.c
extern REBSER *Make_Std_Object(REBCNT index); // f-stubs.c
extern void Set_Object_Values(REBSER *obj, REBVAL *vals); // f-stubs.c
extern void Set_Series(REBINT type, REBVAL *value, REBSER *series); // f-stubs.c
extern void Set_Block(REBVAL *value, REBSER *series); // f-stubs.c
extern void Set_Block_Index(REBVAL *value, REBSER *series, REBCNT index); // f-stubs.c
extern void Set_String(REBVAL *value, REBSER *series); // f-stubs.c
extern void Set_Binary(REBVAL *value, REBSER *series); // f-stubs.c
extern void Set_Tuple(REBVAL *value, REBYTE *bytes, REBCNT len); // f-stubs.c
extern void Set_Object(REBVAL *value, REBSER *series); // f-stubs.c
extern REBCNT Val_Series_Len(REBVAL *value); // f-stubs.c
extern REBCNT Val_Byte_Len(REBVAL *value); // f-stubs.c
extern REBFLG Get_Logic_Arg(REBVAL *arg); // f-stubs.c
extern REBCNT Get_Part_Length(REBVAL *bval, REBVAL *eval); // f-stubs.c
extern REBCNT Partial1(REBVAL *sval, REBVAL *lval); // f-stubs.c
extern REBINT Partial(REBVAL *aval, REBVAL *bval, REBVAL *lval, REBFLG flag); // f-stubs.c
extern void Define_File_Global(REBYTE *name, REBYTE *content); // f-stubs.c
extern int Clip_Int(int val, int mini, int maxi); // f-stubs.c
extern REBDEC Clip_Dec(REBDEC val, REBDEC mind, REBDEC maxd); // f-stubs.c
extern void memswapl(void *m1, void *m2, size_t len); // f-stubs.c
extern i64 Add_Max(int type, i64 n, i64 m, i64 maxi); // f-stubs.c
extern int Mul_Max(int type, i64 n, i64 m, i64 maxi); // f-stubs.c
extern REBVAL *Make_OS_Error(int errnum); // f-stubs.c
extern REBSER *At_Head(REBVAL *value); // f-stubs.c
extern REBSER *Collect_Set_Words(REBVAL *val); // f-stubs.c
extern REBINT What_Reflector(REBVAL *word); // f-stubs.c
extern const REBYTE *Scan_Item(const REBYTE *src, const REBYTE *end, REBUNI term, const REBYTE *invalid); // l-scan.c
extern REBSER *Scan_Source(const REBYTE *src, REBCNT len); // l-scan.c
extern REBINT Scan_Header(const REBYTE *src, REBCNT len); // l-scan.c
extern void Init_Scanner(void); // l-scan.c
extern REBCNT Scan_Word(const REBYTE *cp, REBCNT len); // l-scan.c
extern REBCNT Scan_Issue(const REBYTE *cp, REBCNT len); // l-scan.c
extern const REBYTE *Scan_Hex(const REBYTE *cp, REBI64 *num, REBCNT minlen, REBCNT maxlen); // l-types.c
extern REBOOL Scan_Hex2(const REBYTE *bp, REBUNI *n, REBFLG uni); // l-types.c
extern REBINT Scan_Hex_Bytes(REBVAL *val, REBCNT maxlen, REBYTE *out); // l-types.c
extern REBCNT Scan_Hex_Value(const void *src, REBCNT len, REBOOL uni); // l-types.c
extern const REBYTE *Scan_Dec_Buf(const REBYTE *cp, REBCNT len, REBYTE *buf); // l-types.c
extern const REBYTE *Scan_Decimal(const REBYTE *cp, REBCNT len, REBVAL *value, REBFLG dec_only); // l-types.c
extern const REBYTE *Scan_Integer(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_Money(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_Date(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_String(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_File(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_Email(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_Ref(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_URL(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_Pair(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_Tuple(const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern REBINT Scan_Binary_Base(const REBYTE *cp, REBCNT len); // l-types.c
extern const REBYTE *Scan_Binary(REBINT base, const REBYTE *cp, REBCNT len, REBVAL *value); // l-types.c
extern const REBYTE *Scan_Any(const REBYTE *cp, REBCNT len, REBVAL *value, REBYTE type); // l-types.c
extern REBSER *Load_Markup(const REBYTE *cp, REBINT len); // l-types.c
extern REBFLG Construct_Value(REBVAL *value, REBSER *spec); // l-types.c
extern REBSER *Scan_Net_Header(REBSER *blk, REBYTE *str); // l-types.c
extern REBCNT Recycle(void); // m-gc.c
extern void Save_Series(REBSER *series); // m-gc.c
extern void Guard_Series(REBSER *series); // m-gc.c
extern void Loose_Series(REBSER *series); // m-gc.c
extern void Init_Memory(REBINT scale); // m-gc.c
extern void Dispose_Memory(void); // m-gc.c
extern void *Make_Mem(size_t size); // m-pools.c
extern void Free_Mem(void *mem, size_t size); // m-pools.c
extern void Init_Pools(REBINT scale); // m-pools.c
extern void *Make_Node(REBCNT pool_id); // m-pools.c
extern void Free_Node(REBCNT pool_id, REBNOD *node); // m-pools.c
extern REBSER *Make_Series_Data(REBSER *series, REBCNT length); // m-pools.c
extern REBSER *Make_Series(REBCNT length, REBCNT wide, REBOOL powerof2); // m-pools.c
extern void Free_Series_Data(REBSER *series, REBOOL protect); // m-pools.c
extern void Free_Series(REBSER *series); // m-pools.c
extern void Free_Gob(REBGOB *gob); // m-pools.c
extern void Free_Hob(REBHOB *hob); // m-pools.c
extern void Prop_Series(REBSER *newser, REBSER *oldser); // m-pools.c
extern REBFLG Series_In_Pool(REBSER *series); // m-pools.c
extern REBCNT Check_Memory(void); // m-pools.c
extern void Dump_All(REBINT size); // m-pools.c
extern void Dump_Series_In_Pool(int pool_id); // m-pools.c
extern void Dump_Handles(void); // m-pools.c
extern REBU64 Inspect_Series(REBCNT flags); // m-pools.c
extern void Dispose_Pools(void); // m-pools.c
extern void Expand_Series(REBSER *series, REBCNT index, REBCNT delta); // m-series.c
extern void Extend_Series(REBSER *series, REBCNT delta); // m-series.c
extern REBCNT Insert_Series(REBSER *series, REBCNT index, REBYTE *data, REBCNT len); // m-series.c
extern void Append_Series(REBSER *series, const REBYTE *data, REBCNT len); // m-series.c
extern void Append_Mem_Extra(REBSER *series, const REBYTE *data, REBCNT len, REBCNT extra); // m-series.c
extern REBSER *Copy_Series(REBSER *source); // m-series.c
extern REBSER *Copy_Series_Part(REBSER *source, REBCNT index, REBCNT length); // m-series.c
extern REBSER *Copy_Series_Value(REBVAL *value); // m-series.c
extern REBINT Clone_Series(REBVAL *dst, REBVAL *src); // m-series.c
extern void Remove_Series(REBSER *series, REBCNT index, REBINT len); // m-series.c
extern void Remove_Last(REBSER *series); // m-series.c
extern void Reset_Bias(REBSER *series); // m-series.c
extern void Reset_Series(REBSER *series); // m-series.c
extern void Clear_Series(REBSER *series); // m-series.c
extern void Resize_Series(REBSER *series, REBCNT size); // m-series.c
extern void Terminate_Series(REBSER *series); // m-series.c
extern void Shrink_Series(REBSER *series, REBCNT units); // m-series.c
extern REBYTE *Reset_Buffer(REBSER *buf, REBCNT len); // m-series.c
extern REBSER *Copy_Buffer(REBSER *buf, void *end); // m-series.c
extern void Protected(REBVAL *word); // n-control.c
extern void Protect_Series(REBVAL *val, REBCNT flags); // n-control.c
extern void Protect_Object(REBVAL *value, REBCNT flags); // n-control.c
extern void Init_Crypt(); // n-crypt.c
extern REBSER *Block_To_String_List(REBVAL *blk); // n-io.c
extern REBINT Compare_Values(REBVAL *a, REBVAL *b, REBINT strictness); // n-math.c
extern void Init_Checksum_Scheme(void); // p-checksum.c
extern void Init_Clipboard_Scheme(void); // p-clipboard.c
extern void Ret_Query_Console(REBREQ *req, REBVAL *ret, REBVAL *info, REBVAL *spec); // p-console.c
extern void Init_Console_Scheme(void); // p-console.c
extern void Init_Dir_Scheme(void); // p-dir.c
extern void Init_DNS_Scheme(void); // p-dns.c
extern REBVAL *Append_Event(void); // p-event.c
extern REBVAL *Find_Event (REBINT model, REBINT type, void* ser); // p-event.c
extern void Init_Event_Scheme(void); // p-event.c
extern void Ret_Query_File(REBSER *port, REBREQ *file, REBVAL *ret, REBVAL *info); // p-file.c
extern void Ret_File_Modes(REBSER *port, REBVAL *ret); // p-file.c
extern void Init_File_Scheme(void); // p-file.c
extern void Init_TCP_Scheme(void); // p-net.c
extern void Init_UDP_Scheme(void); // p-net.c
extern void Init_MIDI_Scheme(void); // p-midi.c
extern void Init_Char_Cases(void); // s-cases.c
extern void Dispose_Char_Cases(void); // s-cases.c
extern REBINT Compute_CRC24(REBYTE *str, REBCNT len); // s-crc.c
extern REBINT Hash_String(REBYTE *str, REBCNT len); // s-crc.c
extern REBINT Hash_Word(const REBYTE *str, REBINT len); // s-crc.c
extern REBINT Hash_Value(REBVAL *val, REBCNT hash_size); // s-crc.c
extern REBSER *Make_Hash_Array(REBCNT len); // s-crc.c
extern REBSER *Hash_Block(REBVAL *block, REBCNT cased); // s-crc.c
extern void Init_CRC(void); // s-crc.c
extern void Dispose_CRC(void); // s-crc.c
extern REBINT Compute_IPC(REBYTE *data, REBCNT length); // s-crc.c
extern REBCNT CRC32(REBYTE *buf, REBCNT len); // s-crc.c
extern REBSER *To_REBOL_Path(void *bp, REBCNT len, REBINT uni, REBFLG dir); // s-file.c
extern REBSER *Value_To_REBOL_Path(REBVAL *val, REBOOL dir); // s-file.c
extern REBSER *To_Local_Path(void *bp, REBCNT len, REBOOL uni, REBFLG full); // s-file.c
extern REBSER *Value_To_Local_Path(REBVAL *val, REBFLG full); // s-file.c
extern REBSER *Value_To_OS_Path(REBVAL *val, REBFLG full); // s-file.c
extern REBINT Compare_Binary_Vals(REBVAL *v1, REBVAL *v2); // s-find.c
extern REBINT Compare_Bytes(const REBYTE *b1, const REBYTE *b2, REBCNT len, REBOOL uncase); // s-find.c
extern const REBYTE *Match_Bytes(const REBYTE *src, const REBYTE *pat); // s-find.c
extern REBFLG Match_Sub_Path(REBSER *s1, REBSER *s2); // s-find.c
extern REBINT Compare_Uni_Byte(REBUNI *u1, REBYTE *b2, REBCNT len, REBOOL uncase); // s-find.c
extern REBINT Compare_Uni_Str(REBUNI *u1, REBUNI *u2, REBCNT len, REBOOL uncase); // s-find.c
extern REBINT Compare_String_Vals(REBVAL *v1, REBVAL *v2, REBOOL uncase); // s-find.c
extern REBINT Compare_UTF8(const REBYTE *s1, const REBYTE *s2, REBCNT l2); // s-find.c
extern REBCNT Find_Byte_Str(REBSER *series, REBCNT index, REBYTE *b2, REBCNT l2, REBFLG uncase, REBFLG match); // s-find.c
extern REBCNT Find_Str_Str(REBSER *ser1, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBSER *ser2, REBCNT index2, REBCNT len, REBCNT flags); // s-find.c
extern REBCNT Find_Str_Str_Any(REBSER *ser1, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBSER *ser2, REBCNT index2, REBCNT len, REBCNT flags, REBVAL *wild); // s-find.c
extern REBCNT Find_Str_Char(REBSER *ser, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBUNI c2, REBCNT flags); // s-find.c
extern REBCNT Find_Str_Bitset(REBSER *ser, REBCNT head, REBCNT index, REBCNT tail, REBINT skip, REBSER *bset, REBCNT flags); // s-find.c
extern REBCNT Find_Str_Wild(REBSER *ser, REBCNT index, REBCNT tail); // s-find.c
extern REBCNT Count_Lines(REBYTE *bp, REBCNT len); // s-find.c
extern REBCNT Next_Line(REBYTE **bin); // s-find.c
extern REBSER *Make_Binary(REBCNT length); // s-make.c
extern REBSER *Make_Unicode(REBCNT length); // s-make.c
extern REBSER *Copy_Bytes(const REBYTE *src, REBINT len); // s-make.c
extern REBSER *Copy_Bytes_To_Unicode(REBYTE *src, REBINT len); // s-make.c
extern REBSER *Copy_Wide_Str(void *src, REBINT len); // s-make.c
extern REBSER *Copy_OS_Str(void *src, REBINT len); // s-make.c
extern void Widen_String(REBSER *series); // s-make.c
extern void Insert_Char(REBSER *dst, REBCNT index, REBCNT chr); // s-make.c
extern void Insert_String(REBSER *dst, REBCNT idx, REBSER *src, REBCNT pos, REBCNT len, REBFLG no_expand); // s-make.c
extern REBSER *Copy_String(REBSER *src, REBCNT index, REBINT length); // s-make.c
extern REBCHR *Val_Str_To_OS(REBVAL *val); // s-make.c
extern REBSER *Append_Bytes_Len(REBSER *dst, const REBYTE *src, REBCNT len); // s-make.c
extern REBSER *Append_Bytes(REBSER *dst, const char *src); // s-make.c
extern REBSER *Append_Byte(REBSER *dst, REBCNT chr); // s-make.c
extern void Append_Uni_Bytes(REBSER *dst, REBUNI *src, REBCNT len); // s-make.c
extern void Append_Uni_Uni(REBSER *dst, REBUNI *src, REBCNT len); // s-make.c
extern void Append_String(REBSER *dst, REBSER *src, REBCNT i, REBCNT len); // s-make.c
extern void Append_Boot_Str(REBSER *dst, REBINT num); // s-make.c
extern void Append_Int(REBSER *dst, REBINT num); // s-make.c
extern void Append_Int_Pad(REBSER *dst, REBINT num, REBINT digs); // s-make.c
extern REBSER *Append_UTF8(REBSER *dst, const REBYTE *src, REBINT len); // s-make.c
extern REBSER *Join_Binary(REBVAL *blk); // s-make.c
extern REBSER *Emit(REB_MOLD *mold, char *fmt, ...); // s-mold.c
extern REBSER *Prep_String(REBSER *series, REBYTE **str, REBCNT len); // s-mold.c
extern REBUNI *Prep_Uni_Series(REB_MOLD *mold, REBCNT len); // s-mold.c
extern void Pre_Mold(REBVAL *value, REB_MOLD *mold); // s-mold.c
extern void End_Mold(REB_MOLD *mold); // s-mold.c
extern void Post_Mold(REBVAL *value, REB_MOLD *mold); // s-mold.c
extern void New_Indented_Line(REB_MOLD *mold); // s-mold.c
extern void Mold_Binary(REBVAL *value, REB_MOLD *mold); // s-mold.c
extern void Mold_Value(REB_MOLD *mold, REBVAL *value, REBFLG molded); // s-mold.c
extern REBSER *Copy_Form_Value(REBVAL *value, REBCNT opts); // s-mold.c
extern REBSER *Copy_Mold_Value(REBVAL *value, REBCNT opts); // s-mold.c
extern REBSER *Form_Reduce(REBSER *block, REBCNT index); // s-mold.c
extern REBSER *Form_Tight_Block(REBVAL *blk); // s-mold.c
extern void Reset_Mold(REB_MOLD *mold); // s-mold.c
extern REBSER *Mold_Print_Value(REBVAL *value, REBCNT limit, REBFLG mold); // s-mold.c
extern void Init_Mold(REBCNT size); // s-mold.c
extern void Dispose_Mold(); // s-mold.c
extern REBOOL Is_Not_ASCII(REBYTE *bp, REBCNT len); // s-ops.c
extern REBOOL Is_Wide(REBUNI *up, REBCNT len); // s-ops.c
extern REBYTE *Qualify_String(REBVAL *val, REBINT max_len, REBCNT *length, REBINT opts); // s-ops.c
extern REBSER *Prep_Bin_Str(REBVAL *val, REBCNT *index, REBCNT *length); // s-ops.c
extern REBSER *Xandor_Binary(REBCNT action, REBVAL *value, REBVAL *arg); // s-ops.c
extern REBSER *Complement_Binary(REBVAL *value); // s-ops.c
extern void Shuffle_String(REBVAL *value, REBFLG secure); // s-ops.c
extern REBOOL Cloak(REBOOL decode, REBYTE *cp, REBCNT dlen, REBYTE *kp, REBCNT klen, REBFLG as_is); // s-ops.c
extern void Trim_Tail(REBSER *src, REBYTE chr); // s-ops.c
extern REBCNT Replace_CRLF_to_LF_Bytes(REBYTE *buf, REBCNT len); // s-ops.c
extern REBCNT Replace_CRLF_to_LF_Uni(REBUNI *buf, REBCNT len); // s-ops.c
extern void Replace_CRLF_to_LF(REBVAL *val, REBCNT len); // s-ops.c
extern void Replace_LF_To_CRLF_Bytes(REBSER *ser, REBCNT idx, REBCNT len); // s-ops.c
extern void Replace_LF_To_CRLF_Uni(REBSER *ser, REBCNT idx, REBCNT len); // s-ops.c
extern REBSER *Entab_Bytes(REBYTE *bp, REBCNT index, REBCNT len, REBINT tabsize); // s-ops.c
extern REBSER *Entab_Unicode(REBUNI *bp, REBCNT index, REBCNT len, REBINT tabsize); // s-ops.c
extern REBSER *Detab_Bytes(REBYTE *bp, REBCNT index, REBCNT len, REBINT tabsize); // s-ops.c
extern REBSER *Detab_Unicode(REBUNI *bp, REBCNT index, REBCNT len, REBINT tabsize); // s-ops.c
extern void Change_Case(REBVAL *ds, REBVAL *val, REBVAL *part, REBOOL upper); // s-ops.c
extern REBSER *Split_Lines(REBVAL *val); // s-ops.c
extern void Trim_String(REBSER *ser, REBCNT index, REBCNT len, REBCNT flags, REBVAL *with); // s-trim.c
extern REBINT What_UTF(const REBYTE *bp, REBCNT len); // s-unicode.c
extern REBFLG Legal_UTF8_Char(REBYTE *str, REBCNT len); // s-unicode.c
extern REBYTE *Check_UTF8(REBYTE *str, REBCNT len); // s-unicode.c
extern REBCNT Decode_UTF8_Char(const REBYTE **str, REBCNT *len); // s-unicode.c
extern int Decode_UTF8(REBUNI *dst, const REBYTE *src, REBCNT len, REBFLG ccr); // s-unicode.c
extern int Decode_UTF16(REBUNI *dst, const REBYTE *src, REBCNT len, REBFLG lee, REBFLG ccr); // s-unicode.c
extern int Decode_UTF32(REBUNI *dst, const REBYTE *src, REBINT len, REBFLG lee, REBFLG ccr); // s-unicode.c
extern REBSER *Decode_UTF_String(const REBYTE *bp, REBCNT len, REBINT utf, REBFLG ccr, REBFLG uni); // s-unicode.c
extern REBCNT Length_As_UTF8_Code_Points(REBYTE *src); // s-unicode.c
extern REBCNT Length_As_UTF8(REBUNI *src, REBCNT len, REBOOL uni, REBOOL ccr); // s-unicode.c
extern REBCNT Encode_UTF8_Char(REBYTE *dst, REBCNT src); // s-unicode.c
extern REBCNT Encode_UTF8(REBYTE *dst, REBINT max, void *src, REBCNT *len, REBFLG uni, REBFLG ccr); // s-unicode.c
extern int Encode_UTF8_Line(REBSER *dst, REBSER *src, REBCNT idx); // s-unicode.c
extern REBSER *Encode_UTF8_Value(REBVAL *arg, REBCNT len, REBFLG opts); // s-unicode.c
extern REBSER *Encode_UTF8_String(void *src, REBCNT len, REBFLG uni, REBFLG opts); // s-unicode.c
extern REBINT CT_Bitset(REBVAL *a, REBVAL *b, REBINT mode); // t-bitset.c
extern REBSER *Make_Bitset(REBCNT len); // t-bitset.c
extern void Mold_Bitset(REBVAL *value, REB_MOLD *mold); // t-bitset.c
extern REBFLG MT_Bitset(REBVAL *out, REBVAL *data, REBCNT type); // t-bitset.c
extern REBINT Find_Max_Bit(REBVAL *val); // t-bitset.c
extern REBFLG Check_Bit(REBSER *bset, REBCNT c, REBFLG uncased); // t-bitset.c
extern REBFLG Check_Bit_Cased(REBSER *bset, REBCNT c); // t-bitset.c
extern REBFLG Check_Bit_Str(REBSER *bset, REBVAL *val, REBFLG uncased); // t-bitset.c
extern void Set_Bit(REBSER *bset, REBCNT n, REBOOL set); // t-bitset.c
extern void Set_Bit_Str(REBSER *bset, REBVAL *val, REBOOL set); // t-bitset.c
extern REBFLG Set_Bits(REBSER *bset, REBVAL *val, REBOOL set); // t-bitset.c
extern REBFLG Check_Bits(REBSER *bset, REBVAL *val, REBFLG uncased); // t-bitset.c
extern REBINT PD_Bitset(REBPVS *pvs); // t-bitset.c
extern REBOOL Is_Zero_Bitset(REBSER *bset); // t-bitset.c
extern void Trim_Tail_Zeros(REBSER *ser); // t-bitset.c
extern REBTYPE(Bitset); // t-bitset.c
extern REBINT CT_Block(REBVAL *a, REBVAL *b, REBINT mode); // t-block.c
extern REBFLG MT_Block(REBVAL *out, REBVAL *data, REBCNT type); // t-block.c
extern REBCNT Find_Block(REBSER *series, REBCNT index, REBCNT end, REBVAL *target, REBCNT len, REBCNT flags, REBINT skip); // t-block.c
extern void Modify_Blockx(REBCNT action, REBVAL *block, REBVAL *arg); // t-block.c
extern void Make_Block_Type(REBFLG make, REBVAL *value, REBVAL *arg); // t-block.c
extern void Shuffle_Block(REBVAL *value, REBFLG secure); // t-block.c
extern REBINT PD_Block(REBPVS *pvs); // t-block.c
extern REBVAL *Pick_Block(REBVAL *block, REBVAL *selector); // t-block.c
extern REBTYPE(Block); // t-block.c
extern REBINT CT_Char(REBVAL *a, REBVAL *b, REBINT mode); // t-char.c
extern REBTYPE(Char); // t-char.c
extern REBINT CT_Datatype(REBVAL *a, REBVAL *b, REBINT mode); // t-datatype.c
extern REBFLG MT_Datatype(REBVAL *out, REBVAL *data, REBCNT type); // t-datatype.c
extern REBTYPE(Datatype); // t-datatype.c
extern void Set_Date_UTC(REBVAL *val, REBINT y, REBINT m, REBINT d, REBI64 t, REBINT z); // t-date.c
extern void Set_Date(REBVAL *val, REBOL_DAT *dat); // t-date.c
extern REBINT CT_Date(REBVAL *a, REBVAL *b, REBINT mode); // t-date.c
extern void Emit_Date(REB_MOLD *mold, REBVAL *value); // t-date.c
extern REBCNT Julian_Date(REBDAT date); // t-date.c
extern REBI64 Days_Of_Date(REBINT day, REBINT month, REBINT year ); // t-date.c
extern void Date_Of_Days(REBI64 days, REBDAT *date); // t-date.c
extern REBINT Diff_Date(REBDAT d1, REBDAT d2); // t-date.c
extern REBCNT Week_Day(REBDAT date); // t-date.c
extern void Normalize_Time(REBI64 *sp, REBINT *dp); // t-date.c
extern void Adjust_Date_Zone(REBVAL *d, REBFLG to_utc); // t-date.c
extern void Subtract_Date(REBVAL *d1, REBVAL *d2, REBVAL *result); // t-date.c
extern REBINT Cmp_Date(REBVAL *d1, REBVAL *d2); // t-date.c
extern REBFLG MT_Date(REBVAL *val, REBVAL *arg, REBCNT type); // t-date.c
extern REBINT PD_Date(REBPVS *pvs); // t-date.c
extern REBTYPE(Date); // t-date.c
extern REBFLG MT_Decimal(REBVAL *out, REBVAL *data, REBCNT type); // t-decimal.c
extern REBFLG Eq_Decimal(REBDEC a, REBDEC b); // t-decimal.c
extern REBFLG Eq_Decimal2(REBDEC a, REBDEC b); // t-decimal.c
extern REBINT CT_Decimal(REBVAL *a, REBVAL *b, REBINT mode); // t-decimal.c
extern REBTYPE(Decimal); // t-decimal.c
extern REBINT CT_Event(REBVAL *a, REBVAL *b, REBINT mode); // t-event.c
extern REBINT Cmp_Event(REBVAL *t1, REBVAL *t2); // t-event.c
extern REBFLG MT_Event(REBVAL *out, REBVAL *data, REBCNT type); // t-event.c
extern REBINT PD_Event(REBPVS *pvs); // t-event.c
extern REBTYPE(Event); // t-event.c
extern void Mold_Event(REBVAL *value, REB_MOLD *mold); // t-event.c
extern REBINT CT_Function(REBVAL *a, REBVAL *b, REBINT mode); // t-function.c
extern REBSER *As_Typesets(REBSER *types); // t-function.c
extern REBFLG MT_Function(REBVAL *out, REBVAL *data, REBCNT type); // t-function.c
extern REBTYPE(Function); // t-function.c
extern REBINT CT_Gob(REBVAL *a, REBVAL *b, REBINT mode); // t-gob.c
extern REBGOB *Make_Gob(void); // t-gob.c
extern REBINT Cmp_Gob(REBVAL *g1, REBVAL *g2); // t-gob.c
extern REBSER *Gob_To_Block(REBGOB *gob); // t-gob.c
extern REBFLG MT_Gob(REBVAL *out, REBVAL *data, REBCNT type); // t-gob.c
extern REBINT PD_Gob(REBPVS *pvs); // t-gob.c
extern REBTYPE(Gob); // t-gob.c
extern REBINT CT_Image(REBVAL *a, REBVAL *b, REBINT mode); // t-image.c
extern REBFLG MT_Image(REBVAL *out, REBVAL *data, REBCNT type); // t-image.c
extern void Reset_Height(REBVAL *value); // t-image.c
extern void Set_Pixel_Tuple(REBYTE *dp, REBVAL *tuple); // t-image.c
extern void Set_Tuple_Pixel(REBYTE *dp, REBVAL *tuple); // t-image.c
extern void Fill_Line(REBCNT *ip, REBCNT color, REBCNT len, REBOOL only); // t-image.c
extern void Fill_Rect(REBCNT *ip, REBCNT color, REBCNT w, REBINT dupx, REBINT dupy, REBOOL only); // t-image.c
extern void Fill_Alpha_Line(REBYTE *rgba, REBYTE alpha, REBINT len); // t-image.c
extern void Fill_Alpha_Rect(REBCNT *ip, REBYTE alpha, REBINT w, REBINT dupx, REBINT dupy); // t-image.c
extern REBCNT *Find_Color(REBCNT *ip, REBCNT color, REBCNT len, REBOOL only); // t-image.c
extern REBCNT *Find_Alpha(REBCNT *ip, REBCNT alpha, REBCNT len); // t-image.c
extern void Color_To_Bin(REBYTE *bin, REBYTE *rgba, REBINT len, REBCNT format); // t-image.c
extern void Bin_To_Color(REBYTE *trg, REBYTE *src, REBINT len, REBCNT format); // t-image.c
extern void Bin_To_RGB(REBYTE *rgba, REBCNT size, REBYTE *bin, REBCNT len); // t-image.c
extern void Bin_To_RGBA(REBYTE *rgba, REBCNT size, REBYTE *bin, REBINT len, REBOOL only); // t-image.c
extern void Alpha_To_Bin(REBYTE *bin, REBYTE *rgba, REBINT len, REBCNT type); // t-image.c
extern void Bin_To_Alpha(REBYTE *rgba, REBCNT size, REBYTE *bin, REBINT len, REBCNT type); // t-image.c
extern REBFLG Valid_Tuples(REBVAL *blk); // t-image.c
extern void Tuple_To_Color(REBCNT format, REBVAL *tuple, REBCNT *rgba); // t-image.c
extern void Tuples_To_RGBA(REBYTE *rgba, REBCNT size, REBVAL *blk, REBCNT len); // t-image.c
extern void Image_To_RGBA(REBYTE *rgba, REBYTE *bin, REBINT len); // t-image.c
extern void Average_Image_Color(REBYTE *rgba, REBVAL *clr, REBINT len); // t-image.c
extern void Mold_Image_Data(REBVAL *value, REB_MOLD *mold); // t-image.c
extern REBSER *Make_Image_Binary(REBVAL *image); // t-image.c
extern REBSER *Make_Image(REBCNT w, REBCNT h, REBFLG error); // t-image.c
extern void Clear_Image(REBVAL *img); // t-image.c
extern REBVAL *Create_Image(REBVAL *block, REBVAL *val, REBCNT modes); // t-image.c
extern REBVAL *Modify_Image(REBVAL *ds, REBCNT action); // t-image.c
extern REBVAL *Find_Image(REBVAL *ds); // t-image.c
extern REBFLG Image_Has_Alpha(REBVAL *v, REBFLG save); // t-image.c
extern void Copy_Rect_Data(REBVAL *dst, REBINT dx, REBINT dy, REBINT w, REBINT h, REBVAL *src, REBINT sx, REBINT sy); // t-image.c
extern REBTYPE(Image); // t-image.c
extern REBINT PD_Image(REBPVS *pvs); // t-image.c
extern REBINT CT_Integer(REBVAL *a, REBVAL *b, REBINT mode); // t-integer.c
extern REBTYPE(Integer); // t-integer.c
extern REBINT CT_Logic(REBVAL *a, REBVAL *b, REBINT mode); // t-logic.c
extern REBFLG MT_Logic(REBVAL *out, REBVAL *data, REBCNT type); // t-logic.c
extern REBINT PD_Logic(REBPVS *pvs); // t-logic.c
extern REBTYPE(Logic); // t-logic.c
extern REBINT CT_Map(REBVAL *a, REBVAL *b, REBINT mode); // t-map.c
extern REBINT Find_Key(REBSER *series, REBSER *hser, REBVAL *key, REBINT wide, REBCNT cased, REBYTE mode); // t-map.c
extern REBINT Length_Map(REBSER *series); // t-map.c
extern REBINT PD_Map(REBPVS *pvs); // t-map.c
extern REBFLG MT_Map(REBVAL *out, REBVAL *data, REBCNT type); // t-map.c
extern REBSER *Map_To_Block(REBSER *mapser, REBINT what); // t-map.c
extern void Block_As_Map(REBSER *blk); // t-map.c
extern REBSER *Map_To_Object(REBSER *mapser); // t-map.c
extern REBTYPE(Map); // t-map.c
extern REBINT CT_Money(REBVAL *a, REBVAL *b, REBINT mode); // t-money.c
extern REBINT Emit_Money(REBVAL *value, REBYTE *buf, REBCNT opts); // t-money.c
extern REBINT Bin_To_Money(REBVAL *result, REBVAL *val); // t-money.c
extern REBTYPE(Money); // t-money.c
extern REBINT CT_None(REBVAL *a, REBVAL *b, REBINT mode); // t-none.c
extern REBFLG MT_None(REBVAL *out, REBVAL *data, REBCNT type); // t-none.c
extern REBTYPE(None); // t-none.c
extern REBINT CT_Object(REBVAL *a, REBVAL *b, REBINT mode); // t-object.c
extern REBINT CT_Frame(REBVAL *a, REBVAL *b, REBINT mode); // t-object.c
extern REBFLG MT_Object(REBVAL *out, REBVAL *data, REBCNT type); // t-object.c
extern REBINT PD_Object(REBPVS *pvs); // t-object.c
extern REBTYPE(Object); // t-object.c
extern REBINT PD_Frame(REBPVS *pvs); // t-object.c
extern REBTYPE(Frame); // t-object.c
extern REBINT CT_Pair(REBVAL *a, REBVAL *b, REBINT mode); // t-pair.c
extern REBFLG MT_Pair(REBVAL *out, REBVAL *data, REBCNT type); // t-pair.c
extern REBINT Cmp_Pair(REBVAL *t1, REBVAL *t2); // t-pair.c
extern REBINT Min_Max_Pair(REBVAL *ds, REBFLG maxed); // t-pair.c
extern REBINT PD_Pair(REBPVS *pvs); // t-pair.c
extern REBTYPE(Pair); // t-pair.c
extern REBINT CT_Port(REBVAL *a, REBVAL *b, REBINT mode); // t-port.c
extern REBFLG MT_Port(REBVAL *out, REBVAL *data, REBCNT type); // t-port.c
extern REBTYPE(Port); // t-port.c
extern REBINT CT_String(REBVAL *a, REBVAL *b, REBINT mode); // t-string.c
extern REBFLG MT_String(REBVAL *out, REBVAL *data, REBCNT type); // t-string.c
extern REBINT PD_String(REBPVS *pvs); // t-string.c
extern REBINT PD_File(REBPVS *pvs); // t-string.c
extern REBTYPE(String); // t-string.c
extern void Split_Time(REBI64 t, REB_TIMEF *tf); // t-time.c
extern REBI64 Join_Time(REB_TIMEF *tf); // t-time.c
extern const REBYTE *Scan_Time(const REBYTE *cp, REBCNT len, REBVAL *value); // t-time.c
extern void Emit_Time(REB_MOLD *mold, REBVAL *value, REBOOL iso); // t-time.c
extern REBINT CT_Time(REBVAL *a, REBVAL *b, REBINT mode); // t-time.c
extern REBI64 Make_Time(REBVAL *val); // t-time.c
extern REBFLG MT_Time(REBVAL *out, REBVAL *data, REBCNT type); // t-time.c
extern REBINT Cmp_Time(REBVAL *v1, REBVAL *v2); // t-time.c
extern REBINT PD_Time(REBPVS *pvs); // t-time.c
extern REBTYPE(Time); // t-time.c
extern REBINT CT_Tuple(REBVAL *a, REBVAL *b, REBINT mode); // t-tuple.c
extern REBFLG MT_Tuple(REBVAL *out, REBVAL *data, REBCNT type); // t-tuple.c
extern REBINT Cmp_Tuple(REBVAL *t1, REBVAL *t2); // t-tuple.c
extern REBINT PD_Tuple(REBPVS *pvs); // t-tuple.c
extern REBINT Emit_Tuple(REBVAL *value, REBYTE *out); // t-tuple.c
extern REBTYPE(Tuple); // t-tuple.c
extern REBINT CT_Typeset(REBVAL *a, REBVAL *b, REBINT mode); // t-typeset.c
extern void Init_Typesets(void); // t-typeset.c
extern REBFLG Make_Typeset(REBVAL *block, REBVAL *value, REBFLG load); // t-typeset.c
extern REBFLG MT_Typeset(REBVAL *out, REBVAL *data, REBCNT type); // t-typeset.c
extern REBINT Find_Typeset(REBVAL *block); // t-typeset.c
extern REBSER *Typeset_To_Block(REBVAL *tset); // t-typeset.c
extern REBTYPE(Typeset); // t-typeset.c
extern REBINT CT_Utype(REBVAL *a, REBVAL *b, REBINT mode); // t-utype.c
extern REBFLG MT_Utype(REBVAL *out, REBVAL *data, REBCNT type); // t-utype.c
extern REBTYPE(Utype); // t-utype.c
extern REBSER *Make_Vector_Block(REBVAL *vect); // t-vector.c
extern REBVAL* Math_Op_Vector(REBVAL *v1, REBVAL *v2, REBCNT action); // t-vector.c
extern REBINT Compare_Vector(REBVAL *v1, REBVAL *v2); // t-vector.c
extern void Shuffle_Vector(REBVAL *vect, REBFLG secure); // t-vector.c
extern void Set_Vector_Value(REBVAL *var, REBSER *series, REBCNT index); // t-vector.c
extern REBSER *Make_Vector(REBINT type, REBINT sign, REBINT dims, REBINT bits, REBINT size); // t-vector.c
extern REBVAL *Make_Vector_Spec(REBVAL *bp, REBVAL *value); // t-vector.c
extern REBFLG MT_Vector(REBVAL *out, REBVAL *data, REBCNT type); // t-vector.c
extern REBINT CT_Vector(REBVAL *a, REBVAL *b, REBINT mode); // t-vector.c
extern REBINT PD_Vector(REBPVS *pvs); // t-vector.c
extern REBTYPE(Vector); // t-vector.c
extern void Mold_Vector(REBVAL *value, REB_MOLD *mold, REBFLG molded); // t-vector.c
extern REBINT CT_Word(REBVAL *a, REBVAL *b, REBINT mode); // t-word.c
extern REBTYPE(Word); // t-word.c
extern REBINT Codec_BMP_Image(REBCDI *codi); // u-bmp.c
extern void Init_BMP_Codec(void); // u-bmp.c
extern REBSER *CompressZlib(REBSER *input, REBINT index, REBCNT in_len, REBINT level, REBINT windowBits); // u-compress.c
extern REBSER *DecompressZlib(REBSER *input, REBCNT index, REBINT len, REBCNT limit, REBINT windowBits); // u-compress.c
extern REBSER *CompressLzma(REBSER *input, REBINT index, REBCNT in_len, REBINT level); // u-compress.c
extern REBSER *DecompressLzma(REBSER *input, REBCNT index, REBINT in_len, REBCNT limit); // u-compress.c
extern REBINT Do_Dialect(REBSER *dialect, REBSER *block, REBCNT *index, REBSER **out); // u-dialect.c
extern void Trace_Delect(REBINT level); // u-dialect.c
extern void Decode_LZW(REBCNT *data, REBYTE **cpp, REBYTE *colortab, REBINT w, REBINT h, REBOOL interlaced); // u-gif.c
extern void Decode_GIF_Image(REBCDI *codi); // u-gif.c
extern REBINT Codec_GIF_Image(REBCDI *codi); // u-gif.c
extern void Init_GIF_Codec(void); // u-gif.c
extern REBINT Codec_JPEG_Image(REBCDI *codi); // u-jpg.c
extern void Init_JPEG_Codec(void); // u-jpg.c
extern REBSER *Parse_String(REBSER *series, REBCNT index, REBVAL *rules, REBCNT flags); // u-parse.c
extern REBSER *Parse_Lines(REBSER *src); // u-parse.c
extern void Encode_PNG_Image(REBCDI *codi); // u-png.c
extern void Decode_PNG_Image(REBCDI *codi); // u-png.c
extern REBINT Codec_PNG_Image(REBCDI *codi); // u-png.c
extern void Init_PNG_Codec(void); // u-png.c
extern REBINT Codec_WAV(REBCDI *codi); // u-wav.c
extern void Init_WAV_Codec(void); // u-wav.c
extern REBYTE *MD4(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
extern REBYTE *MD5(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
extern REBYTE *SHA1(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
extern REBYTE *SHA224(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
extern REBYTE *SHA256(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
extern REBYTE *SHA384(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
extern REBYTE *SHA512(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
extern REBYTE *RIPEMD160(REBYTE *d, REBCNT n, REBYTE *md); // u-mbedtls.c
