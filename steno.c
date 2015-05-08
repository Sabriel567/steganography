#include <stdlib.h>
#include <stdio.h>
#include "image.h"
#include "steno.h"

#define DBG_HIDE 1

struct byte_buffer
{
  int buffer;
  int eof_found;
  int size;
};

void shift_and_add(unsigned char *pix, int info);
void shift(unsigned char *pix);
void read_byte(FILE *file, struct byte_buffer *buffer);
int get_encoding_bit(struct byte_buffer *buffer);
unsigned char reverse_byte(unsigned char byte);
void hide_info_in_pixels(char *filename, struct image *image);

void init_buffer(struct byte_buffer *buffer)
{
  buffer->buffer = 0;
  buffer->eof_found = 0;
  buffer->size = 0;
}

/*TODO: Add FILE *file as param*/
void hide_info_in_pixels(char *filename, struct image *image)
{
  int32_t *pixels = image->pixel_array;
  struct byte_buffer *buffer = calloc(1, sizeof(struct byte_buffer));
  init_buffer(buffer);
  FILE *file = fopen(filename ,"rw");
  int num_pixels = image->dib_header->width * image->dib_header->height;
  int pixel_index = 0;
  int bits_written = 0;
  int alpha_byte = image->dib_header->bits_per_pixel/8 == 4 ? 1 : 0;
  int file_finished = 0;
  /*Loop over pixel array until whole file has been hidden*/
  while(pixel_index < num_pixels && !file_finished)
  {
    int32_t pix = pixels[pixel_index];
    unsigned char color;
    /*pixel after modification*/
    int32_t encoded_pixel = 0;
    if(alpha_byte)
    {
      /*Remove the alpha byte*/ 
      pix = pix >> 8;
    }

    /*Edit pixels in order : Red, Green, Blue*/
    if(DBG_HIDE) printf("NEW PIXEL %d\n", pixel_index);
    int i;
    for(i = 2; i>=0; --i)
    {
      if(buffer->eof_found && buffer->size <= 0)
      {
        file_finished = 1;
        if(DBG_HIDE) printf("Hidden full file\n");
        if(DBG_HIDE) printf("%d Pixels needed to hide things\n", pixel_index);
        return;
      }
     // if(!buffer->eof_found && buffer->size != 0)
      {
        /*Read another byte into the cache if there aren't enough for the next color*/
        if(buffer->size < BITS_ENCODED)
        {
          if(DBG_HIDE) printf("Adding to buffer\n");
          read_byte(file, buffer);
        }

        /*Shift the pixel so the color is correct*/
        color = (pix >> i*8) & 0xFF;

        /*Get BITS_ENCODED bits to encode from buffer*/
        int info =  get_encoding_bit(buffer);

        /*Shift off the last two bits and add encoding info*/
        shift_and_add(&color, info);
        encoded_pixel += color << i*8;
        bits_written += BITS_ENCODED;
      }
    }
    if(alpha_byte)
    {
      if(DBG_HIDE) printf("Encoding alpha byte\n");
      encoded_pixel = encoded_pixel << 8;
      //encoded_pixel += (pixels[pixel_index] & 0xFF);
       /*This bit is a silly attempt to put information into the alpha bit*/ 
        /*Get BITS_ENCODED bits to encode from buffer*/
        int info =  get_encoding_bit(buffer);
        unsigned char alpha = pixels[pixel_index] & 0xFF;
        /*Shift off the last two bits and add encoding info*/
        shift_and_add(&alpha, info);
        encoded_pixel += alpha;
        bits_written += BITS_ENCODED;
        /*To stop alpha bit encoding comment out above lines and uncomment line 91 "encoded_pixel += (pixels{..."*/
    }
    pixels[pixel_index] = encoded_pixel;
    ++pixel_index;
  }

}

void shift_and_add(unsigned char *pix, int info)
{
  if(DBG_HIDE) printf("Shifting and adding %X", info);
  *pix = *pix>>BITS_ENCODED;
  *pix = (*pix<<BITS_ENCODED)+info;
  if(DBG_HIDE) printf(" to %X\n", *pix);
}

void shift(unsigned char *pix)
{
  *pix = *pix>>BITS_ENCODED;
  *pix = (*pix<<BITS_ENCODED);
}

void read_byte(FILE *file, struct byte_buffer *buffer )
{
  unsigned char byte;
  if(fread(&byte, 1, 1, file) == 1)
  {
    int int_byte;
    if(DBG_HIDE) printf("Byte read from file %X\n",byte);
    byte = reverse_byte(byte);
    int_byte = byte << buffer->size;
    buffer->buffer = int_byte + buffer->buffer;
    buffer->size += 8;
    if(DBG_HIDE) printf("Byte read from file reversed %X\n",byte);
  }
  else
  {
    buffer->eof_found = 1;
    if(DBG_HIDE) printf("EOF FOUND \n");
  }
}

int get_encoding_bit(struct byte_buffer *buffer)
{
  int info;
  /*AND gets the low order 2 bits 0b11=0x03*/
  info = buffer->buffer & 0x03;
  buffer->buffer = buffer->buffer >> BITS_ENCODED;
  buffer->size -= BITS_ENCODED;
  if(DBG_HIDE) printf("Bits to be encoded %X\n",info);
  return info;
}

unsigned char reverse_byte(unsigned char byte)
{
  unsigned char rev_byte;
  int i;
  for(i = 7; i>=0; --i)
  {
    unsigned char temp_byte;
    temp_byte = (byte >> i) & 0x01;
    rev_byte += temp_byte << (7-i);
  }
  if(DBG_HIDE) printf("Byte reversed: %X \n",rev_byte);
  return rev_byte;
}
