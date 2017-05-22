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
  size_t length;
} priv_t;

static int getopts(sox_effect_t * effp, int argc, char * * argv)
{
  priv_t * p = (priv_t *)effp->priv;
  if (argc != 3)
    return SOX_EOF;

  p->buffer = (sox_sample_t *)argv[1];

  sscanf(argv[2], "%zu", &p->size);
  p->length = 0;

  return SOX_SUCCESS;
}

static int flow(sox_effect_t * effp, const sox_sample_t * ibuf,
    sox_sample_t * obuf, size_t * isamp, size_t * osamp)
{
  priv_t * p = (priv_t *)effp->priv;
  p->length = min(*isamp, p->size);

  obuf = NULL;
  *osamp = 0;

  memcpy(p->buffer, ibuf, p->length * sizeof(*obuf));

  return SOX_SUCCESS;
}

sox_effect_handler_t const * lsx_out_buffer_effect_fn(void)
{
  static sox_effect_handler_t handler = {
    "out_buffer", NULL, SOX_EFF_MCHAN | SOX_EFF_LENGTH | SOX_EFF_INTERNAL,
    getopts, NULL, flow, NULL, NULL, NULL, sizeof(priv_t)
  };
  return &handler;
}

