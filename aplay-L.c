#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <locale.h>

#define _(STR) STR

static void pcm_list(snd_pcm_stream_t stream )
{
      void **hints, **n;
      char *name, *descr, *descr1, *io;
      const char *filter;

      if (snd_device_name_hint(-1, "pcm", &hints) < 0)
            return;
      n = hints;
      filter = stream == SND_PCM_STREAM_CAPTURE ? "Input" : "Output";
      while (*n != NULL) {
            name = snd_device_name_get_hint(*n, "NAME");
            descr = snd_device_name_get_hint(*n, "DESC");
            io = snd_device_name_get_hint(*n, "IOID");
            if (io != NULL && strcmp(io, filter) == 0)
                  goto __end;
            printf("%s\n", name);
            if ((descr1 = descr) != NULL) {
                  printf("    ");
                  while (*descr1) {
                        if (*descr1 == '\n')
                              printf("\n    ");
                        else
                              putchar(*descr1);
                        descr1++;
                  }
                  putchar('\n');
            }
            __end:
                  if (name != NULL)
                        free(name);
            if (descr != NULL)
                  free(descr);
            if (io != NULL)
                  free(io);
            n++;
      }
      snd_device_name_free_hint(hints);
}


int main (int argc, char *argv[])
{
  printf("*********** CAPTURE ***********\n");
  pcm_list(SND_PCM_STREAM_CAPTURE);

  printf("\n\n*********** PLAYBACK ***********\n");
  pcm_list(SND_PCM_STREAM_PLAYBACK);
}
