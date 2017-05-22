/* libSoX effect: a buffer input that can be updated
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "sox_i.h"

typedef struct {
  sox_sample_t * buffer;
  size_t size;
  size_t pos;
} priv_t;

static int getopts(sox_effect_t * effp, int argc, char * * argv)
{
  priv_t * p = (priv_t *)effp->priv;
  if (argc != 3)
    return SOX_EOF;

  p->buffer = (sox_sample_t *)argv[1];

  sscanf(argv[2], "%zu", &p->size);
  p->pos = 0;

  return SOX_SUCCESS;
}

static int drain(
    sox_effect_t * effp, sox_sample_t * obuf, size_t * osamp)
{
  priv_t * p = (priv_t *)effp->priv;

  ssize_t diff = p->size - p->pos;
  *osamp = min((ssize_t)*osamp, diff);

  if (*osamp <= 0) {
    return SOX_EOF;
  }

  /* ensure that *osamp is a multiple of the number of channels. */
  *osamp -= *osamp % effp->out_signal.channels;

  /* Read up to *osamp samples into obuf; store the actual number read
   * back to *osamp */
  memcpy(obuf, &p->buffer[p->pos], *osamp * sizeof(*obuf));

  p->pos += *osamp;

  /* sox_read may return a number that is less than was requested; only if
   * 0 samples is returned does it indicate that end-of-file has been reached
   * or an error has occurred */
  return *osamp? SOX_SUCCESS : SOX_EOF;
}

sox_effect_handler_t const * lsx_in_buffer_effect_fn(void)
{
  static sox_effect_handler_t handler = {
    "in_buffer", NULL, SOX_EFF_MCHAN | SOX_EFF_LENGTH | SOX_EFF_INTERNAL,
    getopts, NULL, NULL, drain, NULL, NULL, sizeof(priv_t)
  };
  return &handler;
}

