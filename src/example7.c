#include "sox.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif

/*
Example of pushing data through an effects chain
Assumes input file is raw 32 bit float
*/

int main(int argc, char * argv[])
{
    static sox_format_t * in, * out; /* input and output files */
    sox_effects_chain_t * chain;
    sox_effect_t * e;
    sox_effect_t * buffer_effect;
    sox_effect_t * out_buffer_effect;
    char * args[10];
    assert(argc == 3);
    assert(sox_init() == SOX_SUCCESS);
    assert(sox_format_init() == SOX_SUCCESS);
    float *fbuffer;
    sox_sample_t *sample_buf;
    sox_sample_t *out_buf;
    char str[25];

    sox_globals_t *globals = sox_get_globals();
    globals->verbosity = 5;

    FILE *f = fopen(argv[1], "rb");

    fseek(f, 0L, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);
    
    fbuffer = malloc(fsize);
    size_t sample_buf_size = fsize/sizeof(*sample_buf);
    fread(fbuffer, sizeof(float), sample_buf_size, f);
    fclose(f);

    sample_buf = malloc(fsize);

    size_t i;
    int x = 0;
    int clips = 0;
    sox_sample_t sox_macro_temp_sample;
    double sox_macro_temp_double;

    for(i = 0; i < sample_buf_size; i++) {
      sample_buf[x++] = SOX_FLOAT_32BIT_TO_SAMPLE(fbuffer[i],clips);
    }

    in= (sox_format_t *) malloc(sizeof (sox_format_t));
    in->encoding.encoding = SOX_ENCODING_SIGN2;
    in->encoding.bits_per_sample = 32;// in->encoding.bits_per_sample;
    in->signal.rate = 44100.0f;
    in->signal.precision = 16;
    in->signal.channels = 2;
    in->signal.length = SOX_UNSPEC;
    in->signal.mult = NULL;

    out= (sox_format_t *) malloc(sizeof (sox_format_t));
    out->encoding.encoding = SOX_ENCODING_SIGN2;
    out->encoding.bits_per_sample = 32;// in->encoding.bits_per_sample;
    out->signal.rate = 44100.0f;
    out->signal.precision = 16;
    out->signal.channels = 2;
    out->signal.length = SOX_UNSPEC;
    out->signal.mult = NULL;

    chain = sox_create_effects_chain(&in->encoding, &out->encoding);

    buffer_effect = sox_create_effect(sox_find_effect("in_buffer"));
    assert(sox_add_effect(chain, buffer_effect, &in->signal, &in->signal) == SOX_SUCCESS);

    e = sox_create_effect(sox_find_effect("echos"));
    args[0] = "0.8";
    args[1] = "0.7";
    args[2] = "700";
    args[3] = "0.25";
    args[4] = "900";
    args[5] = "0.3";
    assert(sox_effect_options(e, 6, args) == SOX_SUCCESS);
    e->handler.flags |= SOX_EFF_LENGTH;
    assert(sox_add_effect(chain, e, &in->signal, &out->signal) == SOX_SUCCESS);
    free(e);

    out_buf = malloc(globals->bufsiz * sizeof(*out_buf));

    out_buffer_effect = sox_create_effect(sox_find_effect("out_buffer"));
    args[0] = (char *)out_buf;
    snprintf(str, sizeof(str), "%ld", globals->bufsiz);
    args[1] = (char *)str;
    assert(sox_effect_options(out_buffer_effect, 2, args) == SOX_SUCCESS);
    assert(sox_add_effect(chain, out_buffer_effect, &in->signal, &in->signal) == SOX_SUCCESS);

    sox_flow_buffer_prepare(chain);

    f = fopen(argv[2], "wb");

    for(i = 0; i < sample_buf_size;) {
      args[0] = (char *)&sample_buf[i];
      size_t bufsize = min(globals->bufsiz, sample_buf_size - i);
      snprintf(str, sizeof(str), "%ld", bufsize);
      args[1] = (char *)str;
      assert(sox_effect_options(buffer_effect, 2, args) == SOX_SUCCESS);

      sox_flow_buffer_effects(chain);

      i += globals->bufsiz;

      fwrite(out_buf, sizeof(*out_buf), bufsize, f);
    }

    sox_end_buffer_effects(chain);

    fclose(f);

    sox_delete_effects_chain(chain);

    free(out);
    free(in);
    free(buffer_effect);
    free(out_buffer_effect);
    free(sample_buf);
    free(fbuffer);
    free(out_buf);

    sox_quit();

    return 0;
}

