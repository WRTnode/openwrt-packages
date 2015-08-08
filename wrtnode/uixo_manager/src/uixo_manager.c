#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <pthread.h>

#include <libubox/blobmsg_json.h>
#include <libubox/list.h>
#include "libubus.h"
#include "utilities.h"
#include "uixo_manager_lib.h"

#define DEBUG

#ifdef DEBUG
#define DBG_PRINT          fprintf
#else
#define DBG_PRINT(...)
#endif

#define MAX_BUF_LEN        512

static struct ubus_context* ctx;
static const char* ubus_socket = NULL;
LIST_HEAD(objects_list);
LIST_HEAD(regwaits_list);

/*
 * Method: mkobj
 */
static inline void
make_object_add_object_to_list
(struct list_head* head, struct uixo_object* object)
{
    list_add_tail(&object->list, head);
}

static int
make_object_add_object_to_ubus(struct uixo_object* obj, const char* conf_file_path)
{
    /* should make a new process. pass conf_file_path new process */
    pid_t pid = fork();

    if(pid < 0) {
        fprintf(stderr,"Add object fork error!\n");
        return -1;
    }
    else if (pid > 0) { //father
        obj->pid = pid;
        DBG_PRINT(stdout,"New object: %s pid=%d!\n",obj->name, pid);
        return 0;
    }
    else { //child
        if(execl("/bin/uixo_manager_fork_object",
                 "uixo_manager_fork_object",
                 "-f",
                 conf_file_path,
                 NULL) <0) {
            fprintf(stderr,"Exec uixo_manager_object_fork error!\n");
            return -1;
        }
        else
            return 0;
    }
}

enum {
    MAKE_OBJECT_CONFIG_FILENAME,
    __MAKE_OBJECT_MAX
};

static const struct blobmsg_policy make_object_policy[] = {
    [MAKE_OBJECT_CONFIG_FILENAME] = { .name = "config file", .type = BLOBMSG_TYPE_STRING }
};

static int
make_object_handler
(struct ubus_context* ctx, struct ubus_object* obj,
 struct ubus_request_data* req, const char* method,
 struct blob_attr* msg)
{
    struct uixo_object* uixo_obj = NULL;
    const char* conf_file_path = NULL;
    static struct blob_buf conf_blob_buf;
    struct blob_attr* tb[__MAKE_OBJECT_MAX];
    int ret = UBUS_STATUS_OK;

    blobmsg_parse(make_object_policy, ARRAY_SIZE(make_object_policy), tb, blob_data(msg), blob_len(msg));
    if(tb[MAKE_OBJECT_CONFIG_FILENAME])
        conf_file_path = blobmsg_get_string(tb[MAKE_OBJECT_CONFIG_FILENAME]);

    memset(&conf_blob_buf, 0 ,sizeof(conf_blob_buf));
    blobmsg_buf_init(&conf_blob_buf);
    if(!blobmsg_add_json_from_file(&conf_blob_buf, conf_file_path)) {
        fprintf(stderr, "failed to convert %s to blobmsg!\n", conf_file_path);
        ret = UBUS_STATUS_INVALID_ARGUMENT;
        goto out;
    }

    uixo_obj = uixo_manager_blob_to_obj(conf_blob_buf.head);

    if(NULL == uixo_obj) {
        fprintf(stderr, "Can not make object!\n");
        ret = UBUS_STATUS_INVALID_ARGUMENT;
        goto out;
    }
    /* make new object on ubus */
    make_object_add_object_to_ubus(uixo_obj, conf_file_path);
	/* save the new object */
	make_object_add_object_to_list(&objects_list, uixo_obj);

out:
    if(conf_blob_buf.buf)
        free(conf_blob_buf.buf);
    return ret;
}


/*
 * Method: rmobj
 */
static struct uixo_object*
find_uixo_object_from_list_by_name(struct list_head* head, const char* name)
{
    struct uixo_object* object = NULL;
    list_for_each_entry(object, &objects_list, list) {
        if(!strcmp(name, object->name)) {
            return object;
        }
    }
    return NULL;
}

static void
stop_uixo_object(pid_t pid)
{
    if(kill(pid, SIGKILL)<0) {
        fprintf(stderr, "failed to kill %d\n", pid);
    }
}

