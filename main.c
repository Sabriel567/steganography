#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "steno.h"

#define DBG_HEADER 1
#define DBG_DIB 1
#define DBG_HIDE 1

int main(int argc, char **argv)
{
  if (argv[1] == NULL || argv[2] == NULL)
  {
    printf("Usage: steno.out <image filename> <information filename>");
  }
  char *filename = argv[1];
  FILE *file;
  file = fopen(filename, "rb");
  if(file == NULL)
  {
    printf("Cannot find file: %s\n", filename);
    return -1;
  }

  int file_magic;
  fread(&file_magic, 2, 1, file);
  if(file_magic != MAGIC_NUMBER)
  {
    printf("Magic number: %d does not match %d\n",
    file_magic, MAGIC_NUMBER);
    return -1;
  }
  struct image *image = calloc(sizeof(struct image), 1);

  read_header(file, image);

  if(DBG_HEADER)
  {
    printf("File size %d\n",image->file_header->file_size);
    printf("File res_1 %d\n",image->file_header->res_1);
    printf("File res_2 %d\n",image->file_header->res_2);
    printf("File offset %d\n",image->file_header->offset);
  }

  read_dib(file, image);

  if(DBG_DIB)
  {
    printf("Header Size: %d\n",image->dib_header->header_size);
    printf("Width: %d\n",image->dib_header->width);
    printf("Height: %d\n",image->dib_header->height);
    printf("Planes: %d\n",image->dib_header->planes);
    printf("Bits Per Pixel: %d\n",
    image->dib_header->bits_per_pixel);
    printf("Compression: %d\n",image->dib_header->compression);
    printf("Image Size: %d\n",image->dib_header->image_size);
    printf("Horizontal Resolution: %d\n",
    image->dib_header->hor_res);
    printf("Verical Resolution: %d\n",
    image->dib_header->vert_res);
    printf("Pallet: %d\n",image->dib_header->pallet);
    printf("Important Colors: %d\n",
    image->dib_header->imp_colors);
  }

  printf("Read BMP file\n");

  create_pixel_array(file, image);
//  print_pixel_array(image);
  if(DBG_HIDE) printf("Hiding info from %s\n", argv[2]);
  hide_info_in_pixels(argv[2], image);
  //print_pixel_array(image);

  write_image_to_file(image);

  free(image->file_header);
  free(image->pixel_array);
  free(image->dib_header);
  free(image);

  return 1;

}

