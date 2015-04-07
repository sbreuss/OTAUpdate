#include "OTAUtils.h"
#include <LTask.h>

#include "vmche.h"
#include "vmio.h"
#include "vmchset.h"

struct vm_hashsum_ctx {
	char name[OTA_MAX_PATH_LEN];
	char digest[DIGEST_SIZE_CHAR];
	vm_che_type type;
	VMUINT type_size;
	
    boolean result;
    vm_hashsum_ctx(const char* name, vm_che_type type, VMUINT type_size) {
        this->result = false;
		this->type = type;
		this->type_size = type_size;
		strcpy(this->name, name);
		memset(this->digest, 0, DIGEST_SIZE_CHAR);
    }
};

struct vm_copy_ctx {
	char src[OTA_MAX_PATH_LEN];
	char dst[OTA_MAX_PATH_LEN];
	boolean result;
	
	vm_copy_ctx(const char* src, const char* dst) {
		this->result = false;
		strncpy(this->src, src, OTA_MAX_PATH_LEN);
		strncpy(this->dst, dst, OTA_MAX_PATH_LEN);
	}
};

boolean vm_hashsum(const char* name, char* digest, vm_che_type type, VMUINT type_size) {
	VMWCHAR src[OTA_MAX_PATH_LEN];
	VMFILE handle;
	VMUINT size, read, done;
	vm_stche che_ctx;
	
	uint8_t buffer[DIGEST_SIZE_BUFFER];
	uint8_t che_hash[DIGEST_SIZE_MAX];
	boolean result = true;

	DEBUG_UTILS("vm_md5sum - checking file %s\r\n", name);
	if (vm_ascii_to_ucs2(src, OTA_MAX_PATH_LEN*sizeof(VMWCHAR), (VMSTR) name) < 0) {
		DEBUG_UTILS("vm_md5sum - error converting path to VMWCHAR\r\n");
		return false;
	}
	if((handle = vm_file_open(src, MODE_READ, true)) < 0) {
		DEBUG_UTILS("vm_md5sum - error opening file\r\n");
		return false;
	}
	if(vm_file_getfilesize(handle, &size) != 0) {
		DEBUG_UTILS("vm_md5sum - error getting file size");
		result = false;
		goto vm_md5sum_cleanup;
	}

	// initialize the che context and hash
	memset(che_hash, 0, type_size);
	vm_che_init(&che_ctx, type);
	done = 0;
	while(done < size) {
		vm_file_read(handle, buffer, DIGEST_SIZE_BUFFER, &read);
		done += read;
		vm_che_process(&che_ctx, type, VM_CHE_MODE_NULL, VM_CHE_HASH, buffer, che_hash, read, done == size);
	}

	for(int i=0; i<type_size; i++) {
		sprintf(&digest[i*2], "%02x", che_hash[i]);
	}
	digest[2*type_size] = 0;
	
	result = true;
	vm_che_deinit(&che_ctx);
	
vm_md5sum_cleanup:
	vm_file_close(handle);
	return result;
}


boolean vm_md5sum(const char* name, char* digest) {
	vm_hashsum(name, digest, VM_CHE_MD5, DIGEST_SIZE_MD5);
	DEBUG_UTILS("vm_md5sum - %s: %s\r\n", name, digest);
	return true;
}

boolean vm_sha256sum(const char* name, char* digest) {
	vm_hashsum(name, digest, VM_CHE_SHA256, DIGEST_SIZE_SHA256);
	DEBUG_UTILS("vm_sha256sum - %s: %s\r\n", name, digest);
	return true;
}

boolean vm_hashsum_wrap(void* userData) {
	vm_hashsum_ctx* ctx = (vm_hashsum_ctx*) userData;
	ctx->result = vm_hashsum(ctx->name, ctx->digest, ctx->type, ctx->type_size);
	return true;
}

boolean md5sum(const char* name, char* digest) {
	vm_hashsum_ctx ctx = vm_hashsum_ctx(name, VM_CHE_MD5, DIGEST_SIZE_MD5);
	LTask.remoteCall(vm_hashsum_wrap, &ctx);
	
	if(ctx.result) {
		strcpy(digest, ctx.digest);
		DEBUG_UTILS("vm_md5sum - %s: %s\r\n", name, digest);
		return true;
	} else {
		return false;
	}
}

boolean sha256sum(const char* name, char* digest) {
	vm_hashsum_ctx ctx = vm_hashsum_ctx(name, VM_CHE_SHA256, DIGEST_SIZE_SHA256);
	LTask.remoteCall(vm_hashsum_wrap, &ctx);
	
	if(ctx.result) {
		strcpy(digest, ctx.digest);
		DEBUG_UTILS("vm_sha256sum - %s: %s\r\n", name, digest);
		return true;
	} else {
		return false;
	}	
}

extern void vm_reboot_normal_start(void);

void vm_reset(void) {
	DEBUG_UTILS("calling vm_reboot_normal_start()\r\n");
	delay(500);
	vm_reboot_normal_start();
}

boolean vm_reset_wrap(void* userData) {
	vm_reset();
	return true;
}

void reset(void) {
	LTask.remoteCall(vm_reset_wrap, NULL);
}


