#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <lotto_ta.h>
#include <sodium.h>
#include <string.h>

static uint8_t g_sk[crypto_sign_ed25519_SECRETKEYBYTES];
static bool g_sk_initialized = false;

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("Lotto TA has been called");
	return TEE_SUCCESS;
}


void TA_DestroyEntryPoint(void)
{
	DMSG("Lotta TA DestroyEntryPoint has been called");
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("Lotto TA: OpenSessionEntryPoint called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	(void)&params;
	(void)&sess_ctx;

	IMSG("Lotto TA: Session opened");

	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx;
	IMSG("Lotto TA: Session closed");
}

static TEE_Result cmd_generate_keys(uint32_t param_types, TEE_Param params[4])
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_OUTPUT,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE,
        TEE_PARAM_TYPE_NONE
    );
    
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    unsigned char seed[32];
    TEE_GenerateRandom(seed, sizeof(seed));

    unsigned char pk[32]; // 32
    unsigned char sk_expanded[64]; // 64

    if (crypto_vrf_keypair_from_seed(pk, sk_expanded, seed) != 0) {
        EMSG("crypto_sign_ed25519_seed_keypair failed");
        return TEE_ERROR_GENERIC;
    }

    memcpy(g_sk, sk_expanded, sizeof(sk_expanded));
    g_sk_initialized = true;

    if (params[0].memref.size < sizeof(pk)) {
        return TEE_ERROR_SHORT_BUFFER;
    }
    
    memcpy(params[0].memref.buffer, pk, sizeof(pk));
    params[0].memref.size = sizeof(pk);

    return TEE_SUCCESS;
}

static TEE_Result cmd_generate_randomness(uint32_t param_types,
                                          TEE_Param params[4])
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_MEMREF_OUTPUT,  // randomness (hash)
        TEE_PARAM_TYPE_MEMREF_OUTPUT,  // proof
        TEE_PARAM_TYPE_NONE
    );
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    if (!g_sk_initialized)
        return TEE_ERROR_BAD_STATE;

    unsigned char proof[crypto_vrf_PROOFBYTES];
    unsigned char randomness[crypto_vrf_OUTPUTBYTES];
    
    if (crypto_vrf_prove(proof, g_sk, 
    			 (const unsigned char*)params[0].memref.buffer,
                         (unsigned long long)params[0].memref.size) != 0) {
        EMSG("VRF prove failed");
        return TEE_ERROR_GENERIC;
    }

    if (crypto_vrf_proof_to_hash(randomness, proof) != 0) {
        EMSG("VRF proof_to_hash failed");
        return TEE_ERROR_GENERIC;
    }

    if (params[1].memref.size < crypto_vrf_OUTPUTBYTES || 
    	params[2].memref.size < crypto_vrf_PROOFBYTES)
        return TEE_ERROR_SHORT_BUFFER;

    // Copy the VRF output (randomness) and proof to the output parameters.
    memcpy(params[1].memref.buffer, randomness, crypto_vrf_OUTPUTBYTES);
    params[1].memref.size = crypto_vrf_OUTPUTBYTES;

    memcpy(params[2].memref.buffer, proof, crypto_vrf_PROOFBYTES);
    params[2].memref.size = crypto_vrf_PROOFBYTES;

    return TEE_SUCCESS;
}

static TEE_Result cmd_verify_randomness(uint32_t param_types,
                                        TEE_Param params[4])
{
    uint32_t exp_param_types = TEE_PARAM_TYPES(
        TEE_PARAM_TYPE_MEMREF_INPUT,
        TEE_PARAM_TYPE_MEMREF_INPUT, // public key 32 byte
        TEE_PARAM_TYPE_MEMREF_INPUT, // randomness 32 byte
        TEE_PARAM_TYPE_MEMREF_INPUT // proof 64 byte
    );
    if (param_types != exp_param_types)
        return TEE_ERROR_BAD_PARAMETERS;

    if (params[1].memref.size != crypto_vrf_PUBLICKEYBYTES ||
        params[2].memref.size != crypto_vrf_OUTPUTBYTES ||
        params[3].memref.size != crypto_vrf_PROOFBYTES) {
        return TEE_ERROR_BAD_PARAMETERS;
    }

    unsigned char beta[crypto_vrf_OUTPUTBYTES];
    if (crypto_vrf_verify(beta,
                          (const unsigned char *)params[1].memref.buffer, 
                          (const unsigned char *)params[3].memref.buffer,
                          (const unsigned char *)params[0].memref.buffer,
                          (unsigned long long)params[0].memref.size) != 0) {
        EMSG("VRF verification failed");
        return TEE_ERROR_GENERIC;
    }
    
    if (memcmp(beta, params[2].memref.buffer, crypto_vrf_OUTPUTBYTES) != 0) {
        EMSG("VRF output mismatch");
        return TEE_ERROR_BAD_PARAMETERS;
    }
    
    return TEE_SUCCESS;
}

TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
                                     uint32_t cmd_id,
                                     uint32_t param_types,
                                     TEE_Param params[4]) {
    (void)sess_ctx;
    (void)param_types;

    switch (cmd_id) {
    case TA_LOTTO_CMD_GENERATE_KEYS:
        return cmd_generate_keys(param_types, params);
    case TA_LOTTO_CMD_GENERATE_RANDOMNESS:
        return cmd_generate_randomness(param_types, params);
    case TA_LOTTO_CMD_VERIFY_RANDOMNESS:
        return cmd_verify_randomness(param_types, params);
    default:
        return TEE_ERROR_BAD_PARAMETERS;
    }
}
