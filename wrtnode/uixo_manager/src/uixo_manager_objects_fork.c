#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>

#include <libubox/blobmsg_json.h>
#include <libubox/list.h>
#include "libubus.h"
#include "utilities.h"
#include "uixo_manager_lib.h"

#define MAX_BUF_LEN        512
#define MAX_ADDR_LEN       256

static struct ubus_context* ctx;
static struct uixo_object* uixo_obj;
static void usage(void)
{
    fprintf(stderr,
            "Usage: [<options>] [arguments...]\n"
            "Options:\n"
            " -f [path]:		Input new object config file path\n"
            " -s [path]:		Input ubus socket path\n"
            "\n");
}

static const struct ubus_method*
find_method(const struct ubus_method* methods, int n, const char* name)
{
    while(n--) {
        if(!strcmp(methods[n].name, name)) {
            return &methods[n];
        }
    }
    return NULL;
}

static void
cmd_to_string(char* buf, const char* cmd)
{
    if(NULL != cmd) {
        strcat(buf, cmd);
        strcat(buf, " ");
    }
}

static void
argv_to_string(char* buf, const char* argv)
{
    if(NULL != argv) {
        strcat(buf, argv);
        strcat(buf, " ");
    }
}

static int
invoke_method_handler(const char* handler, const char* data, char* ret)
{
    FILE* ret_stream = NULL;
    char buf[MAX_BUF_LEN] = {0};
    strcpy(buf, handler);
    strcat(buf, " ");
    strcat(buf, data);

    if((ret_stream = popen(buf, "r")) < 0) {
        return -1;
    }
    fgets(ret, MAX_BUF_LEN, ret_stream);

    pclose(ret_stream);
    return 0;
}

static int
objects_fork_method_handler
(struct ubus_context* ctx, struct ubus_object* obj,
 struct ubus_request_data* req, const char* method,
 struct blob_attr* msg)
{
    const struct ubus_method* _method = find_method(obj->methods, obj->n_methods, method);
    int method_n = uixo_obj->find_method_by_name(uixo_obj->methods_head, method);
    struct blob_attr* tb[_method->n_policy];
    const char* handler = NULL;
#if 0
    const char* agent = NULL;
    unsigned int port = 0;
    const char* addr = NULL;
#endif
    char cmd_argv_buf[MAX_BUF_LEN] = {0};
    const char* cmd = NULL;
    char argv[MAX_BUF_LEN] = {0};
    char method_return_value[MAX_BUF_LEN] = {0};

    blobmsg_parse(_method->policy, _method->n_policy, tb, blob_data(msg), blob_len(msg));
    {
        int n =0;
        cmd = uixo_obj->get_method_cmd(uixo_obj->methods_head, method_n);
        cmd_to_string(cmd_argv_buf, cmd);
        for(; n<_method->n_policy; n++) {
            if(tb[n] != NULL) {
                if(_method->policy[n].type == BLOBMSG_TYPE_STRING) {
                    argv_to_string(cmd_argv_buf, blobmsg_get_string(tb[n]));
                }
                else if(_method->policy[n].type == BLOBMSG_TYPE_INT32) {
                    sprintf(argv, "%d", blobmsg_get_u32(tb[n]));
                    argv_to_string(cmd_argv_buf, argv);
                }
                else {
                    fprintf(stderr,"Only support char and int types\n");
                    return UBUS_STATUS_INVALID_ARGUMENT;
                }
            }
        }
    }

    /* Deal with return. Only support return one variable or no return now */
    if(uixo_obj->get_method_return_n(uixo_obj->methods_head, method_n)!=0) {
    	enum blobmsg_type ret_type = uixo_obj->get_method_return_type
                (uixo_obj->methods_head, method_n, 0);
        if(ret_type != BLOBMSG_TYPE_STRING) {
            //TODO: now, only support string
        	fprintf(stderr, "only support string as return.\n");
        	return UBUS_STATUS_INVALID_ARGUMENT;
        }
    }

