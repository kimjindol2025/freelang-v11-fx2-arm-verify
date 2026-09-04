#include "runtime.h"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"

FLValue fl_user_is_digit_p(FLValue c);
static FLValue __fl_wrap_fl_user_is_digit_p(FLClosure*, int, FLValue*);
FLValue fl_user_is_alpha_p(FLValue c);
static FLValue __fl_wrap_fl_user_is_alpha_p(FLClosure*, int, FLValue*);
FLValue fl_user_is_alnum_p(FLValue c);
static FLValue __fl_wrap_fl_user_is_alnum_p(FLClosure*, int, FLValue*);
FLValue fl_user_is_space_p(FLValue c);
static FLValue __fl_wrap_fl_user_is_space_p(FLClosure*, int, FLValue*);
FLValue fl_user_is_symbol_char_p(FLValue c);
static FLValue __fl_wrap_fl_user_is_symbol_char_p(FLClosure*, int, FLValue*);
FLValue fl_user_make_state(FLValue src);
static FLValue __fl_wrap_fl_user_make_state(FLClosure*, int, FLValue*);
FLValue fl_user_peek_at(FLValue st, FLValue offset);
static FLValue __fl_wrap_fl_user_peek_at(FLClosure*, int, FLValue*);
FLValue fl_user_peek(FLValue st);
static FLValue __fl_wrap_fl_user_peek(FLClosure*, int, FLValue*);
FLValue fl_user_at_end_p(FLValue st);
static FLValue __fl_wrap_fl_user_at_end_p(FLClosure*, int, FLValue*);
FLValue fl_user_advance(FLValue st);
static FLValue __fl_wrap_fl_user_advance(FLClosure*, int, FLValue*);
FLValue fl_user_emit(FLValue st, FLValue kind, FLValue value, FLValue sl, FLValue sc);
static FLValue __fl_wrap_fl_user_emit(FLClosure*, int, FLValue*);
FLValue fl_user_skip_comment_loop(FLValue _cur);
static FLValue __fl_wrap_fl_user_skip_comment_loop(FLClosure*, int, FLValue*);
FLValue fl_user_skip_comment(FLValue st);
static FLValue __fl_wrap_fl_user_skip_comment(FLClosure*, int, FLValue*);
FLValue fl_user_skip_ws_loop(FLValue _cur);
static FLValue __fl_wrap_fl_user_skip_ws_loop(FLClosure*, int, FLValue*);
FLValue fl_user_skip_ws(FLValue st);
static FLValue __fl_wrap_fl_user_skip_ws(FLClosure*, int, FLValue*);
FLValue fl_user_read_number_iter(FLValue _cur, FLValue _res_acc, FLValue _dot, FLValue line, FLValue col);
static FLValue __fl_wrap_fl_user_read_number_iter(FLClosure*, int, FLValue*);
FLValue fl_user_read_number_body(FLValue st, FLValue acc, FLValue has_dot, FLValue line, FLValue col);
static FLValue __fl_wrap_fl_user_read_number_body(FLClosure*, int, FLValue*);
FLValue fl_user_read_number(FLValue st);
static FLValue __fl_wrap_fl_user_read_number(FLClosure*, int, FLValue*);
FLValue fl_user_translate_esc(FLValue c);
static FLValue __fl_wrap_fl_user_translate_esc(FLClosure*, int, FLValue*);
FLValue fl_user_read_string_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col);
static FLValue __fl_wrap_fl_user_read_string_iter(FLClosure*, int, FLValue*);
FLValue fl_user_read_string_body(FLValue st, FLValue acc, FLValue line, FLValue col);
static FLValue __fl_wrap_fl_user_read_string_body(FLClosure*, int, FLValue*);
FLValue fl_user_read_string(FLValue st);
static FLValue __fl_wrap_fl_user_read_string(FLClosure*, int, FLValue*);
FLValue fl_user_read_symbol_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col, FLValue kind);
static FLValue __fl_wrap_fl_user_read_symbol_iter(FLClosure*, int, FLValue*);
FLValue fl_user_read_symbol_body_kind(FLValue st, FLValue acc, FLValue line, FLValue col, FLValue kind);
static FLValue __fl_wrap_fl_user_read_symbol_body_kind(FLClosure*, int, FLValue*);
FLValue fl_user_read_symbol(FLValue st);
static FLValue __fl_wrap_fl_user_read_symbol(FLClosure*, int, FLValue*);
FLValue fl_user_read_variable(FLValue st);
static FLValue __fl_wrap_fl_user_read_variable(FLClosure*, int, FLValue*);
FLValue fl_user_read_keyword(FLValue st);
static FLValue __fl_wrap_fl_user_read_keyword(FLClosure*, int, FLValue*);
FLValue fl_user_read_token(FLValue st);
static FLValue __fl_wrap_fl_user_read_token(FLClosure*, int, FLValue*);
FLValue fl_user_lex_loop(FLValue _cur);
static FLValue __fl_wrap_fl_user_lex_loop(FLClosure*, int, FLValue*);
FLValue fl_user_lex(FLValue src);
static FLValue __fl_wrap_fl_user_lex(FLClosure*, int, FLValue*);
FLValue fl_user_make_literal(FLValue type, FLValue value, FLValue line);
static FLValue __fl_wrap_fl_user_make_literal(FLClosure*, int, FLValue*);
FLValue fl_user_make_variable(FLValue name, FLValue line);
static FLValue __fl_wrap_fl_user_make_variable(FLClosure*, int, FLValue*);
FLValue fl_user_make_keyword(FLValue name, FLValue line);
static FLValue __fl_wrap_fl_user_make_keyword(FLClosure*, int, FLValue*);
FLValue fl_user_make_sexpr(FLValue op, FLValue args, FLValue line);
static FLValue __fl_wrap_fl_user_make_sexpr(FLClosure*, int, FLValue*);
FLValue fl_user_make_number(FLValue v, FLValue line);
static FLValue __fl_wrap_fl_user_make_number(FLClosure*, int, FLValue*);
FLValue fl_user_make_string(FLValue v, FLValue line);
static FLValue __fl_wrap_fl_user_make_string(FLClosure*, int, FLValue*);
FLValue fl_user_make_bool(FLValue v, FLValue line);
static FLValue __fl_wrap_fl_user_make_bool(FLClosure*, int, FLValue*);
FLValue fl_user_make_null(FLValue line);
static FLValue __fl_wrap_fl_user_make_null(FLClosure*, int, FLValue*);
FLValue fl_user_make_symbol(FLValue v, FLValue line);
static FLValue __fl_wrap_fl_user_make_symbol(FLClosure*, int, FLValue*);
FLValue fl_user_make_block(FLValue type, FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_array_block(FLValue items, FLValue line);
static FLValue __fl_wrap_fl_user_make_array_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_map_block(FLValue items, FLValue line);
static FLValue __fl_wrap_fl_user_make_map_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_literal(FLValue value, FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_literal(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_variable(FLValue name, FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_variable(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_wildcard(FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_wildcard(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_list(FLValue items, FLValue rest, FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_list(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_struct(FLValue type_name, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_struct(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_or(FLValue alternatives, FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_or(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_range(FLValue start, FLValue end, FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_range(FLClosure*, int, FLValue*);
FLValue fl_user_make_pattern_match(FLValue value, FLValue cases, FLValue line);
static FLValue __fl_wrap_fl_user_make_pattern_match(FLClosure*, int, FLValue*);
FLValue fl_user_make_match_case(FLValue pattern, FLValue guard, FLValue body, FLValue line);
static FLValue __fl_wrap_fl_user_make_match_case(FLClosure*, int, FLValue*);
FLValue fl_user_make_function_value(FLValue params, FLValue body, FLValue captured_env, FLValue name);
static FLValue __fl_wrap_fl_user_make_function_value(FLClosure*, int, FLValue*);
FLValue fl_user_make_type_class(FLValue name, FLValue generics, FLValue methods, FLValue line);
static FLValue __fl_wrap_fl_user_make_type_class(FLClosure*, int, FLValue*);
FLValue fl_user_make_type_class_instance(FLValue class_name, FLValue type_name, FLValue impls, FLValue line);
static FLValue __fl_wrap_fl_user_make_type_class_instance(FLClosure*, int, FLValue*);
FLValue fl_user_make_module_block(FLValue name, FLValue exports, FLValue body, FLValue line);
static FLValue __fl_wrap_fl_user_make_module_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_import_block(FLValue path, FLValue alias, FLValue names, FLValue line);
static FLValue __fl_wrap_fl_user_make_import_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_open_block(FLValue module_name, FLValue line);
static FLValue __fl_wrap_fl_user_make_open_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_search_block(FLValue query, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_search_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_learn_block(FLValue topic, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_learn_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_reasoning_block(FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_reasoning_block(FLClosure*, int, FLValue*);
FLValue fl_user_make_async_function(FLValue name, FLValue params, FLValue body, FLValue line);
static FLValue __fl_wrap_fl_user_make_async_function(FLClosure*, int, FLValue*);
FLValue fl_user_make_await(FLValue expr, FLValue line);
static FLValue __fl_wrap_fl_user_make_await(FLClosure*, int, FLValue*);
FLValue fl_user_make_try(FLValue body, FLValue catch, FLValue finally, FLValue line);
static FLValue __fl_wrap_fl_user_make_try(FLClosure*, int, FLValue*);
FLValue fl_user_make_catch(FLValue param, FLValue body, FLValue line);
static FLValue __fl_wrap_fl_user_make_catch(FLClosure*, int, FLValue*);
FLValue fl_user_make_throw(FLValue expr, FLValue line);
static FLValue __fl_wrap_fl_user_make_throw(FLClosure*, int, FLValue*);
FLValue fl_user_make_template_string(FLValue parts, FLValue expressions, FLValue line);
static FLValue __fl_wrap_fl_user_make_template_string(FLClosure*, int, FLValue*);
FLValue fl_user_make_loop(FLValue init, FLValue condition, FLValue update, FLValue body, FLValue line);
static FLValue __fl_wrap_fl_user_make_loop(FLClosure*, int, FLValue*);
FLValue fl_user_make_page(FLValue name, FLValue path, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_page(FLClosure*, int, FLValue*);
FLValue fl_user_make_route(FLValue method, FLValue path, FLValue handler, FLValue line);
static FLValue __fl_wrap_fl_user_make_route(FLClosure*, int, FLValue*);
FLValue fl_user_make_component(FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_component(FLClosure*, int, FLValue*);
FLValue fl_user_make_form(FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_fl_user_make_form(FLClosure*, int, FLValue*);
FLValue fl_user_deep_equal_p(FLValue a, FLValue b);
static FLValue __fl_wrap_fl_user_deep_equal_p(FLClosure*, int, FLValue*);
FLValue fl_user_deep_equal_list_p(FLValue a, FLValue b, FLValue i);
static FLValue __fl_wrap_fl_user_deep_equal_list_p(FLClosure*, int, FLValue*);
FLValue fl_user_deep_equal_map_p(FLValue a, FLValue b);
static FLValue __fl_wrap_fl_user_deep_equal_map_p(FLClosure*, int, FLValue*);
FLValue fl_user_keys_no_line(FLValue m);
static FLValue __fl_wrap_fl_user_keys_no_line(FLClosure*, int, FLValue*);
FLValue fl_user_deep_equal_map_keys_p(FLValue a, FLValue b, FLValue ks, FLValue i);
static FLValue __fl_wrap_fl_user_deep_equal_map_keys_p(FLClosure*, int, FLValue*);
FLValue fl_user_json_keys(FLValue m);
static FLValue __fl_wrap_fl_user_json_keys(FLClosure*, int, FLValue*);
FLValue fl_user_p_make(FLValue tokens);
static FLValue __fl_wrap_fl_user_p_make(FLClosure*, int, FLValue*);
FLValue fl_user_p_peek(FLValue p);
static FLValue __fl_wrap_fl_user_p_peek(FLClosure*, int, FLValue*);
FLValue fl_user_p_peek_at(FLValue p, FLValue offset);
static FLValue __fl_wrap_fl_user_p_peek_at(FLClosure*, int, FLValue*);
FLValue fl_user_p_end_p(FLValue p);
static FLValue __fl_wrap_fl_user_p_end_p(FLClosure*, int, FLValue*);
FLValue fl_user_p_advance(FLValue p);
static FLValue __fl_wrap_fl_user_p_advance(FLClosure*, int, FLValue*);
FLValue fl_user_p_with_ast(FLValue p, FLValue ast);
static FLValue __fl_wrap_fl_user_p_with_ast(FLClosure*, int, FLValue*);
FLValue fl_user_p_append_ast(FLValue p, FLValue node);
static FLValue __fl_wrap_fl_user_p_append_ast(FLClosure*, int, FLValue*);
FLValue fl_user_r_pair(FLValue p, FLValue node);
static FLValue __fl_wrap_fl_user_r_pair(FLClosure*, int, FLValue*);
FLValue fl_user_string_contains_p(FLValue s, FLValue substr);
static FLValue __fl_wrap_fl_user_string_contains_p(FLClosure*, int, FLValue*);
FLValue fl_user_parse_atom(FLValue p);
static FLValue __fl_wrap_fl_user_parse_atom(FLClosure*, int, FLValue*);
FLValue fl_user_parse_expr(FLValue p);
static FLValue __fl_wrap_fl_user_parse_expr(FLClosure*, int, FLValue*);
FLValue fl_user_parse_sexpr(FLValue p);
static FLValue __fl_wrap_fl_user_parse_sexpr(FLClosure*, int, FLValue*);
FLValue fl_user_parse_consume_rparen(FLValue p);
static FLValue __fl_wrap_fl_user_parse_consume_rparen(FLClosure*, int, FLValue*);
FLValue fl_user_parse_args(FLValue p, FLValue acc);
static FLValue __fl_wrap_fl_user_parse_args(FLClosure*, int, FLValue*);
FLValue fl_user_parse_bracket(FLValue p);
static FLValue __fl_wrap_fl_user_parse_bracket(FLClosure*, int, FLValue*);
FLValue fl_user_is_block_type_p(FLValue s);
static FLValue __fl_wrap_fl_user_is_block_type_p(FLClosure*, int, FLValue*);
FLValue fl_user_upper_case(FLValue s);
static FLValue __fl_wrap_fl_user_upper_case(FLClosure*, int, FLValue*);
FLValue fl_user_parse_array(FLValue p, FLValue line);
static FLValue __fl_wrap_fl_user_parse_array(FLClosure*, int, FLValue*);
FLValue fl_user_parse_consume_rbracket(FLValue p);
static FLValue __fl_wrap_fl_user_parse_consume_rbracket(FLClosure*, int, FLValue*);
FLValue fl_user_parse_named_block(FLValue p, FLValue line);
static FLValue __fl_wrap_fl_user_parse_named_block(FLClosure*, int, FLValue*);
FLValue fl_user_parse_optional_name(FLValue p);
static FLValue __fl_wrap_fl_user_parse_optional_name(FLClosure*, int, FLValue*);
FLValue fl_user_parse_block_fields(FLValue p, FLValue acc);
static FLValue __fl_wrap_fl_user_parse_block_fields(FLClosure*, int, FLValue*);
FLValue fl_user_parse_map(FLValue p);
static FLValue __fl_wrap_fl_user_parse_map(FLClosure*, int, FLValue*);
FLValue fl_user_parse_consume_rbrace(FLValue p);
static FLValue __fl_wrap_fl_user_parse_consume_rbrace(FLClosure*, int, FLValue*);
FLValue fl_user_parse_all(FLValue p);
static FLValue __fl_wrap_fl_user_parse_all(FLClosure*, int, FLValue*);
FLValue fl_user_parse(FLValue tokens);
static FLValue __fl_wrap_fl_user_parse(FLClosure*, int, FLValue*);
FLValue fl_user_get_block_items(FLValue node);
static FLValue __fl_wrap_fl_user_get_block_items(FLClosure*, int, FLValue*);
FLValue fl_user_c_esc(FLValue s);
static FLValue __fl_wrap_fl_user_c_esc(FLClosure*, int, FLValue*);
FLValue fl_user_c_reserved_p(FLValue s);
static FLValue __fl_wrap_fl_user_c_reserved_p(FLClosure*, int, FLValue*);
FLValue fl_user_c_name(FLValue n);
static FLValue __fl_wrap_fl_user_c_name(FLClosure*, int, FLValue*);
FLValue fl_user_user_c_name(FLValue n);
static FLValue __fl_wrap_fl_user_user_c_name(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_user_defn_p(FLValue n);
static FLValue __fl_wrap_fl_user_cgc_user_defn_p(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_language_special_form_p(FLValue op);
static FLValue __fl_wrap_fl_user_cgc_language_special_form_p(FLClosure*, int, FLValue*);
FLValue fl_user_cgc(FLValue n);
static FLValue __fl_wrap_fl_user_cgc(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_literal(FLValue n);
static FLValue __fl_wrap_fl_user_cgc_literal(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_block(FLValue n);
static FLValue __fl_wrap_fl_user_cgc_block(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_func_block(FLValue n);
static FLValue __fl_wrap_fl_user_cgc_func_block(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_params(FLValue it);
static FLValue __fl_wrap_fl_user_cgc_params(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_params_loop(FLValue _it, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_params_loop(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_extract_name(FLValue node);
static FLValue __fl_wrap_fl_user_cgc_extract_name(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_fncall(FLValue fn_c, FLValue args);
static FLValue __fl_wrap_fl_user_cgc_fncall(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_sexpr(FLValue n);
static FLValue __fl_wrap_fl_user_cgc_sexpr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_dispatch(FLValue op, FLValue args);
static FLValue __fl_wrap_fl_user_cgc_dispatch(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_swap_b(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_swap_b(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_dispatch_fallback(FLValue op, FLValue args);
static FLValue __fl_wrap_fl_user_cgc_dispatch_fallback(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_fn_argv_decls(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_fn_argv_decls(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_fn_param_names(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_fn_param_names(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_collect_vars(FLValue node, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_collect_vars(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_collect_vars_loop(FLValue _args, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_collect_vars_loop(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_fn_env_decls(FLValue _caps, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_fn_env_decls(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_env_arr(FLValue _caps, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_env_arr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_fn_caps_filter(FLValue all_vars, FLValue param_names, FLValue outer, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_fn_caps_filter(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_fn(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_fn(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_list(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_list(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_str(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_str(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_str_arg(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_str_arg(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_if(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_if(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_cond(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_cond(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_cond_nested(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_cond_nested(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_do(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_do(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_let(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_let(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_let_1d(FLValue it, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_let_1d(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_let_2d(FLValue it, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_let_2d(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_body(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_body(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_defn_impl(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_defn_impl(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_defn(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_defn(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_define(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_define(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_binop_chain(FLValue args, FLValue fn);
static FLValue __fl_wrap_fl_user_cgc_binop_chain(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_binop_fold(FLValue args, FLValue fn, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_binop_fold(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_and(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_and(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_and_fold(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_and_fold(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_or(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_or(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_or_fold(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_or_fold(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_args(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_args(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_args_loop(FLValue _args, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_args_loop(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_stmts(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_stmts(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_forward_decls(FLValue nodes);
static FLValue __fl_wrap_fl_user_cgc_forward_decls(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_forward_loop(FLValue _nodes, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_forward_loop(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_wrapper_call_args(FLValue _items, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_wrapper_call_args(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_top_level(FLValue _nodes, FLValue _i, FLValue _stmts, FLValue _fns);
static FLValue __fl_wrap_fl_user_cgc_top_level(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_lambda_fwd_loop(FLValue _defs, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_lambda_fwd_loop(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_lambda_fwds();
static FLValue __fl_wrap_fl_user_cgc_lambda_fwds(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_join_lambda_loop(FLValue _defs, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_fl_user_cgc_join_lambda_loop(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_join_lambdas();
static FLValue __fl_wrap_fl_user_cgc_join_lambdas(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_join_wrappers();
static FLValue __fl_wrap_fl_user_cgc_join_wrappers(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_join_globals();
static FLValue __fl_wrap_fl_user_cgc_join_globals(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_join_hoisted();
static FLValue __fl_wrap_fl_user_cgc_join_hoisted(FLClosure*, int, FLValue*);
FLValue fl_user_generate_c(FLValue nodes);
static FLValue __fl_wrap_fl_user_generate_c(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_set_b(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_set_b(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_while(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_while(FLClosure*, int, FLValue*);
FLValue fl_user_loop_extract_vars(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_loop_extract_vars(FLClosure*, int, FLValue*);
FLValue fl_user_loop_make_decls(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_loop_make_decls(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_loop(FLValue args);
static FLValue __fl_wrap_fl_user_cgc_loop(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_recur_temps(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_recur_temps(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_recur_assigns(FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_recur_assigns(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_recur_stmt(FLValue args, FLValue vars);
static FLValue __fl_wrap_fl_user_cgc_recur_stmt(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_with_recur(FLValue node, FLValue vars);
static FLValue __fl_wrap_fl_user_cgc_with_recur(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_if_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_fl_user_cgc_if_wr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_cond_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_fl_user_cgc_cond_wr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_cond_nested_wr(FLValue args, FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_cond_nested_wr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_do_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_fl_user_cgc_do_wr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_stmts_wr(FLValue args, FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_stmts_wr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_let_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_fl_user_cgc_let_wr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_body_wr(FLValue args, FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_body_wr(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_array_block(FLValue n);
static FLValue __fl_wrap_fl_user_cgc_array_block(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_map_entry_c(FLValue ent);
static FLValue __fl_wrap_fl_user_cgc_map_entry_c(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_map_entries_c(FLValue ents, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_map_entries_c(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_map_key_c(FLValue key_node);
static FLValue __fl_wrap_fl_user_cgc_map_key_c(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_map_items_c(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_fl_user_cgc_map_items_c(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_map_from_items(FLValue items);
static FLValue __fl_wrap_fl_user_cgc_map_from_items(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_map_block(FLValue n);
static FLValue __fl_wrap_fl_user_cgc_map_block(FLClosure*, int, FLValue*);
FLValue fl_user_ir_err(FLValue msg, FLValue n);
static FLValue __fl_wrap_fl_user_ir_err(FLClosure*, int, FLValue*);
FLValue fl_user_ir_chk(FLValue n);
static FLValue __fl_wrap_fl_user_ir_chk(FLClosure*, int, FLValue*);
FLValue fl_user_includes_item(FLValue arr, FLValue val);
static FLValue __fl_wrap_fl_user_includes_item(FLClosure*, int, FLValue*);
FLValue fl_user_ir_validate(FLValue nodes);
static FLValue __fl_wrap_fl_user_ir_validate(FLClosure*, int, FLValue*);
FLValue fl_user_path_dir(FLValue path);
static FLValue __fl_wrap_fl_user_path_dir(FLClosure*, int, FLValue*);
FLValue fl_user_append_all(FLValue acc, FLValue items);
static FLValue __fl_wrap_fl_user_append_all(FLClosure*, int, FLValue*);
FLValue fl_user_expand_loads(FLValue nodes, FLValue base_dir);
static FLValue __fl_wrap_fl_user_expand_loads(FLClosure*, int, FLValue*);
FLValue fl_user_cgc_run(FLValue argv);
static FLValue __fl_wrap_fl_user_cgc_run(FLClosure*, int, FLValue*);
static FLValue fl_user_lambda_id_atom;

static FLValue fl_user_lambda_defs_atom;

static FLValue fl_user_outer_params_atom;

static FLValue fl_user_known_fncall_targets_atom;

static FLValue fl_user_known_defns_atom;

static FLValue fl_user_wrapper_defs_atom;

static FLValue fl_user_global_decls_atom;

static FLValue fl_user_known_user_globals_atom;

static FLValue fl_user_cgc_defn_depth_atom;

static FLValue fl_user_cgc_hoisted_fns_atom;

static FLValue fl_user_cgc_hoisted_fwds_atom;

static FLValue fl_user_ir_kind_set;

static FLValue fl_user_ir_lit_types;


static FLValue __fl_anon_0(FLClosure*, int, FLValue*);

static FLValue __fl_anon_0(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue k __attribute__((unused)) = argv[0];
    return fl_not(fl_eq(k, fl_str_val("line")));
}

static FLValue __fl_wrap_fl_user_is_digit_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_is_digit_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_is_alpha_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_is_alpha_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_is_alnum_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_is_alnum_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_is_space_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_is_space_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_is_symbol_char_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_is_symbol_char_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_make_state(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_state(argv[0]);
}

static FLValue __fl_wrap_fl_user_peek_at(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_peek_at(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_peek(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_peek(argv[0]);
}

static FLValue __fl_wrap_fl_user_at_end_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_at_end_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_advance(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_advance(argv[0]);
}

static FLValue __fl_wrap_fl_user_emit(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_emit(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_fl_user_skip_comment_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_skip_comment_loop(argv[0]);
}

static FLValue __fl_wrap_fl_user_skip_comment(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_skip_comment(argv[0]);
}

static FLValue __fl_wrap_fl_user_skip_ws_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_skip_ws_loop(argv[0]);
}

static FLValue __fl_wrap_fl_user_skip_ws(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_skip_ws(argv[0]);
}

static FLValue __fl_wrap_fl_user_read_number_iter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_number_iter(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_fl_user_read_number_body(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_number_body(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_fl_user_read_number(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_number(argv[0]);
}

static FLValue __fl_wrap_fl_user_translate_esc(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_translate_esc(argv[0]);
}

static FLValue __fl_wrap_fl_user_read_string_iter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_string_iter(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_read_string_body(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_string_body(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_read_string(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_string(argv[0]);
}

static FLValue __fl_wrap_fl_user_read_symbol_iter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_symbol_iter(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_fl_user_read_symbol_body_kind(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_symbol_body_kind(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_fl_user_read_symbol(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_symbol(argv[0]);
}

static FLValue __fl_wrap_fl_user_read_variable(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_variable(argv[0]);
}

static FLValue __fl_wrap_fl_user_read_keyword(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_keyword(argv[0]);
}

static FLValue __fl_wrap_fl_user_read_token(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_read_token(argv[0]);
}

static FLValue __fl_wrap_fl_user_lex_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_lex_loop(argv[0]);
}

static FLValue __fl_wrap_fl_user_lex(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_lex(argv[0]);
}

static FLValue __fl_wrap_fl_user_make_literal(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_literal(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_variable(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_variable(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_keyword(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_keyword(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_sexpr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_sexpr(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_number(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_number(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_string(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_string(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_bool(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_bool(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_null(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_null(argv[0]);
}

static FLValue __fl_wrap_fl_user_make_symbol(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_symbol(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_block(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_array_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_array_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_map_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_map_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_pattern_literal(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_literal(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_pattern_variable(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_variable(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_pattern_wildcard(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_wildcard(argv[0]);
}

static FLValue __fl_wrap_fl_user_make_pattern_list(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_list(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_pattern_struct(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_struct(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_pattern_or(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_or(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_pattern_range(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_range(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_pattern_match(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_pattern_match(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_match_case(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_match_case(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_function_value(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_function_value(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_type_class(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_type_class(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_type_class_instance(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_type_class_instance(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_module_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_module_block(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_import_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_import_block(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_open_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_open_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_search_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_search_block(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_learn_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_learn_block(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_reasoning_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_reasoning_block(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_async_function(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_async_function(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_await(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_await(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_try(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_try(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_catch(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_catch(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_throw(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_throw(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_make_template_string(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_template_string(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_loop(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_fl_user_make_page(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_page(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_route(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_route(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_make_component(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_component(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_make_form(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_make_form(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_deep_equal_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_deep_equal_p(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_deep_equal_list_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_deep_equal_list_p(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_deep_equal_map_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_deep_equal_map_p(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_keys_no_line(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_keys_no_line(argv[0]);
}

static FLValue __fl_wrap_fl_user_deep_equal_map_keys_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_deep_equal_map_keys_p(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_json_keys(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_json_keys(argv[0]);
}

static FLValue __fl_wrap_fl_user_p_make(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_p_make(argv[0]);
}

static FLValue __fl_wrap_fl_user_p_peek(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_p_peek(argv[0]);
}

static FLValue __fl_wrap_fl_user_p_peek_at(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_p_peek_at(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_p_end_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_p_end_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_p_advance(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_p_advance(argv[0]);
}

static FLValue __fl_wrap_fl_user_p_with_ast(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_p_with_ast(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_p_append_ast(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_p_append_ast(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_r_pair(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_r_pair(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_string_contains_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_string_contains_p(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_parse_atom(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_atom(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_expr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_expr(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_sexpr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_sexpr(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_consume_rparen(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_consume_rparen(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_args(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_args(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_parse_bracket(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_bracket(argv[0]);
}

static FLValue __fl_wrap_fl_user_is_block_type_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_is_block_type_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_upper_case(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_upper_case(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_array(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_array(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_parse_consume_rbracket(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_consume_rbracket(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_named_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_named_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_parse_optional_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_optional_name(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_block_fields(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_block_fields(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_parse_map(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_map(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_consume_rbrace(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_consume_rbrace(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse_all(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse_all(argv[0]);
}

static FLValue __fl_wrap_fl_user_parse(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_parse(argv[0]);
}

static FLValue __fl_wrap_fl_user_get_block_items(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_get_block_items(argv[0]);
}

static FLValue __fl_wrap_fl_user_c_esc(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_c_esc(argv[0]);
}

static FLValue __fl_wrap_fl_user_c_reserved_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_c_reserved_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_c_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_c_name(argv[0]);
}

static FLValue __fl_wrap_fl_user_user_c_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_user_c_name(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_user_defn_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_user_defn_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_language_special_form_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_language_special_form_p(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_literal(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_literal(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_block(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_func_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_func_block(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_params(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_params(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_params_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_params_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_extract_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_extract_name(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_fncall(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_fncall(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_sexpr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_sexpr(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_dispatch(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_dispatch(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_swap_b(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_swap_b(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_dispatch_fallback(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_dispatch_fallback(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_fn_argv_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_fn_argv_decls(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_fn_param_names(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_fn_param_names(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_collect_vars(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_collect_vars(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_collect_vars_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_collect_vars_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_fn_env_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_fn_env_decls(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_env_arr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_env_arr(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_fn_caps_filter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_fn_caps_filter(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_fl_user_cgc_fn(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_fn(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_list(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_list(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_str(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_str(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_str_arg(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_str_arg(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_if(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_if(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_cond(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_cond(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_cond_nested(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_cond_nested(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_do(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_do(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_let(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_let(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_let_1d(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_let_1d(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_let_2d(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_let_2d(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_body(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_body(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_defn_impl(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_defn_impl(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_defn(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_defn(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_define(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_define(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_binop_chain(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_binop_chain(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_binop_fold(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_binop_fold(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_cgc_and(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_and(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_and_fold(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_and_fold(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_or(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_or(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_or_fold(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_or_fold(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_args(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_args(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_args_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_args_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_stmts(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_stmts(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_forward_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_forward_decls(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_forward_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_forward_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_wrapper_call_args(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_wrapper_call_args(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_top_level(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_top_level(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_cgc_lambda_fwd_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_lambda_fwd_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_lambda_fwds(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_lambda_fwds();
}

static FLValue __fl_wrap_fl_user_cgc_join_lambda_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_join_lambda_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_join_lambdas(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_join_lambdas();
}

static FLValue __fl_wrap_fl_user_cgc_join_wrappers(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_join_wrappers();
}

static FLValue __fl_wrap_fl_user_cgc_join_globals(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_join_globals();
}

static FLValue __fl_wrap_fl_user_cgc_join_hoisted(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_join_hoisted();
}

static FLValue __fl_wrap_fl_user_generate_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_generate_c(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_set_b(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_set_b(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_while(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_while(argv[0]);
}

static FLValue __fl_wrap_fl_user_loop_extract_vars(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_loop_extract_vars(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_loop_make_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_loop_make_decls(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_loop(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_recur_temps(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_recur_temps(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_recur_assigns(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_recur_assigns(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_recur_stmt(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_recur_stmt(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_with_recur(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_with_recur(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_if_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_if_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_cond_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_cond_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_cond_nested_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_cond_nested_wr(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_cgc_do_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_do_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_stmts_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_stmts_wr(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_cgc_let_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_let_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_body_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_body_wr(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_fl_user_cgc_array_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_array_block(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_map_entry_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_map_entry_c(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_map_entries_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_map_entries_c(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_map_key_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_map_key_c(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_map_items_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_map_items_c(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_fl_user_cgc_map_from_items(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_map_from_items(argv[0]);
}

static FLValue __fl_wrap_fl_user_cgc_map_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_map_block(argv[0]);
}

static FLValue __fl_wrap_fl_user_ir_err(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_ir_err(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_ir_chk(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_ir_chk(argv[0]);
}

static FLValue __fl_wrap_fl_user_includes_item(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_includes_item(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_ir_validate(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_ir_validate(argv[0]);
}

static FLValue __fl_wrap_fl_user_path_dir(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_path_dir(argv[0]);
}

static FLValue __fl_wrap_fl_user_append_all(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_append_all(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_expand_loads(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_expand_loads(argv[0], argv[1]);
}

static FLValue __fl_wrap_fl_user_cgc_run(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return fl_user_cgc_run(argv[0]);
}


FLValue fl_user_is_digit_p(FLValue c) {
    return (fl_truthy(null_p(c)) ? fl_bool(false) : fl_and(fl_gte(c, fl_str_val("0")), fl_lte(c, fl_str_val("9"))));
}

FLValue fl_user_is_alpha_p(FLValue c) {
    return (fl_truthy(null_p(c)) ? fl_bool(false) : fl_or(fl_and(fl_gte(c, fl_str_val("a")), fl_lte(c, fl_str_val("z"))), fl_and(fl_gte(c, fl_str_val("A")), fl_lte(c, fl_str_val("Z")))));
}

FLValue fl_user_is_alnum_p(FLValue c) {
    return fl_or(fl_user_is_digit_p(c), fl_user_is_alpha_p(c));
}

FLValue fl_user_is_space_p(FLValue c) {
    return fl_or(fl_or(fl_or(fl_eq(c, fl_str_val(" ")), fl_eq(c, fl_str_val("\t"))), fl_eq(c, fl_str_val("\n"))), fl_eq(c, fl_str_val("\r")));
}

FLValue fl_user_is_symbol_char_p(FLValue c) {
    return (fl_truthy(null_p(c)) ? fl_bool(false) : fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_user_is_alnum_p(c), fl_eq(c, fl_str_val("-"))), fl_eq(c, fl_str_val("_"))), fl_eq(c, fl_str_val("?"))), fl_eq(c, fl_str_val("!"))), fl_eq(c, fl_str_val("/"))), fl_eq(c, fl_str_val("."))), fl_eq(c, fl_str_val("<"))), fl_eq(c, fl_str_val(">"))), fl_eq(c, fl_str_val("="))), fl_eq(c, fl_str_val("+"))), fl_eq(c, fl_str_val("*"))), fl_eq(c, fl_str_val("%"))), fl_eq(c, fl_str_val("&"))), fl_eq(c, fl_str_val("|"))), fl_eq(c, fl_str_val("^"))), fl_eq(c, fl_str_val("~"))));
}

FLValue fl_user_make_state(FLValue src) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), src, fl_str_val("idx"), fl_int(0), fl_str_val("line"), fl_int(1), fl_str_val("col"), fl_int(1), fl_str_val("tokens"), fl_vec_new()}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_peek_at(FLValue st, FLValue offset) {
    return ((__extension__ ({
    FLValue src = get(st, fl_str_val("src"));
    FLValue i = fl_add(get(st, fl_str_val("idx")), offset);
    (fl_truthy(fl_gte(i, length(src))) ? fl_nil() : char_at(src, i));
})));
}

FLValue fl_user_peek(FLValue st) {
    return fl_user_peek_at(st, fl_int(0));
}

FLValue fl_user_at_end_p(FLValue st) {
    return fl_gte(get(st, fl_str_val("idx")), length(get(st, fl_str_val("src"))));
}

FLValue fl_user_advance(FLValue st) {
    return ((__extension__ ({
    FLValue c = fl_user_peek(st);
    (fl_truthy(fl_eq(c, fl_str_val("\n"))) ? (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), get(st, fl_str_val("src")), fl_str_val("idx"), fl_add(get(st, fl_str_val("idx")), fl_int(1)), fl_str_val("line"), fl_add(get(st, fl_str_val("line")), fl_int(1)), fl_str_val("col"), fl_int(1), fl_str_val("tokens"), get(st, fl_str_val("tokens"))}; fl_map_from_pairs(__fl_kv, 5); })) : (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), get(st, fl_str_val("src")), fl_str_val("idx"), fl_add(get(st, fl_str_val("idx")), fl_int(1)), fl_str_val("line"), get(st, fl_str_val("line")), fl_str_val("col"), fl_add(get(st, fl_str_val("col")), fl_int(1)), fl_str_val("tokens"), get(st, fl_str_val("tokens"))}; fl_map_from_pairs(__fl_kv, 5); })));
})));
}

FLValue fl_user_emit(FLValue st, FLValue kind, FLValue value, FLValue sl, FLValue sc) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), get(st, fl_str_val("src")), fl_str_val("idx"), get(st, fl_str_val("idx")), fl_str_val("line"), get(st, fl_str_val("line")), fl_str_val("col"), get(st, fl_str_val("col")), fl_str_val("tokens"), fl_vec_push(get(st, fl_str_val("tokens")), (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), kind, fl_str_val("type"), kind, fl_str_val("value"), value, fl_str_val("line"), sl, fl_str_val("col"), sc}; fl_map_from_pairs(__fl_kv, 5); })))}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_skip_comment_loop(FLValue _cur) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_or(fl_user_at_end_p(cur), fl_eq(fl_user_peek(cur), fl_str_val("\n")))) ? (fl_truthy(fl_user_at_end_p(cur)) ? cur : fl_user_advance(cur)) : (__extension__ ({
    FLValue _fl_t0 = fl_user_advance(cur);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_skip_comment(FLValue st) {
    return fl_user_skip_comment_loop(st);
}

FLValue fl_user_skip_ws_loop(FLValue _cur) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_user_at_end_p(cur)) ? cur : ((__extension__ ({
    FLValue c = fl_user_peek(cur);
    (fl_truthy(fl_user_is_space_p(c)) ? (__extension__ ({
    FLValue _fl_t0 = fl_user_advance(cur);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})) : (fl_truthy(fl_eq(c, fl_str_val(";"))) ? (__extension__ ({
    FLValue _fl_t0 = fl_user_skip_comment(cur);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})) : cur));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_skip_ws(FLValue st) {
    return fl_user_skip_ws_loop(st);
}

FLValue fl_user_read_number_iter(FLValue _cur, FLValue _res_acc, FLValue _dot, FLValue line, FLValue col) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _res_acc;
    FLValue res_acc = __fl_loop_tmp_2;
    FLValue __fl_loop_tmp_4 = _dot;
    FLValue dot = __fl_loop_tmp_4;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_user_at_end_p(cur)) ? fl_user_emit(cur, fl_str_val("Number"), res_acc, line, col) : ((__extension__ ({
    FLValue c = fl_user_peek(cur);
    (fl_truthy(fl_user_is_digit_p(c)) ? (__extension__ ({
    FLValue _fl_t0 = fl_user_advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    FLValue _fl_t2 = dot;
    cur = _fl_t0;
    res_acc = _fl_t1;
    dot = _fl_t2;
    _fl_looping = 1; fl_nil();
})) : (fl_truthy(fl_and(fl_eq(c, fl_str_val(".")), fl_not(dot))) ? (__extension__ ({
    FLValue _fl_t0 = fl_user_advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    FLValue _fl_t2 = fl_bool(true);
    cur = _fl_t0;
    res_acc = _fl_t1;
    dot = _fl_t2;
    _fl_looping = 1; fl_nil();
})) : fl_user_emit(cur, fl_str_val("Number"), res_acc, line, col)));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_read_number_body(FLValue st, FLValue acc, FLValue has_dot, FLValue line, FLValue col) {
    return fl_user_read_number_iter(st, acc, has_dot, line, col);
}

FLValue fl_user_read_number(FLValue st) {
    return fl_user_read_number_body(st, fl_str_val(""), fl_bool(false), get(st, fl_str_val("line")), get(st, fl_str_val("col")));
}

FLValue fl_user_translate_esc(FLValue c) {
    return (fl_truthy(fl_eq(c, fl_str_val("n"))) ? fl_str_val("\n") : (fl_truthy(fl_eq(c, fl_str_val("t"))) ? fl_str_val("\t") : (fl_truthy(fl_eq(c, fl_str_val("r"))) ? fl_str_val("\r") : (fl_truthy(fl_eq(c, fl_str_val("\""))) ? fl_str_val("\"") : (fl_truthy(fl_eq(c, fl_str_val("\\"))) ? fl_str_val("\\") : c)))));
}

FLValue fl_user_read_string_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _res_acc;
    FLValue res_acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_user_at_end_p(cur)) ? fl_user_emit(cur, fl_str_val("String"), res_acc, line, col) : ((__extension__ ({
    FLValue c = fl_user_peek(cur);
    (fl_truthy(fl_eq(c, fl_str_val("\""))) ? fl_user_emit(fl_user_advance(cur), fl_str_val("String"), res_acc, line, col) : (fl_truthy(fl_eq(c, fl_str_val("\\"))) ? ((__extension__ ({
    FLValue st2 = fl_user_advance(cur);
    FLValue c2 = fl_user_peek(st2);
    (__extension__ ({
    FLValue _fl_t0 = fl_user_advance(st2);
    FLValue _fl_t1 = fl_str_n(2, res_acc, fl_user_translate_esc(c2));
    cur = _fl_t0;
    res_acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))) : (__extension__ ({
    FLValue _fl_t0 = fl_user_advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    cur = _fl_t0;
    res_acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}))));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_read_string_body(FLValue st, FLValue acc, FLValue line, FLValue col) {
    return fl_user_read_string_iter(st, acc, line, col);
}

FLValue fl_user_read_string(FLValue st) {
    return ((__extension__ ({
    FLValue line = get(st, fl_str_val("line"));
    FLValue col = get(st, fl_str_val("col"));
    FLValue st1 = fl_user_advance(st);
    fl_user_read_string_body(st1, fl_str_val(""), line, col);
})));
}

FLValue fl_user_read_symbol_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col, FLValue kind) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _res_acc;
    FLValue res_acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_user_at_end_p(cur)) ? fl_user_emit(cur, kind, res_acc, line, col) : ((__extension__ ({
    FLValue c = fl_user_peek(cur);
    (fl_truthy(fl_user_is_symbol_char_p(c)) ? (__extension__ ({
    FLValue _fl_t0 = fl_user_advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    cur = _fl_t0;
    res_acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})) : fl_user_emit(cur, kind, res_acc, line, col));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_read_symbol_body_kind(FLValue st, FLValue acc, FLValue line, FLValue col, FLValue kind) {
    return fl_user_read_symbol_iter(st, acc, line, col, kind);
}

FLValue fl_user_read_symbol(FLValue st) {
    return fl_user_read_symbol_body_kind(st, fl_str_val(""), get(st, fl_str_val("line")), get(st, fl_str_val("col")), fl_str_val("Symbol"));
}

FLValue fl_user_read_variable(FLValue st) {
    return ((__extension__ ({
    FLValue line = get(st, fl_str_val("line"));
    FLValue col = get(st, fl_str_val("col"));
    FLValue st1 = fl_user_advance(st);
    fl_user_read_symbol_body_kind(st1, fl_str_val(""), line, col, fl_str_val("Variable"));
})));
}

FLValue fl_user_read_keyword(FLValue st) {
    return ((__extension__ ({
    FLValue line = get(st, fl_str_val("line"));
    FLValue col = get(st, fl_str_val("col"));
    FLValue st1 = fl_user_advance(st);
    fl_user_read_symbol_body_kind(st1, fl_str_val(""), line, col, fl_str_val("Keyword"));
})));
}

FLValue fl_user_read_token(FLValue st) {
    return ((__extension__ ({
    FLValue st1 = fl_user_skip_ws(st);
    (fl_truthy(fl_user_at_end_p(st1)) ? st1 : ((__extension__ ({
    FLValue c = fl_user_peek(st1);
    FLValue line = get(st1, fl_str_val("line"));
    FLValue col = get(st1, fl_str_val("col"));
    (fl_truthy(fl_eq(c, fl_str_val("("))) ? fl_user_emit(fl_user_advance(st1), fl_str_val("LParen"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val(")"))) ? fl_user_emit(fl_user_advance(st1), fl_str_val("RParen"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("["))) ? fl_user_emit(fl_user_advance(st1), fl_str_val("LBracket"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("]"))) ? fl_user_emit(fl_user_advance(st1), fl_str_val("RBracket"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("{"))) ? fl_user_emit(fl_user_advance(st1), fl_str_val("LBrace"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("}"))) ? fl_user_emit(fl_user_advance(st1), fl_str_val("RBrace"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("\""))) ? fl_user_read_string(st1) : (fl_truthy(fl_eq(c, fl_str_val("$"))) ? fl_user_read_variable(st1) : (fl_truthy(fl_eq(c, fl_str_val(":"))) ? fl_user_read_keyword(st1) : (fl_truthy(fl_user_is_digit_p(c)) ? fl_user_read_number(st1) : (fl_truthy(fl_and(fl_eq(c, fl_str_val("-")), fl_user_is_digit_p(fl_user_peek_at(st1, fl_int(1))))) ? fl_user_read_number_body(fl_user_advance(st1), fl_str_val("-"), fl_bool(false), line, col) : (fl_truthy(fl_user_is_symbol_char_p(c)) ? fl_user_read_symbol(st1) : fl_user_emit(fl_user_advance(st1), fl_str_val("Unknown"), c, line, col)))))))))))));
}))));
})));
}

FLValue fl_user_lex_loop(FLValue _cur) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = ((__extension__ ({
    FLValue ws = fl_user_skip_ws(cur);
    (fl_truthy(fl_user_at_end_p(ws)) ? get(ws, fl_str_val("tokens")) : (__extension__ ({
    FLValue _fl_t0 = fl_user_read_token(ws);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})));
})));
    }
    _fl_result;
}));
}

FLValue fl_user_lex(FLValue src) {
    return fl_user_lex_loop(fl_user_make_state(src));
}

FLValue fl_user_make_literal(FLValue type, FLValue value, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("literal"), fl_str_val("type"), type, fl_str_val("value"), value, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_variable(FLValue name, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("variable"), fl_str_val("name"), name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_keyword(FLValue name, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("keyword"), fl_str_val("name"), name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_sexpr(FLValue op, FLValue args, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("sexpr"), fl_str_val("op"), op, fl_str_val("args"), args, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_number(FLValue v, FLValue line) {
    return fl_user_make_literal(fl_str_val("number"), v, line);
}

FLValue fl_user_make_string(FLValue v, FLValue line) {
    return fl_user_make_literal(fl_str_val("string"), v, line);
}

FLValue fl_user_make_bool(FLValue v, FLValue line) {
    return fl_user_make_literal(fl_str_val("boolean"), v, line);
}

FLValue fl_user_make_null(FLValue line) {
    return fl_user_make_literal(fl_str_val("null"), fl_nil(), line);
}

FLValue fl_user_make_symbol(FLValue v, FLValue line) {
    return fl_user_make_literal(fl_str_val("symbol"), v, line);
}

FLValue fl_user_make_block(FLValue type, FLValue name, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("block"), fl_str_val("type"), type, fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_array_block(FLValue items, FLValue line) {
    return fl_user_make_block(fl_str_val("Array"), fl_nil(), (__extension__ ({ FLValue __fl_kv[2] = {fl_str_val("items"), items}; fl_map_from_pairs(__fl_kv, 1); })), line);
}

FLValue fl_user_make_map_block(FLValue items, FLValue line) {
    return fl_user_make_block(fl_str_val("Map"), fl_nil(), (__extension__ ({ FLValue __fl_kv[2] = {fl_str_val("items"), items}; fl_map_from_pairs(__fl_kv, 1); })), line);
}

FLValue fl_user_make_pattern_literal(FLValue value, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("pattern-literal"), fl_str_val("value"), value, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_pattern_variable(FLValue name, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("pattern-variable"), fl_str_val("name"), name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_pattern_wildcard(FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("kind"), fl_str_val("pattern-wildcard"), fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 2); }));
}

FLValue fl_user_make_pattern_list(FLValue items, FLValue rest, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-list"), fl_str_val("items"), items, fl_str_val("rest"), rest, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_pattern_struct(FLValue type_name, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-struct"), fl_str_val("type"), type_name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_pattern_or(FLValue alternatives, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("pattern-or"), fl_str_val("alternatives"), alternatives, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_pattern_range(FLValue start, FLValue end, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-range"), fl_str_val("start"), start, fl_str_val("end"), end, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_pattern_match(FLValue value, FLValue cases, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-match"), fl_str_val("value"), value, fl_str_val("cases"), cases, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_match_case(FLValue pattern, FLValue guard, FLValue body, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("match-case"), fl_str_val("pattern"), pattern, fl_str_val("guard"), guard, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_function_value(FLValue params, FLValue body, FLValue captured_env, FLValue name) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("function-value"), fl_str_val("params"), params, fl_str_val("body"), body, fl_str_val("capturedEnv"), captured_env, fl_str_val("name"), name}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_type_class(FLValue name, FLValue generics, FLValue methods, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("type-class"), fl_str_val("name"), name, fl_str_val("generics"), generics, fl_str_val("methods"), methods, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_type_class_instance(FLValue class_name, FLValue type_name, FLValue impls, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("type-class-instance"), fl_str_val("class"), class_name, fl_str_val("type"), type_name, fl_str_val("impls"), impls, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_module_block(FLValue name, FLValue exports, FLValue body, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("module"), fl_str_val("name"), name, fl_str_val("exports"), exports, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_import_block(FLValue path, FLValue alias, FLValue names, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("import"), fl_str_val("path"), path, fl_str_val("alias"), alias, fl_str_val("names"), names, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_open_block(FLValue module_name, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("open"), fl_str_val("module"), module_name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_search_block(FLValue query, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("search-block"), fl_str_val("query"), query, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_learn_block(FLValue topic, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("learn-block"), fl_str_val("topic"), topic, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_reasoning_block(FLValue name, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("reasoning-block"), fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_async_function(FLValue name, FLValue params, FLValue body, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("async-function"), fl_str_val("name"), name, fl_str_val("params"), params, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_await(FLValue expr, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("await"), fl_str_val("expr"), expr, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_try(FLValue body, FLValue catch, FLValue finally, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("try"), fl_str_val("body"), body, fl_str_val("catch"), catch, fl_str_val("finally"), finally, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_catch(FLValue param, FLValue body, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("catch"), fl_str_val("param"), param, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_throw(FLValue expr, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("throw"), fl_str_val("expr"), expr, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_make_template_string(FLValue parts, FLValue expressions, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("template-string"), fl_str_val("parts"), parts, fl_str_val("expressions"), expressions, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_loop(FLValue init, FLValue condition, FLValue update, FLValue body, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[12] = {fl_str_val("kind"), fl_str_val("loop"), fl_str_val("init"), init, fl_str_val("condition"), condition, fl_str_val("update"), update, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 6); }));
}

FLValue fl_user_make_page(FLValue name, FLValue path, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("page"), fl_str_val("name"), name, fl_str_val("path"), path, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_route(FLValue method, FLValue path, FLValue handler, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("route"), fl_str_val("method"), method, fl_str_val("path"), path, fl_str_val("handler"), handler, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); }));
}

FLValue fl_user_make_component(FLValue name, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("component"), fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_make_form(FLValue name, FLValue fields, FLValue line) {
    return (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("form"), fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); }));
}

FLValue fl_user_deep_equal_p(FLValue a, FLValue b) {
    return (fl_truthy(fl_and(null_p(a), null_p(b))) ? fl_bool(true) : (fl_truthy(fl_or(null_p(a), null_p(b))) ? fl_bool(false) : (fl_truthy(fl_and(list_p(a), list_p(b))) ? fl_user_deep_equal_list_p(a, b, fl_int(0)) : (fl_truthy(fl_and(fl_map_p(a), fl_map_p(b))) ? fl_user_deep_equal_map_p(a, b) : fl_eq(a, b)))));
}

FLValue fl_user_deep_equal_list_p(FLValue a, FLValue b, FLValue i) {
    return (fl_truthy(fl_not(fl_eq(length(a), length(b)))) ? fl_bool(false) : (fl_truthy(fl_gte(i, length(a))) ? fl_bool(true) : (fl_truthy(fl_not(fl_user_deep_equal_p(get(a, i), get(b, i)))) ? fl_bool(false) : fl_user_deep_equal_list_p(a, b, fl_add(i, fl_int(1))))));
}

FLValue fl_user_deep_equal_map_p(FLValue a, FLValue b) {
    return ((__extension__ ({
    FLValue ka = fl_user_keys_no_line(a);
    FLValue kb = fl_user_keys_no_line(b);
    (fl_truthy(fl_not(fl_eq(length(ka), length(kb)))) ? fl_bool(false) : fl_user_deep_equal_map_keys_p(a, b, ka, fl_int(0)));
})));
}

FLValue fl_user_keys_no_line(FLValue m) {
    return fl_filter_fn(fl_fn_new(__fl_anon_0, 0, NULL), fl_user_json_keys(m));
}

FLValue fl_user_deep_equal_map_keys_p(FLValue a, FLValue b, FLValue ks, FLValue i) {
    return (fl_truthy(fl_gte(i, length(ks))) ? fl_bool(true) : (fl_truthy(((__extension__ ({
    FLValue k = get(ks, i);
    fl_not(fl_user_deep_equal_p(get(a, k), get(b, k)));
})))) ? fl_bool(false) : fl_user_deep_equal_map_keys_p(a, b, ks, fl_add(i, fl_int(1)))));
}

FLValue fl_user_json_keys(FLValue m) {
    return fl_map_keys(m);
}

FLValue fl_user_p_make(FLValue tokens) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("tokens"), tokens, fl_str_val("idx"), fl_int(0), fl_str_val("ast"), fl_vec_new()}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_p_peek(FLValue p) {
    return ((__extension__ ({
    FLValue i = get(p, fl_str_val("idx"));
    FLValue t = get(p, fl_str_val("tokens"));
    (fl_truthy(fl_gte(i, length(t))) ? fl_nil() : get(t, i));
})));
}

FLValue fl_user_p_peek_at(FLValue p, FLValue offset) {
    return ((__extension__ ({
    FLValue i = fl_add(get(p, fl_str_val("idx")), offset);
    FLValue t = get(p, fl_str_val("tokens"));
    (fl_truthy(fl_gte(i, length(t))) ? fl_nil() : get(t, i));
})));
}

FLValue fl_user_p_end_p(FLValue p) {
    return fl_gte(get(p, fl_str_val("idx")), length(get(p, fl_str_val("tokens"))));
}

FLValue fl_user_p_advance(FLValue p) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("tokens"), get(p, fl_str_val("tokens")), fl_str_val("idx"), fl_add(get(p, fl_str_val("idx")), fl_int(1)), fl_str_val("ast"), get(p, fl_str_val("ast"))}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_p_with_ast(FLValue p, FLValue ast) {
    return (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("tokens"), get(p, fl_str_val("tokens")), fl_str_val("idx"), get(p, fl_str_val("idx")), fl_str_val("ast"), ast}; fl_map_from_pairs(__fl_kv, 3); }));
}

FLValue fl_user_p_append_ast(FLValue p, FLValue node) {
    return fl_user_p_with_ast(p, fl_vec_push(get(p, fl_str_val("ast")), node));
}

FLValue fl_user_r_pair(FLValue p, FLValue node) {
    return (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("p"), p, fl_str_val("node"), node}; fl_map_from_pairs(__fl_kv, 2); }));
}

FLValue fl_user_string_contains_p(FLValue s, FLValue substr) {
    return fl_not(fl_eq(fl_int(-1), str_index_of(s, substr)));
}

FLValue fl_user_parse_atom(FLValue p) {
    return ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    FLValue k = get(t, fl_str_val("kind"));
    FLValue v = get(t, fl_str_val("value"));
    FLValue line = get(t, fl_str_val("line"));
    (fl_truthy(fl_eq(k, fl_str_val("Number"))) ? fl_user_r_pair(fl_user_p_advance(p), fl_user_make_literal(fl_str_val("number"), v, line)) : (fl_truthy(fl_eq(k, fl_str_val("String"))) ? (fl_truthy(fl_user_string_contains_p(v, fl_str_n(2, fl_str_val("$"), fl_str_val("{")))) ? fl_user_r_pair(fl_user_p_advance(p), fl_user_make_template_string(v, fl_vec_new(), line)) : fl_user_r_pair(fl_user_p_advance(p), fl_user_make_literal(fl_str_val("string"), v, line))) : (fl_truthy(fl_eq(k, fl_str_val("Symbol"))) ? (fl_truthy(fl_eq(v, fl_str_val("true"))) ? fl_user_r_pair(fl_user_p_advance(p), fl_user_make_literal(fl_str_val("boolean"), fl_bool(true), line)) : (fl_truthy(fl_eq(v, fl_str_val("false"))) ? fl_user_r_pair(fl_user_p_advance(p), fl_user_make_literal(fl_str_val("boolean"), fl_bool(false), line)) : (fl_truthy(fl_or(fl_eq(v, fl_str_val("nil")), fl_eq(v, fl_str_val("null")))) ? fl_user_r_pair(fl_user_p_advance(p), fl_user_make_literal(fl_str_val("nil"), fl_nil(), line)) : fl_user_r_pair(fl_user_p_advance(p), fl_user_make_literal(fl_str_val("symbol"), v, line))))) : (fl_truthy(fl_eq(k, fl_str_val("Variable"))) ? fl_user_r_pair(fl_user_p_advance(p), fl_user_make_variable(v, line)) : (fl_truthy(fl_eq(k, fl_str_val("Keyword"))) ? fl_user_r_pair(fl_user_p_advance(p), fl_user_make_keyword(v, line)) : fl_user_r_pair(fl_user_p_advance(p), fl_user_make_literal(fl_str_val("unknown"), v, line)))))));
})));
}

FLValue fl_user_parse_expr(FLValue p) {
    return ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    FLValue k = get(t, fl_str_val("kind"));
    FLValue v = get(t, fl_str_val("value"));
    (fl_truthy(fl_eq(k, fl_str_val("LParen"))) ? fl_user_parse_sexpr(p) : (fl_truthy(fl_eq(k, fl_str_val("LBracket"))) ? fl_user_parse_bracket(p) : (fl_truthy(fl_eq(k, fl_str_val("LBrace"))) ? fl_user_parse_map(p) : (fl_truthy(fl_and(fl_eq(k, fl_str_val("Unknown")), fl_eq(v, fl_str_val("@")))) ? ((__extension__ ({
    FLValue p1 = fl_user_p_advance(p);
    FLValue res = fl_user_parse_expr(p1);
    fl_user_r_pair(get(res, fl_str_val("p")), fl_user_make_sexpr(fl_str_val("deref"), (__extension__ ({ FLValue __fl_lst[1] = {get(res, fl_str_val("node"))}; fl_vec_from(__fl_lst, 1); })), get(t, fl_str_val("line"))));
}))) : fl_user_parse_atom(p)))));
})));
}

FLValue fl_user_parse_sexpr(FLValue p) {
    return ((__extension__ ({
    FLValue start_tok = fl_user_p_peek(p);
    FLValue line = get(start_tok, fl_str_val("line"));
    FLValue p1 = fl_user_p_advance(p);
    FLValue first = fl_user_parse_args(p1, fl_vec_new());
    FLValue p2 = get(first, fl_str_val("p"));
    FLValue args = get(first, fl_str_val("node"));
    (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_user_r_pair(fl_user_parse_consume_rparen(p2), fl_user_make_sexpr(fl_str_val(""), fl_vec_new(), line)) : ((__extension__ ({
    FLValue op_node = get(args, fl_int(0));
    FLValue op = (fl_truthy(fl_eq(get(op_node, fl_str_val("kind")), fl_str_val("literal"))) ? get(op_node, fl_str_val("value")) : (fl_truthy(fl_eq(get(op_node, fl_str_val("kind")), fl_str_val("variable"))) ? fl_str_n(2, fl_str_val("$"), get(op_node, fl_str_val("name"))) : fl_str_val("unknown")));
    FLValue rest = substring(args, fl_int(1), length(args));
    (fl_truthy(fl_eq(op, fl_str_val("try"))) ? ((__extension__ ({
    FLValue body = (fl_truthy(fl_gt(length(rest), fl_int(0))) ? get(rest, fl_int(0)) : fl_nil());
    FLValue catch_clause = (fl_truthy(fl_gt(length(rest), fl_int(1))) ? get(rest, fl_int(1)) : fl_nil());
    FLValue finally_clause = (fl_truthy(fl_gt(length(rest), fl_int(2))) ? get(rest, fl_int(2)) : fl_nil());
    fl_user_r_pair(fl_user_parse_consume_rparen(p2), fl_user_make_try(body, catch_clause, finally_clause, line));
}))) : (fl_truthy(fl_eq(op, fl_str_val("loop"))) ? ((__extension__ ({
    FLValue loop_array = (fl_truthy(fl_gt(length(rest), fl_int(0))) ? get(rest, fl_int(0)) : fl_nil());
    FLValue _body_start __attribute__((unused)) = (fl_truthy(fl_gt(length(rest), fl_int(1))) ? get(rest, fl_int(1)) : fl_nil());
    (fl_truthy(null_p(loop_array)) ? fl_user_r_pair(fl_user_parse_consume_rparen(p2), fl_user_make_sexpr(fl_str_val("loop"), fl_vec_new(), line)) : (fl_truthy(fl_not(fl_eq(get(loop_array, fl_str_val("kind")), fl_str_val("array")))) ? fl_user_r_pair(fl_user_parse_consume_rparen(p2), fl_user_make_sexpr(fl_str_val("loop"), rest, line)) : ((__extension__ ({
    FLValue items = get(loop_array, fl_str_val("items"));
    FLValue init = (fl_truthy(fl_gt(length(items), fl_int(0))) ? get(items, fl_int(0)) : fl_nil());
    FLValue condition = (fl_truthy(fl_gt(length(items), fl_int(1))) ? get(items, fl_int(1)) : fl_nil());
    FLValue update = (fl_truthy(fl_gt(length(items), fl_int(2))) ? get(items, fl_int(2)) : fl_nil());
    FLValue body_exprs = substring(rest, fl_int(1), length(rest));
    fl_user_r_pair(fl_user_parse_consume_rparen(p2), fl_user_make_loop(init, condition, update, (fl_truthy(fl_eq(length(body_exprs), fl_int(1))) ? get(body_exprs, fl_int(0)) : fl_user_make_sexpr(fl_str_val("do"), body_exprs, line)), line));
})))));
}))) : (fl_truthy(fl_eq(op, fl_str_val("and"))) ? fl_user_r_pair(fl_user_parse_consume_rparen(p2), (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("and"), fl_str_val("args"), rest, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }))) : (fl_truthy(fl_eq(op, fl_str_val("or"))) ? fl_user_r_pair(fl_user_parse_consume_rparen(p2), (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("or"), fl_str_val("args"), rest, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }))) : fl_user_r_pair(fl_user_parse_consume_rparen(p2), fl_user_make_sexpr(op, rest, line))))));
}))));
})));
}

FLValue fl_user_parse_consume_rparen(FLValue p) {
    return ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    (fl_truthy(fl_and(fl_not(null_p(t)), fl_eq(get(t, fl_str_val("kind")), fl_str_val("RParen")))) ? fl_user_p_advance(p) : p);
})));
}

FLValue fl_user_parse_args(FLValue p, FLValue acc) {
    return ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    (fl_truthy(null_p(t)) ? fl_user_r_pair(p, acc) : (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("RParen"))) ? fl_user_r_pair(p, acc) : (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBracket"))) ? fl_user_r_pair(p, acc) : (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBrace"))) ? fl_user_r_pair(p, acc) : ((__extension__ ({
    FLValue one = fl_user_parse_expr(p);
    fl_user_parse_args(get(one, fl_str_val("p")), fl_vec_push(acc, get(one, fl_str_val("node"))));
})))))));
})));
}

FLValue fl_user_parse_bracket(FLValue p) {
    return ((__extension__ ({
    FLValue tok = fl_user_p_peek(p);
    FLValue line = get(tok, fl_str_val("line"));
    FLValue p1 = fl_user_p_advance(p);
    FLValue next = fl_user_p_peek(p1);
    (fl_truthy(fl_and(fl_and(fl_and(fl_not(null_p(next)), fl_eq(get(next, fl_str_val("kind")), fl_str_val("Symbol"))), fl_user_is_block_type_p(get(next, fl_str_val("value")))), fl_eq(get(next, fl_str_val("value")), fl_user_upper_case(get(next, fl_str_val("value")))))) ? fl_user_parse_named_block(p1, line) : fl_user_parse_array(p1, line));
})));
}

FLValue fl_user_is_block_type_p(FLValue s) {
    return ((__extension__ ({
    FLValue c = char_at(s, fl_int(0));
    fl_and(fl_gte(c, fl_str_val("A")), fl_lte(c, fl_str_val("Z")));
})));
}

FLValue fl_user_upper_case(FLValue s) {
    return s;
}

FLValue fl_user_parse_array(FLValue p, FLValue line) {
    return ((__extension__ ({
    FLValue collected = fl_user_parse_args(p, fl_vec_new());
    FLValue p2 = get(collected, fl_str_val("p"));
    FLValue items = get(collected, fl_str_val("node"));
    fl_user_r_pair(fl_user_parse_consume_rbracket(p2), fl_user_make_array_block(items, line));
})));
}

FLValue fl_user_parse_consume_rbracket(FLValue p) {
    return ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    (fl_truthy(fl_and(fl_not(null_p(t)), fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBracket")))) ? fl_user_p_advance(p) : p);
})));
}

FLValue fl_user_parse_named_block(FLValue p, FLValue line) {
    return ((__extension__ ({
    FLValue type_tok = fl_user_p_peek(p);
    FLValue type = get(type_tok, fl_str_val("value"));
    FLValue p1 = fl_user_p_advance(p);
    FLValue name_info = fl_user_parse_optional_name(p1);
    FLValue p2 = get(name_info, fl_str_val("p"));
    FLValue name = get(name_info, fl_str_val("node"));
    FLValue fields_info = fl_user_parse_block_fields(p2, fl_map_new());
    FLValue p3 = get(fields_info, fl_str_val("p"));
    FLValue fields = get(fields_info, fl_str_val("node"));
    fl_user_r_pair(fl_user_parse_consume_rbracket(p3), fl_user_make_block(type, name, fields, line));
})));
}

FLValue fl_user_parse_optional_name(FLValue p) {
    return ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    (fl_truthy(fl_and(fl_and(fl_not(null_p(t)), fl_eq(get(t, fl_str_val("kind")), fl_str_val("Symbol"))), fl_not(fl_eq(char_at(get(t, fl_str_val("value")), fl_int(0)), fl_str_val(":"))))) ? fl_user_r_pair(fl_user_p_advance(p), get(t, fl_str_val("value"))) : fl_user_r_pair(p, fl_nil()));
})));
}

FLValue fl_user_parse_block_fields(FLValue p, FLValue acc) {
    return (fl_truthy(fl_user_p_end_p(p)) ? fl_user_r_pair(p, acc) : ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("Keyword"))) ? ((__extension__ ({
    FLValue key = get(t, fl_str_val("value"));
    FLValue p1 = fl_user_p_advance(p);
    FLValue val = fl_user_parse_expr(p1);
    fl_user_parse_block_fields(get(val, fl_str_val("p")), fl_map_set(acc, key, get(val, fl_str_val("node"))));
}))) : fl_user_r_pair(p, acc));
}))));
}

FLValue fl_user_parse_map(FLValue p) {
    return ((__extension__ ({
    FLValue tok = fl_user_p_peek(p);
    FLValue line = get(tok, fl_str_val("line"));
    FLValue p1 = fl_user_p_advance(p);
    FLValue collected = fl_user_parse_args(p1, fl_vec_new());
    FLValue p2 = get(collected, fl_str_val("p"));
    FLValue items = get(collected, fl_str_val("node"));
    fl_user_r_pair(fl_user_parse_consume_rbrace(p2), fl_user_make_map_block(items, line));
})));
}

FLValue fl_user_parse_consume_rbrace(FLValue p) {
    return ((__extension__ ({
    FLValue t = fl_user_p_peek(p);
    (fl_truthy(fl_and(fl_not(null_p(t)), fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBrace")))) ? fl_user_p_advance(p) : p);
})));
}

FLValue fl_user_parse_all(FLValue p) {
    return (fl_truthy(fl_user_p_end_p(p)) ? get(p, fl_str_val("ast")) : ((__extension__ ({
    FLValue one = fl_user_parse_expr(p);
    FLValue p2 = fl_user_p_append_ast(get(one, fl_str_val("p")), get(one, fl_str_val("node")));
    fl_user_parse_all(p2);
}))));
}

FLValue fl_user_parse(FLValue tokens) {
    return fl_user_parse_all(fl_user_p_make(tokens));
}

FLValue fl_user_get_block_items(FLValue node) {
    return (fl_truthy(null_p(node)) ? fl_vec_new() : (fl_truthy(fl_and(fl_eq(get(node, fl_str_val("kind")), fl_str_val("block")), fl_eq(get(node, fl_str_val("type")), fl_str_val("Array")))) ? get(get(node, fl_str_val("fields")), fl_str_val("items")) : node));
}

FLValue fl_user_c_esc(FLValue s) {
    return str_replace(str_replace(str_replace(str_replace(str_replace(s, fl_str_val("\\"), fl_str_val("\\\\")), fl_str_val("\""), fl_str_val("\\\"")), fl_str_val("\n"), fl_str_val("\\n")), fl_str_val("\t"), fl_str_val("\\t")), fl_str_val("\r"), fl_str_val("\\r"));
}

FLValue fl_user_c_reserved_p(FLValue s) {
    return fl_user_includes_item((__extension__ ({ FLValue __fl_arr[29] = {fl_str_val("else"), fl_str_val("return"), fl_str_val("for"), fl_str_val("while"), fl_str_val("do"), fl_str_val("int"), fl_str_val("long"), fl_str_val("void"), fl_str_val("char"), fl_str_val("float"), fl_str_val("double"), fl_str_val("struct"), fl_str_val("union"), fl_str_val("enum"), fl_str_val("static"), fl_str_val("const"), fl_str_val("if"), fl_str_val("switch"), fl_str_val("case"), fl_str_val("break"), fl_str_val("continue"), fl_str_val("default"), fl_str_val("goto"), fl_str_val("sizeof"), fl_str_val("auto"), fl_str_val("inline"), fl_str_val("bool"), fl_str_val("true"), fl_str_val("false")}; fl_vec_from(__fl_arr, 29); })), s);
}

FLValue fl_user_c_name(FLValue n) {
    return ((__extension__ ({
    FLValue raw = str_replace(str_replace(str_replace(str_replace(n, fl_str_val("->"), fl_str_val("_to_")), fl_str_val("-"), fl_str_val("_")), fl_str_val("?"), fl_str_val("_p")), fl_str_val("!"), fl_str_val("_b"));
    (fl_truthy(fl_user_c_reserved_p(raw)) ? fl_str_n(2, fl_str_val("fl_"), raw) : raw);
})));
}

FLValue fl_user_user_c_name(FLValue n) {
    return fl_str_n(2, fl_str_val("fl_user_"), fl_user_c_name(n));
}

FLValue fl_user_cgc_user_defn_p(FLValue n) {
    return fl_user_includes_item(fl_atom_deref(fl_user_known_defns_atom), fl_user_user_c_name(n));
}

FLValue fl_user_cgc_language_special_form_p(FLValue op) {
    return fl_user_includes_item((__extension__ ({ FLValue __fl_arr[13] = {fl_str_val("if"), fl_str_val("cond"), fl_str_val("do"), fl_str_val("let"), fl_str_val("defn"), fl_str_val("define"), fl_str_val("set!"), fl_str_val("while"), fl_str_val("loop"), fl_str_val("recur"), fl_str_val("fn"), fl_str_val("and"), fl_str_val("or")}; fl_vec_from(__fl_arr, 13); })), op);
}

FLValue fl_user_cgc(FLValue n) {
    return (fl_truthy(null_p(n)) ? fl_str_val("fl_nil()") : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("literal"))) ? fl_user_cgc_literal(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("variable"))) ? ((__extension__ ({
    FLValue name = get(n, fl_str_val("name"));
    FLValue cn = fl_user_c_name(name);
    FLValue user_cn = fl_user_user_c_name(name);
    FLValue line = get(n, fl_str_val("line"));
    (fl_truthy(fl_user_includes_item(fl_atom_deref(fl_user_known_defns_atom), user_cn)) ? fl_str_n(3, fl_str_val("fl_fn_new(__fl_wrap_"), user_cn, fl_str_val(", 0, NULL)")) : (fl_truthy(fl_user_includes_item(fl_atom_deref(fl_user_known_user_globals_atom), user_cn)) ? user_cn : (fl_truthy(fl_not(fl_user_includes_item(fl_atom_deref(fl_user_known_fncall_targets_atom), cn))) ? (__extension__ ({ fl_println(fl_str_n(3, fl_str_val("[FL Warn] 정의되지 않은 이름: "), name, (fl_truthy(null_p(line)) ? fl_str_val("") : fl_str_n(3, fl_str_val(" (line "), line, fl_str_val(")"))))); cn;  })) : cn)));
}))) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("keyword"))) ? fl_str_n(3, fl_str_val("fl_str_val(\""), fl_user_c_esc(get(n, fl_str_val("name"))), fl_str_val("\")")) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("sexpr"))) ? fl_user_cgc_sexpr(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("block"))) ? fl_user_cgc_block(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("and"))) ? fl_user_cgc_and(get(n, fl_str_val("args"))) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("or"))) ? fl_user_cgc_or(get(n, fl_str_val("args"))) : (__extension__ ({ fl_println(fl_str_n(4, fl_str_val("[CGC-ERR] unsupported IR kind="), get(n, fl_str_val("kind")), fl_str_val(" line="), get(n, fl_str_val("line")))); fl_str_val("fl_nil()");  }))))))))));
}

FLValue fl_user_cgc_literal(FLValue n) {
    return ((__extension__ ({
    FLValue t = get(n, fl_str_val("type"));
    FLValue v = get(n, fl_str_val("value"));
    (fl_truthy(fl_eq(t, fl_str_val("number"))) ? (fl_truthy(fl_str_includes(fl_str_n(1, v), fl_str_val("."))) ? fl_str_n(3, fl_str_val("fl_float("), v, fl_str_val(")")) : fl_str_n(3, fl_str_val("fl_int("), v, fl_str_val(")"))) : (fl_truthy(fl_eq(t, fl_str_val("string"))) ? fl_str_n(3, fl_str_val("fl_str_val(\""), fl_user_c_esc(v), fl_str_val("\")")) : (fl_truthy(fl_eq(t, fl_str_val("boolean"))) ? (fl_truthy(v) ? fl_str_val("fl_bool(true)") : fl_str_val("fl_bool(false)")) : (fl_truthy(fl_eq(t, fl_str_val("nil"))) ? fl_str_val("fl_nil()") : (fl_truthy(fl_eq(t, fl_str_val("symbol"))) ? (fl_truthy(fl_eq(v, fl_str_val("true"))) ? fl_str_val("fl_bool(true)") : (fl_truthy(fl_eq(v, fl_str_val("false"))) ? fl_str_val("fl_bool(false)") : (fl_truthy(fl_or(fl_eq(v, fl_str_val("nil")), fl_eq(v, fl_str_val("null")))) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue cn = fl_user_c_name(v);
    FLValue user_cn = fl_user_user_c_name(v);
    (fl_truthy(fl_user_includes_item(fl_atom_deref(fl_user_known_defns_atom), user_cn)) ? fl_str_n(3, fl_str_val("fl_fn_new(__fl_wrap_"), user_cn, fl_str_val(", 0, NULL)")) : (fl_truthy(fl_user_includes_item(fl_atom_deref(fl_user_known_user_globals_atom), user_cn)) ? user_cn : cn));
})))))) : (__extension__ ({ fl_println(fl_str_n(4, fl_str_val("[CGC-ERR] unsupported literal type="), get(n, fl_str_val("type")), fl_str_val(" line="), get(n, fl_str_val("line")))); fl_str_val("fl_nil()");  })))))));
})));
}

FLValue fl_user_cgc_block(FLValue n) {
    return ((__extension__ ({
    FLValue t = get(n, fl_str_val("type"));
    (fl_truthy(fl_eq(t, fl_str_val("FUNC"))) ? fl_user_cgc_func_block(n) : (fl_truthy(fl_eq(t, fl_str_val("Array"))) ? fl_user_cgc_array_block(n) : (fl_truthy(fl_eq(t, fl_str_val("Map"))) ? fl_user_cgc_map_block(n) : (__extension__ ({ fl_println(fl_str_n(4, fl_str_val("[CGC-ERR] unsupported block type="), t, fl_str_val(" line="), get(n, fl_str_val("line")))); fl_str_val("fl_nil()");  })))));
})));
}

FLValue fl_user_cgc_func_block(FLValue n) {
    return ((__extension__ ({
    FLValue name = fl_user_user_c_name(get(n, fl_str_val("name")));
    FLValue f = get(n, fl_str_val("fields"));
    FLValue params_block = get(f, fl_str_val("params"));
    FLValue body_node = get(f, fl_str_val("body"));
    FLValue ps = fl_user_cgc_params(fl_user_get_block_items(params_block));
    FLValue body = fl_user_cgc(body_node);
    fl_str_n(7, fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(") {\n    return "), body, fl_str_val(";\n}"));
})));
}

FLValue fl_user_cgc_params(FLValue it) {
    return fl_user_cgc_params_loop(it, fl_int(0), fl_str_val(""));
}

FLValue fl_user_cgc_params_loop(FLValue _it, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_it))) ? acc : ((__extension__ ({
    FLValue n = fl_user_cgc_extract_name(get(_it, i));
    FLValue entry = fl_str_n(2, fl_str_val("FLValue "), n);
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy(fl_eq(length(acc), fl_int(0))) ? entry : fl_str_n(3, acc, fl_str_val(", "), entry));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_extract_name(FLValue node) {
    return (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("variable"))) ? fl_user_c_name(get(node, fl_str_val("name"))) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("literal"))) ? fl_user_c_name(fl_str_n(1, get(node, fl_str_val("value")))) : fl_str_val("_anon")));
}

FLValue fl_user_cgc_fncall(FLValue fn_c, FLValue args) {
    return ((__extension__ ({
    FLValue argc = length(args);
    FLValue id = fl_atom_deref(fl_user_lambda_id_atom);
    fl_atom_reset(fl_user_lambda_id_atom, fl_add(fl_atom_deref(fl_user_lambda_id_atom), fl_int(1)));
    (fl_truthy(fl_eq(argc, fl_int(0))) ? fl_str_n(3, fl_str_val("fl_fn_call("), fn_c, fl_str_val(", 0, NULL)")) : fl_str_n(14, fl_str_val("(__extension__ ({ FLValue __fl_ca_"), id, fl_str_val("["), argc, fl_str_val("] = {"), fl_user_cgc_args(args), fl_str_val("};"), fl_str_val(" fl_fn_call("), fn_c, fl_str_val(", "), argc, fl_str_val(", __fl_ca_"), id, fl_str_val("); }))")));
})));
}

FLValue fl_user_cgc_sexpr(FLValue n) {
    return ((__extension__ ({
    FLValue op = get(n, fl_str_val("op"));
    FLValue args = get(n, fl_str_val("args"));
    (fl_truthy(fl_string_p(op)) ? fl_user_cgc_dispatch(op, args) : fl_user_cgc_fncall(fl_user_cgc(op), args));
})));
}

FLValue fl_user_cgc_dispatch(FLValue op, FLValue args) {
    return (fl_truthy(fl_and(fl_user_cgc_user_defn_p(op), fl_not(fl_user_cgc_language_special_form_p(op)))) ? fl_str_n(4, fl_user_user_c_name(op), fl_str_val("("), fl_user_cgc_args(args), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("if"))) ? fl_user_cgc_if(args) : (fl_truthy(fl_eq(op, fl_str_val("cond"))) ? fl_user_cgc_cond(args) : (fl_truthy(fl_eq(op, fl_str_val("do"))) ? fl_user_cgc_do(args) : (fl_truthy(fl_eq(op, fl_str_val("let"))) ? fl_user_cgc_let(args) : (fl_truthy(fl_eq(op, fl_str_val("defn"))) ? fl_user_cgc_defn(args) : (fl_truthy(fl_eq(op, fl_str_val("define"))) ? fl_user_cgc_define(args) : (fl_truthy(fl_eq(op, fl_str_val("+"))) ? fl_user_cgc_binop_chain(args, fl_str_val("fl_add")) : (fl_truthy(fl_eq(op, fl_str_val("-"))) ? (fl_truthy(fl_eq(length(args), fl_int(1))) ? fl_str_n(3, fl_str_val("fl_sub(fl_int(0), "), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : fl_user_cgc_binop_chain(args, fl_str_val("fl_sub"))) : (fl_truthy(fl_eq(op, fl_str_val("*"))) ? fl_user_cgc_binop_chain(args, fl_str_val("fl_mul")) : (fl_truthy(fl_eq(op, fl_str_val("/"))) ? fl_str_n(5, fl_str_val("fl_div("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("%"))) ? fl_str_n(5, fl_str_val("fl_mod("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("="))) ? fl_str_n(5, fl_str_val("fl_eq("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("!="))) ? fl_str_n(5, fl_str_val("fl_neq("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("<"))) ? fl_str_n(5, fl_str_val("fl_lt("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val(">"))) ? fl_str_n(5, fl_str_val("fl_gt("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("<="))) ? fl_str_n(5, fl_str_val("fl_lte("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val(">="))) ? fl_str_n(5, fl_str_val("fl_gte("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("and"))) ? fl_user_cgc_and(args) : (fl_truthy(fl_eq(op, fl_str_val("or"))) ? fl_user_cgc_or(args) : (fl_truthy(fl_eq(op, fl_str_val("not"))) ? fl_str_n(3, fl_str_val("fl_not("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("println"))) ? fl_str_n(3, fl_str_val("fl_println("), fl_user_cgc_str_arg(args), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("print"))) ? fl_str_n(3, fl_str_val("fl_print("), fl_user_cgc_str_arg(args), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str"))) ? fl_user_cgc_str(args) : (fl_truthy(fl_eq(op, fl_str_val("set!"))) ? fl_user_cgc_set_b(args) : (fl_truthy(fl_eq(op, fl_str_val("while"))) ? fl_user_cgc_while(args) : (fl_truthy(fl_eq(op, fl_str_val("atom"))) ? fl_str_n(3, fl_str_val("fl_atom_new("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("deref"))) ? fl_str_n(3, fl_str_val("fl_atom_deref("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("reset!"))) ? fl_str_n(5, fl_str_val("fl_atom_reset("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("swap!"))) ? fl_user_cgc_swap_b(args) : (fl_truthy(fl_eq(op, fl_str_val("includes-item"))) ? fl_str_n(5, fl_str_val("fl_includes_item("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-includes"))) ? fl_str_n(5, fl_str_val("fl_str_includes("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-starts-with"))) ? fl_str_n(5, fl_str_val("fl_str_starts_with("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-ends-with"))) ? fl_str_n(5, fl_str_val("fl_str_ends_with("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("string?"))) ? fl_str_n(3, fl_str_val("fl_string_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-entries"))) ? fl_str_n(3, fl_str_val("fl_map_entries("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("length"))) ? fl_str_n(3, fl_str_val("length("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("count"))) ? fl_str_n(3, fl_str_val("length("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("get"))) ? fl_str_n(5, fl_str_val("get("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("assoc"))) ? fl_str_n(7, fl_str_val("fl_map_set("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("dissoc"))) ? fl_str_n(5, fl_str_val("dissoc("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("reverse"))) ? fl_str_n(3, fl_str_val("reverse("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("sort"))) ? fl_str_n(3, fl_str_val("sort("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("sort-by"))) ? fl_str_n(5, fl_str_val("fl_sort_by("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("range"))) ? fl_str_n(3, fl_str_val("range("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("take"))) ? fl_str_n(5, fl_str_val("take("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("drop"))) ? fl_str_n(5, fl_str_val("drop("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("last"))) ? fl_str_n(3, fl_str_val("fl_vec_last("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("nth"))) ? fl_str_n(5, fl_str_val("get("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("concat"))) ? fl_str_n(5, fl_str_val("fl_concat("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("uuid"))) ? fl_str_val("uuid()") : (fl_truthy(fl_eq(op, fl_str_val("obj-merge"))) ? fl_str_n(5, fl_str_val("fl_map_merge("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-pick"))) ? fl_str_n(5, fl_str_val("select_keys("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-omit"))) ? fl_str_n(5, fl_str_val("fl_obj_omit("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-values"))) ? fl_str_n(3, fl_str_val("vals("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("vals"))) ? fl_str_n(3, fl_str_val("vals("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("entries"))) ? fl_str_n(3, fl_str_val("entries("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("get-in"))) ? fl_str_n(5, fl_str_val("fl_get_in("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map-vals"))) ? fl_str_n(5, fl_str_val("fl_map_vals_fn("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-split"))) ? fl_str_n(5, fl_str_val("str_split("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-slice"))) ? fl_str_n(7, fl_str_val("substring("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-to-upper"))) ? fl_str_n(3, fl_str_val("str_to_upper("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-to-lower"))) ? fl_str_n(3, fl_str_val("str_to_lower("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-trim"))) ? fl_str_n(3, fl_str_val("str_trim("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-pad-left"))) ? fl_str_n(7, fl_str_val("str_pad_left("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-pad-right"))) ? fl_str_n(7, fl_str_val("str_pad_right("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-index-of"))) ? fl_str_n(5, fl_str_val("str_index_of("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-repeat"))) ? fl_str_n(5, fl_str_val("str_repeat("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-length"))) ? fl_str_n(3, fl_str_val("length("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-to-num"))) ? fl_str_n(3, fl_str_val("fl_str_to_num("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("html-escape"))) ? fl_str_n(3, fl_str_val("fl_html_escape("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("number?"))) ? fl_str_n(3, fl_str_val("fl_number_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("boolean?"))) ? fl_str_n(3, fl_str_val("fl_boolean_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("integer?"))) ? fl_str_n(3, fl_str_val("fl_integer_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("float?"))) ? fl_str_n(3, fl_str_val("fl_float_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("array?"))) ? fl_str_n(3, fl_str_val("fl_array_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("vector?"))) ? fl_str_n(3, fl_str_val("fl_array_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map?"))) ? fl_str_n(3, fl_str_val("fl_map_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("fn?"))) ? fl_str_n(3, fl_str_val("fl_fn_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("type-of"))) ? fl_str_n(3, fl_str_val("type_of("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("empty?"))) ? fl_str_n(3, fl_str_val("fl_empty_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("not-empty?"))) ? fl_str_n(3, fl_str_val("fl_not_empty_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("nil-or-empty?"))) ? fl_str_n(3, fl_str_val("fl_nil_or_empty_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("loop"))) ? fl_user_cgc_loop(args) : (fl_truthy(fl_eq(op, fl_str_val("recur"))) ? fl_str_val("/* orphan recur */ fl_nil()") : (fl_truthy(fl_eq(op, fl_str_val("map"))) ? fl_str_n(5, fl_str_val("fl_map_fn("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("filter"))) ? fl_str_n(5, fl_str_val("fl_filter_fn("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("reduce"))) ? fl_str_n(7, fl_str_val("fl_reduce_fn("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("append"))) ? fl_str_n(5, fl_str_val("fl_vec_push("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("push"))) ? fl_str_n(5, fl_str_val("fl_vec_push("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("flatten"))) ? fl_str_n(3, fl_str_val("flatten("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("flatten-1"))) ? fl_str_n(3, fl_str_val("flatten("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("distinct"))) ? fl_str_n(3, fl_str_val("distinct("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("zip"))) ? fl_str_n(5, fl_str_val("zip("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("zip-with"))) ? fl_str_n(5, fl_str_val("zip("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("group-by"))) ? fl_str_n(5, fl_str_val("group_by("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("frequencies"))) ? fl_str_n(3, fl_str_val("frequencies("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("every?"))) ? fl_str_n(5, fl_str_val("fl_every_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("any?"))) ? fl_str_n(5, fl_str_val("fl_any_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("none?"))) ? fl_str_n(5, fl_str_val("fl_none_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("find-first"))) ? fl_str_n(5, fl_str_val("fl_find_first("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("count-if"))) ? fl_str_n(5, fl_str_val("fl_count_if("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("repeat"))) ? fl_str_n(5, fl_str_val("fl_repeat("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map-indexed"))) ? fl_str_n(5, fl_str_val("fl_map_indexed("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("mapcat"))) ? fl_str_n(5, fl_str_val("fl_mapcat("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("keep"))) ? fl_str_n(5, fl_str_val("fl_keep("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("comp"))) ? fl_str_n(5, fl_str_val("fl_comp("), fl_user_cgc_args(args), fl_str_val(", "), length(args), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("conj"))) ? fl_str_n(5, fl_str_val("fl_conj("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("into"))) ? fl_str_n(5, fl_str_val("fl_into("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("keys"))) ? fl_str_n(3, fl_str_val("fl_map_keys("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-keys"))) ? fl_str_n(3, fl_str_val("fl_map_keys("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map-entries"))) ? fl_str_n(3, fl_str_val("fl_map_entries("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("null?"))) ? fl_str_n(3, fl_str_val("null_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("nil?"))) ? fl_str_n(3, fl_str_val("null_p("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("char-at"))) ? fl_str_n(5, fl_str_val("char_at("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("char-code-at"))) ? fl_str_n(5, fl_str_val("char_code_at("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("floor"))) ? fl_str_n(3, fl_str_val("fl_floor("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("ceil"))) ? fl_str_n(3, fl_str_val("fl_ceil("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("abs"))) ? fl_str_n(3, fl_str_val("fl_abs("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("math-sqrt"))) ? fl_str_n(3, fl_str_val("fl_math_sqrt("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("now"))) ? fl_str_val("fl_now()") : (fl_truthy(fl_eq(op, fl_str_val("now-ms"))) ? fl_str_val("fl_now_ms()") : (fl_truthy(fl_eq(op, fl_str_val("str-join"))) ? fl_str_n(5, fl_str_val("join("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-replace"))) ? fl_str_n(7, fl_str_val("str_replace("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("substring"))) ? fl_str_n(7, fl_str_val("substring("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("trim"))) ? fl_str_n(3, fl_str_val("trim("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("slice"))) ? fl_str_n(7, fl_str_val("substring("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("list"))) ? fl_user_cgc_list(args) : (fl_truthy(fl_eq(op, fl_str_val("_fl_map_set"))) ? fl_str_n(7, fl_str_val("fl_map_set("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_read"))) ? fl_str_n(3, fl_str_val("fl_file_read("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_write"))) ? fl_str_n(5, fl_str_val("fl_file_write("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-read"))) ? fl_str_n(3, fl_str_val("fl_file_read("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-write"))) ? fl_str_n(5, fl_str_val("fl_file_write("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-exists"))) ? fl_str_n(3, fl_str_val("file_exists("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file_exists"))) ? fl_str_n(3, fl_str_val("file_exists("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_append"))) ? fl_str_n(5, fl_str_val("_fl_file_append("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-append"))) ? fl_str_n(5, fl_str_val("_fl_file_append("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_copy"))) ? fl_str_n(5, fl_str_val("_fl_file_copy("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_delete"))) ? fl_str_n(3, fl_str_val("_fl_file_delete("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-delete"))) ? fl_str_n(3, fl_str_val("_fl_file_delete("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_mkdir"))) ? fl_str_n(3, fl_str_val("_fl_file_mkdir("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-mkdir"))) ? fl_str_n(3, fl_str_val("_fl_file_mkdir("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_rmdir"))) ? fl_str_n(3, fl_str_val("_fl_file_rmdir("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_list"))) ? fl_str_n(3, fl_str_val("_fl_file_list("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-list"))) ? fl_str_n(3, fl_str_val("_fl_file_list("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_size"))) ? fl_str_n(3, fl_str_val("_fl_file_size("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_modified"))) ? fl_str_n(3, fl_str_val("_fl_file_modified("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_rename"))) ? fl_str_n(5, fl_str_val("_fl_file_rename("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_is_file"))) ? fl_str_n(3, fl_str_val("_fl_file_is_file("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-is-file?"))) ? fl_str_n(3, fl_str_val("_fl_file_is_file("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_is_dir"))) ? fl_str_n(3, fl_str_val("_fl_file_is_dir("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-is-dir?"))) ? fl_str_n(3, fl_str_val("_fl_file_is_dir("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_env_get"))) ? fl_str_n(3, fl_str_val("_fl_env_get("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("env-get"))) ? fl_str_n(3, fl_str_val("_fl_env_get("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_env_set"))) ? fl_str_n(5, fl_str_val("_fl_env_set("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_env_all"))) ? fl_str_val("_fl_env_all()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_run"))) ? fl_str_n(3, fl_str_val("_fl_process_run("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("process-run"))) ? fl_str_n(3, fl_str_val("_fl_process_run("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_run_args"))) ? fl_str_n(5, fl_str_val("_fl_process_run_args("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_exec"))) ? fl_str_n(3, fl_str_val("_fl_process_exec("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_exec_args"))) ? fl_str_n(5, fl_str_val("_fl_process_exec_args("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_spawn"))) ? fl_str_n(5, fl_str_val("_fl_process_spawn("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_kill"))) ? fl_str_n(3, fl_str_val("_fl_process_kill("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_wait"))) ? fl_str_n(3, fl_str_val("_fl_process_wait("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_exists"))) ? fl_str_n(3, fl_str_val("_fl_process_exists("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_getcwd"))) ? fl_str_val("_fl_process_getcwd()") : (fl_truthy(fl_eq(op, fl_str_val("process-cwd"))) ? fl_str_val("_fl_process_getcwd()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_chdir"))) ? fl_str_n(3, fl_str_val("_fl_process_chdir("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_pid"))) ? fl_str_val("_fl_process_pid()") : (fl_truthy(fl_eq(op, fl_str_val("process-pid"))) ? fl_str_val("_fl_process_pid()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_ppid"))) ? fl_str_val("_fl_process_ppid()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_run_inherit"))) ? fl_str_n(3, fl_str_val("_fl_run_inherit("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("run-inherit"))) ? fl_str_n(3, fl_str_val("_fl_run_inherit("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("cli-args"))) ? fl_str_val("fl_get_argv()") : (fl_truthy(fl_eq(op, fl_str_val("bit-xor"))) ? fl_str_n(5, fl_str_val("fl_bit_xor("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-and"))) ? fl_str_n(5, fl_str_val("fl_bit_and("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-or"))) ? fl_str_n(5, fl_str_val("fl_bit_or("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-shl"))) ? fl_str_n(5, fl_str_val("fl_bit_shl("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-shr"))) ? fl_str_n(5, fl_str_val("fl_bit_shr("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("json-parse"))) ? fl_str_n(3, fl_str_val("fl_json_parse("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("json-stringify"))) ? fl_str_n(3, fl_str_val("fl_json_stringify("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("run-parallel"))) ? fl_str_n(5, fl_str_val("fl_run_parallel("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), (fl_truthy(fl_gte(length(args), fl_int(2))) ? fl_user_cgc(get(args, fl_int(1))) : fl_str_val("fl_int(0)")), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-get"))) ? fl_str_n(3, fl_str_val("fl_http_get("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-post"))) ? fl_str_n(7, fl_str_val("fl_http_post("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-get-headers"))) ? fl_str_n(5, fl_str_val("fl_http_get_headers("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-post-headers"))) ? fl_str_n(7, fl_str_val("fl_http_post_headers("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-post-stream"))) ? fl_str_n(9, fl_str_val("fl_http_post_stream("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(3))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-stream-collect"))) ? fl_str_n(7, fl_str_val("fl_http_stream_collect("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("sleep"))) ? fl_str_n(3, fl_str_val("fl_sleep_ms("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("now-ms"))) ? fl_str_val("fl_now_ms()") : (fl_truthy(fl_eq(op, fl_str_val("server-start"))) ? fl_str_n(3, fl_str_val("fl_http_start("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-stop"))) ? fl_str_val("fl_http_stop()") : (fl_truthy(fl_eq(op, fl_str_val("server-html"))) ? fl_str_n(3, fl_str_val("fl_resp_html("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-json"))) ? fl_str_n(3, fl_str_val("fl_resp_json("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-status"))) ? fl_str_n(5, fl_str_val("fl_resp_status("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-redirect"))) ? fl_str_n(3, fl_str_val("fl_resp_redirect("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-html-cookie"))) ? fl_str_n(5, fl_str_val("fl_resp_html_cookie("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-set-cookie"))) ? fl_str_n(7, fl_str_val("fl_resp_set_cookie("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-route"))) ? fl_str_n(7, fl_str_val("fl_http_route("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-get"))) ? fl_str_n(5, fl_str_val("fl_http_route(fl_str_val(\"GET\"), "), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-post"))) ? fl_str_n(5, fl_str_val("fl_http_route(fl_str_val(\"POST\"), "), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-open"))) ? fl_str_n(3, fl_str_val("fl_db_open("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-close"))) ? fl_str_n(3, fl_str_val("fl_db_close("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-query"))) ? fl_str_n(7, fl_str_val("fl_db_query("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-exec"))) ? fl_str_n(7, fl_str_val("fl_db_exec("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("fn"))) ? fl_user_cgc_fn(args) : (fl_truthy(fl_eq(op, fl_str_val("cg-and"))) ? fl_str_n(5, fl_str_val("fl_and("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("cg-or"))) ? fl_str_n(5, fl_str_val("fl_or("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("cg-match"))) ? fl_str_val("fl_nil()") : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-sign"))) ? fl_str_n(7, fl_str_val("fl_jwt_sign("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-verify"))) ? fl_str_n(5, fl_str_val("fl_jwt_verify("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-expired"))) ? fl_str_n(3, fl_str_val("fl_jwt_expired("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-decode"))) ? fl_str_n(3, fl_str_val("fl_jwt_verify("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", fl_str_val(\"\"))")) : (fl_truthy(fl_eq(op, fl_str_val("auth-hash-password"))) ? fl_str_n(3, fl_str_val("fl_hash_password("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-verify-password"))) ? fl_str_n(5, fl_str_val("fl_verify_password("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(", "), fl_user_cgc(get(args, fl_int(1))), fl_str_val(")")) : fl_user_cgc_dispatch_fallback(op, args))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))));
}

FLValue fl_user_cgc_swap_b(FLValue args) {
    return ((__extension__ ({
    FLValue atom_c = fl_user_cgc(get(args, fl_int(0)));
    FLValue fn_node = get(args, fl_int(1));
    FLValue extra = substring(args, fl_int(2), length(args));
    ((__extension__ ({
    FLValue deref = fl_str_n(3, fl_str_val("fl_atom_deref("), atom_c, fl_str_val(")"));
    FLValue op = get(fn_node, fl_str_val("value"));
    FLValue ec = fl_user_cgc_args(extra);
    ((__extension__ ({
    FLValue rhs = (fl_truthy(fl_eq(op, fl_str_val("+"))) ? fl_str_n(5, fl_str_val("fl_add("), deref, fl_str_val(", "), ec, fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("-"))) ? fl_str_n(5, fl_str_val("fl_sub("), deref, fl_str_val(", "), ec, fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("*"))) ? fl_str_n(5, fl_str_val("fl_mul("), deref, fl_str_val(", "), ec, fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("push"))) ? fl_str_n(5, fl_str_val("fl_vec_push("), deref, fl_str_val(", "), ec, fl_str_val(")")) : fl_str_n(5, fl_user_c_name(op), fl_str_val("("), deref, (fl_truthy(fl_eq(length(extra), fl_int(0))) ? fl_str_val("") : fl_str_n(2, fl_str_val(", "), ec)), fl_str_val(")"))))));
    fl_str_n(5, fl_str_val("fl_atom_reset("), atom_c, fl_str_val(", "), rhs, fl_str_val(")"));
})));
})));
})));
}

FLValue fl_user_cgc_dispatch_fallback(FLValue op, FLValue args) {
    return (fl_truthy(fl_eq(op, fl_str_val("first"))) ? fl_str_n(3, fl_str_val("fl_vec_first("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("rest"))) ? fl_str_n(3, fl_str_val("fl_vec_rest("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_str_starts_with(op, fl_str_val("$"))) ? fl_user_cgc_fncall(fl_user_c_name(substring(op, fl_int(1), length(op))), args) : ((__extension__ ({
    FLValue cn = fl_user_c_name(op);
    FLValue user_cn = fl_user_user_c_name(op);
    (fl_truthy(fl_user_cgc_user_defn_p(op)) ? fl_str_n(4, user_cn, fl_str_val("("), fl_user_cgc_args(args), fl_str_val(")")) : (fl_truthy(fl_user_includes_item(fl_atom_deref(fl_user_known_user_globals_atom), user_cn)) ? fl_user_cgc_fncall(user_cn, args) : (fl_truthy(fl_user_includes_item(fl_atom_deref(fl_user_known_fncall_targets_atom), cn)) ? fl_user_cgc_fncall(cn, args) : fl_str_n(4, cn, fl_str_val("("), fl_user_cgc_args(args), fl_str_val(")")))));
}))))));
}

FLValue fl_user_cgc_fn_argv_decls(FLValue items, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(items))) ? acc : ((__extension__ ({
    FLValue name = fl_user_c_name(fl_user_cgc_extract_name(get(items, i)));
    fl_user_cgc_fn_argv_decls(items, fl_add(i, fl_int(1)), fl_str_n(6, acc, fl_str_val("    FLValue "), name, fl_str_val(" __attribute__((unused)) = argv["), i, fl_str_val("];\n")));
}))));
}

FLValue fl_user_cgc_fn_param_names(FLValue items, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(items))) ? acc : fl_user_cgc_fn_param_names(items, fl_add(i, fl_int(1)), fl_vec_push(acc, fl_user_c_name(fl_user_cgc_extract_name(get(items, i))))));
}

FLValue fl_user_cgc_collect_vars(FLValue node, FLValue acc) {
    return (fl_truthy(null_p(node)) ? acc : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("variable"))) ? ((__extension__ ({
    FLValue n = fl_user_c_name(get(node, fl_str_val("name")));
    (fl_truthy(fl_user_includes_item(acc, n)) ? acc : fl_vec_push(acc, n));
}))) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("sexpr"))) ? fl_user_cgc_collect_vars_loop(get(node, fl_str_val("args")), fl_int(0), acc) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("block"))) ? fl_user_cgc_collect_vars_loop(fl_user_get_block_items(node), fl_int(0), acc) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("and"))) ? fl_user_cgc_collect_vars_loop(get(node, fl_str_val("args")), fl_int(0), acc) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("or"))) ? fl_user_cgc_collect_vars_loop(get(node, fl_str_val("args")), fl_int(0), acc) : acc))))));
}

FLValue fl_user_cgc_collect_vars_loop(FLValue _args, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_or(null_p(_args), fl_gte(i, length(_args)))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_user_cgc_collect_vars(get(_args, i), acc);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_fn_env_decls(FLValue _caps, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_caps))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(6, acc, fl_str_val("    FLValue "), get(_caps, i), fl_str_val(" = _self->env["), i, fl_str_val("];\n"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_env_arr(FLValue _caps, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_caps))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy(fl_eq(i, fl_int(0))) ? get(_caps, fl_int(0)) : fl_str_n(3, acc, fl_str_val(", "), get(_caps, i)));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_fn_caps_filter(FLValue all_vars, FLValue param_names, FLValue outer, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(all_vars))) ? acc : ((__extension__ ({
    FLValue v = get(all_vars, i);
    fl_user_cgc_fn_caps_filter(all_vars, param_names, outer, fl_add(i, fl_int(1)), (fl_truthy(fl_and(fl_not(fl_user_includes_item(param_names, v)), fl_user_includes_item(outer, v))) ? fl_vec_push(acc, v) : acc));
}))));
}

FLValue fl_user_cgc_fn(FLValue args) {
    return ((__extension__ ({
    FLValue id = fl_atom_deref(fl_user_lambda_id_atom);
    FLValue param_items = fl_user_get_block_items(get(args, fl_int(0)));
    FLValue param_names = fl_user_cgc_fn_param_names(param_items, fl_int(0), fl_vec_new());
    FLValue body_node = get(args, fl_int(1));
    FLValue all_vars = fl_user_cgc_collect_vars(body_node, fl_vec_new());
    FLValue outer = fl_atom_deref(fl_user_outer_params_atom);
    FLValue caps = fl_user_cgc_fn_caps_filter(all_vars, param_names, outer, fl_int(0), fl_vec_new());
    FLValue nenv = length(caps);
    FLValue env_decls = fl_user_cgc_fn_env_decls(caps, fl_int(0), fl_str_val(""));
    FLValue body_c = fl_user_cgc(body_node);
    FLValue decls = fl_user_cgc_fn_argv_decls(param_items, fl_int(0), fl_str_val(""));
    FLValue fn_name = fl_str_n(2, fl_str_val("__fl_anon_"), id);
    fl_atom_reset(fl_user_lambda_id_atom, fl_add(fl_atom_deref(fl_user_lambda_id_atom), fl_int(1)));
    fl_atom_reset(fl_user_lambda_defs_atom, fl_vec_push(fl_atom_deref(fl_user_lambda_defs_atom), fl_str_n(9, fl_str_val("static FLValue "), fn_name, fl_str_val("(FLClosure* _self, int _argc, FLValue* argv) {\n"), fl_str_val("    (void)_self; (void)_argc;\n"), env_decls, decls, fl_str_val("    return "), body_c, fl_str_val(";\n}"))));
    (fl_truthy(fl_eq(nenv, fl_int(0))) ? fl_str_n(3, fl_str_val("fl_fn_new("), fn_name, fl_str_val(", 0, NULL)")) : fl_str_n(14, fl_str_val("(__extension__ ({ FLValue __env_"), id, fl_str_val("["), nenv, fl_str_val("] = {"), fl_user_cgc_env_arr(caps, fl_int(0), fl_str_val("")), fl_str_val("};"), fl_str_val(" fl_fn_new("), fn_name, fl_str_val(", "), nenv, fl_str_val(", __env_"), id, fl_str_val("); }))")));
})));
}

FLValue fl_user_cgc_list(FLValue args) {
    return (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_vec_new()") : ((__extension__ ({
    FLValue cnt = length(args);
    FLValue vals = fl_user_cgc_args(args);
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_lst["), cnt, fl_str_val("] = {"), vals, fl_str_val("};"), fl_str_val(" fl_vec_from(__fl_lst, "), cnt, fl_str_val("); }))"));
}))));
}

FLValue fl_user_cgc_str(FLValue args) {
    return ((__extension__ ({
    FLValue n = length(args);
    (fl_truthy(fl_eq(n, fl_int(0))) ? fl_str_val("fl_str_val(\"\")") : (fl_truthy(fl_eq(n, fl_int(1))) ? fl_str_n(3, fl_str_val("fl_str_n(1, "), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")")) : fl_str_n(5, fl_str_val("fl_str_n("), n, fl_str_val(", "), fl_user_cgc_args(args), fl_str_val(")"))));
})));
}

FLValue fl_user_cgc_str_arg(FLValue args) {
    return (fl_truthy(fl_eq(length(args), fl_int(1))) ? fl_user_cgc(get(args, fl_int(0))) : fl_user_cgc_str(args));
}

FLValue fl_user_cgc_if(FLValue args) {
    return ((__extension__ ({
    FLValue cond = fl_user_cgc(get(args, fl_int(0)));
    FLValue then = fl_user_cgc(get(args, fl_int(1)));
    FLValue fl_else = (fl_truthy(fl_gte(length(args), fl_int(3))) ? fl_user_cgc(get(args, fl_int(2))) : fl_str_val("fl_nil()"));
    fl_str_n(7, fl_str_val("(fl_truthy("), cond, fl_str_val(") ? "), then, fl_str_val(" : "), fl_else, fl_str_val(")"));
})));
}

FLValue fl_user_cgc_cond(FLValue args) {
    return (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue first = get(args, fl_int(0));
    FLValue nested = fl_and(fl_eq(get(first, fl_str_val("kind")), fl_str_val("block")), fl_eq(get(first, fl_str_val("type")), fl_str_val("Array")));
    (fl_truthy(nested) ? fl_user_cgc_cond_nested(args, fl_sub(length(args), fl_int(1)), fl_str_val("fl_nil()")) : fl_str_val("fl_nil()"));
}))));
}

FLValue fl_user_cgc_cond_nested(FLValue args, FLValue i, FLValue acc) {
    return (fl_truthy(fl_lt(i, fl_int(0))) ? acc : ((__extension__ ({
    FLValue items = fl_user_get_block_items(get(args, i));
    FLValue test = get(items, fl_int(0));
    FLValue body = get(items, fl_int(1));
    FLValue is_else = fl_and(fl_eq(get(test, fl_str_val("kind")), fl_str_val("literal")), fl_or(fl_eq(get(test, fl_str_val("value")), fl_bool(true)), fl_eq(get(test, fl_str_val("value")), fl_str_val("true"))));
    fl_user_cgc_cond_nested(args, fl_sub(i, fl_int(1)), (fl_truthy(is_else) ? fl_user_cgc(body) : fl_str_n(7, fl_str_val("(fl_truthy("), fl_user_cgc(test), fl_str_val(") ? "), fl_user_cgc(body), fl_str_val(" : "), acc, fl_str_val(")"))));
}))));
}

FLValue fl_user_cgc_do(FLValue args) {
    return fl_str_n(3, fl_str_val("(__extension__ ({ "), fl_user_cgc_stmts(args, fl_int(0), fl_str_val("")), fl_str_val(" }))"));
}

FLValue fl_user_cgc_let(FLValue args) {
    return ((__extension__ ({
    FLValue items = fl_user_get_block_items(get(args, fl_int(0)));
    FLValue first = get(items, fl_int(0));
    FLValue nested = fl_and(fl_eq(get(first, fl_str_val("kind")), fl_str_val("block")), fl_eq(get(first, fl_str_val("type")), fl_str_val("Array")));
    FLValue decls = (fl_truthy(nested) ? fl_user_cgc_let_2d(items, fl_int(0), fl_str_val("")) : fl_user_cgc_let_1d(items, fl_int(0), fl_str_val("")));
    FLValue body = fl_user_cgc_body(substring(args, fl_int(1), length(args)), fl_int(0), fl_str_val(""));
    fl_str_n(5, fl_str_val("((__extension__ ({\n"), decls, fl_str_val("    "), body, fl_str_val(";\n})))"));
})));
}

FLValue fl_user_cgc_let_1d(FLValue it, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(it))) ? acc : ((__extension__ ({
    FLValue n = fl_user_cgc_extract_name(get(it, i));
    FLValue v = fl_user_cgc(get(it, fl_add(i, fl_int(1))));
    FLValue attr = (fl_truthy(fl_str_starts_with(n, fl_str_val("_"))) ? fl_str_val(" __attribute__((unused))") : fl_str_val(""));
    fl_atom_reset(fl_user_known_fncall_targets_atom, fl_vec_push(fl_atom_deref(fl_user_known_fncall_targets_atom), n));
    fl_atom_reset(fl_user_outer_params_atom, fl_vec_push(fl_atom_deref(fl_user_outer_params_atom), n));
    fl_user_cgc_let_1d(it, fl_add(i, fl_int(2)), fl_str_n(7, acc, fl_str_val("    FLValue "), n, attr, fl_str_val(" = "), v, fl_str_val(";\n")));
}))));
}

FLValue fl_user_cgc_let_2d(FLValue it, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(it))) ? acc : ((__extension__ ({
    FLValue p = fl_user_get_block_items(get(it, i));
    FLValue n = fl_user_cgc_extract_name(get(p, fl_int(0)));
    FLValue v = fl_user_cgc(get(p, fl_int(1)));
    FLValue attr = (fl_truthy(fl_str_starts_with(n, fl_str_val("_"))) ? fl_str_val(" __attribute__((unused))") : fl_str_val(""));
    fl_atom_reset(fl_user_known_fncall_targets_atom, fl_vec_push(fl_atom_deref(fl_user_known_fncall_targets_atom), n));
    fl_atom_reset(fl_user_outer_params_atom, fl_vec_push(fl_atom_deref(fl_user_outer_params_atom), n));
    fl_user_cgc_let_2d(it, fl_add(i, fl_int(1)), fl_str_n(7, acc, fl_str_val("    FLValue "), n, attr, fl_str_val(" = "), v, fl_str_val(";\n")));
}))));
}

FLValue fl_user_cgc_body(FLValue args, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(args))) ? acc : ((__extension__ ({
    FLValue last = fl_eq(i, fl_sub(length(args), fl_int(1)));
    FLValue c = fl_user_cgc(get(args, i));
    (fl_truthy(last) ? fl_str_n(2, acc, c) : fl_user_cgc_body(args, fl_add(i, fl_int(1)), fl_str_n(3, acc, c, fl_str_val(";\n    "))));
}))));
}

FLValue fl_user_cgc_defn_impl(FLValue args) {
    return ((__extension__ ({
    FLValue name = fl_user_user_c_name(fl_user_cgc_extract_name(get(args, fl_int(0))));
    FLValue param_items = fl_user_get_block_items(get(args, fl_int(1)));
    FLValue ps = fl_user_cgc_params(param_items);
    FLValue pnames = fl_user_cgc_fn_param_names(param_items, fl_int(0), fl_vec_new());
    fl_atom_reset(fl_user_outer_params_atom, pnames);
    ((__extension__ ({
    FLValue call_args = fl_user_cgc_wrapper_call_args(param_items, fl_int(0), fl_str_val(""));
    fl_atom_reset(fl_user_wrapper_defs_atom, fl_vec_push(fl_atom_deref(fl_user_wrapper_defs_atom), fl_str_n(9, fl_str_val("static FLValue __fl_wrap_"), name, fl_str_val("(FLClosure* _s, int _ac, FLValue* argv) {\n"), fl_str_val("    (void)_s; (void)_ac;\n"), fl_str_val("    return "), name, fl_str_val("("), call_args, fl_str_val(");\n}"))));
    ((__extension__ ({
    FLValue bodies = substring(args, fl_int(2), length(args));
    FLValue body = (fl_truthy(fl_eq(length(bodies), fl_int(1))) ? fl_user_cgc(get(bodies, fl_int(0))) : fl_user_cgc_do(bodies));
    fl_str_n(7, fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(") {\n    return "), body, fl_str_val(";\n}"));
})));
})));
})));
}

FLValue fl_user_cgc_defn(FLValue args) {
    return ((__extension__ ({
    FLValue depth = fl_atom_deref(fl_user_cgc_defn_depth_atom);
    FLValue saved_params = fl_atom_deref(fl_user_outer_params_atom);
    fl_heap_retain(saved_params);
    fl_atom_reset(fl_user_cgc_defn_depth_atom, fl_add(fl_atom_deref(fl_user_cgc_defn_depth_atom), fl_int(1)));
    ((__extension__ ({
    FLValue result = fl_user_cgc_defn_impl(args);
    fl_atom_reset(fl_user_cgc_defn_depth_atom, fl_sub(fl_atom_deref(fl_user_cgc_defn_depth_atom), fl_int(1)));
    fl_atom_reset(fl_user_outer_params_atom, saved_params);
    fl_heap_release(saved_params);
    (fl_truthy(fl_gt(depth, fl_int(0))) ? ((__extension__ ({
    FLValue name = fl_user_user_c_name(fl_user_cgc_extract_name(get(args, fl_int(0))));
    FLValue param_items = fl_user_get_block_items(get(args, fl_int(1)));
    FLValue ps = fl_user_cgc_params(param_items);
    fl_atom_reset(fl_user_cgc_hoisted_fns_atom, fl_vec_push(fl_atom_deref(fl_user_cgc_hoisted_fns_atom), result));
    fl_atom_reset(fl_user_known_defns_atom, fl_vec_push(fl_atom_deref(fl_user_known_defns_atom), name));
    fl_atom_reset(fl_user_cgc_hoisted_fwds_atom, fl_str_n(9, fl_atom_deref(fl_user_cgc_hoisted_fwds_atom), fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(");\n"), fl_str_val("static FLValue __fl_wrap_"), name, fl_str_val("(FLClosure*, int, FLValue*);\n")));
    fl_str_val("fl_nil()");
}))) : result);
})));
})));
}

FLValue fl_user_cgc_define(FLValue args) {
    return ((__extension__ ({
    FLValue name = fl_user_user_c_name(fl_user_cgc_extract_name(get(args, fl_int(0))));
    FLValue val = fl_user_cgc(get(args, fl_int(1)));
    fl_atom_reset(fl_user_known_user_globals_atom, fl_vec_push(fl_atom_deref(fl_user_known_user_globals_atom), name));
    fl_atom_reset(fl_user_global_decls_atom, fl_vec_push(fl_atom_deref(fl_user_global_decls_atom), fl_str_n(3, fl_str_val("static FLValue "), name, fl_str_val(";"))));
    fl_str_n(4, name, fl_str_val(" = "), val, fl_str_val(";"));
})));
}

FLValue fl_user_cgc_binop_chain(FLValue args, FLValue fn) {
    return (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_int(0)") : (fl_truthy(fl_eq(length(args), fl_int(1))) ? fl_user_cgc(get(args, fl_int(0))) : fl_user_cgc_binop_fold(args, fn, fl_int(1), fl_user_cgc(get(args, fl_int(0))))));
}

FLValue fl_user_cgc_binop_fold(FLValue args, FLValue fn, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(args))) ? acc : fl_user_cgc_binop_fold(args, fn, fl_add(i, fl_int(1)), fl_str_n(6, fn, fl_str_val("("), acc, fl_str_val(", "), fl_user_cgc(get(args, i)), fl_str_val(")"))));
}

FLValue fl_user_cgc_and(FLValue args) {
    return (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_bool(true)") : (fl_truthy(fl_eq(length(args), fl_int(1))) ? fl_user_cgc(get(args, fl_int(0))) : fl_user_cgc_and_fold(args, fl_int(1), fl_user_cgc(get(args, fl_int(0))))));
}

FLValue fl_user_cgc_and_fold(FLValue args, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(args))) ? acc : fl_user_cgc_and_fold(args, fl_add(i, fl_int(1)), fl_str_n(5, fl_str_val("fl_and("), acc, fl_str_val(", "), fl_user_cgc(get(args, i)), fl_str_val(")"))));
}

FLValue fl_user_cgc_or(FLValue args) {
    return (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_bool(false)") : (fl_truthy(fl_eq(length(args), fl_int(1))) ? fl_user_cgc(get(args, fl_int(0))) : fl_user_cgc_or_fold(args, fl_int(1), fl_user_cgc(get(args, fl_int(0))))));
}

FLValue fl_user_cgc_or_fold(FLValue args, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(args))) ? acc : fl_user_cgc_or_fold(args, fl_add(i, fl_int(1)), fl_str_n(5, fl_str_val("fl_or("), acc, fl_str_val(", "), fl_user_cgc(get(args, i)), fl_str_val(")"))));
}

FLValue fl_user_cgc_args(FLValue args) {
    return fl_user_cgc_args_loop(args, fl_int(0), fl_str_val(""));
}

FLValue fl_user_cgc_args_loop(FLValue _args, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_or(null_p(_args), fl_gte(i, length(_args)))) ? acc : ((__extension__ ({
    FLValue c = fl_user_cgc(get(_args, i));
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy(fl_eq(i, fl_int(0))) ? c : fl_str_n(3, acc, fl_str_val(", "), c));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_stmts(FLValue args, FLValue i, FLValue acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(args))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(3, acc, fl_user_cgc(get(args, i)), fl_str_val("; "));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_forward_decls(FLValue nodes) {
    return fl_user_cgc_forward_loop(nodes, fl_int(0), fl_str_val(""));
}

FLValue fl_user_cgc_forward_loop(FLValue _nodes, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_nodes))) ? acc : ((__extension__ ({
    FLValue n = get(_nodes, i);
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy(fl_and(fl_eq(get(n, fl_str_val("kind")), fl_str_val("sexpr")), fl_eq(get(n, fl_str_val("op")), fl_str_val("defn")))) ? ((__extension__ ({
    FLValue name = fl_user_user_c_name(fl_user_cgc_extract_name(get(get(n, fl_str_val("args")), fl_int(0))));
    FLValue ps = fl_user_cgc_params(fl_user_get_block_items(get(get(n, fl_str_val("args")), fl_int(1))));
    fl_atom_reset(fl_user_known_defns_atom, fl_vec_push(fl_atom_deref(fl_user_known_defns_atom), name));
    fl_str_n(9, acc, fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(");\n"), fl_str_val("static FLValue __fl_wrap_"), name, fl_str_val("(FLClosure*, int, FLValue*);\n"));
}))) : acc);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_wrapper_call_args(FLValue _items, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_items))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(5, acc, (fl_truthy(fl_eq(length(acc), fl_int(0))) ? fl_str_val("") : fl_str_val(", ")), fl_str_val("argv["), i, fl_str_val("]"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_top_level(FLValue _nodes, FLValue _i, FLValue _stmts, FLValue _fns) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _stmts;
    FLValue stmts = __fl_loop_tmp_2;
    FLValue __fl_loop_tmp_4 = _fns;
    FLValue fns = __fl_loop_tmp_4;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_nodes))) ? (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("stmts"), stmts, fl_str_val("fns"), fns}; fl_map_from_pairs(__fl_kv, 2); })) : ((__extension__ ({
    FLValue n = get(_nodes, i);
    FLValue k = get(n, fl_str_val("kind"));
    FLValue is_defn = fl_and(fl_eq(k, fl_str_val("sexpr")), fl_eq(get(n, fl_str_val("op")), fl_str_val("defn")));
    FLValue is_block_func = fl_and(fl_eq(k, fl_str_val("block")), fl_eq(get(n, fl_str_val("type")), fl_str_val("FUNC")));
    (fl_truthy(fl_or(is_defn, is_block_func)) ? (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = stmts;
    FLValue _fl_t2 = fl_str_n(3, fns, fl_user_cgc(n), fl_str_val("\n\n"));
    i = _fl_t0;
    stmts = _fl_t1;
    fns = _fl_t2;
    _fl_looping = 1; fl_nil();
})) : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(4, stmts, fl_str_val("    "), fl_user_cgc(n), fl_str_val(";\n"));
    FLValue _fl_t2 = fns;
    i = _fl_t0;
    stmts = _fl_t1;
    fns = _fl_t2;
    _fl_looping = 1; fl_nil();
})));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_lambda_fwd_loop(FLValue _defs, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_defs))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(4, acc, fl_str_val("static FLValue __fl_anon_"), i, fl_str_val("(FLClosure*, int, FLValue*);\n"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_lambda_fwds() {
    return fl_user_cgc_lambda_fwd_loop(fl_atom_deref(fl_user_lambda_defs_atom), fl_int(0), fl_str_val(""));
}

FLValue fl_user_cgc_join_lambda_loop(FLValue _defs, FLValue _i, FLValue _acc) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_defs))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(3, acc, get(_defs, i), fl_str_val("\n\n"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_join_lambdas() {
    return fl_user_cgc_join_lambda_loop(fl_atom_deref(fl_user_lambda_defs_atom), fl_int(0), fl_str_val(""));
}

FLValue fl_user_cgc_join_wrappers() {
    return fl_user_cgc_join_lambda_loop(fl_atom_deref(fl_user_wrapper_defs_atom), fl_int(0), fl_str_val(""));
}

FLValue fl_user_cgc_join_globals() {
    return fl_user_cgc_join_lambda_loop(fl_atom_deref(fl_user_global_decls_atom), fl_int(0), fl_str_val(""));
}

FLValue fl_user_cgc_join_hoisted() {
    return fl_user_cgc_join_lambda_loop(fl_atom_deref(fl_user_cgc_hoisted_fns_atom), fl_int(0), fl_str_val(""));
}

FLValue fl_user_generate_c(FLValue nodes) {
    return ((__extension__ ({
    FLValue fwd = fl_user_cgc_forward_decls(nodes);
    FLValue parts = fl_user_cgc_top_level(nodes, fl_int(0), fl_str_val(""), fl_str_val(""));
    FLValue fns = get(parts, fl_str_val("fns"));
    FLValue stmts = get(parts, fl_str_val("stmts"));
    FLValue gdecls = fl_user_cgc_join_globals();
    FLValue lfwd = fl_user_cgc_lambda_fwds();
    FLValue ldefs = fl_user_cgc_join_lambdas();
    FLValue wdefs = fl_user_cgc_join_wrappers();
    FLValue hfwd = fl_atom_deref(fl_user_cgc_hoisted_fwds_atom);
    FLValue hfns = fl_user_cgc_join_hoisted();
    fl_str_n(19, fl_str_val("#include \"runtime.h\"\n"), fl_str_val("#pragma GCC diagnostic ignored \"-Wunused-function\"\n"), fl_str_val("#pragma GCC diagnostic ignored \"-Wunused-parameter\"\n\n"), fwd, hfwd, gdecls, fl_str_val("\n"), lfwd, fl_str_val("\n"), ldefs, wdefs, fl_str_val("\n"), hfns, fns, fl_str_val("int main(int argc, char** argv) {\n"), fl_str_val("    fl_init_argv(argc, argv);\n"), stmts, fl_str_val("    return 0;\n"), fl_str_val("}\n"));
})));
}

FLValue fl_user_cgc_set_b(FLValue args) {
    return fl_str_n(5, fl_str_val("(__extension__ ({ "), fl_user_cgc(get(args, fl_int(0))), fl_str_val(" = "), fl_user_cgc(get(args, fl_int(1))), fl_str_val("; fl_nil(); }))"));
}

FLValue fl_user_cgc_while(FLValue args) {
    return fl_str_n(5, fl_str_val("(__extension__ ({ while (fl_truthy("), fl_user_cgc(get(args, fl_int(0))), fl_str_val(")) { "), fl_user_cgc(get(args, fl_int(1))), fl_str_val("; } fl_nil(); }))"));
}

FLValue fl_user_loop_extract_vars(FLValue items, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(items))) ? acc : fl_user_loop_extract_vars(items, fl_add(i, fl_int(2)), fl_vec_push(acc, fl_user_cgc_extract_name(get(items, i)))));
}

FLValue fl_user_loop_make_decls(FLValue items, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(items))) ? acc : ((__extension__ ({
    FLValue name = fl_user_cgc_extract_name(get(items, i));
    FLValue val = fl_user_cgc(get(items, fl_add(i, fl_int(1))));
    FLValue tmp = fl_str_n(2, fl_str_val("__fl_loop_tmp_"), i);
    fl_user_loop_make_decls(items, fl_add(i, fl_int(2)), fl_str_n(11, acc, fl_str_val("    FLValue "), tmp, fl_str_val(" = "), val, fl_str_val(";\n"), fl_str_val("    FLValue "), name, fl_str_val(" = "), tmp, fl_str_val(";\n")));
}))));
}

FLValue fl_user_cgc_loop(FLValue args) {
    return ((__extension__ ({
    FLValue items = fl_user_get_block_items(get(args, fl_int(0)));
    FLValue body = get(args, fl_int(1));
    FLValue vars = fl_user_loop_extract_vars(items, fl_int(0), fl_vec_new());
    FLValue decls = fl_user_loop_make_decls(items, fl_int(0), fl_str_val(""));
    FLValue body_c = fl_user_cgc_with_recur(body, vars);
    fl_str_n(8, fl_str_val("(__extension__ ({\n"), decls, fl_str_val("    int _fl_looping = 1; FLValue _fl_result = fl_nil();\n"), fl_str_val("    while (_fl_looping) { _fl_looping = 0;\n"), fl_str_val("    _fl_result = "), body_c, fl_str_val(";\n    }\n"), fl_str_val("    _fl_result;\n}))"));
})));
}

FLValue fl_user_cgc_recur_temps(FLValue args, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(args))) ? acc : fl_user_cgc_recur_temps(args, fl_add(i, fl_int(1)), fl_str_n(6, acc, fl_str_val("    FLValue _fl_t"), i, fl_str_val(" = "), fl_user_cgc(get(args, i)), fl_str_val(";\n"))));
}

FLValue fl_user_cgc_recur_assigns(FLValue vars, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(vars))) ? acc : fl_user_cgc_recur_assigns(vars, fl_add(i, fl_int(1)), fl_str_n(6, acc, fl_str_val("    "), get(vars, i), fl_str_val(" = _fl_t"), i, fl_str_val(";\n"))));
}

FLValue fl_user_cgc_recur_stmt(FLValue args, FLValue vars) {
    return ((__extension__ ({
    FLValue temps = fl_user_cgc_recur_temps(args, fl_int(0), fl_str_val(""));
    FLValue assigns = fl_user_cgc_recur_assigns(vars, fl_int(0), fl_str_val(""));
    fl_str_n(4, fl_str_val("(__extension__ ({\n"), temps, assigns, fl_str_val("    _fl_looping = 1; fl_nil();\n}))"));
})));
}

FLValue fl_user_cgc_with_recur(FLValue node, FLValue vars) {
    return (fl_truthy(null_p(node)) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue k = get(node, fl_str_val("kind"));
    (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? ((__extension__ ({
    FLValue op = get(node, fl_str_val("op"));
    FLValue args = get(node, fl_str_val("args"));
    (fl_truthy(fl_eq(op, fl_str_val("recur"))) ? fl_user_cgc_recur_stmt(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("if"))) ? fl_user_cgc_if_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("cond"))) ? fl_user_cgc_cond_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("do"))) ? fl_user_cgc_do_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("let"))) ? fl_user_cgc_let_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("loop"))) ? fl_user_cgc_loop(args) : fl_user_cgc(node)))))));
}))) : fl_user_cgc(node));
}))));
}

FLValue fl_user_cgc_if_wr(FLValue args, FLValue vars) {
    return ((__extension__ ({
    FLValue cond = fl_user_cgc(get(args, fl_int(0)));
    FLValue then = fl_user_cgc_with_recur(get(args, fl_int(1)), vars);
    FLValue fl_else = (fl_truthy(fl_gte(length(args), fl_int(3))) ? fl_user_cgc_with_recur(get(args, fl_int(2)), vars) : fl_str_val("fl_nil()"));
    fl_str_n(7, fl_str_val("(fl_truthy("), cond, fl_str_val(") ? "), then, fl_str_val(" : "), fl_else, fl_str_val(")"));
})));
}

FLValue fl_user_cgc_cond_wr(FLValue args, FLValue vars) {
    return (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue first = get(args, fl_int(0));
    FLValue nested = fl_and(fl_eq(get(first, fl_str_val("kind")), fl_str_val("block")), fl_eq(get(first, fl_str_val("type")), fl_str_val("Array")));
    (fl_truthy(nested) ? fl_user_cgc_cond_nested_wr(args, vars, fl_sub(length(args), fl_int(1)), fl_str_val("fl_nil()")) : fl_str_val("fl_nil()"));
}))));
}

FLValue fl_user_cgc_cond_nested_wr(FLValue args, FLValue vars, FLValue i, FLValue acc) {
    return (fl_truthy(fl_lt(i, fl_int(0))) ? acc : ((__extension__ ({
    FLValue items = fl_user_get_block_items(get(args, i));
    FLValue test = get(items, fl_int(0));
    FLValue body = get(items, fl_int(1));
    FLValue is_else = fl_and(fl_eq(get(test, fl_str_val("kind")), fl_str_val("literal")), fl_or(fl_eq(get(test, fl_str_val("value")), fl_bool(true)), fl_eq(get(test, fl_str_val("value")), fl_str_val("true"))));
    fl_user_cgc_cond_nested_wr(args, vars, fl_sub(i, fl_int(1)), (fl_truthy(is_else) ? fl_user_cgc_with_recur(body, vars) : fl_str_n(7, fl_str_val("(fl_truthy("), fl_user_cgc(test), fl_str_val(") ? "), fl_user_cgc_with_recur(body, vars), fl_str_val(" : "), acc, fl_str_val(")"))));
}))));
}

FLValue fl_user_cgc_do_wr(FLValue args, FLValue vars) {
    return fl_str_n(3, fl_str_val("(__extension__ ({ "), fl_user_cgc_stmts_wr(args, vars, fl_int(0), fl_str_val("")), fl_str_val(" }))"));
}

FLValue fl_user_cgc_stmts_wr(FLValue args, FLValue vars, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(args))) ? acc : fl_user_cgc_stmts_wr(args, vars, fl_add(i, fl_int(1)), fl_str_n(3, acc, fl_user_cgc_with_recur(get(args, i), vars), fl_str_val("; "))));
}

FLValue fl_user_cgc_let_wr(FLValue args, FLValue vars) {
    return ((__extension__ ({
    FLValue items = fl_user_get_block_items(get(args, fl_int(0)));
    FLValue first = get(items, fl_int(0));
    FLValue nested = fl_and(fl_eq(get(first, fl_str_val("kind")), fl_str_val("block")), fl_eq(get(first, fl_str_val("type")), fl_str_val("Array")));
    FLValue decls = (fl_truthy(nested) ? fl_user_cgc_let_2d(items, fl_int(0), fl_str_val("")) : fl_user_cgc_let_1d(items, fl_int(0), fl_str_val("")));
    FLValue body_c = fl_user_cgc_body_wr(substring(args, fl_int(1), length(args)), vars, fl_int(0), fl_str_val(""));
    fl_str_n(5, fl_str_val("((__extension__ ({\n"), decls, fl_str_val("    "), body_c, fl_str_val(";\n})))"));
})));
}

FLValue fl_user_cgc_body_wr(FLValue args, FLValue vars, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(args))) ? acc : ((__extension__ ({
    FLValue last = fl_eq(i, fl_sub(length(args), fl_int(1)));
    FLValue c = fl_user_cgc_with_recur(get(args, i), vars);
    (fl_truthy(last) ? c : fl_user_cgc_body_wr(args, vars, fl_add(i, fl_int(1)), fl_str_n(3, acc, c, fl_str_val(";\n    "))));
}))));
}

FLValue fl_user_cgc_array_block(FLValue n) {
    return ((__extension__ ({
    FLValue items = get(get(n, fl_str_val("fields")), fl_str_val("items"));
    (fl_truthy(fl_or(null_p(items), fl_eq(length(items), fl_int(0)))) ? fl_str_val("fl_vec_new()") : ((__extension__ ({
    FLValue cnt = length(items);
    FLValue vals = fl_user_cgc_args(items);
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_arr["), cnt, fl_str_val("] = {"), vals, fl_str_val("};"), fl_str_val(" fl_vec_from(__fl_arr, "), cnt, fl_str_val("); }))"));
}))));
})));
}

FLValue fl_user_cgc_map_entry_c(FLValue ent) {
    return fl_str_n(4, fl_str_val("fl_str_val(\""), fl_user_c_esc(get(ent, fl_int(0))), fl_str_val("\"), "), fl_user_cgc(get(ent, fl_int(1))));
}

FLValue fl_user_cgc_map_entries_c(FLValue ents, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(ents))) ? acc : ((__extension__ ({
    FLValue sep = (fl_truthy(fl_eq(i, fl_int(0))) ? fl_str_val("") : fl_str_val(", "));
    FLValue ec = fl_user_cgc_map_entry_c(get(ents, i));
    fl_user_cgc_map_entries_c(ents, fl_add(i, fl_int(1)), fl_str_n(3, acc, sep, ec));
}))));
}

FLValue fl_user_cgc_map_key_c(FLValue key_node) {
    return (fl_truthy(fl_eq(get(key_node, fl_str_val("kind")), fl_str_val("keyword"))) ? fl_str_n(3, fl_str_val("fl_str_val(\""), fl_user_c_esc(get(key_node, fl_str_val("name"))), fl_str_val("\")")) : fl_user_cgc(key_node));
}

FLValue fl_user_cgc_map_items_c(FLValue items, FLValue i, FLValue acc) {
    return (fl_truthy(fl_gte(i, length(items))) ? acc : ((__extension__ ({
    FLValue sep = (fl_truthy(fl_eq(i, fl_int(0))) ? fl_str_val("") : fl_str_val(", "));
    FLValue key_c = fl_user_cgc_map_key_c(get(items, i));
    FLValue val_c = fl_user_cgc(get(items, fl_add(i, fl_int(1))));
    fl_user_cgc_map_items_c(items, fl_add(i, fl_int(2)), fl_str_n(5, acc, sep, key_c, fl_str_val(", "), val_c));
}))));
}

FLValue fl_user_cgc_map_from_items(FLValue items) {
    return (fl_truthy(fl_or(null_p(items), fl_eq(length(items), fl_int(0)))) ? fl_str_val("fl_map_new()") : ((__extension__ ({
    FLValue n_pairs = fl_div(length(items), fl_int(2));
    FLValue n_items = length(items);
    FLValue kvs = fl_user_cgc_map_items_c(items, fl_int(0), fl_str_val(""));
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_kv["), n_items, fl_str_val("] = {"), kvs, fl_str_val("};"), fl_str_val(" fl_map_from_pairs(__fl_kv, "), n_pairs, fl_str_val("); }))"));
}))));
}

FLValue fl_user_cgc_map_block(FLValue n) {
    return ((__extension__ ({
    FLValue fields = get(n, fl_str_val("fields"));
    FLValue items = get(fields, fl_str_val("items"));
    (fl_truthy(fl_and(fl_not(null_p(items)), fl_eq(type_of(items), fl_str_val("array")))) ? fl_user_cgc_map_from_items(items) : ((__extension__ ({
    FLValue ents = fl_map_entries(fields);
    (fl_truthy(fl_or(null_p(ents), fl_eq(length(ents), fl_int(0)))) ? fl_str_val("fl_map_new()") : ((__extension__ ({
    FLValue n_pairs = length(ents);
    FLValue n_items = fl_mul(n_pairs, fl_int(2));
    FLValue kvs = fl_user_cgc_map_entries_c(ents, fl_int(0), fl_str_val(""));
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_kv["), n_items, fl_str_val("] = {"), kvs, fl_str_val("};"), fl_str_val(" fl_map_from_pairs(__fl_kv, "), n_pairs, fl_str_val("); }))"));
}))));
}))));
})));
}

FLValue fl_user_ir_err(FLValue msg, FLValue n) {
    return (__extension__ ({ fl_println(fl_str_n(6, fl_str_val("[IR-ERR] "), msg, fl_str_val(" | kind="), get(n, fl_str_val("kind")), fl_str_val(" line="), get(n, fl_str_val("line")))); fl_bool(false);  }));
}

FLValue fl_user_ir_chk(FLValue n) {
    return (fl_truthy(null_p(n)) ? fl_bool(true) : ((__extension__ ({
    FLValue k = get(n, fl_str_val("kind"));
    (fl_truthy(null_p(k)) ? fl_user_ir_err(fl_str_val("missing :kind"), n) : (fl_truthy(fl_not(fl_user_includes_item(fl_user_ir_kind_set, k))) ? fl_println(fl_str_n(4, fl_str_val("[IR-WARN] unknown kind="), k, fl_str_val(" line="), get(n, fl_str_val("line")))) : (fl_truthy(fl_eq(k, fl_str_val("literal"))) ? ((__extension__ ({
    FLValue t = get(n, fl_str_val("type"));
    FLValue v = get(n, fl_str_val("value"));
    (fl_truthy(null_p(t)) ? fl_user_ir_err(fl_str_val("literal missing :type"), n) : (fl_truthy(fl_not(fl_user_includes_item(fl_user_ir_lit_types, t))) ? fl_user_ir_err(fl_str_n(2, fl_str_val("literal bad type="), t), n) : (fl_truthy(fl_and(fl_eq(t, fl_str_val("boolean")), fl_not(fl_eq(type_of(v), fl_str_val("boolean"))))) ? fl_user_ir_err(fl_str_n(2, fl_str_val("boolean value must be FL boolean, got "), type_of(v)), n) : (fl_truthy(fl_and(fl_eq(t, fl_str_val("nil")), fl_not(null_p(v)))) ? fl_user_ir_err(fl_str_val("nil literal value must be nil"), n) : fl_bool(true)))));
}))) : (fl_truthy(fl_eq(k, fl_str_val("variable"))) ? (fl_truthy(null_p(get(n, fl_str_val("name")))) ? fl_user_ir_err(fl_str_val("variable missing :name"), n) : fl_bool(true)) : (fl_truthy(fl_eq(k, fl_str_val("keyword"))) ? (fl_truthy(null_p(get(n, fl_str_val("name")))) ? fl_user_ir_err(fl_str_val("keyword missing :name"), n) : fl_bool(true)) : (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? (fl_truthy(null_p(get(n, fl_str_val("op")))) ? fl_user_ir_err(fl_str_val("sexpr missing :op"), n) : (fl_truthy(fl_not(fl_eq(type_of(get(n, fl_str_val("op"))), fl_str_val("string")))) ? fl_user_ir_err(fl_str_val("sexpr :op must be string"), n) : (fl_truthy(fl_or(fl_eq(get(n, fl_str_val("op")), fl_str_val("and")), fl_eq(get(n, fl_str_val("op")), fl_str_val("or")))) ? fl_user_ir_err(fl_str_n(2, fl_str_val("and/or must be canonical kind node, not sexpr op="), get(n, fl_str_val("op"))), n) : (fl_truthy(null_p(get(n, fl_str_val("args")))) ? fl_user_ir_err(fl_str_val("sexpr missing :args"), n) : (fl_truthy(fl_not(fl_eq(type_of(get(n, fl_str_val("args"))), fl_str_val("array")))) ? fl_user_ir_err(fl_str_val("sexpr :args must be array"), n) : fl_bool(true)))))) : (fl_truthy(fl_eq(k, fl_str_val("block"))) ? (fl_truthy(null_p(get(n, fl_str_val("type")))) ? fl_user_ir_err(fl_str_val("block missing :type"), n) : fl_bool(true)) : (fl_truthy(fl_or(fl_eq(k, fl_str_val("and")), fl_eq(k, fl_str_val("or")))) ? (fl_truthy(fl_or(null_p(get(n, fl_str_val("args"))), fl_not(fl_eq(type_of(get(n, fl_str_val("args"))), fl_str_val("array"))))) ? fl_user_ir_err(fl_str_n(2, k, fl_str_val(" missing/invalid :args")), n) : fl_bool(true)) : fl_bool(true)))))))));
}))));
}

FLValue fl_user_includes_item(FLValue arr, FLValue val) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(arr))) ? fl_bool(false) : (fl_truthy(fl_eq(get(arr, i), val)) ? fl_bool(true) : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    i = _fl_t0;
    _fl_looping = 1; fl_nil();
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_ir_validate(FLValue nodes) {
    return (fl_truthy(fl_or(null_p(nodes), fl_eq(length(nodes), fl_int(0)))) ? fl_bool(true) : (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(nodes))) ? fl_bool(true) : (__extension__ ({ fl_user_ir_chk(get(nodes, i)); (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    i = _fl_t0;
    _fl_looping = 1; fl_nil();
}));  })));
    }
    _fl_result;
})));
}

FLValue fl_user_path_dir(FLValue path) {
    return ((__extension__ ({
    FLValue len = length(path);
    FLValue last = (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_sub(len, fl_int(1));
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = fl_int(-1);
    FLValue r = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_lt(i, fl_int(0))) ? r : (fl_truthy(fl_eq(char_at(path, i), fl_str_val("/"))) ? i : (__extension__ ({
    FLValue _fl_t0 = fl_sub(i, fl_int(1));
    FLValue _fl_t1 = r;
    i = _fl_t0;
    r = _fl_t1;
    _fl_looping = 1; fl_nil();
}))));
    }
    _fl_result;
}));
    (fl_truthy(fl_lt(last, fl_int(0))) ? fl_str_val("") : substring(path, fl_int(0), fl_add(last, fl_int(1))));
})));
}

FLValue fl_user_append_all(FLValue acc, FLValue items) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = acc;
    FLValue a = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(items))) ? a : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_vec_push(a, get(items, i));
    i = _fl_t0;
    a = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
}));
}

FLValue fl_user_expand_loads(FLValue nodes, FLValue base_dir) {
    return (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = fl_vec_new();
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(nodes))) ? acc : ((__extension__ ({
    FLValue node = get(nodes, i);
    (fl_truthy(fl_and(fl_eq(get(node, fl_str_val("kind")), fl_str_val("sexpr")), fl_eq(get(node, fl_str_val("op")), fl_str_val("load")))) ? ((__extension__ ({
    FLValue path_node = get(get(node, fl_str_val("args")), fl_int(0));
    FLValue rel_path = get(path_node, fl_str_val("value"));
    FLValue try1 = (fl_truthy(fl_str_starts_with(rel_path, fl_str_val("/"))) ? rel_path : fl_str_n(2, base_dir, rel_path));
    FLValue src1 = fl_file_read(try1);
    FLValue abs_path = (fl_truthy(null_p(src1)) ? rel_path : try1);
    FLValue src = (fl_truthy(null_p(src1)) ? fl_file_read(rel_path) : src1);
    (fl_truthy(null_p(src)) ? (__extension__ ({ fl_println(fl_str_n(2, fl_str_val("[load] 파일 없음: "), rel_path)); (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = acc;
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));  })) : ((__extension__ ({
    FLValue sub_dir = fl_user_path_dir(abs_path);
    FLValue sub_raw = fl_user_parse(fl_user_lex(src));
    FLValue expanded = fl_user_expand_loads(sub_raw, sub_dir);
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_user_append_all(acc, expanded);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))));
}))) : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_vec_push(acc, node);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
}))));
    }
    _fl_result;
}));
}

FLValue fl_user_cgc_run(FLValue argv) {
    return ((__extension__ ({
    FLValue input = get(argv, fl_int(0));
    FLValue output = get(argv, fl_int(1));
    (fl_truthy(fl_or(null_p(input), null_p(output))) ? fl_println(fl_str_val("usage: cgc-bin <input.fl> <output.c>")) : ((__extension__ ({
    FLValue src = fl_file_read(input);
    FLValue base_dir = fl_user_path_dir(input);
    FLValue raw = fl_user_parse(fl_user_lex(src));
    FLValue nodes = fl_user_expand_loads(raw, base_dir);
    FLValue _v __attribute__((unused)) = fl_user_ir_validate(nodes);
    FLValue c_code = fl_user_generate_c(nodes);
    fl_file_write(output, c_code);
    fl_println(fl_str_n(4, fl_str_val("Compiled "), input, fl_str_val(" -> "), output));
}))));
})));
}

int main(int argc, char** argv) {
    fl_init_argv(argc, argv);
    fl_user_lambda_id_atom = fl_atom_new(fl_int(0));;
    fl_user_lambda_defs_atom = fl_atom_new(fl_vec_new());;
    fl_user_outer_params_atom = fl_atom_new(fl_vec_new());;
    fl_user_known_fncall_targets_atom = fl_atom_new(fl_vec_new());;
    fl_user_known_defns_atom = fl_atom_new(fl_vec_new());;
    fl_user_wrapper_defs_atom = fl_atom_new(fl_vec_new());;
    fl_user_global_decls_atom = fl_atom_new(fl_vec_new());;
    fl_user_known_user_globals_atom = fl_atom_new(fl_vec_new());;
    fl_user_cgc_defn_depth_atom = fl_atom_new(fl_int(0));;
    fl_user_cgc_hoisted_fns_atom = fl_atom_new(fl_vec_new());;
    fl_user_cgc_hoisted_fwds_atom = fl_atom_new(fl_str_val(""));;
    fl_user_ir_kind_set = (__extension__ ({ FLValue __fl_arr[7] = {fl_str_val("literal"), fl_str_val("variable"), fl_str_val("keyword"), fl_str_val("sexpr"), fl_str_val("block"), fl_str_val("and"), fl_str_val("or")}; fl_vec_from(__fl_arr, 7); }));;
    fl_user_ir_lit_types = (__extension__ ({ FLValue __fl_arr[5] = {fl_str_val("number"), fl_str_val("string"), fl_str_val("boolean"), fl_str_val("nil"), fl_str_val("symbol")}; fl_vec_from(__fl_arr, 5); }));;
    fl_user_cgc_run(fl_get_argv());
    return 0;
}
