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

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <hello_world_ta.h>
#include <string.h>
#include <stdlib.h>
/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
									TEE_Param __maybe_unused params[4],
									void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

static TEE_Result inc_value(uint32_t param_types,
							TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
											   TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a++;
	IMSG("Increase value to: %u", params[0].value.a);

	IMSG("Got value1: %u from NW", params[1].value.a);

	return TEE_SUCCESS;
}

static TEE_Result dec_value(uint32_t param_types,
							TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	IMSG("Got value: %u from NW", params[0].value.a);
	params[0].value.a--;
	IMSG("Decrease value to: %u", params[0].value.a);

	return TEE_SUCCESS;
}

static TEE_Result tee_time_wait(uint32_t param_types, TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
	DMSG("tee_time_wait has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	uint32_t wait_ms = params[0].value.a;
	IMSG("tee_time_wait wait %d ms", wait_ms);

	return TEE_Wait(wait_ms);
}

static TEE_Result invoke_multi_command_1(uint32_t param_types, TEE_Param params[4])
{
	TEE_Result ret = TEE_SUCCESS;
	uint8_t *buf = NULL;
	uint32_t buf_size = 0;
	// uint8_t *buf_out = NULL;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
											   TEE_PARAM_TYPE_MEMREF_OUTPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	buf = params[0].memref.buffer;
	buf_size = params[0].memref.size;
	// buf_out = params[1].memref.buffer;
	IMSG("buf_size is %d", buf_size);
	if (buf_size != 10 * 1024)
	{
		EMSG("buf_size is not 10 * 1024");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	IMSG("buf[0]: 0x%x", buf[0]);
	if (buf[0] == 0x22)
	{
		memset(buf, 0x23, buf_size);
	}
	IMSG("after buf[0]: 0x%x", buf[0]);
	TEE_MemMove(params[1].memref.buffer, buf, buf_size);
	params[1].memref.size = buf_size;
	return ret;
}

static TEE_Result invoke_multi_command_2(uint32_t param_types, TEE_Param params[4])
{
	TEE_Result ret = TEE_SUCCESS;
	uint8_t *buf = NULL;
	uint32_t buf_size = 0;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
											   TEE_PARAM_TYPE_MEMREF_OUTPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	buf = params[0].memref.buffer;
	buf_size = params[0].memref.size;
	IMSG("buf_size is %d", buf_size);
	if (buf_size != 10 * 1024)
	{
		EMSG("buf_size is not 10 * 1024");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	IMSG("buf[0]: 0x%x", buf[0]);
	if (buf[0] == 0x33)
		memset(buf, 0x34, buf_size);
	IMSG("after buf[0]: 0x%x", buf[0]);
	// params[1].memref.buffer = buf;
	TEE_MemMove(params[1].memref.buffer, buf, buf_size);
	params[1].memref.size = buf_size;
	return ret;
}

static TEE_Result invoke_share_mem_test(uint32_t param_types, TEE_Param params[4])
{
	TEE_Result ret = TEE_SUCCESS;
	uint8_t *buf = NULL;
	uint32_t buf_size = 0;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	buf = (uint8_t *)params[0].memref.buffer;
	buf_size = params[0].memref.size;
	IMSG("buf_size is %d", buf_size);
	if (buf_size != 10 * 1024)
	{
		EMSG("buf_size is not 10 * 1024");
		return TEE_ERROR_BAD_PARAMETERS;
	}
	// IMSG("buf[0]: 0x%x", buf[0]);
	// if (buf[0] == 0x22)
	// 	// memset(buf, 0x34, buf_size + 1);
	// 	TEE_MemFill(buf, 0x34, buf_size);
	// buf = TEE_Malloc(buf_size + 1, 0);
	TEE_MemFill(buf, 0x34, buf_size);
	IMSG("after buf[0]: 0x%x\n", buf[0]);
	// params[0].memref.buffer = buf;
	params[0].memref.size = params[0].memref.size;
	// TEE_Free(buf);
	return ret;
}

const char *SFS_FILE_NAME = "sfs_write_test_01";

static TEE_Result sfs_write_test(uint32_t param_types, TEE_Param params[4])
{
	TEE_Result ret = TEE_SUCCESS;
	uint8_t *buf = NULL;
	uint32_t buf_size = 0;
	TEE_ObjectHandle object = TEE_HANDLE_NULL;
	uint32_t flags = TEE_DATA_FLAG_ACCESS_WRITE | TEE_DATA_FLAG_ACCESS_WRITE_META;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	buf = (uint8_t *)params[0].memref.buffer;
	buf_size = params[0].memref.size;
	// 创建或者覆盖
	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE,
									 SFS_FILE_NAME,
									 strlen(SFS_FILE_NAME),
									 flags,
									 NULL,
									 0,
									 &object);
	if (res != TEE_SUCCESS)
	{
		EMSG("TEE_CreatePersistentObject failed 0x%x", res);
		return res;
	}
	// 写入数据
	EMSG("write data to sfs: %s", buf);
	res = TEE_WriteObjectData(object, buf, buf_size);
	if (res != TEE_SUCCESS)
	{
		EMSG("TEE_WriteObjectData failed 0x%x", res);
		TEE_CloseObject(object);
		return res;
	}
	// 关闭对象
	TEE_CloseObject(object);

	// 验证写入
	object = TEE_HANDLE_NULL;
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
								   SFS_FILE_NAME,
								   strlen(SFS_FILE_NAME),
								   TEE_DATA_FLAG_ACCESS_READ,
								   &object);
	if (res != TEE_SUCCESS)
	{
		EMSG("TEE_OpenPersistentObject failed 0x%x", res);
		return res;
	}
	char *read_buf = TEE_Malloc(buf_size, 0);
	if (read_buf)
	{
		TEE_SeekObjectData(object, 0, TEE_DATA_SEEK_SET);
		TEE_ReadObjectData(object, read_buf, buf_size, &buf_size);
		// 确保字符串正确终止
		if (buf_size > 0)
		{
			read_buf[buf_size - 1] = '\0';
		}
		EMSG("read data from sfs: %s", read_buf);
	}
	else
	{
		EMSG("TEE_Malloc failed 0x%x", res);
	}
	TEE_CloseObject(object);
	TEE_Free(read_buf);
	return res;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
									  uint32_t cmd_id,
									  uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id)
	{
	case TA_HELLO_WORLD_CMD_INC_VALUE:
		return inc_value(param_types, params);
	case TA_HELLO_WORLD_CMD_DEC_VALUE:
		return dec_value(param_types, params);
	case TA_EHLLO_WORLD_CMD_TIME_WAIT:
		return tee_time_wait(param_types, params);
	case TA_HELLO_WORLD_CMD_MULTI_COMMAND1:
		return invoke_multi_command_1(param_types, params);
	case TA_HELLO_WORLD_CMD_MULTI_COMMAND2:
		return invoke_multi_command_2(param_types, params);
	case TA_HELLO_WORLD_CMD_SHARE_MEM_TEST:
		return invoke_share_mem_test(param_types, params);
	case TA_CMD_SFS_WRITE_TEST:

	case TA_CMD_SFS_WRITE_TEST:
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
