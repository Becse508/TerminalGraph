#include <argp.h>
#include <asm-generic/ioctls.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "converters.h"
#include "defs.h"
#include "parser.h"
#include "termgraph.h"


#if defined(__unix__) && defined(__has_include)
#  if __has_include(<sys/ioctl.h>)
#    include <sys/ioctl.h>
#    include <unistd.h>
#    define HAVE_SYS_IOCTL_H
#  endif
#endif



const char *argp_program_version = "0.1";
const char *argp_program_bug_address = "<https://github.com/Becse508/TerminalGraph/issues>";
static char doc[] = 
    "Draw graphs in the terminal";

static char args_doc[] = "INPUT_FILE INPUT_FORMAT";

static struct argp_option options[] = {
    // BASIC
    {"output",  'o', "FILE", 0, "Output to FILE instead of standard output."},
    {"skip", 's', "COUNT", 0, "Skip COUNT lines before starting to read data from the file."},

    // CONVERT OPTIONS
    {"color-format",            501, "FORMAT", 0, "How the output should be colored. Possible values for FORMAT: NOCOLOR, TRUECOLOR, ANSI16, ANSI256. Default: TRUECOLOR"},
    {"cf",                      501, 0, OPTION_ALIAS},
    {"use-background",          502, 0, 0, "Turn on background coloring. Setting any background color turns this on. Turned off by default."},
    {"dont-append-color-reset", 503, 0, 0, "Doesn't append the ANSI color reset sequence (\"\\x1b[0m\") after the last character. This will cause the console to stay colored after displaying the graph."},
    {"empty-char",              504, "CHAR", 0, "Sets what the program uses for empty space in the graph. Default is a space character (' ')."},

    // SIZE AND ALIGNMENT
    {"width",   'w', "WIDTH", 0, "Width of the graph. Default: terminal width"},
    {"height",  'h', "HEIGHT", 0, "Height of the graph. Default: terminal height - 4"},

    {"padding-left",    300, "INT", 0, "Left padding."},
    {"pl",              300, 0, OPTION_ALIAS},
    {"padding-right",   301, "INT", 0, "Right padding."},
    {"pr",              301, 0, OPTION_ALIAS},
    {"padding-top",     302, "INT", 0, "Top padding."},
    {"pt",              302, 0, OPTION_ALIAS},
    {"padding-bottom",  303, "INT", 0, "Bottom padding."},
    {"pb",              303, 0, OPTION_ALIAS},

    // DATA
    {"minx", 200, "FLOAT", 0, "The lowest X value on the graph. Lines going under will be clipped. Default: unset (auto)"},
    {"maxx", 201, "FLOAT", 0, "The highest X value on the graph. Lines going over will be clipped. Default: unset (auto)"},
    {"miny", 202, "FLOAT", 0, "The lowest Y value on the graph. Lines going under will be clipped. Default: unset (auto)"},
    {"maxy", 203, "FLOAT", 0, "The highest Y value on the graph. Lines going over will be clipped. Default: unset (auto)"},
    

    // COMPONENTS AND LOOK
    {"foreground",  403, "COLOR", 0, "Foreground RGB color in base 16 (e.g. 0xffffff)."},
    {"fg",          403, 0, OPTION_ALIAS},
    {"background",  404, "COLOR", 0, "Background RGB color in base 16 (e.g. 0xffffff)."},
    {"bg",          404, 0, OPTION_ALIAS},
    
    {"no-crosses",      415, 0, 0, "DON'T draw 'cross' characters where 2 grid lines intersect to smoothly connect them."},
    {"grid-vertical",   411, "COUNT", 0, "How many grid lines to draw vertically. Sets --indicator-vertical to the same value. 0 to disable."},
    {"gv",              411, 0, OPTION_ALIAS},
    {"grid-horizontal", 412, "COUNT", 0, "How many grid lines to draw horizontally. Sets --indicator-horizontal to the same value. 0 to disable."},
    {"gh",              412, 0, OPTION_ALIAS},
    {"grid-foreground", 413, "COLOR", 0, "Grid foreground RGB color in base 16 (e.g. 0xffffff)."},
    {"gfg",             413, 0, OPTION_ALIAS},
    {"grid-background", 414, "COLOR", 0, "Grid background RGB color in base 16 (e.g. 0xffffff)."},
    {"gbg",             414, 0, OPTION_ALIAS},

