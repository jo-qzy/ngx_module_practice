#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t hello_string;
    ngx_int_t hello_counter;
} ngx_http_hello_loc_conf_t;

// 模块初始化，挂载
static ngx_int_t ngx_http_hello_init(ngx_conf_t* cf);

// 模块配置创建
static void* ngx_http_hello_create_loc_conf(ngx_conf_t* cf);

// 模块配置合并
//static char *ngx_http_hello_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);

// 模块配置hello_string读取
static char* ngx_http_hello_string(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);

// 模块配置hello_counter读取
static char* ngx_http_hello_counter(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);

// 模块handler函数
static ngx_int_t ngx_http_hello_handler(ngx_http_request_t* r);

// 模块配置指令
static ngx_command_t ngx_http_hello_commands[] = {
    {
        // 配置名称
        ngx_string("hello_string"),
        // 可以出现在http server块里面的location，接受一个参数或者不接受参数
        NGX_HTTP_LOC_CONF | NGX_CONF_NOARGS | NGX_CONF_TAKE1,
        // 配置解析、分解、处理函数，可以使用一些预置的函数
        ngx_http_hello_string,
        // 配置信息存储的位置，这里是存在http配置中的location
        NGX_HTTP_LOC_CONF_OFFSET,
        // 指定配置项值的存放位置(结构体偏移的位置)
        offsetof(ngx_http_hello_loc_conf_t, hello_string),
        // 指针，指向读取配置过程中需要的数据
        NULL },
    {
        // 配置名称
        ngx_string("hello_counter"),
        // 可以出现在http server块里面的location，可以配置on或off
        NGX_HTTP_LOC_CONF | NGX_CONF_FLAG,
        // 配置解析、分解、处理函数，可以使用一些预置的函数
        ngx_http_hello_counter,
        // 配置信息存储的位置，这里是存在http配置中的location
        NGX_HTTP_LOC_CONF_OFFSET,
        // 指定配置项值的存放位置(结构体偏移的位置)
        offsetof(ngx_http_hello_loc_conf_t, hello_counter),
        // 指针，指向读取配置过程中需要的数据
        NULL },
    ngx_null_command
};

static int ngx_hello_visited_times = 0;
static u_char ngx_hello_default_string[] = "Default String: Hello, jo-qzy!";

// 模块上下文信息
static ngx_http_module_t ngx_http_hello_module_ctx = {
    NULL,                           // 在创建和读取该模块的配置信息之前被调用
    ngx_http_hello_init,            // 在创建和读取该模块的配置信息之后被调用

    NULL,                           // 创建本模块位于http block的配置信息存储结构
    NULL,                           // 初始化本模块位于http block的配置信息存储结构

    NULL,                           // 创建本模块位于http server block的配置信息存储结构
    NULL,                           // 合并可能同时出现在http block和http server block的配置

    ngx_http_hello_create_loc_conf, // 创建本模块位于location block的配置信息存储结构
    NULL                            // 和上面的合并类似，也是合并配置的一个地方
};

// 模块的定义
ngx_module_t ngx_http_hello_module = {
        NGX_MODULE_V1,                 // 结构体中有些成员用该宏去填充
        &ngx_http_hello_module_ctx,    // void类型，指向模块上下文信息的结构体对象
        ngx_http_hello_commands,       // 指向模块配置指令
        NGX_HTTP_MODULE,               // 模块类型标识
        NULL,                          // 目前nginx不会调用
        NULL,                          // 在ngx_init_cycle里被调用
        NULL,                          // 在ngx_single_process_cycle/ngx_worker_process_init里调用
        NULL,                          // 目前nginx不会调用
        NULL,                          // 目前nginx不会调用
        NULL,                          // 在ngx_worker_process_exit调用
        NULL,                          // 在ngx_master_process_exit里调用
        NGX_MODULE_V1_PADDING          // 后面的成员由NGX_MODULE_V1_PADDING填充
};

static ngx_int_t ngx_http_hello_init(ngx_conf_t* cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    // 获取http配置
    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    // 挂载到CONTENT PHASE阶段
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_hello_handler;

    return NGX_OK;
}

static void* ngx_http_hello_create_loc_conf(ngx_conf_t* cf)
{
    // 为模块配置申请空间
    ngx_http_hello_loc_conf_t* local_conf = NULL;
    local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_loc_conf_t));
    if (local_conf == NULL) {
        return NULL;
    }

    // 给两个参数赋初值
    ngx_str_null(&local_conf->hello_string);
    local_conf->hello_counter = NGX_CONF_UNSET;

    return local_conf;
}

static char* ngx_http_hello_string(ngx_conf_t* cf, ngx_command_t* cmd, void* conf)
{
    ngx_http_hello_loc_conf_t* local_conf = conf;

    // 将字符串类型的配置读取进配置结构体中，也可以直接将commands内直接用该函数读取
    char* rv = ngx_conf_set_str_slot(cf, cmd, conf);

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello string: %s", local_conf->hello_string);

    return rv;
}

static char* ngx_http_hello_counter(ngx_conf_t* cf, ngx_command_t* cmd, void* conf)
{
    ngx_http_hello_loc_conf_t* local_conf = conf;

    // 将字符串类型的配置读取进配置结构体中，也可以直接将commands内直接用该函数读取
    char* rv = ngx_conf_set_flag_slot(cf, cmd, conf);

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello counter: %d", local_conf->hello_counter);

    return rv;
}

static ngx_int_t ngx_http_hello_handler(ngx_http_request_t* r)
{
    ngx_int_t                  rc             = NGX_ERROR;
    ngx_uint_t                 content_length = 0;
    ngx_buf_t                 *b              = NULL;
    ngx_chain_t                out            = { NULL, NULL };
    ngx_http_hello_loc_conf_t *hello_conf     = NULL;
    u_char                     ngx_hello_string[1024];

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_hello_handler is called!");

    hello_conf = ngx_http_get_module_loc_conf(r, ngx_http_hello_module);

    if (hello_conf->hello_string.len == 0) {
        // hello string未配置，返回错误
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "hello string is empty!");
        return NGX_DECLINED;
    }

    // 检查hello counter是否被配置
    if (hello_conf->hello_counter == NGX_CONF_UNSET || hello_conf->hello_counter == 0) {
        // 未配置hello counter，则只返回hello string
        ngx_sprintf(ngx_hello_default_string, "Hi %s, welcome!", hello_conf->hello_string.data);
    } else {
        // 配置了hello counter
        ngx_sprintf(ngx_hello_default_string, "Hi %s, you've visited this page %d times.",
            hello_conf->hello_string, ++ngx_hello_visited_times);
    }

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "hello string: %s", ngx_hello_string);
    content_length = ngx_strlen(ngx_hello_string);

    // 如果方法不是GET或者HEAD，一律拒绝
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
        return NGX_HTTP_NOT_ALLOWED;

    // 丢弃请求的body，因为我们根本不需要
    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK)
        return rc;

    // 设置Content-Type字段
    ngx_str_set(&r->headers_out.content_type, "text/html");

    // HEAD方法，只发送报头即可
    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = content_length;

        return ngx_http_send_header(r);
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL)
        return NGX_HTTP_INTERNAL_SERVER_ERROR;

    out.buf = b;
    out.next = NULL;

    // 对ngx_buf的内容开始，结尾等信息进行赋值
    b->pos = ngx_hello_string;
    b->last = ngx_hello_string + content_length;
    b->memory = 1; // 表示这段buffer位于内存
    b->last_buf = 1; // 表示buf是chain里面最后一段buf

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = content_length;
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
        return rc;

    return ngx_http_output_filter(r, &out);
}