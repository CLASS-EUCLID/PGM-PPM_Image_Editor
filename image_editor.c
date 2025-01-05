#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
to do

1 ignorare comentarii cand citesti poze |text?|
4 testeaza ce se intampla cu selectia cand dai load la alta poza daca pica ceva teste suspecte
5 all rotations have inverted column/row logic, fix it
*/

//  __attribute__((packed))
typedef struct{
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

// am o presimitre ca e un mem leak aici ca nu omor memoria care era inainte de crop
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
		for(int j = photo->x1 * photo->pixel_depth; j < photo->x2 * photo->pixel_depth; j++)
		{
			photo->pixels[a][b++] = photo->pixels[i][j];
		}
		a++;
		b = 0;
	}
	photo->length = photo->x2 - photo->x1;
	photo->width = photo->y2 - photo->y1;
    photo->x1 = 0;
    photo->y1 = 0;
    photo->x2 = photo->length;
    photo->y2 = photo->width;
	printf("Image cropped\n");
}

void save_binary_image(Image *photo, char *file_path) {
    if (!photo->length) {
        printf("No image loaded\n");
        return;
    }
    file_path[strlen(file_path)-1] = '\0';
    FILE *file = fopen(file_path, "wb"); // Open the file in binary mode

    if (!file) {
        perror("Failed to open file");
        return;
    }
    // Determine the format (P5 for grayscale, P6 for RGB)
    char *format = NULL;
    if (file_path[strlen(file_path) - 2] == 'g') { // Assuming 'g' indicates grayscale
        format = "P5";
    } else if (file_path[strlen(file_path) - 2] == 'p') { // Assuming 'p' indicates RGB
        format = "P6";
    } else {
        printf("WRONG format\n");
        fclose(file);
        return;
    }

    // Write the header
    fprintf(file, "%s\n%d %d\n%d\n", format, photo->length, photo->width, photo->max_value);

    // Write pixel data
    // for (int i = 0; i < photo->width; i++) {
    //     fwrite(photo->pixels[i], sizeof(unsigned char), photo->length, file);
    // }
    for(int i = 0;i<photo->width;i++)
    {
        for(int j = 0;j<photo->length * photo->pixel_depth;j++)
        {
            unsigned char pixel_value = photo->pixels[i][j];
            fwrite(&pixel_value, sizeof(unsigned char),1,file);
        }
    }

    fclose(file);
    printf("Saved %s\n",file_path);
}

