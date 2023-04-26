#ifndef HEADERFILE_H
#define HEADERFILE_H

// import std library
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
// import apr library
#include <apr-1.0/apr.h>
#include <apr-1.0/apr_hash.h>
#include <apr-1.0/apr_general.h>
#include <apr-1.0/apr_file_io.h>
#include <apr-1.0/apr_strings.h>

// import json lib
#include <json-c/json.h>

#define DEBUG_MODE

#define DICTIONARY_FILE     "dictionary.tlv"
#define OUTPUT_FILE         "output.tlv"
#define INPUT_FILE          "input.json"

/**
* @remark Define TLV types
* @warning Don't use 0x00 to avoid problems EOF
*/

#define TAG_KEY             1
#define TLV_TYPE_BOOL       0x01
#define TLV_TYPE_INT        0x02
#define TLV_TYPE_STRING     0x03

#define MAX_LINE_LENGTH     4096
#define MAX_KEY_LENGTH      10

// Define dictionary entry struct
typedef struct
{
    char *key;
    int32_t value;
} DictionaryEntry;

void write_tlv_data(apr_file_t *file, int32_t type, int32_t length, const void *value);
void write_dictionary(apr_file_t *file, apr_hash_t *dictionary);
void write_json_to_tlv_file(json_object *json, apr_file_t *file);

#endif
