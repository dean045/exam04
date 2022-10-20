# include <unistd.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <string.h>
 #include <stdio.h>
# include <sys/types.h>

typedef struct s_data
{
    char *cmd;
    char **args;
    int  first;
    int  has_pipe;
    struct s_data *next;
}   t_data;

char **get_args(char **av)
{
    int i = 0;
    int j = 0;
    char **args;
    while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|"))
        i++;
    args = malloc(sizeof(char *) * (i + 1));
    while (j < i)
    {
        args[j] = av[j];
        j++;
    }
    args[j] = NULL;
    return (args);
}

t_data *pars(char **av, int ac)
{
    int first = 1;
    int i = 1;
    t_data *data;
    t_data *rendu;

    data = malloc(sizeof(t_data));
    rendu = data;
    while (av[i])
    {
        while (av[i] && strcmp(av[i], ";") == 0)
            i++;
        if (i < ac)
        {
            data->cmd = av[i];
            data->first = first;
            first = 0;
            data->args = get_args(av + i);
            while (av[i] && strcmp(av[i], "|") && strcmp(av[i], ";"))
                i++;
            if (av[i] && !strcmp(av[i], "|"))
            {
                data->has_pipe = 1;
                data->next = malloc(sizeof(t_data));
                data = data->next;
            }
            else if (av[i] && !strcmp(av[i], ";"))
            {
                data->has_pipe = 0;
                first = 1;
                data->next = malloc(sizeof(t_data));
                data = data->next;
            }
            else
            {
                data->has_pipe = 0;
                data->next = NULL;
            }
        }
        i++;
    }
    return (rendu);
}

int main(int ac, char **av, char **envp)
{
    (void)envp;
    t_data *data;
  //  t_data *rendu;
    pid_t pid;
    int fd[2];
    int prev;
    int i = 0;
    int status;

    fd[0] = -1;
    fd[1] = -1;
    if (ac == 1)
        return (0);
    data = pars(av, ac);
    //rendu = data;
/*    while (rendu)
    {
        printf("cmd => %s // has pipe => %i // first => %i \n", rendu->cmd, rendu->has_pipe, rendu->first);
        rendu = rendu->next;
    }*/

    while (data)
    {
        prev = fd[0];
        if (data->has_pipe == 1)
        {
            if (pipe(fd) == -1)
                return 1; //gerer msg d erreur
            //printf("fd[0] = %i ,  fd[1] = %i\n", fd[0], fd[1]);
        }
        i++;
        pid = fork();
        if (pid < 0)
            return (1);
        else if (pid == 0)
        {
            if (data->first == 0)
                dup2(prev, 0);
            if (data->has_pipe == 1)
                dup2(fd[1], 1);
            if (fd[1] > 0)
                close(fd[1]);
            if (fd[0] > 0)
                close(fd[0]);
            execve(data->cmd, data->args, envp);
            exit (1);
        }
        else
        {
            if (prev != -1)
                close(prev);
            if (fd[1] != -1)            
                close(fd[1]);
            if (data->has_pipe == 0)
            {
                while (i)
                {
                    wait(&status);
                    i--;
                }
                if (fd[0] > 0)
                    close(fd[0]);
            }
        }
        data = data->next;
    }
    if (fd[0] != -1)
        close(fd[0]);
    if (fd[1] != -1)
        close(fd[1]);
    if (prev != -1)
        close(prev);
    return (0);
}
