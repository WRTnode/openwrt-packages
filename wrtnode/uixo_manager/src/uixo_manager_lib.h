#ifndef __UIXO_MANAGER_LIB_H__
#define __UIXO_MANAGER_LIB_H__

#include <libubox/list.h>

struct uixo_object {
    /* internal used */
    struct list_head list;
    pid_t pid;

    /* interface */
    const char* name;
    const char* description;
    struct list_head* methods_head;
    int methods_count;
    const char* agent;
    unsigned int port;
    const char* addr;

    /* functions */
    /* object->methods */
    int (*find_method_by_name)(struct list_head* head, const char* name);
    const char* (*get_method_name)(struct list_head* head, const int method_n);
    const char* (*get_method_handler)(struct list_head* head, const int method_n);
    const struct blobmsg_policy* (*get_method_policy)(struct list_head* head, const int method_n);
    int (*get_method_n_policy)(struct list_head* head, const int method_n);
    enum blobmsg_type (*get_method_return_type)(struct list_head* head, const int method_n, const int ret_n);
    int (*get_method_return_n)(struct list_head* head, const int method_n);
    /* object->method->data */
    const char* (*get_method_cmd)(struct list_head* head, const int method_n);
    const char* (*get_method_policy_name)(struct list_head* head, const int method_n, const int policy_n);
};

#if 0
struct uixo_subscribe {
    struct list_head list;

    const char* name;
    const char* cmd;
    const char* callback;
//TODO: need return?
    struct list_head* rets_head;
    int rets_count;
};
#endif

void uixo_manager_print_attr(struct blob_attr* attr);

/*
 *  UIXO objects function
 */
struct uixo_object*
uixo_manager_blob_to_obj(struct blob_attr* attr);
void
uixo_manager_free_obj(struct uixo_object* obj);

/*
 *  UIXO objects agent function
 */
int uixo_manager_agent_read
(unsigned int port, const char* addr, char* ptr, const int size);
int uixo_manager_agent_write
(unsigned int port, const char* addr, const char* str, const int size);

#if 0
/*
 *  UIXO subscribes function
 */
struct uixo_subscribe*
uixo_manager_blob_to_subscribe(struct blob_attr* attr);
void
uixo_manager_free_subscribe(struct uixo_subscribe* subscribe);
#endif

#endif
