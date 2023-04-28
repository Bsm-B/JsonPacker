/**
 * @file mainc.C
 * @brief C Program: JSON Packer
 * @author Bessem Bousselmi
 */

#include "main.h"

// Function to write TLV encoded data to file
void write_tlv_data(apr_file_t *file, int32_t type, int32_t length, const void *value)
{
    apr_file_write_full(file, &type, sizeof(int32_t), NULL);
    apr_file_write_full(file, &length, sizeof(int32_t), NULL);
    apr_file_write_full(file, value, length, NULL);
}

// Function to encode and write dictionary to file
void write_dictionary(apr_file_t *file, apr_hash_t *dictionary)
{
    apr_hash_index_t *hi;
    for (hi = apr_hash_first(NULL, dictionary); hi; hi = apr_hash_next(hi))
    {
        char *key;
        int32_t value;
        apr_hash_this(hi, &key, NULL, &value);
#ifdef DEBUG_MODE
        printf("%s => %d\n", (char *)key, (int32_t *)value);
#endif
        int32_t key_length = strlen(key) + 1; // Include null terminator
        write_tlv_data(file, TLV_TYPE_STRING, key_length, key);
        write_tlv_data(file, TLV_TYPE_INT, sizeof(int32_t), &value);
    }
}

// Function to encode and write json to file
void write_json_to_tlv_file(json_object *json, apr_file_t *file)
{

    int8_t val_type;
    int8_t object_type;

    // Loop through JSON object and write TLV-encoded values to file
    json_object_object_foreach(json, key, val)
    {

        // Determine value type and encode TLV accordingly
        val_type = json_object_get_type(val);
        const void *value;
        apr_size_t value_size;

        switch (val_type)
        {

        case json_type_boolean:

            object_type = TLV_TYPE_BOOL;
            int8_t bool_val = json_object_get_boolean(val) ? 1 : 0;
#ifdef DEBUG_MODE
            printf("boolean: %d\n", bool_val);
#endif
            value = &bool_val;
            value_size = sizeof(int8_t);
            break;

        case json_type_int:

            object_type = TLV_TYPE_INT;
            int32_t int_val = json_object_get_int(val);
#ifdef DEBUG_MODE
            printf("int: %d\n", int_val);
#endif
            value = &int_val;
            value_size = sizeof(int32_t);
            break;

        case json_type_string:

            object_type = TLV_TYPE_STRING;
            const char *str_val = (char *)json_object_get_string(val);
#ifdef DEBUG_MODE
            printf("string: %s\n", str_val);
#endif
            value = str_val;
            value_size = strlen(str_val) + 1;
            break;

        default:
#ifdef DEBUG_MODE
            printf("error get object value");
#endif
            continue;
            break;
        }
        // Write TLV-encoded value to file
        // uint8_t tag = TAG_KEY;
        // apr_file_write_full(file, &tag, 1, NULL); // Write tag
        apr_file_write_full(file, key, strlen(key) + 1, NULL);         // Write key
        apr_file_write_full(file, &object_type, sizeof(int8_t), NULL); // Write type
        apr_file_write_full(file, value, value_size, NULL);            // Write value
    }
}

