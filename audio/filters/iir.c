/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2014 - Brad Miller
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dspfilter.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI		3.1415926535897932384626433832795
#endif
#define sqr(a) ((a) * (a))

/* filter types */
enum IIRFilter {
	LPF, /* low pass filter */
	HPF, /* High pass filter */
	BPCSGF,/* band pass filter 1 */
	BPZPGF,/* band pass filter 2 */
	APF, /* Allpass filter*/
	NOTCH, /* Notch Filter */
	RIAA_phono, /* RIAA record/tape deemphasis */
	PEQ, /* Peaking band EQ filter */
	BBOOST, /* Bassboost filter */
	LSH, /* Low shelf filter */
	HSH, /* High shelf filter */
	RIAA_CD, /* CD de-emphasis */
	GBP /* Game Boy Player Start-Up Disc */
};

struct iir_data
{
   float b0, b1, b2;
   float a0, a1, a2;

   struct
   {
      float xn1, xn2;
      float yn1, yn2;
   } l, r;
};

static void iir_free(void *data)
{
   free(data);
}

static void iir_process(void *data, struct dspfilter_output *output,
      const struct dspfilter_input *input)
{
   unsigned i;
   struct iir_data *iir = (struct iir_data*)data;

   output->samples = input->samples;
   output->frames  = input->frames;

   float *out = output->samples;

   float b0 = iir->b0;
   float b1 = iir->b1;
   float b2 = iir->b2;
   float a0 = iir->a0;
   float a1 = iir->a1;
   float a2 = iir->a2;

   float xn1_l = iir->l.xn1;
   float xn2_l = iir->l.xn2;
   float yn1_l = iir->l.yn1;
   float yn2_l = iir->l.yn2;

   float xn1_r = iir->r.xn1;
   float xn2_r = iir->r.xn2;
   float yn1_r = iir->r.yn1;
   float yn2_r = iir->r.yn2;

   for (i = 0; i < input->frames; i++, out += 2)
   {
      float in_l = out[0];
      float in_r = out[1];

      float l = (b0 * in_l + b1 * xn1_l + b2 * xn2_l - a1 * yn1_l - a2 * yn2_l) / a0;
      float r = (b0 * in_r + b1 * xn1_r + b2 * xn2_r - a1 * yn1_r - a2 * yn2_r) / a0;

      xn2_l = xn1_l;
      xn1_l = in_l;
      yn2_l = yn1_l;
      yn1_l = l;

      xn2_r = xn1_r;
      xn1_r = in_r;
      yn2_r = yn1_r;
      yn1_r = r;

      out[0] = l;
      out[1] = r;
   }

   iir->l.xn1 = xn1_l;
   iir->l.xn2 = xn2_l;
   iir->l.yn1 = yn1_l;
   iir->l.yn2 = yn2_l;

   iir->r.xn1 = xn1_r;
   iir->r.xn2 = xn2_r;
   iir->r.yn1 = yn1_r;
   iir->r.yn2 = yn2_r;
}

#define CHECK(x) if (!strcmp(str, #x)) return x
static enum IIRFilter str_to_type(const char *str)
{
   CHECK(LPF);
   CHECK(HPF);
   CHECK(BPCSGF);
   CHECK(BPZPGF);
   CHECK(APF);
   CHECK(NOTCH);
   CHECK(RIAA_phono);
   CHECK(PEQ);
   CHECK(BBOOST);
   CHECK(LSH);
   CHECK(HSH);
   CHECK(RIAA_CD);
   CHECK(GBP);
   return LPF; // Fallback.
}

static void make_poly_from_roots(
      const double *roots, unsigned num_roots, float *poly)
{
   unsigned i, j;
   poly[0] = 1;
   poly[1] = -roots[0];
   memset(poly + 2, 0, (num_roots + 1 - 2) * sizeof(*poly));
   for (i = 1; i < num_roots; i++)
      for (j = num_roots; j > 0; j--)
         poly[j] -= poly[j - 1] * roots[i];
}

