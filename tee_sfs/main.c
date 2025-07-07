/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <hello_world_ta.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
static const uint8_t random_count = 64;

/**
 * @brief Test function demonstrating basic TEE (Trusted Execution Environment) operations.
 *
 * This function performs the following sequence of operations:
 * 1. Initializes a TEE context
 * 2. Opens a session with the "hello world" TA (Trusted Application)
 * 3. Invokes commands in the TA to:
 *    - Process an input value (increment operation)
 *    - Wait for a specified time period
 * 4. Closes the session and finalizes the context
 *
 * The function demonstrates error handling for TEE operations and shows how to:
 * - Pass parameters to TA commands
 * - Handle random conditions in TA operations
 * - Perform timing operations in the TA
 *
 * @return TEEC_Result - Returns TEEC_SUCCESS on success, or a TEE error code on failure.
 */
static void test1()
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
	uint32_t err_origin;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/*
	 * Open a session to the "hello world" TA, the TA will print "hello
	 * world!" in the log when the session is created.
	 */
	printf("Opening a session to the TA\n");
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
						   TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			 res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, in this case
	 * we're incrementing a number.
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */
	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	srand((unsigned int)time(NULL));
	uint8_t random_result = rand() % 199;
	printf("random_count: %d, random_result: %d \n", random_count, random_result);

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_VALUE_INPUT,
									 TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = 42;

	if (random_count == random_result)
	{
		op.params[1].value.a = 1;
	}
	else
	{
		op.params[1].value.a = 0;
	}

	/*
	 * TA_HELLO_WORLD_CMD_INC_VALUE is the actual function in the TA to be
	 * called.
	 */
	printf("Invoking TA to increment %d\n", op.params[0].value.a);
	res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_INC_VALUE, &op,
							 &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			 res, err_origin);
	printf("TA incremented value to %d\n", op.params[0].value.a);

	printf("invoke TA to wait 0.5s\n");
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
									 TEEC_NONE,
									 TEEC_NONE,
									 TEEC_NONE);
	uint32_t wait_time_ms = 500;
	op.params[0].value.a = wait_time_ms;
	res = TEEC_InvokeCommand(&sess, TA_EHLLO_WORLD_CMD_TIME_WAIT, &op,
							 &err_origin);
	if (res)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			 res, err_origin);
	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);
}

static void call_multi_command()
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
	uint32_t err_origin;
	uint8_t *data1 = NULL;
	uint8_t *data2 = NULL;

	// initialize a context connecting us to the TEE
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res)
	{
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
		return;
	}
	// open a session to the "hello world" TA
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res)
	{
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
		return;
	}
	// send commands to TA
	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));
	data1 = malloc(10 * 1024);
	if (!data1)
	{
		err(1, "Cannot allocate data1");
		return;
	}
	memset(data1, 0x22, 10 * 1024);
	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
									 TEEC_MEMREF_TEMP_OUTPUT,
									 TEEC_NONE,
									 TEEC_NONE);

	op.params[0].tmpref.buffer = data1;
	op.params[0].tmpref.size = 10 * 1024;
	op.params[1].tmpref.buffer = malloc(10 * 1024);
	res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_MULTI_COMMAND1, &op, &err_origin);
	if (res)
	{
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
		return;
	}
	printf("out put data[0]: 0x%x\n", ((uint8_t *)op.params[1].tmpref.buffer)[0]);
	printf("out put data len: %d\n", op.params[1].tmpref.size);

	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);

	// initialize a context connecting us to the TEE
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res)
	{
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
		return;
	}
	// open a session to the "hello world" TA
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res)
	{
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
		return;
	}

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));
	data2 = malloc(10 * 1024);
	if (!data2)
	{
		err(1, "Cannot allocate data1");
		return;
	}
	memset(data2, 0x33, sizeof(data2));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
									 TEEC_MEMREF_TEMP_OUTPUT,
									 TEEC_NONE,
									 TEEC_NONE);
	op.params[0].tmpref.buffer = data2;
	op.params[0].tmpref.size = 10 * 1024;
	op.params[1].tmpref.buffer = malloc(10 * 1024);
	res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_MULTI_COMMAND1, &op, &err_origin);
	if (res)
	{
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
		return;
	}

	// // result = op.params[1].tmpref.buffer;
	printf("out put data[0]: ox%x\n", ((uint8_t *)op.params[1].tmpref.buffer)[0]);
	printf("out put data len: %d\n", op.params[1].tmpref.size);

	// // close the session and destroy the context
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
}