int main()
{

    /**
     * @brief Program
     *
     * 1. Read the input file line by line, parsing each line as JSON using the "json-c" library.
     * 2. For each parsed JSON object, create a dictionary mapping the keys to integers.
     * 3. Replace the keys in the JSON object with the corresponding integers from the dictionary.
     * 4. Encode the resulting JSON object using TLV encoding and write it to the output file.
     * 5. Also encode the dictionary using TLV encoding and write it to a separate file.
     *
     */

    // Init ARP
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);

    // Create dictionary
    apr_hash_t *dictionary = apr_hash_make(pool);
    int32_t next_key = 1;

    apr_status_t status;

    // Open input file for reading
    apr_file_t *input_file;
    // Open output file for writing
    apr_file_t *output_file_json;

    status = apr_file_open(&input_file, INPUT_FILE, APR_FOPEN_READ, APR_OS_DEFAULT, pool);
    if (status != APR_SUCCESS)
    {
        printf("Error 01: error opening 'input.json' file \n");
        return -1;
    }

    // APR_FOPEN_BINARY This flag is ignored on UNIX
    status = apr_file_open(&output_file_json, OUTPUT_FILE, APR_FOPEN_CREATE | APR_TRUNCATE | APR_FOPEN_WRITE | APR_FOPEN_APPEND, APR_OS_DEFAULT, pool);
    if (status != APR_SUCCESS)
    {
        printf("Error 02 : error opening json output file\n");
        return -1;
    }

    // Read input file line by line
    char line[MAX_LINE_LENGTH]; // Assume maximum line length of 4096 characters
    apr_size_t len = MAX_LINE_LENGTH;
    ssize_t read;

    while ((read = apr_file_gets(line, len, input_file)) != APR_EOF)
    {
        // Parse JSON object from line
        json_object *object = json_tokener_parse(line);

#ifdef DEBUG_MODE
        printf("input: %s \n", line);
#endif

        if (object == NULL)
        {
            printf("Failed to parse JSON object: %s\n", line);
            continue;
        }

        // Create dictionary entries for each key in JSON object
        json_object_object_foreach(object, key, val)
        {

            // Check if key is already in dictionary
            DictionaryEntry *entry = apr_hash_get(dictionary, key, APR_HASH_KEY_STRING);
            if (entry == NULL)
            {
                // Add new entry to dictionary
                entry = apr_palloc(pool, sizeof(DictionaryEntry));
                entry->key = apr_pstrdup(pool, key);
                entry->value = next_key;
                apr_hash_set(dictionary, entry->key, APR_HASH_KEY_STRING, entry->value);
                next_key++;
            }
        }

        apr_hash_index_t *hi;
        for (hi = apr_hash_first(pool, dictionary); hi; hi = apr_hash_next(hi))
        {
            const char *old_key = apr_hash_this_key(hi);
            int32_t *new_key = apr_hash_this_val(hi);
            // Check if the old key exists in the JSON object
            
            json_object *value = json_object_object_get(object, old_key);

            if (value != NULL)
            {

                char str[MAX_KEY_LENGTH];
                sprintf(str, "%d", new_key);

                json_type value_type = json_object_get_type(value);
                if (value_type == json_type_null) {
                    
                continue;
                    
                } else if (value_type == json_type_boolean) {
                    
                json_object_object_add(object, str, json_object_new_boolean( json_object_get_boolean(value)));

                } else if (value_type == json_type_int) {
                 
                json_object_object_add(object, str, json_object_new_int( json_object_get_int(value)));
               
                } else if (value_type == json_type_string) {
               
                 json_object_object_add(object, str, json_object_new_string( json_object_get_string(value)));
                
                }
                // Remove the old key from the JSON object
                json_object_object_del(object, old_key);

            }
        }


#ifdef DEBUG_MODE
        // Serialize the JSON object to a string
        const char *new_json_str = json_object_to_json_string(object);
        printf("the final is resultat is \n");
        printf("%s\n", new_json_str);
#endif
        write_json_to_tlv_file(object, output_file_json);
        json_object_put(object);
    }

    // Open output file for writing
    apr_file_t *output_file;
    // APR_FOPEN_BINARY This flag is ignored on UNIX
    status = apr_file_open(&output_file, DICTIONARY_FILE, APR_FOPEN_CREATE | APR_FOPEN_WRITE | APR_FOPEN_TRUNCATE, APR_OS_DEFAULT, pool);
    if (status != APR_SUCCESS)
    {
        printf("Error 04 : error opening dictionary output file\n");
        return -1;
    }

    write_dictionary(output_file, dictionary);

    // Close file and cleanup
    apr_file_close(input_file);
    apr_file_close(output_file);
    apr_file_close(output_file_json);

    apr_pool_destroy(pool);
    apr_terminate();

    return 0;
}
