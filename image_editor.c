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
		photo->x1 = 0;
		photo->x2 = photo->length;
		photo->y1 = 0;
		photo->y2 = photo->width;
		printf(("Selected ALL\n"));
		return;
	}
	if (sscanf(argument, "%d %d %d %d", &x1, &y1, &x2, &y2) != 4) {
        printf("Error parsing numbers\n");
		return;
    }
	sort_int(&x1,&x2);
	sort_int(&y1,&y2);
	// checks if out of bounds (adica nush daca trebuie pus -1 la fiecare coordonata sau nu depinde daca poti sa iei marginea sau nu)
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

void freeImage(Image *photo) {
    if (photo) {
        for (int i = 0; i < photo->width; i++) {
            free(photo->pixels[i]);
        }
        free(photo->pixels);
        //free(photo);
    }
}


// trebuie verificat ce considera ei ca fiind out of bounds pls ( defapt asta ar fi la select dar whatever)
void handle_crop(Image *photo,char *argument)
{
	if(!photo->length)
	{
		printf("No image loaded\n");
		return;
	}
	int a = 0,b = 0;
	for(int i = photo->y1;i < photo->y2; i++)
	{
		for(int j = photo->x1; j < photo->x2 * photo->pixel_depth; j++)
		{
			photo->pixels[a][b++] = photo->pixels[i][j];
		}
		a++;
		b = 0;
	}
	photo->length = photo->x2 - photo->x1;
	photo->width = photo->y2 - photo->y1;
	printf("Image cropped\n");
}

void handle_save(Image *photo,char *argument)
{
	return;
}
void handle_exit(Image *photo,char *argument)

{
	exit(0);
}
void handle_print(Image *photo,char *argument)
{
	if(!photo->length)
	{
		printf("NO IMAGE LOADED\n");
		return;
	}
	for(int i = 0;i < photo->width; i++)
	{
		for(int j =0; j < photo->length * photo->pixel_depth; j++)
		{
			printf("%d ",photo->pixels[i][j]);
		}
		printf("\n");
	}
}
void handle_histogram(Image *photo,char *argument)
{
	return;
}
void handle_equalize(Image *photo,char *argument)
{
	return;
}

void rotate_grayscale(Image *photo, int degrees) {
    int rows = photo->x2 - photo->x1;
    int cols = photo->y2 - photo->y1;

    // Temporary matrix for the rotated submatrix
    int **temp = (int **)malloc(cols * sizeof(int *));
    for (int i = 0; i < cols; i++) {
        temp[i] = (int *)malloc(rows * sizeof(int));
    }

    if (degrees == 90) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                temp[j][rows - i - 1] = photo->pixels[photo->x1 + i][photo->y1 + j];
            }
        }
    } else if (degrees == 180) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                temp[rows - i - 1][cols - j - 1] = photo->pixels[photo->x1 + i][photo->y1 + j];
            }
        }
    } else if (degrees == 270) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                temp[cols - j - 1][i] = photo->pixels[photo->x1 + i][photo->y1 + j];
            }
        }
    } else if (degrees == 360) {
        // 360-degree rotation is a no-op; copy the original
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                temp[i][j] = photo->pixels[photo->x1 + i][photo->y1 + j];
            }
        }
    }

    // Copy rotated submatrix back to the original matrix
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            photo->pixels[photo->x1 + i][photo->y1 + j] = temp[i][j];
        }
    }

    // Free the temporary matrix
    for (int i = 0; i < cols; i++) {
        free(temp[i]);
    }
    free(temp);

	printf("Rotated %d\n", degrees);
}

void rotate_rgb(Image *photo, int degrees) {
    int rows = photo->x2 - photo->x1;
    int cols = photo->y2 - photo->y1;

    // Temporary matrix for the rotated submatrix
    int **temp = (int **)malloc(cols * sizeof(int *));
    for (int i = 0; i < cols; i++) {
        temp[i] = (int *)malloc(rows * 3 * sizeof(int)); // 3 channels per pixel
    }

    if (degrees == 90) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                for (int k = 0; k < 3; k++) {
                    temp[j][(rows - i - 1) * 3 + k] = photo->pixels[photo->x1 + i][photo->y1 * 3 + j * 3 + k];
                }
            }
        }
    } else if (degrees == 180) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                for (int k = 0; k < 3; k++) {
                    temp[rows - i - 1][(cols - j - 1) * 3 + k] = photo->pixels[photo->x1 + i][photo->y1 * 3 + j * 3 + k];
                }
            }
        }
    } else if (degrees == 270) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                for (int k = 0; k < 3; k++) {
                    temp[cols - j - 1][i * 3 + k] = photo->pixels[photo->x1 + i][photo->y1 * 3 + j * 3 + k];
                }
            }
        }
    } else if (degrees == 360) {
        // 360-degree rotation is a no-op; copy the original
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                for (int k = 0; k < 3; k++) {
                    temp[i][j * 3 + k] = photo->pixels[photo->x1 + i][photo->y1 * 3 + j * 3 + k];
                }
            }
        }
    }

    // Copy rotated submatrix back to the original matrix
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            for (int k = 0; k < 3; k++) {
                photo->pixels[photo->x1 + i][photo->y1 * 3 + j * 3 + k] = temp[i][j * 3 + k];
            }
        }
    }

    // Free the temporary matrix
    for (int i = 0; i < cols; i++) {
        free(temp[i]);
    }
    free(temp);

	printf("Rotated %d\n", degrees);
}

void handle_rotate(Image *photo, char *argument) {
    // Parse the degree of rotation from the argument
    int degrees = atoi(argument);
    // Normalize degrees to the range [0, 360]
    if (degrees == -90) {
    degrees = 270; // Rotating 90 degrees left is equivalent to 270 degrees right
	} else if (degrees == -270) {
    degrees = 90; // Rotating 270 degrees left is equivalent to 90 degrees right
	} else if (degrees == -180) {
    degrees = 180; // Rotating 180 degrees left is the same as rotating 180 degrees right
	} else if (degrees == -360 || degrees == 360) {
    degrees = 0; // Full circle rotation has no effect
	}
    // Check pixel depth and call the appropriate function
    if (photo->pixel_depth == 1) {
        rotate_grayscale(photo, degrees);
    } else if (photo->pixel_depth == 3) {
        rotate_rgb(photo, degrees);
    } else {
        fprintf(stderr, "Unsupported pixel depth: %d\n", photo->pixel_depth);
    }
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
        }
    }
	printf("Loaded %s\n",file_name);
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
    fclose(file);
	printf("Loaded %s\n",file_name);
    return;
}

void handle_load(Image *photo, char *file_name)
{
	char format[3] = {0};
	if(file_name == NULL) {
		printf("Failed to load %s\n", file_name);
		return;
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
	const char *commands[] = {"LOAD", "CROP\n", "SAVE", "EXIT\n", "SELECT",
							  "HISTOGRAM", "EQUALIZE", "ROTATE", "PRINT\n"};
	void (*handlers[])() = {handle_load, handle_crop, handle_save, handle_exit,
                            handle_select, handle_histogram, handle_equalize, handle_rotate, handle_print};
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
