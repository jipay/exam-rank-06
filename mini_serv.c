#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int		sd_arr[65536];
int		max = 0;
int		total = 0;
fd_set	readfds, writefds, active;
char	buf_read[4096 * 42], buf_write[4096 * 42 + 42];
char	*buf_arr[4096 * 42];

void	ft_error()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void	send2all(int sock)
{
	for (int i = 0; i <= max; i++)
	{
		if (FD_ISSET(i, &writefds) && i != sock)
		{
			if (send(i, buf_write, strlen(buf_write), 0) < 0)
				ft_error();
		}
	}
}

// given by main.c from subject
int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

// given by main.c from subject
char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

int		main(int argc, char **argv)
{
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	bzero(&sd_arr, sizeof(sd_arr));
	FD_ZERO(&active);
	int port = atoi(argv[1]);
	int sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd < 0)
		ft_error();
	FD_SET(sd, &active);
	max = sd;
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(2130706433);
	addr.sin_port = htons(port);

	if (bind(sd, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
		ft_error();
	if (listen(sd, 10) < 0)
		ft_error();

	while(1)
	{
		readfds = writefds = active;

		if (select(max + 1, &readfds, &writefds, NULL, NULL) < 0)
			ft_error();
		for (int i = 0; i <= max; i++)
		{
			if (FD_ISSET(i, &readfds))
			{
				if (i == sd)
				{
					int new_client = accept(i, (struct sockaddr *)&addr, &len);
					if (new_client < 0)
						ft_error();
					sd_arr[new_client] = total++;
					buf_arr[new_client] = NULL;
					max = (new_client > max) ? new_client : max;
					FD_SET(new_client, &active);
					
					sprintf(buf_write, "server: client %d just arrived\n", sd_arr[new_client]);
					send2all(new_client);
				}
				else
				{
					int res = recv(i, buf_read, 4096 * 42, 0);
					if (res <= 0)
					{
						sprintf(buf_write, "server: client %d just left\n", sd_arr[i]);
						send2all(i);
						close(i);
						free(buf_arr[i]);
						buf_arr[i] = NULL;
						FD_CLR(i, &active);
						break;
					}
					else
					{
						buf_read[res] = '\0';
						buf_arr[i] = str_join(buf_arr[i], buf_read);
						char *s = NULL;
						while (extract_message(&buf_arr[i], &s))
						{
							sprintf(buf_write, "client %d: %s", sd_arr[i], s);
							send2all(i);
							free(s);
							s = NULL;
						}
					}
				}
			}
		}
	}
	return 0;
}