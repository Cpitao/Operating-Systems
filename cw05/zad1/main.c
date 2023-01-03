#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct ExecData {
    char* program_name;
    int argc;
    char** args;
} ExecData;

char*** get_components(FILE* fp, int* comp_count);
char** get_commands(FILE* fp, int* commands_count);
int* parse_command(char* command, char*** components, int components_count,
                   int* command_components_count);
int get_all(ExecData* execData, char* command, int* programCount);
void run_commands(char** commands, int commands_count, char*** components, int components_count);




int main(int argc, char** argv) {
    if (argc < 2)
    {
        fprintf(stderr, "File path not specified\n");
        return 1;
    }

    FILE* fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s for reading", argv[1]);
        return 1;
    }

    int components_count = 0;
    char*** components = get_components(fp, &components_count);

    int commands_count = 0;
    char** commands = get_commands(fp, &commands_count);

    run_commands(commands, commands_count, components, components_count);
    // Free allocated memory, clean up etc.
    // free components memory

    for (int i=0; i < components_count; i++)
    {
        free(components[i][0]);
        free(components[i][1]);
        free(components[i]);
    }
    free(components);
    // free commands memory
    for (int i=0; i < commands_count; i++) free(commands[i]);
    free(commands);
    fclose(fp);
    return 0;
}

char*** get_components(FILE* fp, int* comp_count)
{
    size_t components_count = 0;
    size_t components_size = 16;
    char*** components = (char***) malloc(components_size * sizeof (char**));
    // single component will be [component_name, component_value]
    while(1) {
        size_t comp_name_len = 0;
        size_t comp_name_size = 32;
        char* component_name = (char*) malloc(comp_name_size);
        if (component_name == NULL)
        {
            fprintf(stderr, "Unable to allocate memory");
            return NULL;
        }

        int end_of_components = 0;

        while (1) {
            char c;
            if (fread(&c, 1, 1, fp) != 1) {
                fprintf(stderr, "Unexpected end of file");
                return NULL;
            }
            if (c == '\n') {
                end_of_components = 1;
                break;
            }
            if (c == '=' || c == ' ')
            {
                while (c != '=')
                {
                    if (fread(&c, 1, 1, fp) == 0)
                    {
                        fprintf(stderr, "Unexpected end of file");
                        return NULL;
                    }

                }
                fread(&c, 1, 1, fp);
                if (c != ' ') fseek(fp, -1, SEEK_CUR);
                break;
            }

            if (comp_name_len >= comp_name_size - 2) {
                comp_name_size *= 2;
                if ((component_name = (char*)realloc(component_name, comp_name_size)) == NULL) {
                    fprintf(stderr, "Unable to allocate enough memory for component name");
                    return NULL;
                }
            }

            component_name[comp_name_len] = c;
            comp_name_len++;
            component_name[comp_name_len] = '\0';
        }

        if (end_of_components)
        {
            free(component_name);
            break;
        }

        size_t comp_val_len = 0;
        size_t comp_val_size = 64;
        char* component_value = (char*) malloc(comp_val_size);
        if (component_value == NULL)
        {
            fprintf(stderr, "Unable to allocate enough memory");
            return NULL;
        }
        while (1) {
            char c;
            if (fread(&c, 1, 1, fp) != 1) {
                fprintf(stderr, "Unexpected end of file");
                return NULL;
            }
            if (c == '\n') break;

            if (comp_val_len >= comp_val_size - 2) {
                comp_val_size *= 2;
                if ((component_value = (char*)realloc(component_value, comp_val_size)) == NULL) {
                    fprintf(stderr, "Unable to allocate enough memory for component value");
                    return NULL;
                }
            }

            component_value[comp_val_len] = c;
            comp_val_len++;
            component_value[comp_val_len] = '\0';
        }

        if (components_count == components_size)
        {
            components_size *= 2;
            if ((components = (char***) realloc(components, components_size * sizeof(char**))) == NULL)
            {
                fprintf(stderr, "Unable to allocate enough memory for components");
                return NULL;
            }
        }
        char** new_data = (char**) malloc(2 * sizeof(char*));
        new_data[0] = (char*)malloc(strlen(component_name) + 1);
        strcpy(new_data[0], component_name);
        new_data[1] = (char*)malloc(strlen(component_value) + 1);
        strcpy(new_data[1], component_value);
	free(component_name);
	free(component_value);
        components[components_count] = new_data;
        components_count++;
    }

    *comp_count = components_count;
    return components;
}