static void
free_uixo_object(struct uixo_object* obj)
{
    list_del(&obj->list);
    uixo_manager_free_obj(obj);
}

enum {
    REMOVE_OBJECT_NAME,
    __REMOVE_OBJECT_MAX
};

static const struct blobmsg_policy remove_object_policy[] = {
    [REMOVE_OBJECT_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING }
};

static int
remove_object_handler
(struct ubus_context* ctx, struct ubus_object* obj,
 struct ubus_request_data* req, const char* method,
 struct blob_attr* msg)
{
    struct uixo_object* uixo_obj = NULL;
    struct blob_attr* tb[__REMOVE_OBJECT_MAX];
    const char* obj_name = NULL;
    int ret = UBUS_STATUS_OK;

    blobmsg_parse(remove_object_policy, ARRAY_SIZE(remove_object_policy), tb, blob_data(msg), blob_len(msg));
    if(tb[REMOVE_OBJECT_NAME])
        obj_name = blobmsg_get_string(tb[REMOVE_OBJECT_NAME]);

    uixo_obj = find_uixo_object_from_list_by_name(&objects_list, obj_name);
    if(NULL == uixo_obj) {
        fprintf(stderr, "Can not find object: %s!\n", obj_name);
        ret = UBUS_STATUS_INVALID_ARGUMENT;
        goto out;
    }
    else {
        stop_uixo_object(uixo_obj->pid);
        free_uixo_object(uixo_obj);
    }

out:
    return ret;
}

/*
 * Method: regwait
 */
struct uixo_manager_regwait {
    struct list_head list;

    const char* name;
    const char* cmd;
    const char* callback;
    int wait_times;

    struct ubus_event_handler* ev;
    int got_times;
};

static inline struct uixo_manager_regwait*
get_regwait_from_list_by_name(struct list_head* head, const char* name)
{
    struct uixo_manager_regwait* rw = NULL;
    list_for_each_entry(rw, head, list) {
        if(!strcmp(name, rw->name)) {
            return rw;
        }
    }
    return NULL;
}

static void
free_uixo_regwait(struct uixo_manager_regwait* rw)
{
    list_del(&rw->list);
    u_free(rw->ev);
    u_free(rw);
}

static void
reg_wait_remove(struct uixo_manager_regwait* rw)
{
    ubus_unregister_event_handler(ctx, rw->ev);
    free_uixo_regwait(rw);
}

enum {
    REG_WAIT_CB_DATA,
    __REG_WAIT_CB_MAX
};

static const struct blobmsg_policy reg_wait_cb_policy[] = {
    [REG_WAIT_CB_DATA] = { .name = "broadcast_data", .type = BLOBMSG_TYPE_STRING },
};

static void
reg_wait_cb
(struct ubus_context* ctx, struct ubus_event_handler* ev, const char* name,
 struct blob_attr* msg)
{
    const char* data = NULL;
    struct uixo_manager_regwait* rw = NULL;
    char buf[MAX_BUF_LEN] = {0};
    struct blob_attr* tb[__REG_WAIT_CB_MAX];

    rw = get_regwait_from_list_by_name(&regwaits_list, name);
    if(NULL == rw) {
        fprintf(stderr, "Failed to find reg wait event: %s\n", name);
        return;
    }

    blobmsg_parse(reg_wait_cb_policy, ARRAY_SIZE(reg_wait_cb_policy), tb, blob_data(msg), blob_len(msg));
    if(tb[REG_WAIT_CB_DATA])
        data = blobmsg_get_string(tb[REG_WAIT_CB_DATA]);

    strcpy(buf, rw->callback);
    strcat(buf, " ");
    if(NULL != rw->cmd) {
        strcat(buf, rw->cmd);
        strcat(buf, " ");
    }
    if(NULL != data) {
        strcat(buf, data);
    }

    system(buf);

    if(rw->wait_times != 0) {
        rw->got_times++;
        if(rw->got_times == rw->wait_times) {
            reg_wait_remove(rw);
        }
    }
}

