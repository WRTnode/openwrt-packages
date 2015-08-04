#include <assert.h>

extern unsigned long mem_count;
static inline void* u_calloc(size_t n, size_t size){
	void* ret;
	ret = calloc(n, size);
	assert(NULL != ret);
	mem_count ++;
	return ret;
}

static inline void u_free(void* ptr){
	free(ptr);
	mem_count --;
}

static inline char* u_strcpy(const char* src){
	char* dest = NULL;
	int len = strlen(src);
	if(len > 0) {
		dest = (char*)u_calloc(1,len+1);
		return strcpy(dest, src);
	}
	else {
		return NULL;
	}
}
