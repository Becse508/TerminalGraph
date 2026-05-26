## Current capabilities:
- draw simple line graphs only
- braille/custom characters
- draw grid
- draw x/y value indicators
- ascii/utf8
- ANSI 16/256/truecolor
<img width="722" height="380" alt="image" src="https://github.com/user-attachments/assets/e8e44289-d9a6-4eec-9521-398f5041f4ae" />

## Example code:
```C
float datax[] = {1, 2, 3, 4, 5};
float datay[] = {2, 4, 0, 8, 3};

tg_render_opts opt = tg_default_render_opts_braille();
tg_convert_opts copt = tg_default_convert_opts();
copt.use_background = 0;

tg_cell *buffer = malloc(opt.width * opt.height * sizeof(tg_cell));

tg_clear(buffer, opt.width, opt.height);
tg_render(buffer, datax, datay, 5, &opt);
char *graph = tg_to_utf8_alloc(buffer, opt.width, opt.height, &copt);

puts(graph);

free(buffer);
free(graph);
```
