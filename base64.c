#include <stdio.h>

int encode(char* file_name);

char base64set[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

int encode(char* file_name) {
	FILE *f_in = fopen(file_name, "r");
	FILE *f_out = fopen("encoded.txt", "w");

	// get size of f_in
	fseek(f_in, 0, SEEK_END);
	int sz = ftell(f_in);
	rewind(f_in);

	char buffer[3];
	int h = sz / 3, k = sz % 3, j = 0;
	while (j < h)  {
		fread(buffer, 1, 3, f_in);
		char b[4] = { 
			base64set[ buffer[0] >> 2 ] , 
	        base64set[ ((0x03 & buffer[0]) << 4) | ((buffer[1] & 0xf0) >> 4) ] ,
	        base64set[ ((0x0f & buffer[1]) << 2) | ((buffer[2] & 0xc0) >> 6) ] ,
	        base64set[ 0x3f & buffer[2] ] }; 
		fprintf(f_out,"%s", b);
		j ++;
	}
	if (k != 0) {
		fread(buffer, 1, 3, f_in);
		if (k == 2) {
			buffer[2] = '\0';
			char b[4] = {
				base64set[ buffer[0] >> 2 ] , 
		        base64set[ ((0x03 & buffer[0]) << 4) | ((buffer[1] & 0xf0) >> 4) ] ,
		        base64set[ ((0x0f & buffer[1]) << 2) | ((buffer[2] & 0xc0) >> 6) ] , '='		
			};
			fprintf(f_out, "%s", b);
		}

		if (k == 1) {
			buffer[1] = '\0';
			char b[4] = {
				base64set[ buffer[0] >> 2],
				base64set[ ((0x03 & buffer[0]) << 4) | ((buffer[1] & 0xf0) >> 4) ] , '=', '='
			};
			fprintf(f_out, "%s", b);
		}
	}
	fclose(f_in);
	fclose(f_out);
	return 0;
}

int decode(char* file_name) {
	FILE *f_in = fopen(file_name, "r");
	FILE *f_out = fopen("decoded.txt", "w");


}

int main() {
	encode("khanhlt.txt");
	return 0;
}