    {"indicator-vertical",      421, "COUNT", 0, "How many number indicators to draw at the bottom of the graph."},
    {"iv",                      421, 0, OPTION_ALIAS},
    {"indicator-horizontal",    422, "COUNT", 0, "How many number indicators to draw at the side of the graph."},
    {"ih",                      422, 0, OPTION_ALIAS},
    {"indicator-foreground",    423, "COLOR", 0, "Number indicator foreground RGB color in base 16 (e.g. 0xffffff)."},
    {"ifg",                     423, 0, OPTION_ALIAS},
    {"indicator-background",    424, "COLOR", 0, "Number indicator background RGB color in base 16 (e.g. 0xffffff)."},
    {"ibg",                     424, 0, OPTION_ALIAS},
    {"indicator-decimals",      425, "N", 0, "How many decimal places the indicators should have."},
    {"id",                      425, 0, OPTION_ALIAS},
    
    // SHORTHANDS
    {"padding", 'p', "INT", 0, "Set all padding values."},
    {0, 'G', 0, 0, "Don't draw grid. Doesnt't turn off indicators like setting --grid-horizontal and --grid-vertical to 0 would."},
    {0, 'I', 0, 0, "Don't draw indicators. Equivalent to setting --indicator-horizontal and --indicator-vertical to 0."},
    {0}
};



static char *output_file;
static int skip = 0;
static tg_render_opts opt;
static tg_convert_opts copt;

#define ERROR(arg) \
    do { \
        fprintf(stderr, "Invalid argument '%s'\n", arg); \
        exit(EINVAL); \
    } while (0);\

#define SET_GRID_COL(cells, channel, val) \
    cells.bottom.channel = val; \
    cells.left.channel = val; \
    cells.right.channel = val; \
    cells.top.channel = val;

#define SET_INDICATOR_COL(channel, val) \
    opt.indicator.x.channel = val; \
    opt.indicator.y.channel = val; 

// parsed integers cannot be negative
#define _PARSE_INT(var, base, arg) \
    var = strtol(arg, &end, base); \
    if ((*end && *end != '\n') || var == INT_MAX || var < 0) ERROR(arg)

#define PARSE_INT(var) _PARSE_INT(var, 10, arg)

#define PARSE_INT_NONZERO(var) \
    var = strtol(arg, 0, 10); \
    if (var == 0 || var == INT_MAX || var < 1) ERROR(arg) 

#define PARSE_COLOR(var) _PARSE_INT(var, 16, arg)

#define PARSE_FLOAT(var) \
    errno = 0; \
    var = strtof(arg, &end); \
    if ((*end && *end != '\n') || var == HUGE_VAL || var == HUGE_VALF || var == HUGE_VALL) ERROR(arg)


struct arguments {
    char *args[2];
};

char *format_names[] = {"NOCOLOR", "TRUECOLOR", "ANSI16", "ANSI256"};
tg_color_format format_vals[] = {TG_NOCOLOR, TG_TRUECOLOR, TG_ANSI_16, TG_ANSI_256};


