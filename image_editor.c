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

typedef struct __attribute__((packed)){
    int **pixels;   // 2D array for pixel values
    int width;      // Image width
    int length;     // Image length
    int max_value;  // Maximum pixel value
    int pixel_depth; // Number of channels (1 for grayscale, 3 for RGB)
	int x1,x2,y1,y2; // Values that represent the selected portion of the image
} Image;

void swap(int *x, int *y)
{
	int z = 0;
	z = *x;
	*x = *y;
	*y = z;
}

void sort_int(int *x,int *y)
{
	if(*x > *y)
	{
		swap(x,y);
	}
}

void initialize_struct(Image *photo)
{
	photo->pixels = NULL;
    photo->width = 0;
    photo->length = 0;
    photo->max_value = 0;
   	photo->pixel_depth = 0;
	photo->x1 = 0;
	photo->y1 = 0;
	photo->x2 = 0;
	photo->y2 = 0;
}

// ceva gresit ori aici ori in modul in care e pusa poza in matrice
void free_image(Image *photo) {
    if (photo == NULL) return; // If the image structure itself is NULL, do nothing.

    // Free each row of the pixels
    for (int i = 0; i < photo->width; i++) {
        if (photo->pixels[i] != NULL) {
            free(photo->pixels[i]);  // Free the row
        }
    }

    // Free the row pointers
    if (photo->pixels != NULL) {
        free(photo->pixels);  // Free the array of row pointers
    }

    // Reset the fields of the structure
    photo->pixels = NULL;
    photo->width = 0;
    photo->length = 0;
    photo->max_value = 0;
    photo->pixel_depth = 0;
    photo->x1 = 0;
    photo->x2 = 0;
	photo->y1 = 0;
    photo->y2 = 0;
}

void freeImage(Image *photo) {
    if (photo) {
        for (int i = 0; i < photo->width; i++) {
            free(photo->pixels[i]);
        }
        free(photo->pixels);
        //free(photo);
    }
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
	exit(0);
}

void handle_select(Image *photo,char *argument)
{
	int x1,x2,y1,y2;
	if(!strcmp(argument,"ALL\n"))
	{
		if(!photo->length)
		{
			printf("No image loaded\n");
			return;
		}
		printf(("Selected ALL\n"));
		return;
	}
	if (sscanf(argument, "%d %d %d %d", &x1, &y1, &x2, &y2) != 4) {
        printf("Error parsing numbers\n");
		return;
    }
	sort_int(&x1,&x2);
	sort_int(&y1,&y2);
	// checks if out of bounds
	if(x1 > photo->length || x2 > photo->length || y1 > photo->width || y2 > photo->width)
	{
		printf("Invalid set of coordinates\n");
		return;
	}
	photo->x1 = x1;
	photo->y1 = y1;
	photo->x2 = x2;
	photo->y2 = y2;
	printf("Selected %d %d %d %d\n",photo->x1,photo->y1,photo->x2,photo->y2);
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
	//free_image(photo); // trebie rezolvat pt mem leak dar momentan whatever
	freeImage(photo);
	FILE *file = fopen(file_name,"r");
	fseek(file,2,SEEK_SET);
	fscanf(file," %d %d %d",&photo->length,&photo->width,&photo->max_value);

	// Allocate memory for the row pointers
    photo->pixels = (int **)calloc(photo->width, sizeof(int *));
    if (photo->pixels == NULL) {
        perror("Failed to allocate row pointers");
		fclose(file);
        return;
    }

    // Allocate memory for each row
    for (int i = 0; i < photo->width; i++) {
        (photo->pixels)[i] = (int *)calloc(photo->length * photo->pixel_depth, sizeof(int));
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
	fclose(file);
}

void binary_file_image(Image *photo, char * file_name)
{
	//free_image(photo); // trebie rezolvat pt mem leak dar momentan whatever
	FILE *file = fopen(file_name, "rb");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    // Read image dimensions and properties
    fread(&photo->length, sizeof(int), 1, file);   // Read number of columns
    fread(&photo->width, sizeof(int), 1, file);    // Read number of rows
    fread(&photo->max_value, sizeof(int), 1, file); // Read max value

    // Allocate memory for 2D pixel array
    photo->pixels = (int **)calloc(photo->width, sizeof(int *));
    if (!photo->pixels) {
        perror("Failed to allocate memory for pixel rows");
        free(photo);
        fclose(file);
        return;
    }

    for (int i = 0; i < photo->width; i++) {
        photo->pixels[i] = (int *)calloc(photo->length * photo->pixel_depth,sizeof(int));
        if (!photo->pixels[i]) {
            perror("Failed to allocate memory for pixel columns");
            for (int j = 0; j < i; j++) {
                free(photo->pixels[j]);
            }
            free(photo->pixels);
            free(photo);
            fclose(file);
            return;
        }
    }

    // Read pixel data into the 2D array
    for (int i = 0; i < photo->width; i++) {
        fread(photo->pixels[i], sizeof(int), photo->length * photo->pixel_depth, file);
    }

	for (int i = 0; i < photo->width; i++) {
        for (int j = 0; j < photo->length * photo->pixel_depth; j++) {
            printf("%d ", photo->pixels[i][j]); // to remove later
        }
        printf("\n");
    }

    fclose(file);
    return;
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
		fclose(file);
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
	if(!strcmp(format,"P5"))
	{
		photo->pixel_depth = 1;
		binary_file_image(photo,file_name);
	}
	if(!strcmp(format,"P6"))
	{
		photo->pixel_depth = 3;
		binary_file_image(photo,file_name);
	}
	return;
}


void get_input(Image *photo)
{
	char input[101] = {'\0'};
	char *mode;
	char *argument;
	const char *commands[] = {"LOAD", "CROP", "SAVE", "EXIT\n", "SELECT",
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
		if(i == list_length-1)
		{
			printf("INVALID\n");
		}
	}
	get_input(photo);
}

int main()
{
	Image photo;
	initialize_struct(&photo);

	get_input(&photo);
	freeImage(&photo);

	initialize_struct(&photo);
	return 0;
}
