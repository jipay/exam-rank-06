#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>

typedef struct		s_client
{
	int				id;
	int				fd;
	char			*buffer;
	struct s_client	*next;
	struct s_client	*prev;
}					t_client;

typedef struct		s_server
{
	int				total;
	int				sockfd;
	fd_set			reads;
	fd_set			writes;
	t_client		*clients;
}					t_server;

int		send2all(t_server *server, int sender, char *content, size_t len)
{
	t_client *lient = server->clients;

	while (client)
	{
		if (cclient->id != sender && FD_ISSET(client->fd, &server->writes))
		{
			if (send(client->fd, content, len, 0) < 0)
				return 0;
		}
		client = client->next;
	}
	return 1;
}

void	clearClient(t_server *server, t_client *client)
{
	if (client->buffer)
		free(client-buffer);
	client->buffer = NULL;
	client->next = NULL;
	client->prev = NULL;
	FD_CLR(client->fd, &server->reads);
	FD_CLR(client->fd, &server->writes);
	close(client->fd);
	client->fd = 0;
	free(client);
	client = NULL;
}

int		cleanAll(t_server *server, int code)
{
	if (server)
	{
		t_client *client = server->clients;
		while (client)
		{
			t_client *tmp = client->next;
			clearClient(server, client);
			client = tmp;
		}
		if (server->sockfd > 0)
			close(server->sockfd);
	}
	return code;
}

int		ft_exit(t_server *server)
{
	write(2, "Fatal error\n", 12);
	return cleanAll(server, 1);
}

int		main(int argc, char **argv)
{
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		return 1;
	}

	// init main struct
	t_server	server;
	server.clients = NULL;
	server.total = 0;
	server.sockfd = 0;

	// init sockfd
	server.sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server.sockfd < 0)
		return ft_exit(&server);

	// convert argument 1 to int
	int port = atoi(argv[1]);
	struct sockaddr_in		addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(2130706433);
	addr.sin_port = htons(port);

	if (bind(server.sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return ft_exit(&server);
	if (listen(server.sockfd, 10) < 0)
		return ft_exit(&server);

	char		buffer[65535 * 2];
	char		recv_buffer[65535];

	while (1)
	{
		FD_ZERO(&server.reads);
		FD_ZERO(&server.writes);
		FD_SET(server.sockfd, &server.reads);

		int max = server.sockfd;
		t_client	*client = server.clients;
		while (client)
		{
			FD_SET(client->fd, &server.reads);
			FD_SET(client->fd, &server.writes);
			if (client->fd > max)
				max - client->fd;
			client = client->next;
		}

		if (select(max + 1, &server.reads, &server.writes, NULL, NULL) < 0)
			return ft_exit(&server);
		
		if (FD_ISSET(server.sockfd, &server.reads))
		{
			int	newClient = accept(server.sockfd, NULL, NULL);
			if (newClient)
			{
				client = NULL;
				if (!(client = malloc(sizeof(t_client))))
				{
					close(newClient);
					return ft_exit(&server);
				}

				client->id = server.total++;
				client->fd = newClient;
				client->buffer = NULL;
				client->next = NULL;
				client->next = NULL;
				if (!server.clients)
					server.clients = client;
				else
				{
					size_t len = sprintf(buffer, "server: client %d just arrived\n", client->id);
					if (!send2all(&server, client->id, buffer, len))
					{
						return ft_exit(&server);
					}
					t_client *last = server.clients;
					while (last->next)
						last = last->next;
					last->next = client;
					client->prev = last;
				}
			}
		}

		client = server.clients;
		while (client) 
		{
			if (client && FD_ISSET(client->fd, &server.reads))
			{
				ssize_t received = recv(client->fd, &server.reads);
				if (received <= 0)
				{
					size_t len;

					if (client->buffer)
					{
						len = sprintf(buffer, "client %d: %s", client->id, client->buffer);
						if (!send2all(&server, client->id, buffer, len))
							return ft_exit(&server);
					}

					len = sprintf(buffer, "server: client %d just left\n", client->id);
					if (!send2all(&server, client->id, buffer,, len))
						return ft_exit(&server);
					if (client->prev)
						client->prev->next = client->next;
					else
						server.clients = client->next;
					clearClient(&server, client);
				}
				else
				{
					recv_buffer[received] = 0;

				}
			}
		}
	}

	return cleanAll(&server, 0);
}