char** get_commands(FILE* fp, int* commands_count)
{
    size_t comm_count = 0;
    size_t commands_size = 2;
    char** commands = (char**) malloc(commands_size * sizeof(char*));

    // loop over all commands
    int end_of_file = 0;
    while (1) {
        char c;
        size_t command_length = 0;
        size_t command_max_length = 255;
        char *command = (char *) malloc(command_max_length);
        command[0] = '\0';
        // loop over a command
        while (1)
        {
            if (fread(&c, 1, 1, fp) == 0)
            {
                end_of_file = 1;
                break;
            }

            if (command_length >= command_max_length - 2)
            {
                command_max_length *= 2;
                if ((command = (char*) realloc(command, command_max_length)) == NULL)
                {
                    fprintf(stderr, "Unable to save command");
                    return NULL;
                }
            }

            if (c == '\n')
                break;

            command[command_length] = c;
            command_length++;
            command[command_length] = '\0';
        }
        if (strcmp(command, "") == 0)
        {
            free(command);
            break;
        }
        if (comm_count >= commands_size - 2)
        {
            commands_size *= 2;
            if ((commands = (char**) realloc(commands, commands_size * sizeof(char*))) == NULL)
            {
                fprintf(stderr, "Unable to save all commands");
                return NULL;
            }
        }
        commands[comm_count] = command;
        comm_count++;

        if (end_of_file) break;
    }

    *commands_count = comm_count;
    return commands;
}

void run_commands(char** commands, int commands_count, char*** components, int components_count)
{
    for (int i=0; i < commands_count; i++) {
        int command_components_count = 0;
        int *component_indexes = parse_command(commands[i], components, components_count,
                                               &command_components_count);

        char* full_command = (char*) calloc(512, 1);
        strcpy(full_command, components[component_indexes[0]][1]);
	
        for (int j=1; j < command_components_count; j++)
	{
	    strcat(full_command, " | ");
            strcat(full_command, components[component_indexes[j]][1]);
	}

        // assume upper limit of piped programs on one command to be 16
        int program_count = 0;

        ExecData* execData = malloc(16 * sizeof(*execData));
        get_all(execData, full_command, &program_count);

	int **fds = malloc(program_count * sizeof(int*));
	for (int i=0; i < program_count; i++)
	    fds[i] = malloc(2 * sizeof(int)); 
        for (int j=0; j < program_count; j++)
        {
            if (pipe(fds[j]))
            {
                fprintf(stderr, "Error opening pipe");
                free(full_command);
                for (int k=0; k < program_count; k++)
                {
                    for (int l=0; l < execData[k].argc; l++)
                        free(execData[k].args[l]);
                    free(execData[k].program_name);
                    free(execData[k].args);
                }
                free(execData);
		free(component_indexes);
                return;
            }
            pid_t pid = fork();

            if (pid < 0)
            {
                fprintf(stderr, "Error forking");
                free(full_command);
                for (int k=0; k < program_count; k++)
                {
                    for (int l=0; l < execData[k].argc; l++)
                        free(execData[k].args[l]);
                    free(execData[k].program_name);
                    free(execData[k].args);
                }
                free(execData);
		free(component_indexes);
                return;
            }
            else if (pid == 0)
            {
                if (j == 0) {
                    close(fds[j][0]);
		    close(STDIN_FILENO);
                    dup2(fds[j][1], STDOUT_FILENO);
                }
                else
                {
                    close(fds[j][0]);
                    dup2(fds[j-1][0], STDIN_FILENO);
                    dup2(fds[j][1], STDOUT_FILENO);
                }
                execvp(execData[j].program_name, execData[j].args);
                perror("Exec:");
            }
	    close(fds[j][1]);
        }
	while (wait(NULL) > 0);
        char c;
	while (read(fds[program_count-1][0], &c, 1) > 0) printf("%c", c);
	for (int i=0; i < program_count; i++) free(fds[i]);
	free(fds);
        free(full_command);
        for (int k=0; k < program_count; k++)
        {
            for (int l=0; l < execData[k].argc; l++)
                free(execData[k].args[l]);
            //free(execData[k].program_name);
            free(execData[k].args);
        }
	free(component_indexes);
        free(execData);
    }
}

