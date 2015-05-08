#include <stdio.h>
#include <stdlib.h>
#include "image.h"

#define DBG_HEADER 1
#define DBG_DIB 1

void read_header(FILE *file, struct image *image);
void read_dib(FILE *file, struct image *image);
void write_dib(FILE *file, struct image *image);
void write_header(FILE *file, struct image *image);
void create_pixel_array(FILE *file, struct image *image);
void print_pixel_array(struct image *image);


void read_header(FILE *file, struct image *image)
{
  image->file_header = calloc(sizeof(struct file_header), 1);
  struct file_header *file_header = image->file_header;

  fread(&file_header->file_size, 4, 1, file);
  fread(&file_header->res_1, 2, 1, file);
  fread(&file_header->res_2, 2, 1, file);
  fread(&file_header->offset, 4, 1, file);

}

void read_dib(FILE *file, struct image *image)
{
  image->dib_header = calloc(sizeof(struct dib_header), 1);
  struct dib_header *dib_header = image->dib_header;
  
  fread(&dib_header->header_size, 4, 1, file);
  fread(&dib_header->width, 4, 1, file);
  fread(&dib_header->height, 4, 1, file);
  fread(&dib_header->planes, 2, 1, file);
  fread(&dib_header->bits_per_pixel, 2, 1, file);
  fread(&dib_header->compression, 4, 1, file);
  fread(&dib_header->image_size, 4, 1, file);
  fread(&dib_header->hor_res, 4, 1, file);
  fread(&dib_header->vert_res, 4, 1, file);
  fread(&dib_header->pallet, 4, 1, file);
  fread(&dib_header->imp_colors, 4, 1, file);
}
void write_dib(FILE *file, struct image *image)
{
  struct dib_header *dib_header = image->dib_header;
  
  fwrite(&dib_header->header_size, 4, 1, file);
  fwrite(&dib_header->width, 4, 1, file);
  fwrite(&dib_header->height, 4, 1, file);
  fwrite(&dib_header->planes, 2, 1, file);
  fwrite(&dib_header->bits_per_pixel, 2, 1, file);
  fwrite(&dib_header->compression, 4, 1, file);
  fwrite(&dib_header->image_size, 4, 1, file);
  fwrite(&dib_header->hor_res, 4, 1, file);
  fwrite(&dib_header->vert_res, 4, 1, file);
  fwrite(&dib_header->pallet, 4, 1, file);
  fwrite(&dib_header->imp_colors, 4, 1, file);
}

void write_header(FILE *file, struct image *image)
{
  struct file_header *file_header = image->file_header;

  fwrite(&file_header->file_size, 4, 1, file);
  fwrite(&file_header->res_1, 2, 1, file);
  fwrite(&file_header->res_2, 2, 1, file);
  fwrite(&file_header->offset, 4, 1, file);
}

void create_pixel_array(FILE *file, struct image *image)
{
  int width = image->dib_header->width;
  int height = image->dib_header->height;
  int pixel_count = width * height;
  int array_size = pixel_count * sizeof(int32_t);
  image->pixel_array = calloc(array_size, 1);
  int32_t *pix_array = image->pixel_array;
  int bytes_per_pixel = image->dib_header->bits_per_pixel/8;

  printf("Creating pixel array size: %d\n", pixel_count);
  printf("Bytes per pixel: %d\n", bytes_per_pixel);
  
  fseek(file , image->file_header->offset , SEEK_SET);
  
  int row;
  int index = 0;
  for(row = 0; row < height; ++row)
  {
    int alignment = 0;
    int col;
    for(col = 0; col < width; ++col)
    {
      fread(&pix_array[index], bytes_per_pixel, 1, file);
      ++index;
      alignment += bytes_per_pixel;
    }
    /*Move the file position to the next row by skipping any padding added
    to the file to align the pixels by 4 bytes*/
    if(alignment%4 != 0)
    {
      printf("Seeking forward %d bytes\n", 4-alignment%4);
      fseek(file , 4 - alignment%4, SEEK_CUR);
    }
  }
}

void write_pixel_array(FILE *file, struct image *image)
{
  int width = image->dib_header->width;
  int height = image->dib_header->height;
  int pixel_count = width * height;
  int array_size = pixel_count * sizeof(int32_t);
  int32_t *pix_array = image->pixel_array;
  int bytes_per_pixel = image->dib_header->bits_per_pixel/8;

  printf("Creating pixel array size: %d\n", pixel_count);
  printf("Bytes per pixel: %d\n", bytes_per_pixel);
  
  fseek(file , image->file_header->offset , SEEK_SET);
  
  int row;
  int index = 0;
  for(row = 0; row < height; ++row)
  {
    int alignment = 0;
    int col;
    for(col = 0; col < width; ++col)
    {
      fwrite(&pix_array[index], bytes_per_pixel, 1, file);
      ++index;
      alignment += bytes_per_pixel;
    }
    /*Move the file position to the next row by skipping any padding added
    to the file to align the pixels by 4 bytes*/
    if(alignment%4 != 0)
    {
      int i;
      for(i = 0; i < 4 - alignment%4; ++i)
      {
        unsigned char zero_byte = 0;
        fwrite(&zero_byte , 1, 1, file);
      }
    }
  }
}

void write_image_to_file(struct image *image)
{
  FILE *file = fopen("test.bmp", "w+");
  char magic = 0x42;
  char number = 0x4D;
  fwrite(&magic, 1, 1, file);
  fwrite(&number, 1, 1, file);
  write_header(file, image);
  write_dib(file, image);
  write_pixel_array(file, image);
}

void print_pixel_array(struct image *image)
{
  int width = image->dib_header->width;
  int height = image->dib_header->height;
  int pixel_count = width * height;
  int32_t *pix_array = image->pixel_array;
  
  printf("Pixel Array:\n");

  int i;
  for(i = 0; i < pixel_count; ++i)
  {
    printf("  %08X\n", pix_array[i]);
//    write(stdout, pix_array[i], image->dib_header->bits_per_pixel/8);
  }
}


void hide_info(FILE *file, struct image *image)
{
  int bits_left;
}

