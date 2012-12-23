#if !defined __INPUTSYM_H__
#define __INPUTSYM_H__

// The mappings here should, for the most part, match SDL mappings

enum InputKeySym
{
    InputKeySym_unknown = 0,

    // ASCII mapped keys
    InputKeySym_backspace = '\b',
    InputKeySym_tab = '\t',
    InputKeySym_return = '\r',
    InputKeySym_pause = 19,
    InputKeySym_escape = 27,
    InputKeySym_space = ' ',
    InputKeySym_dbl_quote = '\"',
    InputKeySym_hash = '#',
    InputKeySym_dollar = '$',
    InputKeySym_ampersand = '&',
    InputKeySym_quote = '\'',
    InputKeySym_left_paren = '(',
    InputKeySym_right_paren = ')',
    InputKeySym_asterisk = '*',
    InputKeySym_plus = '+',
    InputKeySym_comma = ',',
    InputKeySym_minus = '-',
    InputKeySym_period = '.',
    InputKeySym_slash = '/',
    InputKeySym_0 = '0',
    InputKeySym_1,
    InputKeySym_2,
    InputKeySym_3,
    InputKeySym_4,
    InputKeySym_5,
    InputKeySym_6,
    InputKeySym_7,
    InputKeySym_8,
    InputKeySym_9,
    InputKeySym_colon = ':',
    InputKeySym_semicolon = ';',
    InputKeySym_less = '<',
    InputKeySym_equals = '=',
    InputKeySym_greater = '>',
    InputKeySym_question = '?',
    InputKeySym_at = '@',

    InputKeySym_left_bracket = '[',
    InputKeySym_backslash = '\\',
    InputKeySym_right_bracket = ']',
    InputKeySym_caret = '^',
    InputKeySym_underscore = '_',
    InputKeySym_backquote = '`',

    InputKeySym_a = 'a',
    InputKeySym_b,
    InputKeySym_c,
    InputKeySym_d,
    InputKeySym_e,
    InputKeySym_f,
    InputKeySym_g,
    InputKeySym_h,
    InputKeySym_i,
    InputKeySym_j,
    InputKeySym_k,
    InputKeySym_l,
    InputKeySym_m,
    InputKeySym_n,
    InputKeySym_o,
    InputKeySym_p,
    InputKeySym_q,
    InputKeySym_r,
    InputKeySym_s,
    InputKeySym_t,
    InputKeySym_u,
    InputKeySym_v,
    InputKeySym_w,
    InputKeySym_x,
    InputKeySym_y,
    InputKeySym_z,

    InputKeySym_delete = 127,

    // non-ASCII keys
    InputKeySym_up = 273,
    InputKeySym_down,
    InputKeySym_right,
    InputKeySym_left,
    InputKeySym_insert,
    InputKeySym_home,
    InputKeySym_end,
    InputKeySym_pageup,
    InputKeySym_pagedown,

    InputKeySym_f1 = 282,
    InputKeySym_f2,
    InputKeySym_f3,
    InputKeySym_f4,
    InputKeySym_f5,
    InputKeySym_f6,
    InputKeySym_f7,
    InputKeySym_f8,
    InputKeySym_f9,
    InputKeySym_f10,
    InputKeySym_f11,
    InputKeySym_f12,
    InputKeySym_f13,
    InputKeySym_f14,
    InputKeySym_f15,
    InputKeySym_f16,

    InputKeySym_numlock = 300,
    InputKeySym_capslock,
    InputKeySym_scrolllock,
    InputKeySym_rshift,
    InputKeySym_lshift,
    InputKeySym_rctrl,
    InputKeySym_lctrl,
    InputKeySym_ralt,
    InputKeySym_lalt,

    InputKeySym_help = 315,
    InputKeySym_print,
    InputKeySym_sysreq,
    InputKeySym_break,

    InputKeySym_last
};

enum KeyModSym
{
    KeyModNone = 0,
    KeyModLShift = 1,
    KeyModRShift = 2,
    KeyModLCtrl = 4,
    KeyModRCtrl = 8,
    KeyModLAlt = 16,
    KeyModRAlt = 32,
};

enum MouseSym
{
    MouseButton_left = 1,
    MouseButton_middle,
    MouseButton_right,
    MouseButton_max,
};

#endif
