#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include <libubox/blobmsg_json.h>
#include <libubox/list.h>
#include "utilities.h"
#include "uixo_manager_lib.h"

void uixo_manager_print_attr(struct blob_attr* attr)
{
    fprintf(stderr, "type: %d\n",blobmsg_type(attr));
    fprintf(stderr, "name: %s\n",blobmsg_name(attr));
}

static int
get_list_count(struct list_head* head)
{
    int count = 0;
    struct list_head* list = head;
    if(NULL == head) {
        return 0;
    }
    while(!list_is_last(list, head)) {
        count++;
        list = list->next;
    }
    return count;
}

static struct list_head*
list_n(struct list_head* head, const int n)
{
    int i = n;
    struct list_head* list_ptr = head;

    while(i--) {
        list_ptr = list_ptr->next;
    }
    return list_ptr;
}

/*
 * UIXO value
 */
struct uixo_value {
    struct list_head list;

    const char* name;
    const char* type;
    char* val;
};

enum {
    UIXO_VALUE_NAME,
    UIXO_VALUE_TYPE,
    UIXO_VALUE_VAL,
    __UIXO_VALUE_MAX
};

static const struct blobmsg_policy uixo_value_policy[] = {
    [UIXO_VALUE_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [UIXO_VALUE_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
    [UIXO_VALUE_VAL] = { .name = "val", .type = BLOBMSG_TYPE_STRING }
};

static inline void
add_value_to_list
(struct list_head* head, struct uixo_value* value)
{
    list_add_tail(&value->list, head);
}

static struct uixo_value*
blob_to_value(struct blob_attr* attr)
{
    struct uixo_value* value = NULL;
    struct blob_attr* tb[__UIXO_VALUE_MAX];
    int i = 0;

    if(blobmsg_parse(uixo_value_policy, ARRAY_SIZE(uixo_value_policy), tb, blob_data(attr), blob_len(attr)) < 0) {
        goto blob_to_value_out;
    }
    for(i=0; i<__UIXO_VALUE_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__UIXO_VALUE_MAX == i) {
        return NULL;
    }
    value = (struct uixo_value*)u_calloc(1, sizeof(*value));

    if(tb[UIXO_VALUE_NAME])
        value->name = blobmsg_get_string(tb[UIXO_VALUE_NAME]);
    if(tb[UIXO_VALUE_TYPE])
        value->type = blobmsg_get_string(tb[UIXO_VALUE_TYPE]);
    if(tb[UIXO_VALUE_VAL])
        value->val = blobmsg_get_string(tb[UIXO_VALUE_VAL]);

blob_to_value_out:
    return value;
}

static struct list_head*
blob_to_values(struct blob_attr* attr)
{
    struct list_head* values_list_head = NULL;
    struct uixo_value* tmp_value = NULL;
    struct blob_attr* cur = NULL;
    struct blob_attr* head = blobmsg_data(attr);
    int data_len = blobmsg_data_len(attr);
    if(data_len <= 0) {
        return NULL;
    }

    values_list_head = (struct list_head*)u_calloc(1, sizeof(*values_list_head));
    INIT_LIST_HEAD(values_list_head);

    __blob_for_each_attr(cur, head, data_len) {
        tmp_value = blob_to_value(blob_data(cur));
        if(NULL != tmp_value) {
            add_value_to_list(values_list_head, tmp_value);
        }
        tmp_value = NULL;
    }

    return values_list_head;
}

static inline void
__free_value(struct uixo_value* value)
{
    u_free(value);
}

static inline void
__free_values(struct list_head* head)
{
    struct uixo_value* tmp_value = NULL;
    while((NULL!=head) && (!list_empty(head))) {
        tmp_value = list_first_entry(head, struct uixo_value, list);
        list_del(&tmp_value->list);
        __free_value(tmp_value);
    }
    u_free(head);
}

static struct uixo_value*
__get_argument_from_list(struct list_head* head, int n)
{
    if(n > get_list_count(head)) {
        return NULL;
    }
    return list_entry(list_n(head,n), struct uixo_value, list);
}

static enum blobmsg_type
__get_argument_type(struct list_head* head, const int n)
{
    struct uixo_value* argument = __get_argument_from_list(head, n+1);

    if(!strcmp(argument->type,"string")) {
        return BLOBMSG_TYPE_STRING;
    }
    else if(!strcmp(argument->type,"int")) {
        return BLOBMSG_TYPE_INT32;
    }
    else {
        fprintf(stderr,"Only support string and int type\n");
        return BLOBMSG_TYPE_UNSPEC;
    }
}

static const char*
__get_argument_name(struct list_head* head, int n)
{
    struct uixo_value* argument = __get_argument_from_list(head, n+1);
    return argument->name;
}

static const struct blobmsg_policy*
__arguments_to_policy(struct list_head* head, int n)
{
    struct blobmsg_policy* policy = NULL;
    struct blobmsg_policy* tmp_policy = NULL;
    int i = 0;

    policy = u_calloc(n, sizeof(*policy));
    tmp_policy = policy;
    for(i=0; i<n; i++) {
        tmp_policy->name = __get_argument_name(head, i);
        tmp_policy->type = __get_argument_type(head, i);
        tmp_policy++;
    }

    return policy;
}

/*
 * UIXO data
 */
struct uixo_object_data {
    const char* cmd;
    struct list_head* args_head;
    int args_count;
};

enum {
    UIXO_DATA_CMD,
    UIXO_DATA_ARGS,
    __UIXO_DATA_MAX
};

static const struct blobmsg_policy uixo_data_policy[] = {
    [UIXO_DATA_CMD] = { .name = "cmd", .type = BLOBMSG_TYPE_STRING },
    [UIXO_DATA_ARGS] = { .name = "arguments", .type = BLOBMSG_TYPE_ARRAY }
};

static struct uixo_object_data*
blob_to_datum(struct blob_attr* attr)
{
    struct uixo_object_data* datum = NULL;
    struct blob_attr* tb[__UIXO_DATA_MAX];
    int i;
    if(blobmsg_parse(uixo_data_policy,
                     ARRAY_SIZE(uixo_data_policy),
                     tb, blob_data(attr), blob_len(attr)) < 0) {
        goto blob_to_datum_out;
    }
    for(i=0; i<__UIXO_DATA_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__UIXO_DATA_MAX == i) {
        return NULL;
    }
    datum = (struct uixo_object_data*)u_calloc(1, sizeof(*datum));

    if(tb[UIXO_DATA_CMD])
        datum->cmd= blobmsg_get_string(tb[UIXO_DATA_CMD]);
    if(tb[UIXO_DATA_ARGS])
        datum->args_head = blob_to_values(tb[UIXO_DATA_ARGS]);

    datum->args_count= get_list_count(datum->args_head);

blob_to_datum_out:
    return datum;
}

#if 0
/*
 * UIXO subscribe
 */
enum {
    UIXO_SUBSCRIBE_NAME,
    UIXO_SUBSCRIBE_CMD,
    UIXO_SUBSCRIBE_CALLBACK,
    UIXO_SUBSCRIBE_RETURN,
    __UIXO_SUBSCRIBE_MAX
};

static const struct blobmsg_policy uixo_subscribe_policy[] = {
    [UIXO_SUBSCRIBE_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [UIXO_SUBSCRIBE_CMD] = { .name = "cmd", .type = BLOBMSG_TYPE_STRING },
    [UIXO_SUBSCRIBE_CALLBACK] = { .name = "callback", .type = BLOBMSG_TYPE_STRING },
    [UIXO_SUBSCRIBE_RETURN] = { .name = "return", .type = BLOBMSG_TYPE_ARRAY }
};

static inline void
add_subscribe_to_list
(struct list_head* head, struct uixo_subscribe* subscribe)
{
    list_add_tail(&subscribe->list, head);
}

struct uixo_subscribe*
uixo_manager_blob_to_subscribe(struct blob_attr* attr)
{
    struct uixo_subscribe* subscribe = NULL;
    struct blob_attr* tb[__UIXO_SUBSCRIBE_MAX];
    int i;

    if(blobmsg_parse(uixo_subscribe_policy,
                     ARRAY_SIZE(uixo_subscribe_policy),
                     tb, blob_data(attr), blob_len(attr)) < 0) {
        goto blob_to_subscribe_out;
    }
    for(i=0; i<__UIXO_SUBSCRIBE_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__UIXO_SUBSCRIBE_MAX == i) {
        return NULL;
    }

    subscribe = (struct uixo_subscribe*)u_calloc(1, sizeof(*subscribe));

    if(tb[UIXO_SUBSCRIBE_NAME])
        subscribe->name = blobmsg_get_string(tb[UIXO_SUBSCRIBE_NAME]);
    if(tb[UIXO_SUBSCRIBE_CMD])
        subscribe->cmd = blobmsg_get_string(tb[UIXO_SUBSCRIBE_CMD]);
    if(tb[UIXO_SUBSCRIBE_CALLBACK])
        subscribe->callback = blobmsg_get_string(tb[UIXO_SUBSCRIBE_CALLBACK]);
    if(tb[UIXO_SUBSCRIBE_RETURN])
        subscribe->rets_head = blob_to_values(tb[UIXO_SUBSCRIBE_RETURN]);

    subscribe->rets_count = get_list_count(subscribe->rets_head);

blob_to_subscribe_out:
    return subscribe;
}

static struct list_head*
blob_to_subscribes(struct blob_attr* attr)
{
    struct list_head* subscribes_list_head = NULL;
    struct uixo_subscribe* tmp_subscribe = NULL;
    struct blob_attr* cur = NULL;
    struct blob_attr* head = blobmsg_data(attr);
    int data_len = blobmsg_data_len(attr);
    if(data_len <= 0) {
        return NULL;
    }

    subscribes_list_head = (struct list_head*)u_calloc(1, sizeof(*subscribes_list_head));
    INIT_LIST_HEAD(subscribes_list_head);

    __blob_for_each_attr(cur, head, data_len) {
        tmp_subscribe = uixo_manager_blob_to_subscribe(blob_data(cur));
        if(NULL != tmp_subscribe) {
            add_subscribe_to_list(subscribes_list_head, tmp_subscribe);
        }
        tmp_subscribe = NULL;
    }

    return subscribes_list_head;
}

void
uixo_manager_free_subscribe(struct uixo_subscribe* subscribe)
{
    free_values(subscribe->rets_head);
    u_free(subscribe);
}

static inline void
free_subscribes(struct list_head* head)
{
    struct uixo_subscribe* tmp_subscribe = NULL;
    while((NULL!=head) && (!list_empty(head))) {
        tmp_subscribe = list_first_entry(head, struct uixo_subscribe, list);
        list_del(&tmp_subscribe->list);
        uixo_manager_free_subscribe(tmp_subscribe);
    }
    u_free(head);
}
#endif

/*
 * UIXO method
 */
struct uixo_object_method {
    struct list_head list;

    const char* name;
    struct uixo_object_data* data;
    const char* handler;
    struct list_head* rets_head;
    int rets_count;
};

enum {
    UIXO_METHOD_NAME,
    UIXO_METHOD_DATA,
    UIXO_METHOD_HANDLER,
    UIXO_METHOD_RETURN,
    __UIXO_METHOD_MAX
};

static const struct blobmsg_policy uixo_method_policy[] = {
    [UIXO_METHOD_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [UIXO_METHOD_DATA] = { .name = "data", .type = BLOBMSG_TYPE_TABLE },
    [UIXO_METHOD_HANDLER] = { .name = "handler", .type = BLOBMSG_TYPE_STRING },
    [UIXO_METHOD_RETURN] = { .name = "return", .type = BLOBMSG_TYPE_ARRAY }
};

static inline void
add_method_to_list
(struct list_head* head, struct uixo_object_method* method)
{
    list_add_tail(&method->list, head);
}

static inline struct uixo_object_data*
blob_to_method_data(struct blob_attr* attr)
{
    return blob_to_datum(blob_data(blob_data(attr)));
}

static inline void
__free_method(struct uixo_object_method* method)
{
	__free_values(method->data->args_head);
	u_free(method->data);
    __free_values(method->rets_head);
    u_free(method);
}

static inline bool
is_valid_method_handler(const char* handler){
	if(access(handler, X_OK|F_OK)==0){
		return true;
	}
	else{
		fprintf(stderr, "%s is an invalid method handler\n", handler);
		return false;
	}
}

static struct uixo_object_method*
blob_to_method(struct blob_attr* attr)
{
    struct uixo_object_method* method = NULL;
    struct blob_attr* tb[__UIXO_METHOD_MAX];
    int i = 0;

    if(blobmsg_parse(uixo_method_policy,
                     ARRAY_SIZE(uixo_method_policy),
                     tb, blob_data(attr), blob_len(attr)) < 0) {
        return NULL;
    }
    for(i=0; i<__UIXO_METHOD_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__UIXO_METHOD_MAX == i) {
        return NULL;
    }

    method = (struct uixo_object_method*)u_calloc(1, sizeof(*method));

    if(tb[UIXO_METHOD_NAME])
        method->name = blobmsg_get_string(tb[UIXO_METHOD_NAME]);
    if(tb[UIXO_METHOD_DATA])
        method->data= blob_to_method_data(tb[UIXO_METHOD_DATA]);
    if(tb[UIXO_METHOD_HANDLER])
        method->handler = blobmsg_get_string(tb[UIXO_METHOD_HANDLER]);
    if(tb[UIXO_METHOD_RETURN])
        method->rets_head = blob_to_values(tb[UIXO_METHOD_RETURN]);

    method->rets_count = get_list_count(method->rets_head);

    if((NULL == method->handler) || is_valid_method_handler(method->handler)) {
    	return method;
    }
    else {
    	__free_method(method);
    	return NULL;
    }
}

static struct list_head*
blob_to_methods(struct blob_attr* attr)
{
    struct list_head* methods_list_head = NULL;
    struct uixo_object_method* tmp_method = NULL;
    struct blob_attr* cur = NULL;
    struct blob_attr* head = blobmsg_data(attr);
    int data_len = blobmsg_data_len(attr);
    if(data_len <= 0) {
        return NULL;
    }

    methods_list_head = (struct list_head*)u_calloc(1, sizeof(*methods_list_head));
    INIT_LIST_HEAD(methods_list_head);

    __blob_for_each_attr(cur, head, data_len) {
        tmp_method = blob_to_method(blob_data(cur));
        if(NULL != tmp_method) {
            add_method_to_list(methods_list_head, tmp_method);
        }
        tmp_method = NULL;
    }

    return methods_list_head;
}

static inline void
__free_methods(struct list_head* head)
{
    struct uixo_object_method* tmp_method = NULL;
    while((NULL!=head) && (!list_empty(head))) {
        tmp_method = list_first_entry(head, struct uixo_object_method, list);
        list_del(&tmp_method->list);
        __free_method(tmp_method);
    }
    u_free(head);
}

static struct uixo_object_method*
__get_method_from_list(struct list_head* head, const int n)
{
    if(n > get_list_count(head)) {
        return NULL;
    }
    return list_entry(list_n(head,n), struct uixo_object_method, list);
}

static int
find_method_by_name(struct list_head* head, const char* name)
{
    struct uixo_object_method* method = NULL;
    int n = 0;
    list_for_each_entry(method, head, list) {
        n++;
        if(!strcmp(name, method->name)) {
            return n;
        }
    }
    return 0;
}

static const char*
get_method_name(struct list_head* head, const int method_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if(NULL!=method) {
        return method->name;
    }
    else {
        return NULL;
    }
}

static const char*
get_method_handler(struct list_head* head, const int method_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if(NULL!=method) {
        return method->handler;
    }
    else {
        return NULL;
    }
}

static const struct blobmsg_policy*
get_method_policy(struct list_head* head, const int method_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if((NULL!=method) && (0!=method->data->args_count)) {
        return __arguments_to_policy(method->data->args_head, method->data->args_count);
    }
    else {
        return NULL;
    }
}

static int
get_method_n_policy(struct list_head* head, const int method_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if(NULL!=method) {
    	return method->data->args_count;
    }
    else {
        return -1;
    }
}

static enum blobmsg_type
get_method_return_type(struct list_head* head, const int method_n, const int ret_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if((NULL!=method) && (0!=method->rets_count) && (ret_n<method->rets_count)) {
    	return __get_argument_type(method->rets_head, ret_n);
    }
    return BLOBMSG_TYPE_UNSPEC;
}

static int
get_method_return_n(struct list_head* head, const int method_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if(NULL!=method) {
        return method->rets_count;
    }
    else {
        return -1;
    }
}

static const char*
get_method_cmd(struct list_head* head, const int method_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if(NULL!=method) {
    	return method->data->cmd;
    }
    else {
        return NULL;
    }
}

static const char*
get_method_policy_name(struct list_head* head, const int method_n, const int policy_n)
{
    struct uixo_object_method* method = NULL;

    method = __get_method_from_list(head, method_n);
    if(NULL!=method) {
    	struct uixo_value* value = NULL;
    	value = __get_argument_from_list(method->data->args_head, policy_n);
        if(NULL!=value) {
            return value->name;
        }
        else {
            return NULL;
        }
    }
    else {
        return NULL;
    }
}

/*
 * UIXO object
 */
void
uixo_manager_free_obj(struct uixo_object* obj)
{
    __free_methods(obj->methods_head);
    u_free(obj);
}

static inline bool
is_valid_obj_agent(const char* agent){
	if(access(agent, X_OK|F_OK)==0){
		return true;
	}
	else{
		fprintf(stderr, "%s is an invalid uixo object agent\n", agent);
		return false;
	}
}

enum {
    UIXO_OBJECT_NAME,
    UIXO_OBJECT_DESCRIPTION,
    UIXO_OBJECT_METHODS,
    UIXO_OBJECT_AGENT,
    UIXO_OBJECT_PORT,
    UIXO_OBJECT_ADDR,
    __UIXO_OBJECT_MAX
};

static const struct blobmsg_policy uixo_object_policy[] = {
    [UIXO_OBJECT_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [UIXO_OBJECT_DESCRIPTION] = { .name = "description", .type = BLOBMSG_TYPE_STRING },
    [UIXO_OBJECT_METHODS] = { .name = "methods", .type = BLOBMSG_TYPE_ARRAY },
    [UIXO_OBJECT_AGENT] = { .name = "agent", .type = BLOBMSG_TYPE_STRING },
    [UIXO_OBJECT_PORT] = { .name = "port", .type = BLOBMSG_TYPE_INT32 },
    [UIXO_OBJECT_ADDR] = { .name = "addr", .type = BLOBMSG_TYPE_STRING }
};

struct uixo_object*
uixo_manager_blob_to_obj(struct blob_attr* attr)
{
    struct uixo_object* obj = NULL;
    struct blob_attr* tb[__UIXO_OBJECT_MAX];
    int i = 0;

    if(blobmsg_parse(uixo_object_policy,
                     ARRAY_SIZE(uixo_object_policy),
                     tb, blob_data(attr), blob_len(attr)) < 0) {
        return NULL;
    }
    for(i=0; i<__UIXO_OBJECT_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__UIXO_OBJECT_MAX == i) {
        return NULL;
    }

    obj = (struct uixo_object*)u_calloc(1, sizeof(*obj));
    obj->find_method_by_name = find_method_by_name;
    obj->get_method_name = get_method_name;
    obj->get_method_handler = get_method_handler;
    obj->get_method_policy = get_method_policy;
    obj->get_method_n_policy = get_method_n_policy;
    obj->get_method_return_type = get_method_return_type;
    obj->get_method_return_n = get_method_return_n;
    obj->get_method_cmd = get_method_cmd;
    obj->get_method_policy_name = get_method_policy_name;

    if(tb[UIXO_OBJECT_NAME])
        obj->name = blobmsg_get_string(tb[UIXO_OBJECT_NAME]);
    if(tb[UIXO_OBJECT_DESCRIPTION])
        obj->description = blobmsg_get_string(tb[UIXO_OBJECT_DESCRIPTION]);
    if(tb[UIXO_OBJECT_METHODS])
        obj->methods_head = blob_to_methods(tb[UIXO_OBJECT_METHODS]);
    if(tb[UIXO_OBJECT_AGENT])
        obj->agent = blobmsg_get_string(tb[UIXO_OBJECT_AGENT]);
    if(tb[UIXO_OBJECT_PORT])
        obj->port = blobmsg_get_u32(tb[UIXO_OBJECT_PORT]);
    if(tb[UIXO_OBJECT_ADDR])
        obj->addr = blobmsg_get_string(tb[UIXO_OBJECT_ADDR]);

    obj->methods_count = get_list_count(obj->methods_head);


    if((NULL == obj->agent) || is_valid_obj_agent(obj->agent)) {
    	return obj;
    }
    else {
    	uixo_manager_free_obj(obj);
    	return NULL;
    }
}

/*
 *  UIXO objects agent function
 */
static int
connect_retry(int sockfd, const struct sockaddr* addr, socklen_t alen)
{
#define MAXSLEEP  128
    int nsec;

    for(nsec=1; nsec<=MAXSLEEP; nsec<<=1) {
        if(connect(sockfd, addr, alen) == 0) {
            return 0;
        }
        if(nsec <= MAXSLEEP/2) {
            sleep(nsec);
        }
    }
    return -1;
}

int uixo_manager_agent_read
(unsigned int port, const char* addr, char* ptr, const int size)
{
    int sockfd;
    struct sockaddr_in server_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"can't connect to %s: %s\n",addr ,strerror(errno));
        return -1;
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, addr, &server_addr.sin_addr);
    if(connect_retry(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr,"can't connect to %s: %s\n",addr ,strerror(errno));
        return -1;
    }
    recv(sockfd, ptr, size, 0);
    close(sockfd);
    return 0;
}

int uixo_manager_agent_write
(unsigned int port, const char* addr, const char* str, const int size)
{
    int sockfd;
    struct sockaddr_in server_addr;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"can't connect to %s: %s\n",addr ,strerror(errno));
        return -1;
    }
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)port);
    inet_pton(AF_INET, addr, &server_addr.sin_addr);
    if(connect_retry(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr,"can't connect to %s: %s\n",addr ,strerror(errno));
        return -1;
    }
    send(sockfd, str, size, 0);
    close(sockfd);
    return 0;
}