static error_t parse_opts(int key, char *arg, struct argp_state *state) {
    char *end; // used for number parsing

    switch (key) {

        case 'o':
            output_file = arg; break;
        case 's':
            PARSE_INT(skip); break;
        
        case 'w':
            PARSE_INT_NONZERO(opt.width) break;
        case 'h':
            PARSE_INT_NONZERO(opt.height) break;
        
        case 501:
            for (int i = 0; i < 4; i++) {
                if (strcmp(arg, format_names[i]) == 0) {
                    copt.color_format = format_vals[i];
                    return 0;
                }
            }
            fprintf(stderr, "Invalid color format\n");
            exit(1);
            break;
        
        case 502:
            copt.use_background = 1;
            break;
        case 503:
            copt.append_color_reset = 0;
            break;
        case 504:
            if (strlen(arg) != 1) {
                fputs("--empty-char can only be one character!", stderr);
                exit(1);
            }
            copt.empty_char = arg[0];
            break;
        
        case 300:
            PARSE_INT(opt.padding.left) break;
        case 301:
            PARSE_INT(opt.padding.right) break;
        case 302:
            PARSE_INT(opt.padding.top) break;
        case 303:
            PARSE_INT(opt.padding.bottom) break;
        case 'p':
            PARSE_INT(opt.padding.left)
            opt.padding.right = opt.padding.left;
            opt.padding.top = opt.padding.left;
            opt.padding.bottom = opt.padding.left;
            break;

        case 200:
            PARSE_FLOAT(opt.x.min.value) opt.x.min.unset = 0; break;
        case 201:
            PARSE_FLOAT(opt.x.max.value) opt.x.max.unset = 0; break;
        case 202:
            PARSE_FLOAT(opt.y.min.value) opt.y.min.unset = 0; break;
        case 203:
            PARSE_FLOAT(opt.y.max.value) opt.y.max.unset = 0; break;
        
        case 403:
            PARSE_COLOR(opt.line.braille.fg) break;
        case 404:
            copt.use_background = 1;
            PARSE_COLOR(opt.line.braille.bg) break;
        case 415:
            opt.grid.draw_crosses = 0; break;
        case 411:
            PARSE_INT(opt.grid.vertical.count)
            opt.indicator.y.count = opt.grid.vertical.count;
            break;
        case 412:
            PARSE_INT(opt.grid.horizontal.count)
            opt.indicator.x.count = opt.grid.horizontal.count;
            break;
        case 413:
            PARSE_COLOR(opt.grid.vertical.cells.base.fg)
            SET_GRID_COL(opt.grid.vertical.cells, fg, opt.grid.vertical.cells.base.fg)
            SET_GRID_COL(opt.grid.horizontal.cells, fg, opt.grid.vertical.cells.base.fg)
            break;
        case 414:
            copt.use_background = 1;
            PARSE_COLOR(opt.grid.horizontal.cells.base.bg)
            SET_GRID_COL(opt.grid.vertical.cells, bg, opt.grid.vertical.cells.base.bg)
            SET_GRID_COL(opt.grid.horizontal.cells, bg, opt.grid.vertical.cells.base.bg)
            break;

        case 421:
            PARSE_INT(opt.indicator.y.count) break;
        case 422:
            PARSE_INT(opt.indicator.x.count) break;
        case 423:
            PARSE_COLOR(opt.indicator.y.fg)
            SET_INDICATOR_COL(fg, opt.indicator.y.fg)
            break;
        case 424:
            copt.use_background = 1;
            PARSE_COLOR(opt.indicator.x.bg)
            SET_INDICATOR_COL(bg, opt.indicator.x.bg)
            break;
        case 425:
            PARSE_INT(opt.indicator.x.decimal_places)
            opt.indicator.y.decimal_places = opt.indicator.x.decimal_places;
            break;

        case 'G':
            opt.grid.horizontal.count = 0;
            opt.grid.vertical.count = 0;
            break;
        case 'I':
            opt.indicator.x.count = 0;
            opt.indicator.y.count = 0;
            break;
            

        case ARGP_KEY_ARG:
            if (state->arg_num >= 2)
                argp_usage (state);

            ((struct arguments*)state->input)->args[state->arg_num] = arg;
            break;

        case ARGP_KEY_END:
            if (state->arg_num < 2)
                argp_usage (state);
            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp = {options, parse_opts, args_doc, doc};



int main(int argc, char *argv[]) {
    struct arguments args;
    opt = tg_default_render_opts_braille(0, 0x00FF00);
    copt = tg_default_convert_opts();
    
    // get terminal size on unix with ioctl
    #if defined(HAVE_SYS_IOCTL_H) && defined(TIOCGWINSZ)
    
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        opt.width = w.ws_col;
        opt.height = w.ws_row - 4;
    }
    #endif

    argp_parse(&argp, argc, argv, 0, 0, &args);
    
    if (opt.padding.right + opt.padding.left   > opt.width ||
        opt.padding.top   + opt.padding.bottom > opt.height) {
        fputs("Padding cannot be bigger than width/height!\n", stderr);
        return 1;
    }

    vector_float datax, datay;
    vector_float_init(&datax);
    vector_float_init(&datay);
    
    tg_parse_file(args.args[0], args.args[1], skip, &datax, &datay);

    tg_cell *buffer = malloc(opt.width * opt.height * sizeof(tg_cell));
    tg_clear(buffer, opt.width, opt.height);
    tg_render(buffer, datax.data, datay.data, datax.count, &opt);
    
    char *output = tg_to_utf8_alloc(buffer, opt.width, opt.height, &copt);

    FILE *stream = stdout;
    if (output_file != 0) {
        stream = fopen(output_file, "w");
        if (stream == 0) {
            fprintf(stderr, "failed to open file '%s'", output_file);
            vector_float_destroy(&datax);
            vector_float_destroy(&datay);
            return 1;
        }
    }
    fputs(output, stream);

    if (stream != stdout)
        fclose(stream);

    free(buffer);
    free(output);
    vector_float_destroy(&datax);
    vector_float_destroy(&datay);
    return 0;
}