static void share_mem_test()
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
	uint32_t err_origin;
	TEEC_SharedMemory shm;

	// initialize a context connecting us to the TEE
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res)
	{
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
		return;
	}
	// open a session to the "hello world" TA
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res)
	{
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
		return;
	}

	shm.size = 10 * 1024;
	shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	res = TEEC_AllocateSharedMemory(&ctx, &shm);
	if (res)
	{
		TEEC_CloseSession(&sess);
		TEEC_FinalizeContext(&ctx);
		errx(1, "TEEC_AllocateSharedMemory failed with code 0x%x", res);
		return;
	}
	uint8_t *data = shm.buffer;
	memset(data, 0x22, shm.size);
	// send commands to TA
	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));
	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INOUT,
									 TEEC_NONE,
									 TEEC_NONE,
									 TEEC_NONE);

	op.params[0].memref.parent = &shm;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = shm.size;

	res = TEEC_InvokeCommand(&sess, TA_HELLO_WORLD_CMD_SHARE_MEM_TEST, &op, &err_origin);
	if (res)
	{
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
		goto end;
	}
	printf("out put data[0]: 0x%x\n", data[0]);
	printf("out put data len: %d\n", shm.size);

end:
	// free(data);
	// // close the session and destroy the context
	TEEC_ReleaseSharedMemory(&shm);
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
}

/**
 * @brief Main entry point for the hello_world host application.
 *
 * Executes test1() and call_multi_command() functions before exiting.
 */
// int main(void)
// {
// 	// test1();
// 	// call_multi_command();
// 	share_mem_test();
// 	return 0;
// }

// SFS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>

static void test_sfs_write(int argc, char *argv[])
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_SFS_UUID;
	uint32_t err_origin;
	TEEC_SharedMemory shm;

	const char *data_to_write = argv[2];
	printf("data_to_write: %s\n", data_to_write);
	size_t data_to_write_size = strlen(data_to_write);

	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
	{
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
		return;
	}
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
	{
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
		return;
	}
	memset(&shm, 0, sizeof(shm));
	shm.size = data_to_write_size;
	shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	res = TEEC_AllocateSharedMemory(&ctx, &shm);
	if (res)
	{
		TEEC_CloseSession(&sess);
		TEEC_FinalizeContext(&ctx);
		errx(1, "TEEC_AllocateSharedMemory failed with code 0x%x", res);
		return;
	}
	memcpy(shm.buffer, data_to_write, data_to_write_size);

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INOUT,
									 TEEC_NONE,
									 TEEC_NONE,
									 TEEC_NONE);
	op.params[0].memref.parent = &shm;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = data_to_write_size;

	res = TEEC_InvokeCommand(&sess, TA_CMD_SFS_WRITE_TEST, &op, &err_origin);
	if (res != TEEC_SUCCESS)
	{
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
		goto end;
	}

	// release
	TEEC_ReleaseSharedMemory(&shm);
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
	return;
}

static void sfs_read_test(int argc, char *argv[])
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_SFS_UUID;
	uint32_t err_origin;
	TEEC_SharedMemory shm;

	// const char *data_to_write = argv[2];
	// printf("data_to_write: %s\n", data_to_write);
	// size_t data_to_write_size = strlen(data_to_write);

	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
	{
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);
		return;
	}
	res = TEEC_OpenSession(&ctx, &sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
	{
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
		return;
	}
	memset(&shm, 0, sizeof(shm));
	shm.size = MAX_BUFFER_SIZE;
	shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;
	res = TEEC_AllocateSharedMemory(&ctx, &shm);
	if (res)
	{
		TEEC_CloseSession(&sess);
		TEEC_FinalizeContext(&ctx);
		errx(1, "TEEC_AllocateSharedMemory failed with code 0x%x", res);
		return;
	}
	// memcpy(shm.buffer, data_to_write, data_to_write_size);

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_PARTIAL_INOUT,
									 TEEC_NONE,
									 TEEC_NONE,
									 TEEC_NONE);
	op.params[0].memref.parent = &shm;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = MAX_BUFFER_SIZE;

	res = TEEC_InvokeCommand(&sess, TA_CMD_SFS_READ_TEST, &op, &err_origin);
	if (res != TEEC_SUCCESS)
	{
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
		goto end;
	}

	// release
	TEEC_ReleaseSharedMemory(&shm);
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
	return;
}

int main(int argc, char *argv[])
{
	if (argc == 3)
	{
		stfs_write_test(argc, argv);
	}
	else
	{
		sfs_read_test(argc, argv);
	}
}