static void iir_filter_init(struct iir_data *iir,
      float sample_rate, float freq, float qual, float gain, enum IIRFilter filter_type)
{
	double omega = 2.0 * M_PI * freq / sample_rate;
   double cs = cos(omega);
   double sn = sin(omega);
   double a1pha = sn / (2.0 * qual);
   double A = exp(log(10.0) * gain / 40.0);
   double beta = sqrt(A + A);

   float b0 = 0.0, b1 = 0.0, b2 = 0.0, a0 = 0.0, a1 = 0.0, a2 = 0.0;

   // Set up filter coefficients according to type
   switch (filter_type)
   {
      case LPF:
         b0 =  (1.0 - cs) / 2.0;
         b1 =   1.0 - cs ;
         b2 =  (1.0 - cs) / 2.0;
         a0 =   1.0 + a1pha;
         a1 =  -2.0 * cs;
         a2 =   1.0 - a1pha;
         break;
	  case GBP:
         b0 =  -1.965293407440185546875;
         b1 =   -1.965293407440185546875;
         b2 =  .965885460376739501953125;
         a0 =   .000148021979839541018009185791015625;
         a1 =  .00029604395967908203601837158203125;
         a2 =   .000148021979839541018009185791015625;
         break;
      case HPF:
         b0 =  (1.0 + cs) / 2.0;
         b1 = -(1.0 + cs);
         b2 =  (1.0 + cs) / 2.0;
         a0 =   1.0 + a1pha;
         a1 =  -2.0 * cs;
         a2 =   1.0 - a1pha;
         break;
      case APF:
         b0 =  1.0 - a1pha;
         b1 = -2.0 * cs;
         b2 =  1.0 + a1pha;
         a0 =  1.0 + a1pha;
         a1 = -2.0 * cs;
         a2 =  1.0 - a1pha;
         break;
      case BPZPGF:
         b0 =  a1pha;
         b1 =  0.0;
         b2 = -a1pha;
         a0 =  1.0 + a1pha;
         a1 = -2.0 * cs;
         a2 =  1.0 - a1pha;
         break;
      case BPCSGF:
         b0 =  sn / 2.0;
         b1 =  0.0;
         b2 = -sn / 2.0;
         a0 =  1.0 + a1pha;
         a1 = -2.0 * cs;
         a2 =  1.0 - a1pha;
         break;
      case NOTCH: 
         b0 =  1.0;
         b1 = -2.0 * cs;
         b2 =  1.0;
         a0 =  1.0 + a1pha;
         a1 = -2.0 * cs;
         a2 =  1.0 - a1pha;
         break;
      case RIAA_phono: /* http://www.dsprelated.com/showmessage/73300/3.php */
      {
         float b[3], a[3];

         if ((int)sample_rate == 44100)
         {
            static const double zeros[] = {-0.2014898, 0.9233820};
            static const double poles[] = {0.7083149, 0.9924091};
            make_poly_from_roots(zeros, 2, b);
            make_poly_from_roots(poles, 2, a);
         }
         else if ((int)sample_rate == 48000)
         {
            static const double zeros[] = {-0.1766069, 0.9321590};
            static const double poles[] = {0.7396325, 0.9931330};
            make_poly_from_roots(zeros, 2, b);
            make_poly_from_roots(poles, 2, a);
         }
         else if ((int)sample_rate == 88200)
         {
            static const double zeros[] = {-0.1168735, 0.9648312};
            static const double poles[] = {0.8590646, 0.9964002};
            make_poly_from_roots(zeros, 2, b);
            make_poly_from_roots(poles, 2, a);
         }
         else if ((int)sample_rate == 96000)
         {
            static const double zeros[] = {-0.1141486, 0.9676817};
            static const double poles[] = {0.8699137, 0.9966946};
            make_poly_from_roots(zeros, 2, b);
            make_poly_from_roots(poles, 2, a);
         }
         
         b0 = b[0];
         b1 = b[1];
         b2 = b[2];
         a0 = a[0];
         a1 = a[1];
         a2 = a[2];

         /* Normalise to 0dB at 1kHz (Thanks to Glenn Davis) */
         double y = 2.0 * M_PI * 1000.0 / sample_rate;
         double b_re = b0 + b1 * cos(-y) + b2 * cos(-2.0 * y);
         double a_re = a0 + a1 * cos(-y) + a2 * cos(-2.0 * y);
         double b_im = b1 * sin(-y) + b2 * sin(-2.0 * y);
         double a_im = a1 * sin(-y) + a2 * sin(-2.0 * y);
         double g = 1.0 / sqrt((sqr(b_re) + sqr(b_im)) / (sqr(a_re) + sqr(a_im)));
         b0 *= g; b1 *= g; b2 *= g;
         break;
      }
      case PEQ: 
         b0 =  1.0 + a1pha * A;
         b1 = -2.0 * cs;
         b2 =  1.0 - a1pha * A;
         a0 =  1.0 + a1pha / A;
         a1 = -2.0 * cs;
         a2 =  1.0 - a1pha / A;
         break; 
      case BBOOST:       
         beta = sqrt((A * A + 1) / 1.0 - (pow((A - 1), 2)));
         b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
         b1 = 2 * A * ((A - 1) - (A + 1) * cs);
         b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
         a0 = ((A + 1) + (A - 1) * cs + beta * sn);
         a1 = -2 * ((A - 1) + (A + 1) * cs);
         a2 = (A + 1) + (A - 1) * cs - beta * sn;
         break;
      case LSH:
         b0 = A * ((A + 1) - (A - 1) * cs + beta * sn);
         b1 = 2 * A * ((A - 1) - (A + 1) * cs);
         b2 = A * ((A + 1) - (A - 1) * cs - beta * sn);
         a0 = (A + 1) + (A - 1) * cs + beta * sn;
         a1 = -2 * ((A - 1) + (A + 1) * cs);
         a2 = (A + 1) + (A - 1) * cs - beta * sn;
         break;
      case RIAA_CD:
         omega = 2.0 * M_PI * 5283.0 / sample_rate;
         cs = cos(omega);
         sn = sin(omega);
         a1pha = sn / (2.0 * 0.4845);
         A = exp(log(10.0) * -9.477 / 40.0);
         beta = sqrt(A + A);
      case HSH:
         b0 = A * ((A + 1.0) + (A - 1.0) * cs + beta * sn);
         b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cs);
         b2 = A * ((A + 1.0) + (A - 1.0) * cs - beta * sn);
         a0 = (A + 1.0) - (A - 1.0) * cs + beta * sn;
         a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cs);
         a2 = (A + 1.0) - (A - 1.0) * cs - beta * sn;
         break;
      default:
         break;
   }

   iir->b0 = b0;
   iir->b1 = b1;
   iir->b2 = b2;
   iir->a0 = a0;
   iir->a1 = a1;
   iir->a2 = a2;
}

