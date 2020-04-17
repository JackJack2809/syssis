#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#define sz 4

typedef struct
{
    char RIFF_marker[sz], filetype_header[sz], format_marker[sz];

    uint32_t file_size;
    uint32_t data_header_length;

    uint16_t format_type;
    uint16_t number_of_channels;

    uint32_t sample_rate;
    uint32_t bytes_per_second;
    uint16_t bytes_per_frame;
    uint16_t bits_per_sample;

} HeaderStructForWave;

HeaderStructForWave* genericWAVHeader(uint32_t sample_rate, uint16_t bit_depth, uint16_t channels){
    HeaderStructForWave *hdr;
    hdr = (HeaderStructForWave *)malloc(sizeof(*hdr));
    if(hdr){
        memcpy(&hdr->RIFF_marker, "RIFF", 4);
        memcpy(&hdr->filetype_header, "WAVE", 4);
        memcpy(&hdr->format_marker, "fmt ", 4);
        {
            // Set values
            hdr->data_header_length = 16;
            hdr->format_type = 1;
            hdr->number_of_channels = channels;
            hdr->sample_rate = sample_rate;
            hdr->bytes_per_second = sample_rate * channels * bit_depth / 8;
            hdr->bytes_per_frame = channels * bit_depth / 8;
            hdr->bits_per_sample = bit_depth;
        }
        return hdr;
    }
    return NULL;
}

int writeWAVHeader(int fd, HeaderStructForWave *hdr)
{
    if (!hdr)
        return -1;

    write(fd, &hdr->RIFF_marker, 4);
    write(fd, &hdr->file_size, 4);
    write(fd, &hdr->filetype_header, 4);
    write(fd, &hdr->format_marker, 4);
    write(fd, &hdr->data_header_length, 4);
    write(fd, &hdr->format_type, 2);
    write(fd, &hdr->number_of_channels, 2);
    write(fd, &hdr->sample_rate, 4);
    write(fd, &hdr->bytes_per_second, 4);
    write(fd, &hdr->bytes_per_frame, 2);
    write(fd, &hdr->bits_per_sample, 2);
    write(fd, "data", 4);

    uint32_t data_size = hdr->file_size - 36;
    write(fd, &data_size, 4);

    return 0;
}
void ErrorMessage(int param, int err){
    if(param == 1) fprintf (stderr, "cannot open audio device %s (%s)\n","default", snd_strerror (err));
    if(param == 2) fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror (err));
    if(param == 3) fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror (err));
    if(param == 4) fprintf (stderr, "cannot set access type (%s)\n", snd_strerror (err));
    if(param == 5) fprintf (stderr, "cannot set sample format (%s)\n", snd_strerror (err));
    if(param == 6) fprintf (stderr, "cannot set sample rate (%s)\n", snd_strerror (err));
    if(param == 7) fprintf (stderr, "cannot set channel count (%s)\n", snd_strerror (err));
    if(param == 8) fprintf (stderr, "cannot set parameters (%s)\n", snd_strerror (err));
    if(param == 9) fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
    if(param == 10) fprintf (stderr, "read from audio interface failed (%s)\n", snd_strerror (err));
    exit(0);
}
int main(){

  int i, err;


  char *buffer;
  int buffer_frames = 320;
  unsigned int rate = 44100;

  snd_pcm_t *capture_handle;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
  int filedesc;

  HeaderStructForWave *hdr  = genericWAVHeader(44100,8,2);

  {
          // PREPARING everything and handling Errors
          if ((err = snd_pcm_open (&capture_handle, "default", SND_PCM_STREAM_CAPTURE, 0)) < 0) ErrorMessage(1, err);
          fprintf(stdout, "microphone defined and open...\n");

          if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)  ErrorMessage(2, err);
          fprintf(stdout, "hardware parameters allocated\n");

          if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) ErrorMessage(3, err);
          fprintf(stdout, "hardware parameters initd\n");

          if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) ErrorMessage(4, err);
          fprintf(stdout, "hardware parameters access set...\n");

          if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) ErrorMessage(5,err);
          fprintf(stdout,
"hardware parameters format set...\n");

          if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) ErrorMessage(6,err);
          fprintf(stdout, "hardware parameters rate set...\n");

          if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 2)) < 0) ErrorMessage(7,err);
          fprintf(stdout, "hardware parameters channels set...\n");

          if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) ErrorMessage(8,err);
          fprintf(stdout, "hardware parameters set...\n");

          snd_pcm_hw_params_free (hw_params);
          fprintf(stdout, "hardware parameters free...\n");

          if ((err = snd_pcm_prepare (capture_handle)) < 0) ErrorMessage(9,err);
          fprintf(stdout, "audio interface prepared...\n");
  }


  int size = 128 * snd_pcm_format_width(format) / 8 * 2;
  buffer = malloc(size);

  fprintf(stdout, "buffer allocated %d\n", snd_pcm_format_width(format) / 8 * 2);
  uint32_t pcm_data_size = hdr->sample_rate * hdr->bytes_per_frame * (5000 / 1000);
  int fd = open("in.pcm", O_CREAT | O_RDWR, 0666);
  filedesc = open("in.wav", O_WRONLY | O_CREAT, 0644);
  hdr->file_size = pcm_data_size + 36;
  err = writeWAVHeader(filedesc, hdr);

  if( (err=writeWAVHeader(filedesc, hdr)) <0)
{
      fprintf(stderr, "error of writing .wаv hеаder.");
      snd_pcm_close(capture_handle);
      free(buffer);
      close(filedesc);
      return err;
  }
  for (i = 5000; i >= 0; --i)
  {
    if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) ErrorMessage(10,err);
    write(fd, buffer, size);
    write(filedesc, buffer, size);
  }
  close(fd);
  close(filedesc);
  free(buffer);
  fprintf(stdout, "buffer free... \n");
  snd_pcm_close (capture_handle);
  fprintf(stdout, "audio interface closing...\n");
  exit (0);
}