    handler = uixo_obj->get_method_handler(uixo_obj->methods_head, method_n);
    if(NULL != handler)
    { /* send data to method handler */
    	struct blob_buf* b = NULL;
        invoke_method_handler(handler, cmd_argv_buf, method_return_value);
        /* print method return value to stdout as json format */
        b = (struct blob_buf*)u_calloc(1, sizeof(*b));
        blob_buf_init(b, 0);
        if(blobmsg_add_string(b, "method", _method->name)!=0) {
        	fprintf(stderr, "Failed to add return\n");
            return UBUS_STATUS_INVALID_ARGUMENT;
        }
        if(blobmsg_add_string(b, "return", method_return_value)!=0) {
        	fprintf(stderr, "Failed to add return\n");
            return UBUS_STATUS_INVALID_ARGUMENT;
        }
        //fprintf(stdout,"%s\n",blobmsg_format_json(b->head, true));
        ubus_send_reply(ctx, req, b->head);
        u_free(b);
    }

#if 0
    else { /* send data to agent socket */
        agent = uixo_obj->agent;
        port = uixo_obj->port;
        addr = uixo_obj->addr;
        if(port < 1024) {
            fprintf(stderr,"agent port %d is invalid.\n", port);
            return 0;
        }
        else if((NULL==addr)||(NULL==agent)) {
            fprintf(stderr,"agent or address is NULL.\n");
            return 0;
        }
        else {
            send_to_agent(agent, port, addr, cmd_argv_buf, method_return_value, sizeof(int));
        }
    }
#endif
    return 0;
}

static void
objects_fork_main(struct ubus_object* new_object)
{
    int ret;

    ret = ubus_add_object(ctx, new_object);
    if (ret)
        fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
#if 0
    ret = ubus_register_subscriber(ctx, new_object);
    if (ret)
        fprintf(stderr, "Failed to add watch handler: %s\n", ubus_strerror(ret));
#endif
    uloop_run();
}

static struct ubus_object*
object_fork_uixo_obj_to_ubus_obj(struct uixo_object* uixo_obj)
{
    struct ubus_object* ubus_obj = NULL;
    struct ubus_object_type* ubus_type = NULL;
    struct ubus_method* ubus_methods = NULL;
    int i = 0;

    /* methods */
    ubus_methods = (struct ubus_method*)u_calloc(uixo_obj->methods_count, sizeof(*ubus_methods));
    for(i=0; i<uixo_obj->methods_count; i++) {
        ubus_methods[i].name = uixo_obj->get_method_name(uixo_obj->methods_head, i+1);
        ubus_methods[i].handler = objects_fork_method_handler;
        ubus_methods[i].policy = uixo_obj->get_method_policy(uixo_obj->methods_head, i+1);
        ubus_methods[i].n_policy = uixo_obj->get_method_n_policy(uixo_obj->methods_head, i+1);
    }
    /* type */
    ubus_type = (struct ubus_object_type*)u_calloc(1, sizeof(*ubus_type));
    ubus_type->name = uixo_obj->name;
    ubus_type->id = 0;
    ubus_type->methods = ubus_methods;
    ubus_type->n_methods = uixo_obj->methods_count;
    /* object */
    ubus_obj = (struct ubus_object*)u_calloc(1, sizeof(*ubus_obj));
    ubus_obj->name = uixo_obj->name;
    ubus_obj->type = ubus_type;
    ubus_obj->methods = ubus_methods;
    ubus_obj->n_methods = uixo_obj->methods_count;

    return ubus_obj;
}

static void
run_object_agent(const char* agent)
{
    char buf[MAX_BUF_LEN];

    strcpy(buf, agent);
    strcat(buf, " &");

    system(buf);
}

int main(int argc, char** argv)
{
    int ret = 0;
    const char* conf_file_path = NULL;
    const char* ubus_socket = NULL;
    int ch;
    static struct blob_buf conf_blob_buf;
    struct ubus_object* new_ubus_obj = NULL;

    while ((ch = getopt(argc, argv, "f:s:")) != -1) {
        switch (ch) {
            case 'f':
                conf_file_path = optarg;
                break;
            case 's':
                ubus_socket = optarg;
                break;
            default:
                usage();
                return -1;
                break;
        }
    }

    argc -= optind;
    argv += optind;

    if(NULL == conf_file_path) {
        usage();
        return -1;
    }

    memset(&conf_blob_buf, 0 ,sizeof(conf_blob_buf));
    blobmsg_buf_init(&conf_blob_buf);
    if(!blobmsg_add_json_from_file(&conf_blob_buf, conf_file_path)) {
        ret = -1;
        goto out;
    }
    uixo_obj = uixo_manager_blob_to_obj(conf_blob_buf.head);
    if(NULL != uixo_obj->agent) {
        run_object_agent(uixo_obj->agent);
    }

    new_ubus_obj = object_fork_uixo_obj_to_ubus_obj(uixo_obj);

    uloop_init();
    signal(SIGPIPE, SIG_IGN);

    ctx = ubus_connect(ubus_socket);
    if (!ctx) {
        fprintf(stderr, "Failed to connect to ubus\n");
        return -1;
    }

    ubus_add_uloop(ctx);

    objects_fork_main(new_ubus_obj);

    uixo_manager_free_obj(uixo_obj);
    ubus_free(ctx);
    uloop_done();

out:
    if(conf_blob_buf.buf)
        free(conf_blob_buf.buf);
    return ret;
}

