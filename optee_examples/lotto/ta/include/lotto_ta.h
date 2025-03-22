#ifndef LOTTO_TA_H
#define LOTTO_TA_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define LOTTO_TA_UUID \
	{ 0x1ee382f6, 0x9e82, 0x4228, \
		{ 0xab, 0xaf, 0x4c, 0xcb, 0xd0, 0xeb, 0xb7, 0xa0} }
		
#define TA_LOTTO_CMD_GENERATE_KEYS        0
#define TA_LOTTO_CMD_GENERATE_RANDOMNESS  1
#define TA_LOTTO_CMD_VERIFY_RANDOMNESS    2

#endif /* LOTTO_TA_H */

