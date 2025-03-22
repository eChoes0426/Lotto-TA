#include <err.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <tee_client_api.h>
#include <lotto_ta.h>

#define MESSAGE "Test Message"

int main(void)
{
    TEEC_Result res;
    TEEC_Context ctx;
    TEEC_Session sess;
    TEEC_Operation op;
    TEEC_UUID uuid = LOTTO_TA_UUID;
    uint32_t err_origin;

    // Initialize the context to connect to the TEE
    res = TEEC_InitializeContext(NULL, &ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    // Open a session to the Lotto TA
    res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_OpenSession failed with code 0x%x origin 0x%x",
             res, err_origin);

    // -------------------------------------------------------------------------
    // Test 1: Generate Keys (public key output)
    // -------------------------------------------------------------------------
    unsigned char public_key[32] = {0};

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_OUTPUT,
        TEEC_NONE, TEEC_NONE, TEEC_NONE
    );
    op.params[0].tmpref.buffer = public_key;
    op.params[0].tmpref.size   = sizeof(public_key);

    res = TEEC_InvokeCommand(&sess, TA_LOTTO_CMD_GENERATE_KEYS, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "Generate keys failed with code 0x%x origin 0x%x", res, err_origin);

    printf("Public Key: ");
    for (int i = 0; i < sizeof(public_key); i++) {
        printf("%02x", public_key[i]);
    }
    printf("\n");

    // -------------------------------------------------------------------------
    // Test 2: Generate VRF Randomness and Proof for a message
    // -------------------------------------------------------------------------
    unsigned char randomness[64] = {0};
    unsigned char proof[80] = {0};
    const char *message = MESSAGE;
    size_t message_len = strlen(message);

    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_INPUT,    // Input message
        TEEC_MEMREF_TEMP_OUTPUT,   // VRF output (randomness)
        TEEC_MEMREF_TEMP_OUTPUT,   // VRF proof
        TEEC_NONE
    );
    op.params[0].tmpref.buffer = (void *)message;
    op.params[0].tmpref.size   = message_len;
    op.params[1].tmpref.buffer = randomness;
    op.params[1].tmpref.size   = sizeof(randomness);
    op.params[2].tmpref.buffer = proof;
    op.params[2].tmpref.size   = sizeof(proof);

    res = TEEC_InvokeCommand(&sess, TA_LOTTO_CMD_GENERATE_RANDOMNESS, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "Generate randomness failed with code 0x%x origin 0x%x", res, err_origin);

    printf("VRF Randomness: ");
    for (int i = 0; i < sizeof(randomness); i++) {
        printf("%02x", randomness[i]);
    }
    printf("\n");

    printf("VRF Proof: ");
    for (int i = 0; i < sizeof(proof); i++) {
        printf("%02x", proof[i]);
    }
    printf("\n");

    // -------------------------------------------------------------------------
    // Test 3: Verify VRF Proof and Randomness for the same message
    // -------------------------------------------------------------------------
    memset(&op, 0, sizeof(op));
    op.paramTypes = TEEC_PARAM_TYPES(
        TEEC_MEMREF_TEMP_INPUT,  // message
        TEEC_MEMREF_TEMP_INPUT,  // public key
        TEEC_MEMREF_TEMP_INPUT,  // expected randomness
        TEEC_MEMREF_TEMP_INPUT   // proof
    );
    op.params[0].tmpref.buffer = (void *)message;
    op.params[0].tmpref.size   = message_len;
    op.params[1].tmpref.buffer = public_key;
    op.params[1].tmpref.size   = sizeof(public_key);
    op.params[2].tmpref.buffer = randomness;
    op.params[2].tmpref.size   = sizeof(randomness);
    op.params[3].tmpref.buffer = proof;
    op.params[3].tmpref.size   = sizeof(proof);

    res = TEEC_InvokeCommand(&sess, TA_LOTTO_CMD_VERIFY_RANDOMNESS, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        fprintf(stderr, "VRF verification failed with code 0x%x origin 0x%x\n", res, err_origin);
    } else {
        printf("VRF verification succeeded.\n");
    }

    // -------------------------------------------------------------------------
    // Cleanup: Close session and finalize context
    // -------------------------------------------------------------------------
    TEEC_CloseSession(&sess);
    TEEC_FinalizeContext(&ctx);
    return 0;
}

