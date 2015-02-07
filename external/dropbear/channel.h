
#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "includes.h"
#include "buffer.h"
#include "circbuffer.h"

/* channel->type values */
#define CHANNEL_ID_NONE 0
#define CHANNEL_ID_SESSION 1
#define CHANNEL_ID_X11 2
#define CHANNEL_ID_AGENT 3
#define CHANNEL_ID_TCPDIRECT 4
#define CHANNEL_ID_TCPFORWARDED 5

#define SSH_OPEN_ADMINISTRATIVELY_PROHIBITED    1
#define SSH_OPEN_CONNECT_FAILED                 2
#define SSH_OPEN_UNKNOWN_CHANNEL_TYPE           3
#define SSH_OPEN_RESOURCE_SHORTAGE              4

/* Not a real type */
#define SSH_OPEN_IN_PROGRESS					99

#define MAX_CHANNELS 100 /* simple mem restriction, includes each tcp/x11
							connection, so can't be _too_ small */

#define CHAN_EXTEND_SIZE 3 /* how many extra slots to add when we need more */

#define RECV_MAXWINDOW 8000 /* tweak */
#define RECV_WINDOWEXTEND 1000 /* We send a "window extend" every
								RECV_WINDOWEXTEND bytes */
#define RECV_MAXPACKET RECV_MAXWINDOW /* tweak */

struct ChanType;

struct Channel {

	unsigned int index; /* the local channel index */
	unsigned int remotechan;
	unsigned int recvwindow, transwindow;
	unsigned int recvdonelen;
	unsigned int recvmaxpacket, transmaxpacket;
	void* typedata; /* a pointer to type specific data */
	int writefd; /* read from wire, written to insecure side */
	int readfd; /* read from insecure size, written to wire */
	int errfd; /* used like writefd or readfd, depending if it's client or server.
				  Doesn't exactly belong here, but is cleaner here */
	circbuffer *writebuf; /* data from the wire, for local consumption */
	circbuffer *extrabuf; /* extended-data for the program - used like writebuf
					     but for stderr */

	/* whether close/eof messages have been exchanged */
	int sent_close, recv_close;
	int recv_eof, sent_eof;

	int initconn; /* used for TCP forwarding, whether the channel has been
					 fully initialised */

	int await_open; /* flag indicating whether we've sent an open request
					   for this channel (and are awaiting a confirmation
					   or failure). */

	int flushing;

	const struct ChanType* type;

};

struct ChanType {

	int sepfds; /* Whether this channel has seperate pipes for in/out or not */
	char *name;
	int (*inithandler)(struct Channel*);
	int (*check_close)(struct Channel*);
	void (*reqhandler)(struct Channel*);
	void (*closehandler)(struct Channel*);

};

void chaninitialise(const struct ChanType *chantypes[]);
void chancleanup();
void setchannelfds(fd_set *readfd, fd_set *writefd);
void channelio(fd_set *readfd, fd_set *writefd);
struct Channel* getchannel();
struct Channel* newchannel(unsigned int remotechan, 
		const struct ChanType *type, 
		unsigned int transwindow, unsigned int transmaxpacket);

void recv_msg_channel_open();
void recv_msg_channel_request();
void send_msg_channel_failure(struct Channel *channel);
void send_msg_channel_success(struct Channel *channel);
void recv_msg_channel_data();
void recv_msg_channel_extended_data();
void recv_msg_channel_window_adjust();
void recv_msg_channel_close();
void recv_msg_channel_eof();

void common_recv_msg_channel_data(struct Channel *channel, int fd, 
		circbuffer * buf);

#ifdef DROPBEAR_CLIENT
extern const struct ChanType clichansess;
#endif

#if defined(USING_LISTENERS) || defined(DROPBEAR_CLIENT)
int send_msg_channel_open_init(int fd, const struct ChanType *type);
void recv_msg_channel_open_confirmation();
void recv_msg_channel_open_failure();
#endif

#endif /* _CHANNEL_H_ */
