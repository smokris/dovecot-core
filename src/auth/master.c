/* Copyright (C) 2002 Timo Sirainen */

#include "common.h"
#include "iobuffer.h"
#include "network.h"
#include "cookie.h"

#define MAX_OUTBUF_SIZE (10 * sizeof(AuthCookieReplyData))

static AuthCookieReplyData failure_reply;

static IOBuffer *outbuf;
static IO io_master;

static unsigned int master_pos;
static char master_buf[sizeof(AuthCookieRequestData)];

static void master_handle_request(AuthCookieRequestData *request,
				  int fd __attr_unused__)
{
	CookieData *cookie;
        AuthCookieReplyData *reply, temp_reply;

	cookie = cookie_lookup_and_remove(request->cookie);
	if (cookie == NULL)
		reply = &failure_reply;
	else {
		if (cookie->auth_fill_reply(cookie, &temp_reply))
			reply = &temp_reply;
		else
			reply = &failure_reply;
		cookie->free(cookie);
	}

	reply->id = request->id;
	switch (io_buffer_send(outbuf, reply, sizeof(AuthCookieReplyData))) {
	case -2:
		i_fatal("Master transmit buffer full, aborting");
	case -1:
		/* master died, kill ourself too */
		io_loop_stop(ioloop);
		break;
	}
}

static void master_input(void *context __attr_unused__, int fd,
			 IO io __attr_unused__)
{
	int ret;

	ret = net_receive(fd, master_buf + master_pos,
			  sizeof(master_buf) - master_pos);
	if (ret < 0) {
		/* master died, kill ourself too */
		io_loop_stop(ioloop);
		return;
	}

	master_pos += ret;
	if (master_pos < sizeof(master_buf))
		return;

	/* reply is now read */
	master_handle_request((AuthCookieRequestData *) master_buf, fd);
	master_pos = 0;
}

void master_init(void)
{
	memset(&failure_reply, 0, sizeof(failure_reply));

	master_pos = 0;
	outbuf = io_buffer_create(MASTER_SOCKET_FD, default_pool,
				  IO_PRIORITY_DEFAULT, MAX_OUTBUF_SIZE);
	io_master = io_add(MASTER_SOCKET_FD, IO_READ, master_input, NULL);
}

void master_deinit(void)
{
	io_buffer_unref(outbuf);
	io_remove(io_master);
}
