/* Copyright (C) 2002 Timo Sirainen */

#include "common.h"
#include "commands.h"
#include "imap-expunge.h"

int cmd_close(struct client *client)
{
	struct mailbox *mailbox = client->mailbox;
	struct mail_storage *storage = mailbox_get_storage(mailbox);

	if (!client_verify_open_mailbox(client))
		return TRUE;

	client->mailbox = NULL;

	if (!mailbox_is_readonly(mailbox)) {
		if (!imap_expunge(mailbox))
			client_send_untagged_storage_error(client, storage);
	}

	if (mailbox_close(mailbox) < 0)
                client_send_untagged_storage_error(client, storage);

	client_send_tagline(client, "OK Close completed.");
	return TRUE;
}