void handle_save(Image *photo,char *argument)
{
    // For ascii the implementation is here
    // For binary in a seperate function we call here
    if(!photo->length)
    {
        printf("No image loaded\n");
    }
	if(argument[strlen(argument)-2] == 'i')
    {
        char *file_path = strtok(argument," ");
        int lossy_compression = 0;
        char *format = NULL;
        FILE *file = fopen(file_path,"w");
        if (!file) {
            perror("Failed to open file");
            return;
        }
        // P2
        if(file_path[strlen(file_path)-2] == 'g') {
            fprintf(file,"P2");
            format = "P2";
            if(photo->pixel_depth == 3)
            {
                lossy_compression = 1;
            }
        } else if (file_path[strlen(file_path)-2] == 'p') {
            fprintf(file,"P3");
            format = "P3";
        }
        else{
            printf("WRONG format\n");
            return;
        }
        fprintf(file,"\n%d %d\n%d\n",photo->length,photo->width,photo->max_value);
        if(photo->pixel_depth == 1 && !strcmp(format,"P3"))
        {
            for(int i = 0;i<photo->width;i++)
            {
                for(int j = 0; j < photo->length; j++)
                {
                    fprintf(file,"%d 0 0 ",photo->pixels[i][j]);
                }
                fprintf(file,"\n");
            }
        }
        else{
            for(int i = 0;i<photo->width;i++)
            {
                for(int j = 0; j < photo->length * photo->pixel_depth; j = j + 1 + 2 * lossy_compression)
                {
                    fprintf(file,"%d ",photo->pixels[i][j]);
                }
                fprintf(file,"\n");
            }
        }
        printf("Saved %s\n",file_path);
        fclose(file);
    }
    else
    {
        save_binary_image(photo,argument);
    }
}
void handle_exit(Image *photo,char *argument)
{
    if(!photo->length)
    {
        printf("No image loaded\n");
        return;
    }
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

void rotate_full_matrix(Image *photo, int degrees) {
    // Normalize degrees to [0, 360)
    degrees = (degrees % 360 + 360) % 360;

    int new_width = (degrees == 90 || degrees == 270) ? photo->length : photo->width;
    int new_length = (degrees == 90 || degrees == 270) ? photo->width : photo->length;
    // Allocate a new matrix for the rotated image
    int **rotated_pixels = (int **)malloc(new_width * sizeof(int *));
    for (int i = 0; i < new_width; i++) {
        rotated_pixels[i] = (int *)malloc(new_length * photo->pixel_depth * sizeof(int)); // 3 channels per pixel
    }
    if (degrees == 90) {
        for (int i = 0; i < photo->width; i++) {       // Rows
            for (int j = 0; j < photo->length; j++) {  // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    rotated_pixels[j][(photo->width - 1 - i) * photo->pixel_depth + k] = photo->pixels[i][j * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 180) {
        for (int i = 0; i < photo->width; i++) {       // Rows
            for (int j = 0; j < photo->length; j++) {  // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    rotated_pixels[photo->width - 1 - i][(photo->length - 1 - j) * photo->pixel_depth + k] = photo->pixels[i][j * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 270) {
        for (int i = 0; i < photo->width; i++) {       // Rows
            for (int j = 0; j < photo->length; j++) {  // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    rotated_pixels[photo->length - 1 - j][i * photo->pixel_depth + k] = photo->pixels[i][j * photo->pixel_depth + k];
                }
            }
        }
    }

    // Free the old pixel matrix and assign the new one
    for (int i = 0; i < photo->width; i++) {
        free(photo->pixels[i]);
    }
    free(photo->pixels);

    // Update photo properties
    photo->pixels = rotated_pixels;
    photo->width = new_width;   // Update the number of rows
    photo->length = new_length; // Update the number of columns
}

void rotate_selection(Image *photo, int degrees) {
    int cols = photo->x2 - photo->x1; // Number of columns (x-axis range)
    int rows = photo->y2 - photo->y1; // Number of rows (y-axis range)

    // Normalize negative rotations
    if (degrees == -90) {
        degrees = 270;
    } else if (degrees == -270) {
        degrees = 90;
    } else if (degrees == -180) {
        degrees = 180;
    }

    // Temporary matrix for the rotated submatrix
    int **temp = (int **)malloc(cols * sizeof(int *));
    for (int i = 0; i < cols; i++) {
        temp[i] = (int *)malloc(rows * photo->pixel_depth * sizeof(int));
    }

    if (degrees == 90) {
        for (int i = 0; i < rows; i++) { // Rows
            for (int j = 0; j < cols; j++) { // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    temp[j][(rows - i - 1) * photo->pixel_depth + k] =
                        photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 180) {
        for (int i = 0; i < rows; i++) { // Rows
            for (int j = 0; j < cols; j++) { // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    temp[rows - i - 1][(cols - j - 1) * photo->pixel_depth + k] =
                        photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k];
                }
            }
        }
    } else if (degrees == 270) {
        for (int i = 0; i < rows; i++) { // Rows
            for (int j = 0; j < cols; j++) { // Columns
                for (int k = 0; k < photo->pixel_depth; k++) {
                    temp[cols - j - 1][i * photo->pixel_depth + k] =
                        photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k];
                }
            }
        }
    } else {
        printf("WRONG DEGREE\n");
    }

    // Copy rotated submatrix back to the original matrix
    for (int i = 0; i < rows; i++) { // Rows
        for (int j = 0; j < cols; j++) { // Columns
            for (int k = 0; k < photo->pixel_depth; k++) {
                photo->pixels[photo->y1 + i][(photo->x1 + j) * photo->pixel_depth + k] =
                    temp[i][j * photo->pixel_depth + k];
            }
        }
    }

    // Free the temporary matrix
    for (int i = 0; i < cols; i++) {
        free(temp[i]);
    }
    free(temp);
}

void handle_rotate(Image *photo, char *argument) {
    // Parse the degree of rotation from the argument
    int degrees = atoi(argument);
    // Normalize degrees to the range [0, 360]
    if(degrees%90)
    {
        printf("Unsupported rotation angle\n");
        return;
    }
	if(!(degrees%360))
	{
        printf("Rotated %d\n", degrees);
		return;
	}
    if(!photo->x1 && !photo->y1 && photo->x2 == photo->length && photo->y2 == photo->width && photo->width != photo->length)
	{
		rotate_full_matrix(photo,degrees);
	}
	else
	{
		rotate_selection(photo,degrees);
	}
	printf("Rotated %d\n", degrees);
    // Check pixel depth and call the appropriate function
	// verifica instant daca selectia e ALL, daca nu baga ce e mai sus cu schimbarea de unghiuri in fucntia respectiva ca sa poti sa printezi ce trebuie
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
	const char *commands[] = {"LOAD", "CROP\n", "SAVE", "EXIT", "SELECT",
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
            printf("%s %s\n", mode,argument);
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
