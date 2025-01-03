#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <handlers.h>

/*
to do

1 ignorare comentarii cand citesti poze |text?|
2 loop in care poti sa bagi infinitate comenzi pana EXIT
3 abilitatea de a citi alte imagini consectuive de marimi diferite
*/

typedef struct {
    int **pixels;   // 2D array for pixel values
    int width;      // Image width
    int length;     // Image length
    int max_value;  // Maximum pixel value
    int pixel_depth; // Number of channels (1 for grayscale, 3 for RGB)
	int upper, lower; // Values that represent the selected portion of the image
} Image;


void free_image(Image *photo)
{
    // Check if there are any pixels allocated
    if (photo->pixels != NULL) {
        // Free each row
        for (int i = 0; i < photo->length * photo->pixel_depth; i++) {
            if (photo->pixels[i] != NULL) {
                free(photo->pixels[i]);
                photo->pixels[i] = NULL;
            }
        }
        // Free the array of row pointers
        free(photo->pixels);
        photo->pixels = NULL;
    }

    // Reset other struct members to default values
    photo->width = 0;
    photo->length = 0;
    photo->max_value = 0;
    photo->pixel_depth = 0;
    photo->upper = 0;
    photo->lower = 0;
}


void handle_crop(Image *photo,char *argument)
{
	return;
}
void handle_save(Image *photo,char *argument)
{
	return;
}
void handle_exit(Image *photo,char *argument)
{
	return;
}
void handle_select(Image *photo,char *argument)
{
	return;
}
void handle_histogram(Image *photo,char *argument)
{
	return;
}
void handle_equalize(Image *photo,char *argument)
{
	return;
}
void handle_rotate(Image *photo,char *argument)
{
	return;
}




void text_file_image(Image *photo, char * file_name)
{
	// this function will take the current loaded file and repplace it with the new one
	free_image(photo);
	FILE *file = fopen(file_name,"r");
	fseek(file,2,SEEK_SET);
	fscanf(file," %d %d %d",&photo->length,&photo->width,&photo->max_value);

	// Allocate memory for the row pointers
    photo->pixels = (int **)calloc(photo->length * photo->pixel_depth, sizeof(int *));
    if (photo->pixels == NULL) {
        perror("Failed to allocate row pointers");
		fclose(file);
        return;
    }

    // Allocate memory for each row
    for (int i = 0; i < photo->length * photo->pixel_depth; i++) {
        (photo->pixels)[i] = (int *)calloc(photo->width, sizeof(int));
        if ((photo->pixels)[i] == NULL) {
            perror("Failed to allocate row");
            // Free any previously allocated rows
            for (int j = 0; j < i; j++) {
                free((photo->pixels)[j]);
            }
            free(photo->pixels);
            *photo->pixels = NULL;
			fclose(file);
            return;
        }
    }
	for (int i = 0; i < photo->width; i++) {
        for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
            fscanf(file, "%d", &photo->pixels[i][j]);
            printf("%d ", photo->pixels[i][j]); // to remove later
        }
        printf("\n");
    }

}

void binary_file_image()
{
	// same but for binary files
}

void handle_load(Image *photo, char *file_name)
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
        return; // Failed to load
    }
	fread(format,sizeof(char),2,file);
	fclose(file);
	if(!strcmp(format,"P2"))
	{
		photo->pixel_depth = 1;
		text_file_image(photo,file_name);
	}
	if(!strcmp(format,"P3"))
	{
		photo->pixel_depth = 3;
		text_file_image(photo,file_name);
	}
	else if(format == "P5" || format == "P6")
	{
		//binary_file_image();
		// split these up;
	}
	return;
}


void get_input(Image *photo)
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
			handlers[i](photo,argument);
			break;
		}
	}
	if(!strcmp(mode,"EXIT\n"))
	{
		return;
	}
	get_input(photo);
}

int main()
{
	Image photo;
	get_input(&photo);
	free_image(&photo);
	return 0;
}
