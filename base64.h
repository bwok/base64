/*
 ============================================================================
 Name        : base64.h
 Author      : James Fuge
 Version     : 0.1
 Copyright   : © 2019 James Fuge (james.fuge@gmail.com)
                      Released under the GNU General Public License
 Description : Base64 en and decoder
 ============================================================================
 */

/*
 * Text content    |         M       |        a        |      n          |     a           |        =         |
 * ASCII           |     77 (0x4d)   |    97 (0x61)    |   110 (0x6e)    |    97 (0x61)    |     padding      |
 * Bit pattern     |0 1 0 0 1 1 | 0 1 0 1 1 0 | 0 0 0 1 0 1 | 1 0 1 1 1 0|0 1 1 0 0 0 | 0 1 0 0 0 0 | 0 0 0 0 0 0 |
 * Index           |     19     |     22      |       5     |      46    |    24      |     16      |             |
 * Base64-encoded  |     T      |      W      |       F     |       u    |     Y      |      Q      |     =       |
 * */

/*
 * Encodes a character byte array as base64.
 * The output array should be big enough to hold the encoded data.
 * Returns the number of characters written
 * to the output array.
 */
int encode(const unsigned char *input, unsigned int in_len, unsigned char *output, int add_padding){
	const static char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	int out_index = 0;
	int in_index = 0;
	int byte_counter = 0;
	int partial_char = 0;

	if(in_len == 0){
		return 0;
	}

	for(in_index=0; in_index < in_len; in_index++){
		if(byte_counter == 0){
			output[out_index++] = base64_alphabet[ (input[in_index] & 0xfc) >> 2  ];
			partial_char = (input[in_index] & 0x3) << 4;
		}
		else if(byte_counter == 1){
			output[out_index++] = base64_alphabet[ partial_char | ((input[in_index] & 0xf0 ) >> 4) ];
			partial_char = (input[in_index] & 0xf ) << 2;
		}
		else if(byte_counter == 2){
			output[out_index++] = base64_alphabet[ partial_char | ((input[in_index] & 0xc0 ) >> 6) ];
			output[out_index++] = base64_alphabet[ (input[in_index] & 0x3f) ];
			partial_char = 0;
			byte_counter = -1;
		}
		byte_counter++;
	}

	/* Add the partially encoded character if there is one */
	if(byte_counter > 0){
		output[out_index++] = base64_alphabet[partial_char];
	}

	/* Add padding if the user asked for it */
	if(add_padding){
		if(byte_counter == 1){
			output[out_index++] = base64_alphabet[64];
			output[out_index++] = base64_alphabet[64];
		}
		else if(byte_counter == 2){
			output[out_index++] = base64_alphabet[64];
		}
	}

	output[out_index] = '\0';
	return out_index;
}


/*
 * Text content    |         f       |                 |                 |
 * ASCII           |    102 (0x4d)   |        0        |        0        |
 * Bit pattern     |0 1 1 0 0 1 | 1 0 0 0 0 0 | 0 0 0 0 0 0 | 0 0 0 0 0 0|
 * Index           |     25     |     32      |             |            |
 * Base64-encoded  |     Z      |      g      |       =     |       =    |
 * */

/**
 * Decodes a base64 encoded array.
 */
int decode(const unsigned char *input, unsigned int in_len, unsigned char *output){
	static const char decoding[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};
	unsigned int in_index = 0;
	int out_index = 0;
	int byte_counter = 0;
	int partial_char = 0;
	int temp_char = 0;
	int out_char = 0;

	for(in_index=0; in_index < in_len; in_index++){
		int decode_index = input[in_index]-43;
		if(decode_index < 0 || decode_index > 79){
			return -1;		// Out of array range, invalid input
		}

		int decode_val = decoding[decode_index];

		if(decode_val == -1){
			return -1;
		}else if(decode_val == -2){	// padding char
			temp_char = 0;
		}else{
			temp_char = decode_val;
		}

		if(byte_counter == 0){
			partial_char = temp_char << 2;
		}else if(byte_counter == 1){
			out_char = partial_char + ((temp_char & 0x30) >> 4);
			partial_char = (temp_char & 0xf) << 4;
		}else if(byte_counter == 2){
			out_char = partial_char + ((temp_char & 0x3c) >> 2);
			partial_char = (temp_char & 0x3) << 6;
		}else if(byte_counter == 3){
			out_char = partial_char + temp_char;
			partial_char = 0;
			byte_counter = -1;
		}

		/* Don't output out_char if value is 0, and it included a padding char. */
		if(byte_counter != 0 && decode_val != -2){
			output[out_index++] = out_char;
			out_char = 0;
		}else if(out_char > 0){
			output[out_index++] = out_char;
			out_char = 0;
		}
		byte_counter++;
	}

	output[out_index] = '\0';
	return out_index;
}
