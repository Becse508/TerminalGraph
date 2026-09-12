#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// always save state in bit flags so nobody else can read your code ;)
#define PH_FIRST (1 << 0) // first placeholder found
#define PH_XFIRST (1 << 1) // the first placeholder is %x (if this is not set, its obviously %y)
#define PH_SECOND (1 << 2) // second placeholder found


// returns 0 on error, 1 if x is first, 2 if y is first
int convert_format(const char *user_format, char *scan_format, size_t scan_format_size) {
    char placeholder_state = 0; // state flags

    const char *src = user_format;
    char *dst = scan_format;

    while (*src && (size_t)(dst - scan_format) < scan_format_size - 1) {
        if (src[0] == '%' && src[1] == 'x') {
            if (placeholder_state & PH_SECOND) {
                fprintf(stderr, "Too many placeholders\n");
                return 0;
            }

            *dst++ = '%';
            *dst++ = 'f';
            src += 2;

            if (placeholder_state & PH_FIRST) {
                if (placeholder_state & PH_XFIRST) {
                    fprintf(stderr, "Two x placeholders are not allowed\n");
                    return 0;
                }
                placeholder_state |= PH_SECOND;
            }
            else {
                placeholder_state |= PH_FIRST;
                placeholder_state |= PH_XFIRST;
            }

        }
        else if (src[0] == '%' && src[1] == 'y') {
            if (placeholder_state & PH_SECOND) {
                fprintf(stderr, "Format must contain exactly one %%x and one %%y\n");
                return 0;
            }

            *dst++ = '%';
            *dst++ = 'f';
            src += 2;


            if (placeholder_state & PH_FIRST) {
                if ((placeholder_state & PH_XFIRST) == 0) {
                    fprintf(stderr, "Format must contain exactly one %%x and one %%y\n");
                    return 0;
                }
                placeholder_state |= PH_SECOND;
            }
            else {
                placeholder_state |= PH_FIRST;
            }
        } else if (src[0] == '%') {
            fprintf(stderr, "Invalid placeholder in format\n");
            return 0;
        } else {
            *dst++ = *src++;
        }
    }

    *dst = '\0';

    if ((placeholder_state & PH_FIRST) == 0 || (placeholder_state & PH_SECOND) == 0) {
        fprintf(stderr, "Format must contain exactly one %%x and one %%y\n");
        return 0;
    }

    return placeholder_state & PH_XFIRST ? 1 : 2;
}

int tg_parse_file(const char *path, const char *format, size_t skip, vector_float *datax, vector_float *datay) {
    
    char scan_format[255];
    int order = convert_format(format, scan_format, 255);
    if (order == 0) // error
        return 1;
    
    vector_float *data1 = order == 1 ? datax : datay;
    vector_float *data2 = order == 1 ? datay : datax;

    
    FILE *stream = fopen(path, "r");
    if (stream == 0) {
        perror("Could not open file");
        return 1;
    }
    // skip lines
    for (; skip-- != SIZE_MAX;) // i'm using this loop and you can't stop me
        for (int c; (c = fgetc(stream)) != '\n' && c != EOF;);


    float val1, val2;
    while (fscanf(stream, scan_format, &val1, &val2) == 2) {
        vector_float_push(data1, val1);
        vector_float_push(data2, val2);
    }

    fclose(stream);
    return 0;
}