static void
reg_wait_add_event_to_ubus(struct uixo_manager_regwait* rw)
{
    struct ubus_event_handler* event_hd = NULL;
    int ret = 0;

    event_hd = (struct ubus_event_handler*)u_calloc(1, sizeof(*event_hd));
    event_hd->cb = reg_wait_cb;
    rw->ev = event_hd;
    ret = ubus_register_event_handler(ctx, event_hd, rw->name);
    if(ret) {
        fprintf(stderr, "Failed to add reg wait event: %s\n", ubus_strerror(ret));
    }
}

static inline void
reg_wait_add_to_list
(struct list_head* head, struct uixo_manager_regwait* rw)
{
    list_add_tail(&rw->list, head);
}

enum {
    REG_WAIT_NAME,
    REG_WAIT_CMD,
    REG_WAIT_CALLBACK,
    REG_WAIT_TIMES,
    __REG_WAIT_MAX
};

static const struct blobmsg_policy reg_wait_policy[] = {
    [REG_WAIT_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [REG_WAIT_CMD] = { .name = "cmd", .type = BLOBMSG_TYPE_STRING},
    [REG_WAIT_CALLBACK] = { .name = "callback", .type = BLOBMSG_TYPE_STRING},
    [REG_WAIT_TIMES] = { .name = "times", .type = BLOBMSG_TYPE_INT32},
};

static int
reg_wait_handler
(struct ubus_context* ctx, struct ubus_object* obj,
 struct ubus_request_data* req, const char* method,
 struct blob_attr* msg)
{
    struct blob_attr* tb[__REG_WAIT_MAX];
    int ret = UBUS_STATUS_OK;
    struct uixo_manager_regwait* rw;
    int i = 0;

    blobmsg_parse(reg_wait_policy, ARRAY_SIZE(reg_wait_policy), tb, blob_data(msg), blob_len(msg));
    for(i=0; i<__REG_WAIT_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__REG_WAIT_MAX == i) {
        DBG_PRINT(stderr, "error reg wait parameters\n");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    rw = (struct uixo_manager_regwait*)u_calloc(1, sizeof(*rw));

    if(tb[REG_WAIT_NAME])
        rw->name = u_strcpy(blobmsg_get_string(tb[REG_WAIT_NAME]));
    if(tb[REG_WAIT_CMD])
        rw->cmd = u_strcpy(blobmsg_get_string(tb[REG_WAIT_CMD]));
    if(tb[REG_WAIT_CALLBACK])
        rw->callback = u_strcpy(blobmsg_get_string(tb[REG_WAIT_CALLBACK]));
    if(tb[REG_WAIT_TIMES])
        rw->wait_times = blobmsg_get_u32(tb[REG_WAIT_TIMES]);

    /* add new subscribe on ubus */
    reg_wait_add_event_to_ubus(rw);
    /* save the new subscribe */
    reg_wait_add_to_list(&regwaits_list, rw);

    return ret;
}

/*
 * Method: broadcast
 */
struct uixo_manager_broadcast {
    const char* name;
    const char* data;
};

enum {
    BROADCAST_NAME,
    BROADCAST_DATA,
    __BROADCAST_MAX
};

static const struct blobmsg_policy broadcast_policy[] = {
    [BROADCAST_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
    [BROADCAST_DATA] = { .name = "data", .type = BLOBMSG_TYPE_STRING },
};

static int
broadcast_send(struct uixo_manager_broadcast* bc)
{
    struct ubus_context* ctx_send = NULL;
    struct blob_buf* b = NULL;
    int ret = UBUS_STATUS_OK;

    ctx_send = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Broadcast failed to connect to ubus\n");
        return -1;
    }
    if(NULL != bc->data) {
        b = (struct blob_buf*)u_calloc(1, sizeof(*b));
        blob_buf_init(b, 0);
        if(blobmsg_add_string(b, "broadcast_data", bc->data)!=0) {
            fprintf(stderr, "Failed to add data\n");
            return UBUS_STATUS_INVALID_ARGUMENT;
        }
    }
    if((ret = ubus_send_event(ctx_send, bc->name, b->head))!=0) {
        fprintf(stderr, "Failed to send event:%s\n",ubus_strerror(ret));
    }
    u_free(b);
    ubus_free(ctx_send);

    return 0;
}

static int
broadcast_handler
(struct ubus_context* ctx, struct ubus_object* obj,
 struct ubus_request_data* req, const char* method,
 struct blob_attr* msg)
{
    struct blob_attr* tb[__BROADCAST_MAX];
    struct uixo_manager_broadcast bc;
    int i = 0;
    int ret = UBUS_STATUS_OK;

    blobmsg_parse(broadcast_policy, ARRAY_SIZE(broadcast_policy), tb, blob_data(msg), blob_len(msg));
    for(i=0; i<__BROADCAST_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__BROADCAST_MAX == i) {
        DBG_PRINT(stderr, "error broadcast parameters\n");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    if(tb[BROADCAST_NAME])
        bc.name = blobmsg_get_string(tb[BROADCAST_NAME]);
    if(tb[BROADCAST_DATA])
        bc.data = blobmsg_get_string(tb[BROADCAST_DATA]);

    broadcast_send(&bc);
    return ret;
}

#ifdef UIXO_USE_RAGENT
/*
 * Method: ragent
 */
struct uixo_manager_read_agent {
    const char* obj_name;
    int data_len;
};

enum {
    READ_AGENT_OBJECT_NAME,
    READ_AGENT_DATA_LEN,
    __READ_AGENT_MAX
};

static const struct blobmsg_policy read_agent_policy[] = {
    [READ_AGENT_OBJECT_NAME] = { .name = "objname", .type = BLOBMSG_TYPE_STRING },
    [READ_AGENT_DATA_LEN] = { .name = "len", .type = BLOBMSG_TYPE_INT32 },
};

static int
read_agent(struct uixo_manager_read_agent* ra)
{
    struct uixo_object* obj = find_uixo_object_from_list_by_name(&objects_list, ra->obj_name);
    if((NULL == obj->agent) || (0 == obj->port) || (NULL == obj->addr)) {
        fprintf(stderr, "Read agent err: No agent!\n");
        return -1;
    }
    char* buf = (char*)u_calloc(ra->data_len+1, sizeof(char));
    uixo_manager_agent_read(obj->port, obj->addr, buf, ra->data_len);
    fprintf(stdout, "%s\n", buf);
    u_free(buf);
    return 0;
}

static int
read_agent_handler
(struct ubus_context* ctx, struct ubus_object* obj,
 struct ubus_request_data* req, const char* method,
 struct blob_attr* msg)
{
    struct blob_attr* tb[__READ_AGENT_MAX];
    struct uixo_manager_read_agent ra;
    int i = 0;
    int ret = UBUS_STATUS_OK;

    blobmsg_parse(read_agent_policy, ARRAY_SIZE(read_agent_policy), tb, blob_data(msg), blob_len(msg));
    for(i=0; i<__READ_AGENT_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__READ_AGENT_MAX == i) {
        DBG_PRINT(stderr, "error ragent parameters\n");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    if(tb[READ_AGENT_OBJECT_NAME])
        ra.obj_name = blobmsg_get_string(tb[READ_AGENT_OBJECT_NAME]);
    if(tb[READ_AGENT_DATA_LEN])
        ra.data_len = blobmsg_get_u32(tb[READ_AGENT_DATA_LEN]);

    read_agent(&ra);
    return ret;
}
#endif

#ifdef UIXO_USE_WAGENT
/*
 * Method: wagent
 */
struct uixo_manager_write_agent {
    const char* obj_name;
    char* data;
    int data_len;
};

enum {
    WRITE_AGENT_OBJECT_NAME,
    WRITE_AGENT_DATA,
    WRITE_AGENT_DATA_LEN,
    __WRITE_AGENT_MAX
};

static const struct blobmsg_policy write_agent_policy[] = {
    [WRITE_AGENT_OBJECT_NAME] = { .name = "objname", .type = BLOBMSG_TYPE_STRING },
    [WRITE_AGENT_DATA] = { .name = "data", .type = BLOBMSG_TYPE_STRING },
    [WRITE_AGENT_DATA_LEN] = { .name = "len", .type = BLOBMSG_TYPE_INT32 },
};

static int
write_agent(struct uixo_manager_write_agent* wa)
{
    struct uixo_object* obj = find_uixo_object_from_list_by_name(&objects_list, wa->obj_name);
    if((NULL == obj) || (NULL == obj->agent) || (0 == obj->port) || (NULL == obj->addr)) {
        fprintf(stderr, "Write agent err: No agent!\n");
        return -1;
    }
    uixo_manager_agent_write(obj->port, obj->addr, wa->data, wa->data_len);
    return 0;
}

static int
write_agent_handler
(struct ubus_context* ctx, struct ubus_object* obj,
 struct ubus_request_data* req, const char* method,
 struct blob_attr* msg)
{
    struct blob_attr* tb[__WRITE_AGENT_MAX];
    struct uixo_manager_write_agent wa;
    int i = 0;
    int ret = UBUS_STATUS_OK;

    blobmsg_parse(write_agent_policy, ARRAY_SIZE(write_agent_policy), tb, blob_data(msg), blob_len(msg));
    for(i=0; i<__WRITE_AGENT_MAX; i++) {
        if(NULL != tb[i])
            break;
    }
    if(__WRITE_AGENT_MAX == i) {
        DBG_PRINT(stderr, "error wagent parameters\n");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }

    if(tb[WRITE_AGENT_OBJECT_NAME])
        wa.obj_name = blobmsg_get_string(tb[WRITE_AGENT_OBJECT_NAME]);
    if(tb[WRITE_AGENT_DATA])
        wa.data = blobmsg_get_string(tb[WRITE_AGENT_DATA]);
    if(tb[WRITE_AGENT_DATA_LEN])
        wa.data_len = blobmsg_get_u32(tb[WRITE_AGENT_DATA_LEN]);

    write_agent(&wa);
    return ret;
}
#endif

/*
 * uixo_manager_main
 */
static const struct ubus_method uixo_manager_methods[] = {
    UBUS_METHOD("mkobj", make_object_handler, make_object_policy),
    UBUS_METHOD("rmobj", remove_object_handler, remove_object_policy),
    UBUS_METHOD("regwait", reg_wait_handler, reg_wait_policy),
    UBUS_METHOD("broadcast", broadcast_handler, broadcast_policy),
#ifdef UIXO_USE_RAGENT
    UBUS_METHOD("ragent", read_agent_handler, read_agent_policy),
#endif
#ifdef UIXO_USE_WAGENT
    UBUS_METHOD("wagent", write_agent_handler, write_agent_policy),
#endif
};

static struct ubus_object_type uixo_manager_object_type =
    UBUS_OBJECT_TYPE("uixo_manager", uixo_manager_methods);

static struct ubus_object uixo_manager_object = {
    .name = "uixo_manager",
    .type = &uixo_manager_object_type,
    .methods = uixo_manager_methods,
    .n_methods = ARRAY_SIZE(uixo_manager_methods),
};

static void uixo_manager_main(void)
{
    int ret;

    ret = ubus_add_object(ctx, &uixo_manager_object);
    if (ret)
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
    uloop_run();
}

static void
free_uixo_objects(struct list_head* head)
{
    struct uixo_object* uixo_obj = NULL;
    while(!list_empty(head)) {
        uixo_obj = list_first_entry(head, struct uixo_object, list);
        free_uixo_object(uixo_obj);
    }
}

static void
free_uixo_regwaits(struct list_head* head)
{
    struct uixo_manager_regwait* uixo_regwait = NULL;
    while(!list_empty(head)) {
        uixo_regwait = list_first_entry(head, struct uixo_manager_regwait, list);
        free_uixo_regwait(uixo_regwait);
    }
}
#if 0
static void
free_subscribes(struct list_head* head)
{
    struct uixo_subscribe* subscribe = NULL;
    while(!list_empty(head)) {
        subscribe = list_first_entry(head, struct uixo_subscribe, list);
        //free_subscribe(subscribe);
        list_del(&subscribe->list);
        uixo_manager_free_subscribe(subscribe);
    }
}
#endif
int main(int argc, char** argv)
{
    int ch;
    while ((ch = getopt(argc, argv, "s:")) != -1) {
        switch (ch) {
            case 's':
                ubus_socket = optarg;
                break;
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;

    uloop_init();
    signal(SIGPIPE, SIG_IGN);

    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
    }

    ubus_add_uloop(ctx);

    uixo_manager_main();

    free_uixo_objects(&objects_list);
    free_uixo_regwaits(&regwaits_list);
    ubus_free(ctx);
    uloop_done();
    return 0;
}

