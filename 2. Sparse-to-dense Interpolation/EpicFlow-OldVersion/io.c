#include "io.h"

/*********** EDGES and MATCHES ***********/

/* read edges from a binary file containing width*height float32 values */
float_image read_edges(const char *filename, const int width, const int height)
{
    float_image res = empty_image(float, width, height);
    FILE *fid = fopen(filename, "rb");
    assert(fread(res.pixels, sizeof(float), width * height, fid) == width * height);
    fclose(fid);
    return res;
}

/* read matches, stored as x1 y1 x2 y2 per line (other values on the same is not taking into account */
float_image read_matches(const char *filename)
{
    float_image res = empty_image(float, 4, 100000);
    FILE *fid = fopen(filename, "r");
    int nmatch = 0;
    float x1, x2, y1, y2;
    while (!feof(fid) && fscanf(fid, "%f %f %f %f%*[^\n]", &x1, &y1, &x2, &y2) == 4)
    {
        res.pixels[4 * nmatch] = x1;
        res.pixels[4 * nmatch + 1] = y1;
        res.pixels[4 * nmatch + 2] = x2;
        res.pixels[4 * nmatch + 3] = y2;
        nmatch++;
    }
    res.pixels = (float *)realloc(res.pixels, sizeof(float) * 4 * nmatch);
    res.ty = nmatch;
    fclose(fid);
    return res;
}

/******* FLOW ********/

/* read a flow file and returns a pointer with two images containing the flow along x and y axis */
image_t **readFlowFile(const char *filename)
{
    FILE *fid = fopen(filename, "rb");
    if (fid == 0)
    {
        fprintf(stderr, "readFlow() error: could not open file  %s\n", filename);
        exit(1);
    }
    float help;
    fread(&help, sizeof(float), 1, fid);
    int aXSize, aYSize;
    fread(&aXSize, sizeof(int), 1, fid);
    fread(&aYSize, sizeof(int), 1, fid);
    image_t **flow = (image_t **)malloc(sizeof(image_t *) * 2);
    flow[0] = image_new(aXSize, aYSize);
    flow[1] = image_new(aXSize, aYSize);
    int x, y;
    for (y = 0; y < aYSize; y++)
        for (x = 0; x < aXSize; x++)
        {
            fread(&(flow[0]->data[y * flow[0]->stride + x]), sizeof(float), 1, fid);
            fread(&(flow[1]->data[y * flow[0]->stride + x]), sizeof(float), 1, fid);
        }
    fclose(fid);
    return flow;
}

/* write a flow to a file */
void writeFlowFile(const char *filename, const image_t *flowx, const image_t *flowy)
{
    FILE *stream = fopen(filename, "wb");
    if (stream == 0)
    {
        fprintf(stderr, "Error while opening %s\n", filename);
        exit(1);
    }
    const float help = 202021.25;
    fwrite(&help, sizeof(float), 1, stream);
    const int aXSize = flowx->width, aYSize = flowx->height;
    fwrite(&aXSize, sizeof(int), 1, stream);
    fwrite(&aYSize, sizeof(int), 1, stream);
    int y, x;
    for (y = 0; y < aYSize; y++)
        for (x = 0; x < aXSize; x++)
        {
            fwrite(&flowx->data[y * flowx->stride + x], sizeof(float), 1, stream);
            fwrite(&flowy->data[y * flowy->stride + x], sizeof(float), 1, stream);
        }
    fclose(stream);
}

/********************* IMAGE ***********************/

// PPM

typedef struct
{
    int magic;
    int width;
    int height;
    int pixmax;
} ppm_hdr_t;

static void get_magic(FILE *fp, ppm_hdr_t *ppm_hdr)
{
    char str[1024];
    fgets(str, 1024, fp);
    if (str[0] == 'P' && (str[1] <= '6' || str[1] >= '1'))
    {
        ppm_hdr->magic = str[1] - '0';
    }
}

