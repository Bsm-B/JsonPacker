#include <CUnit/Basic.h>
#include <apr-1.0/apr.h>
#include <apr-1.0/apr_file_io.h>

#define MAX_BUFFER_SIZE 1024


// Function to write TLV encoded data to file
void write_tlv_data(apr_file_t *file, int32_t type, int32_t length, const void *value)
{
    apr_file_write_full(file, &type, sizeof(int32_t), NULL);
    apr_file_write_full(file, &length, sizeof(int32_t), NULL);
    apr_file_write_full(file, value, length, NULL);
}


// Define a test function for the write_tlv_data function
void test_write_tlv_data()
{   

    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool, NULL);

    // Create a temporary file to write the TLV data to
    apr_file_t *file;
    apr_status_t status;
    status = apr_file_open(&file, "test_tlv_file.bin", APR_FOPEN_CREATE | APR_FOPEN_WRITE | APR_FOPEN_TRUNCATE, APR_OS_DEFAULT, pool);
    CU_ASSERT_EQUAL(status, APR_SUCCESS);


    // Read the TLV data from the file and verify that it matches the expected values
    apr_file_t *read_file;
    status = apr_file_open(&read_file, "test_tlv_file.bin", APR_FOPEN_READ, APR_OS_DEFAULT, pool);
    CU_ASSERT_EQUAL(status, APR_SUCCESS);
    // Define some test data to write using TLV encoding
    int32_t type = 0x12345678;
    int32_t length = 6;
    char value[] = "Hello!";

    // Write the test data to the file using TLV encoding
    write_tlv_data(file, type, length, value);


    int32_t read_type, read_length;
    char read_value[7];

    apr_file_read_full(read_file, &read_type, sizeof(int32_t), NULL);
    apr_file_read_full(read_file, &read_length, sizeof(int32_t), NULL);
    apr_file_read_full(read_file, read_value, length, NULL);

    CU_ASSERT_EQUAL(read_type, type);
    CU_ASSERT_EQUAL(read_length, length);
    CU_ASSERT_STRING_EQUAL(read_value, value);
    // Close the file
    apr_file_close(file);
    apr_file_close(read_file);

}


int main() {


    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    CU_pSuite suite = CU_add_suite("Suite", NULL, NULL);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }



    if ((NULL == CU_add_test(suite, "test of write_tlv_data()", test_write_tlv_data))) {
        CU_cleanup_registry();
        return CU_get_error();
    }


    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