int* parse_command(char* command, char*** components, int components_count,
                   int* command_components_count)
{
    int* component_indexes = (int*) calloc(64, sizeof(int));
    int component_index = 0;
    int pos = 0;
    int start;

    while (command[pos] != '\0') {
        start = pos;

        while (command[pos] != '|' && command[pos] != ' ' && command[pos] != '\0')
            pos++;

        char *component = (char *) malloc(pos - start + 1);
        if (component == NULL)
        {
            fprintf(stderr, "Unable to allocate memory");
            return NULL;
        }
        strncpy(component, command + start, pos - start);
        component[pos-start] = '\0';

        while (command[pos] != '\0' && (command[pos] == ' ' || command[pos] == '|'))
            pos++;

        int component_found = 0;
        for (int j=0; j < components_count; j++)
        {
            if (strcmp(components[j][0], component) == 0) {
                component_indexes[component_index] = j;
                component_index++;
                component_found = 1;
                free(component);
                break;
            }
        }

        if (!component_found)
        {
            fprintf(stderr, "No such component: %s", component);
            free(component);
            *command_components_count = component_index;
            return component_indexes;
        }
    }

    *command_components_count = component_index;
    return component_indexes;
}

int get_all(ExecData* execData, char* command, int* programCount)
{
    size_t n = strlen(command);
    int counter = 0;
    int program_count = 0;
    while (counter < n)
    {
        // get program name
        char* program = (char*) calloc(64, 1);
	while (command[counter] == ' ') counter++;
	int beginning = counter;
        while (command[counter] != ' ' && command[counter] != '\0' && command[counter] != '|')
        {
            counter++;
        }
        strncpy(program, command + beginning, counter - beginning);

        if (command[counter] == '|')
        {
	    if (strlen(program) > 0)
	    {
                ExecData newExecData;
                newExecData.program_name = program;
                newExecData.argc = 1;
		newExecData.args = calloc(2, sizeof(char*));
		newExecData.args[0] = program;
                execData[program_count] = newExecData;
                program_count++;
	    }
	    else free(program);
	    counter++;
            continue;
        }

        counter++;
        // get arguments
        char** args = (char**) calloc(16, sizeof(char*)); // max 16 arguments
        int arg_count = 1;
	args[0] = program;
        while (command[counter] != '|' && command[counter] != '\0')
        {
            beginning = counter;
            while (command[counter] != ' ' && command[counter] != '\0' && command[counter] != '|') counter++;
            if (counter != beginning)
            {
                char* arg = (char*) calloc(32, 1);
		int f = 0;
		if ((command[beginning] == '\'' && command[counter-1] == '\'') || (command[beginning] == '\"' && command[counter-1] == '\"'))
		{
		    if (counter - 1 > beginning + 1)
		    {
		    	strncpy(arg, command + beginning + 1, counter - beginning - 2);
		    }
		    else
		    {
		        free(arg);
			f = 1;
		    }
		}
		else strncpy(arg, command + beginning, counter - beginning);
		if (!f)
		{
                    args[arg_count] = arg;
                    arg_count++;
		}
            }

            if (command[counter] == '|' || command[counter] == '\0') break;
            counter++;
        }
        ExecData newExecData;
        newExecData.program_name = program;
        newExecData.args = args;
        newExecData.argc = arg_count;
        execData[program_count] = newExecData;
        program_count++;
    }

    *programCount = program_count;
    return 0;
}
