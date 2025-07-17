/*
 * hexcrc is a command line utility to test the CRC of each line of an
 * intel hex file.
 *
 * Author: Omar Rampado <omar@ognibit.it>
 * Version: 1.0.0
 *
 * Use: hexrc <file.hex>
 *
 * An hex file is a sequence of records written in ASCII characters.
 * A record starts with ':'.
 * A record is composed by pairs of hex characters, uppercase.
 * Every pair represents a byte value in hex.
 * A sequence of pairs must sum to zero because the CRC at the end is built to
 * create that behavior.
 * Between record there can be any character.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

/* every case out of these states is an error */
enum State {
    COMMENT,
    NEW_RECORD,
    BYTE_START,
    BYTE_END
};

/* verify that the charact c is a correct hex digit */
bool is_hex_char(int c)
{
    /* must be a valid HEX digit */
    if (!isxdigit(c)){
        return false;
    }

    /* must be uppercase */
    if (isalpha(c) && islower(c)){
        return false;
    }

    return true;
}/* is_hex_char */

/* Convert an HEX digit c into a value [0,255] */
int hex_to_dec(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    abort();
    return -1;
}/* hex_to_dec */

void log_error(const char *msg, int line, int col, int input)
{
    fprintf(stderr, "ERROR at line %i:%i: %s\n", line, col, msg);
    fprintf(stderr, "INVALID INPUT: '%c'\n", input);
}/* log_error */

/* Verify the low significan byte is zero */
bool check_crc(uint64_t recsum)
{
    uint8_t lsb = recsum & 0xFF;
    return lsb == 0;
}/* check_crc */

/* Verify HEX CRC for each record in file.
 * Print the error and return if an error occurs.
 * Return 0 on success, -1 on error.
 */
bool check_file(FILE *file)
{
    int line = 1;         /* number of line */
    int col = 1;          /* column number */
    int input = EOF;      /* current character */
    uint8_t byte = 0;     /* 2 hex digit representation */
    uint64_t recordSum = 0; /* collect the sum of bytes */
    size_t cnt = 0;       /* record counter */
    enum State state = COMMENT;

    while ((input = fgetc(file)) != EOF){

        /* Finite State Automata for checking the sum */
        switch (state){
        case COMMENT:
            if (input == ':'){
                cnt++;
                state = NEW_RECORD;
            }
            /* otherwise discard the input */
            break;
        case NEW_RECORD:
            if (is_hex_char(input)){
                recordSum = 0;
                /* set upper part of byte */
                byte = 0;
                byte = hex_to_dec(input) << 4;

                state = BYTE_START;
            } else {
                log_error("Expected uppercase hex digit.", line, col, input);
                return false;
            }
            break;
        case BYTE_START:
            if (is_hex_char(input)){
                /* set lower part of byte */
                byte = byte | hex_to_dec(input);

                state = BYTE_END;
            } else {
                log_error("Expected uppercase hex digit.", line, col, input);
                return false;
            }
            break;
        case BYTE_END:
            recordSum += byte;
            if (is_hex_char(input)){
                /* set upper part of byte */
                byte = 0;
                byte = hex_to_dec(input) << 4;

                state = BYTE_START;
            } else if (isalnum(input)) {
                log_error("Expected uppercase hex digit.", line, col, input);
                return false;
            } else {
                /* no more bytes, check the CRC */
                if (!check_crc(recordSum)){
                    log_error("Wrong CRC.", line, col, (int)recordSum);
                    return false;
                }
                state = COMMENT;
            }

            /* overwrite the state = COMMENT in case of : */
            if (input == ':') {
                state = NEW_RECORD;
            }
            break;
        default:
            fprintf(stderr, "Unknown FSA state: %i\n", state);
            return false;
            break;
        }/* switch state */

        /* update context information */
        col++;
        if (input == '\n'){
            line++;
            col = 1;
        }
    }/* while input */

    /* empty file */
    if (line == 1 && col == 1){
        fprintf(stderr, "ERROR: empty file.\n");
        return false;
    }

    if (cnt == 0){
        fprintf(stderr, "ERROR: No record found.\n");
        return false;
    }
    return true;
}/* check_file */

int main(int argc, char *argv[])
{
    /* check mandatory argument */
    if (argc != 2){
        fprintf(stderr, "Error: missing argument. Use %s file.hex\n", argv[0]);
        return 1;
    }

    int rc = 0;
    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL){
        perror(filename);
        return 1;
    }

    if (!check_file(file)){
        rc = 1;
    }

    fclose(file);

    return rc;
}/* main */
