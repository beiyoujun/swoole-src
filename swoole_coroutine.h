/*
 +----------------------------------------------------------------------+
 | Swoole                                                               |
 +----------------------------------------------------------------------+
 | Copyright (c) 2012-2015 The Swoole Group                             |
 +----------------------------------------------------------------------+
 | This source file is subject to version 2.0 of the Apache license,    |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.apache.org/licenses/LICENSE-2.0.html                      |
 | If you did not receive a copy of the Apache2.0 license and are unable|
 | to obtain it through the world-wide-web, please send a note to       |
 | license@swoole.com so we can mail you a copy immediately.            |
 +----------------------------------------------------------------------+
 | Author: Tianfeng Han  <mikan.tenny@gmail.com>                        |
 |         Xinyu    Zhu  <xyzhu1120@gmail.com>                          |
 +----------------------------------------------------------------------+
 */

#ifdef SW_COROUTINE
#ifndef _PHP_SWOOLE_COROUTINE_H_
#define _PHP_SWOOLE_COROUTINE_H_

#include "coroutine.h"
#include <setjmp.h>

#define DEFAULT_MAX_CORO_NUM 3000

#define CORO_END 0
#define CORO_YIELD 1
#define CORO_LIMIT 2
#define CORO_SAVE 3

typedef struct _php_context php_context;
typedef struct _coro_task coro_task;

struct _php_context
{
    zval **current_coro_return_value_ptr_ptr;
    zval *current_coro_return_value_ptr;
    void *coro_params;
    void (*onTimeout)(struct _php_context *cxt);
    zval **current_eg_return_value_ptr_ptr;
    zend_execute_data *current_execute_data;
    zend_op **current_opline_ptr;
    zend_op *current_opline;
    zend_op_array *current_active_op_array;
    HashTable *current_active_symbol_table;
    zval *current_this;
    zend_class_entry *current_scope;
    zend_class_entry *current_called_scope;
    zend_vm_stack current_vm_stack;
    void *parent;
    coro_task *current_task;
};

typedef struct _coro_global
{
    uint32_t coro_num;
    uint32_t max_coro_num;
    zend_vm_stack origin_vm_stack;
    zend_execute_data *origin_ex;
    coro_task *current_coro;
    zend_bool require;
} coro_global;

struct _coro_task
{
    uint32_t *cid;
    time_t start_time;
    void (*post_callback)(void *param);
    void *post_callback_params;
};

typedef struct
{
    uint32_t cnt;
    jmp_buf checkpoints[10];
} coro_checkpoint_stack;

extern zend_fcall_info_cache *php_sw_server_caches[PHP_SERVER_CALLBACK_NUM];

extern coro_global COROG;

static sw_inline zend_fcall_info_cache* php_swoole_server_get_cache(swServer *serv, int server_fd, int event_type)
{
    swListenPort *port = serv->connection_list[server_fd].object;
    swoole_server_port_property *property = port->ptr;
    if (!property)
    {
        return php_sw_server_caches[event_type];
    }
    zend_fcall_info_cache* cache = property->caches[event_type];
    if (!cache)
    {
        return php_sw_server_caches[event_type];
    }
    else
    {
        return cache;
    }
}

int coro_init(TSRMLS_D);
int coro_create(zend_fcall_info_cache *op_array, zval **argv, int argc, zval **retval, void *post_callback, void *param);
void coro_check(TSRMLS_D);
void coro_close(TSRMLS_D);
php_context *coro_save(zval *return_value, zval **return_value_ptr, php_context *sw_php_context);
int coro_resume(php_context *sw_current_context, zval *retval, zval **coro_retval);
long php_swoole_add_timer_coro(int ms, int cli_fd, int *timeout_id, void* param TSRMLS_DC);
int php_swoole_clear_timer_coro(long id TSRMLS_DC);

#endif
#endif