static int skip_comment(FILE *fp)
{
    char c;
    do
    {
        c = (char)fgetc(fp);
    } while (c == ' ' || c == '\t' || c == '\n');
    if (c == '#')
    {
        do
        {
            c = (char)fgetc(fp);

        } while (c != 0x0A);
        return 1;
    }
    else
    {
        ungetc(c, fp);
    }
    return 0;
}

/*----------------------------------------------------------------------------*/

static void skip_comments(FILE *fp)
{
    while (skip_comment(fp))
        ;
}

/*----------------------------------------------------------------------------*/

static int get_image_size(FILE *fp, ppm_hdr_t *ppm_hdr)
{
    skip_comments(fp);
    if (fscanf(fp, "%d %d", &ppm_hdr->width, &ppm_hdr->height) != 2)
    {
        fprintf(stderr, "Warning: PGM --> File currupted\n");
        return 0;
    }
    return 1;
}

/*----------------------------------------------------------------------------*/

static int get_pixmax(FILE *fp, ppm_hdr_t *ppm_hdr)
{
    skip_comments(fp);
    ppm_hdr->pixmax = 1;
    if (ppm_hdr->magic == 2 || ppm_hdr->magic == 3 || ppm_hdr->magic == 5 || ppm_hdr->magic == 6)
    {
        if (fscanf(fp, "%d", &ppm_hdr->pixmax) != 1)
        {
            fprintf(stderr, "Warning: PGM --> pixmax not valid\n");
            return 0;
        }
    }
    fgetc(fp);
    return 1;
}

/*----------------------------------------------------------------------------*/

static int get_ppm_hdr(FILE *fp, ppm_hdr_t *ppm_hdr)
{
    get_magic(fp, ppm_hdr);
    if (!get_image_size(fp, ppm_hdr))
    {
        return 0;
    }
    if (!get_pixmax(fp, ppm_hdr))
    {
        return 0;
    }
    return 1;
}

static void raw_read_color(FILE *fp, color_image_t *image)
{
    int j;
    for (j = 0; j < image->height; j++)
    {
        int o = j * image->stride, i;
        for (i = 0; i < image->width; i++, o++)
        {
            image->c1[o] = (float)fgetc(fp);
            image->c2[o] = (float)fgetc(fp);
            image->c3[o] = (float)fgetc(fp);
        }
    }
}

color_image_t *color_image_pnm_load(FILE *fp)
{
    color_image_t *image = NULL;
    ppm_hdr_t ppm_hdr;
    if (!get_ppm_hdr(fp, &ppm_hdr))
    {
        return NULL;
    }
    switch (ppm_hdr.magic)
    {
    case 1: /* PBM ASCII */
    case 2: /* PGM ASCII */
    case 3: /* PPM ASCII */
    case 4: /* PBM RAW */
    case 5: /* PGM RAW */
        fprintf(stderr, "color_image_pnm_load: only PPM raw with maxval 255 supported\n");
        break;
    case 6: /* PPM RAW */
        image = color_image_new(ppm_hdr.width, ppm_hdr.height);
        raw_read_color(fp, image);
        break;
    }
    return image;
}

// GENERAL LOAD

/* load a color image from a file */
color_image_t *color_image_load(const char *fname)
{
    FILE *fp;
    char magic[2];
    unsigned short *magic_short = (unsigned short *)magic;
    color_image_t *image = NULL;
    if ((fp = fopen(fname, "rb")) == NULL)
    {
        fprintf(stderr, "Error in color_image_load() - can not open file `%s' !\n", fname);
        exit(1);
    }
    fread(magic, sizeof(char), 2, fp);
    rewind(fp);
    if (magic_short[0] == 0xd8ff)
    {
        // image = color_image_jpeg_load(fp);
    }
    else if (magic[0] == 'P' && (magic[1] == '6' || magic[1] == '5'))
    { /* PPM raw */
        image = color_image_pnm_load(fp);
    }
    else if (magic[0] == -119 && magic[1] == 'P')
    {
        // image = color_image_png_load(fp, fname);
    }
    else
    {
        fprintf(stderr, "Error in color_image_load(%s) - image format not supported, can only read jpg or ppm\n", fname);
        exit(1);
    }
    fclose(fp);
    return image;
}
