#include "wl_def.h"

#define PACKED __attribute__((packed))

typedef struct pcx_header_type
{
        char manufacturer;
        char version;
        char encoding;
        char bits_per_pixel;
        short int x, y;
        short int width, height;
        short int horz_res;
        short int virt_res;
        char ega_palette[48];
        char reserved;
        char num_color_planes;
        short int byte_per_line;
        short int palette_type;
        short int hscreen_size;
        short int vscreen_size; 
        char padding[54];
} PACKED pcx_header, *pcx_header_ptr;
	
void SavePCX256ToFile(unsigned char *buf, int width, int height, unsigned char *pal, char *name)
{
	FILE *fp;
	pcx_header ph;
	unsigned char *dat, *ptr, *ptrd, ch;
	int x, y, z;

	ph.manufacturer		= 10;
	ph.version		= 5;
	ph.encoding		= 1;
	ph.bits_per_pixel	= 8;
	ph.x = ph.y		= 0;
	ph.width		= width - 1;
	ph.height		= height - 1;
	ph.horz_res = ph.virt_res = 0;
	for (x = 0; x < sizeof(ph.ega_palette); x++) 
		ph.ega_palette[x] = 0;
	ph.reserved		= 0;
	ph.num_color_planes	= 1;
	ph.byte_per_line	= width;
	ph.palette_type		= 1;
	ph.hscreen_size		= width;
	ph.vscreen_size		= height;
	for (x = 0; x < sizeof(ph.padding); x++)
		ph.padding[x] = 0;

#if 0
	dat = malloc(width * height * 2);
	for (x = 0; x < width * height; x++) {
		*(dat + x*2) = 0xC1;
		*(dat + x*2+1) = *(buf + x);
	} 
	z = width * height * 2;
#else
	dat = malloc(width * height * 2);
	ptr = buf; ptrd = dat;
	x = 0; z = 0;
	while (x < width * height) {
		ch = *ptr;
		ptr++;
		x++;
		y = 0xC1;
		while((x < width * height) && (*ptr == ch) && (y < 0xFF)) {
			x++; y++; ptr++;
		}
		*ptrd = y;
		ptrd++;
		*ptrd = ch;
		ptrd++;
		z += 2;
	}
#endif
	
	fp = fopen(name, "wb");	
	fwrite(&ph, sizeof(ph), 1, fp);
	fwrite(dat, 1, z, fp);
	fputc(12, fp);
	fwrite(pal, 1, 768, fp);
	fclose(fp);
	
	free(dat);	
}

void SavePCXRGBToFile(unsigned char *buf, int width, int height, char *name)
{
	FILE *fp;
	pcx_header ph;
	unsigned char *dat;
	int x, y, s;
	
	memset(&ph, 0, sizeof(ph));
	ph.manufacturer 	= 10;
	ph.version 		= 5;
	ph.encoding		= 1;
	ph.bits_per_pixel 	= 8;
	ph.x = ph.y 		= 0;
	ph.width		= width - 1;
	ph.height		= height - 1;
	ph.horz_res = ph.virt_res = 0;
	ph.num_color_planes 	= 3;
	ph.byte_per_line 	= width;
	ph.palette_type 	= 1;
	ph.hscreen_size 	= width;
	ph.vscreen_size 	= height;

	dat = malloc(width * height * 2 * 3);
	for (y = 0; y < height; y++) {
		for (s = 0; s < 3; s++) {
			for (x = 0; x < width; x++) {
				*(dat + (y*(width*3) + (width*s) + x)*2) = 0xC1;
				*(dat + (y*(width*3) + (width*s) + x)*2+1) = *(buf + y*(width*3) + x*3 + s);
			}
		}
	}
	
	fp = fopen(name, "wb");	
	fwrite(&ph, sizeof(ph), 1, fp);
	fwrite(dat, 1, width * height * 2 * 3, fp);
	fclose(fp);
	
	free(dat);
}
