#include <stdio.h>
#include <string.h>
//#include <handlers.h>

void handle_crop(char *current_file,char *argument)
{
	return;
}
void handle_save(char *current_file,char *argument)
{
	return;
}
void handle_exit(char *current_file,char *argument)
{
	return;
}
void handle_select(char *current_file,char *argument)
{
	return;
}
void handle_histogram(char *current_file,char *argument)
{
	return;
}
void handle_equalize(char *current_file,char *argument)
{
	return;
}
void handle_rotate(char *current_file,char *argument)
{
	return;
}

void text_file_image(int in_color, char * file_name)
{
	// this function will take the current loaded file and repplace it with the new one
	int length,width;char format[3];
	FILE *file = fopen(file_name,"r");
	fseek(file,0,SEEK_SET);
	fscanf(file," %s",format);
	fscanf(file," %d %d",&length,&width);
	printf("%s\n%d\n%d",format,length,width);
}

void binary_file_image()
{
	// same but for binary files
}

void handle_load(char *current_file, char *file_name)
{
	char format[3] = {0};
	if(file_name == NULL) {
		printf("file_name invalid, %d", __LINE__);
	}
	int l = strlen(file_name);
	file_name[l-1] = '\0';
	FILE *file = fopen(file_name,"rb");
	if (!file) {
        printf("Failed to load %s\n", file_name);
        current_file[0] = '\0';  // Clear current file if loading fails
        return; // Failed to load
    }
	fread(format,sizeof(char),2,file);
	fclose(file);
	if(!strcmp(format,"P2"))
	{
		text_file_image(0,file_name);
	}
	if(format == "P3")
	{
		text_file_image(1,file_name);
	}
	else if(format == "P5" || format == "P6")
	{
		//binary_file_image();
		// split these up;
	}
	return;
}


void get_input(char *image)
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
			handlers[i](image,argument);
			return;
		}
	}
}

int main()
{
	char *image;
	get_input(image);
	return 0;
}
