#include <stdio.h>

int encode(char* file_name);
int decode(char* file_name);
int find_b64_code(char c);

char b64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

int encode(char* file_name) {
	FILE *f_in = fopen(file_name, "r");
	FILE *f_out = fopen("final_result.txt", "w");

	// get size of f_in
	fseek(f_in, 0, SEEK_END);
	int sz = ftell(f_in);
	rewind(f_in);

	char buf[4];
	char tmp[3];
	int i = 0, j = 0;

	while (sz--) {
		tmp[i++] = fgetc(f_in);

		// when read 3 bytes
		if (i == 3) {
			buf[0] = b64[(tmp[0] & 0xfc) >> 2];
		    buf[1] = b64[((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4)];
		    buf[2] = b64[((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6)];
		    buf[3] = b64[tmp[2] & 0x3f];
			fprintf(f_out, "%s", buf);
			i = 0;
		}
	}
	if (i == 1) {
		buf[0] = b64[(tmp[0] & 0xfc) >> 2];
		buf[1] = b64[((tmp[0] & 0x03) << 4)];
		buf[2] = '=';
		buf[3] = '=';
		fprintf(f_out, "%s", buf);
	}
	if (i == 2) {
		buf[0] = b64[(tmp[0] & 0xfc) >> 2];
		buf[1] = b64[((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4)];
		buf[2] = b64[((tmp[1] & 0x0f) << 2)];
		buf[3] = '=';
		fprintf(f_out, "%s", buf);
	}

	
	fclose(f_in);
	fclose(f_out);
	return 0;
}

int find_b64_code(char c) {
	for (int i = 0; i < 64; i++) 
		if(c == b64[i])
			return i;
	return -1;
}

int decode(char* file_name) {
	FILE *f_in = fopen(file_name, "r");
	FILE *f_out = fopen("final_result.txt", "w");

	char buffer[4], a[4]="";

	while (fread(buffer, 1, 4, f_in) == 4) {
		char c1 = (find_b64_code(buffer[0])) << 2 | ((0x30 & find_b64_code(buffer[1])) >> 4);
		char c2 = ((0x0f & find_b64_code(buffer[1])) << 4) | ((0x3c & find_b64_code(buffer[2])) >> 2);
		char c3 = ((0x03 & find_b64_code(buffer[2])) << 6) | find_b64_code(buffer[3]);

		if (buffer[3] == '=') {
			if (buffer[2] == '=') {
				a[0] = c1; 
				a[1] = '\0';
				a[2] = '\0';
			}
			else {
				a[0] = c1; a[1] = c2; 
				a[2] = '\0';
			}

		} else {
			a[0] = c1; a[1] = c2; a[2] = c3; a[3] = '\0';
		}
		fputs(a, f_out);
	}
	fclose(f_in);
	fclose(f_out);

	return 0;
}