static void *iir_init(const struct dspfilter_info *info,
      const struct dspfilter_config *config, void *userdata)
{
   struct iir_data *iir = (struct iir_data*)calloc(1, sizeof(*iir));
   if (!iir)
      return NULL;

   float freq, qual, gain;
   config->get_float(userdata, "frequency", &freq, 1024.0f);
   config->get_float(userdata, "quality", &qual, 0.707f);
   config->get_float(userdata, "gain", &gain, 0.0f);

   char *type = NULL;
   config->get_string(userdata, "type", &type, "LPF");

   enum IIRFilter filter = str_to_type(type);
   config->free(type);
//printf("amountSR: %.4f", info->input_rate);
   iir_filter_init(iir, info->input_rate, freq, qual, gain, filter);
   return iir;
}

static const struct dspfilter_implementation iir_plug = {
   iir_init,
   iir_process,
   iir_free,

   DSPFILTER_API_VERSION,
   "IIR",
   "iir",
};

#ifdef HAVE_FILTERS_BUILTIN
#define dspfilter_get_implementation iir_dspfilter_get_implementation
#endif

const struct dspfilter_implementation *dspfilter_get_implementation(dspfilter_simd_mask_t mask)
{
   (void)mask;
   return &iir_plug;
}

#undef dspfilter_get_implementation

