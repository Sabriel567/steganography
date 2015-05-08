/*Compressions*/
#define BI_BITFIELDS 3
#define MAGIC_NUMBER 0x4D42

struct file_header
{
  int MAGIC;        /*Number at start of file 0x424D*/
  int file_size;    /*Size of BPM file*/
  int res_1;
  int res_2;
  int offset;       /*starting address of byte where pix array starts*/
};

struct dib_header
{
  int header_size;
  int width;
  int height;
  int planes;
  int bits_per_pixel;
  int compression;
  int image_size;
  int hor_res;
  int vert_res;
  int pallet;
  int imp_colors;
};


struct image
{
  struct file_header *file_header;
  struct dib_header *dib_header;
  int32_t *pixel_array;
};

void read_header(FILE *file, struct image *image);
void read_dib(FILE *file, struct image *image);
void create_pixel_array(FILE *file, struct image *image);
void print_pixel_array(struct image *image);
void write_image_to_file(struct image *image);
