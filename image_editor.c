#include <stdio.h>
#include <string.h>
#include <handlers.h>

void handle_load(char *current_file, char *argument)
{
	char format[3] = {0};
	if(argument == NULL)
	{
		printf("argument invalid, %d", __LINE__);
	}

	FILE *file = fopen(argument,"rb");
	if (!file) {
        printf("Failed to load %s\n", argument);
        current_file[0] = '\0';  // Clear current file if loading fails
        return 0; // Failed to load
    }
	fread(format,sizeof(char),2,file);
	return;
}


void get_input()
{
	char input[101] = {'\0'};
	char *mode;
	char *argument;
	const char *commands[] = {"LOAD", "CROP", "SAVE", "EXIT", "SELECT",
							  "HISTOGRAM", "EQUALIZE", "ROTATE"};
	void (*handlers[])() = {handle_load, handle_crop, handle_save, handle_exit,
                            handle_select, handle_histogram, handle_equalize, handle_rotate};
	int list_length = sizeof(commands) / sizeof(commands[0]);

	if (fgets(input, sizeof(input), stdin) == NULL) {
		printf("EROARE LA CITIRE LINII");
		return;
    }

	mode = strtok(input, " ");
	argument = strtok(NULL,"");
	for(int i = 0; i < list_length; i++)
	{
		if(!strcmp(mode,commands[i]))
		{
			handlers[i](argument);
			return;
		}
	}
}

int main()
{
	get_input();
	return 0;
}
